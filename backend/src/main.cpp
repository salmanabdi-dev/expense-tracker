// main.cpp
// --------
// The main API server. This is where every HTTP route (URL) the React
// frontend talks to gets defined.
//
// How authentication works here (kept intentionally simple for beginners):
//   - When a user registers, we store their username + a HASHED password.
//   - When a user logs in, we generate a random "session token", remember
//     it in memory (mapped to that user's id), and send it to the browser
//     as a cookie.
//   - On every later request, the browser sends that cookie back, and we
//     look up the token in our in-memory map to know who's asking.
//
// Build & run instructions are in the top-level README.md.

#include "crow.h"
#include "crow/middlewares/cors.h"
#include "database.hpp"
#include "auth.hpp"

#include <cmath>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <vector>

// ---------------------------------------------------------------------------
// Simple in-memory session store: session_token -> user_id
// A std::mutex protects it since Crow may handle requests on multiple
// threads at once.
// ---------------------------------------------------------------------------
std::unordered_map<std::string, int> g_sessions;
std::mutex g_sessions_mutex;

// The fixed list of categories the frontend dropdowns use.
const std::vector<std::string> CATEGORIES = {
    "Food", "Transport", "Entertainment", "Bills", "Shopping", "Health", "Other"
};

// Pulls the "session_token" value out of the request's Cookie header.
// Cookie headers look like: "session_token=abc123; other=xyz"
std::string get_cookie_value(const crow::request& req, const std::string& cookie_name) {
    std::string cookie_header = req.get_header_value("Cookie");
    if (cookie_header.empty()) return "";

    size_t pos = 0;
    while (pos < cookie_header.size()) {
        size_t semi = cookie_header.find(';', pos);
        std::string piece = cookie_header.substr(pos, semi == std::string::npos ? std::string::npos : semi - pos);

        size_t eq = piece.find('=');
        if (eq != std::string::npos) {
            std::string name = piece.substr(0, eq);
            // trim leading spaces
            size_t start = name.find_first_not_of(' ');
            if (start != std::string::npos) name = name.substr(start);

            if (name == cookie_name) {
                return piece.substr(eq + 1);
            }
        }

        if (semi == std::string::npos) break;
        pos = semi + 1;
    }
    return "";
}

// Looks up the currently logged-in user's id from the request's session
// cookie. Returns -1 if the user is not logged in.
int get_logged_in_user_id(const crow::request& req) {
    std::string token = get_cookie_value(req, "session_token");
    if (token.empty()) return -1;

    std::lock_guard<std::mutex> lock(g_sessions_mutex);
    auto it = g_sessions.find(token);
    if (it == g_sessions.end()) return -1;
    return it->second;
}

// Small helper to build a JSON error response.
crow::response error_response(int status_code, const std::string& message) {
    crow::json::wvalue body;
    body["error"] = message;
    crow::response res(status_code, body);
    res.set_header("Content-Type", "application/json");
    return res;
}

int main() {
    sqlite3* db = open_database("expenses.db");
    init_database(db);

    crow::App<crow::CORSHandler> app;

    // --- CORS setup: allow the React dev server to call this API and to
    // send/receive the session cookie. ---
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .origin("http://localhost:3000")
        .allow_credentials()
        .headers("Content-Type")
        .methods("GET"_method, "POST"_method, "DELETE"_method, "OPTIONS"_method);

    // -------------------------------------------------------------------
    // AUTH ROUTES
    // -------------------------------------------------------------------

    CROW_ROUTE(app, "/api/register").methods("POST"_method)
    ([db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) return error_response(400, "Invalid JSON body.");

        std::string username = body.has("username") ? body["username"].s() : "";
        std::string password = body.has("password") ? body["password"].s() : "";

        if (username.empty() || password.empty()) {
            return error_response(400, "Username and password are required.");
        }
        if (password.size() < 4) {
            return error_response(400, "Password must be at least 4 characters.");
        }

        // Check if username is already taken.
        sqlite3_stmt* check_stmt;
        sqlite3_prepare_v2(db, "SELECT id FROM users WHERE username = ?", -1, &check_stmt, nullptr);
        sqlite3_bind_text(check_stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        bool already_exists = (sqlite3_step(check_stmt) == SQLITE_ROW);
        sqlite3_finalize(check_stmt);

        if (already_exists) {
            return error_response(409, "That username is already taken.");
        }

        std::string password_hash = hash_password(password);

        sqlite3_stmt* insert_stmt;
        sqlite3_prepare_v2(db, "INSERT INTO users (username, password_hash) VALUES (?, ?)", -1, &insert_stmt, nullptr);
        sqlite3_bind_text(insert_stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insert_stmt, 2, password_hash.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(insert_stmt);
        sqlite3_finalize(insert_stmt);

        crow::json::wvalue result;
        result["message"] = "Account created successfully. You can now log in.";
        crow::response res(201, result);
        res.set_header("Content-Type", "application/json");
        return res;
    });

    CROW_ROUTE(app, "/api/login").methods("POST"_method)
    ([db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) return error_response(400, "Invalid JSON body.");

        std::string username = body.has("username") ? body["username"].s() : "";
        std::string password = body.has("password") ? body["password"].s() : "";

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "SELECT id, password_hash FROM users WHERE username = ?", -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_ROW) {
            sqlite3_finalize(stmt);
            return error_response(401, "Incorrect username or password.");
        }

        int user_id = sqlite3_column_int(stmt, 0);
        std::string stored_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        sqlite3_finalize(stmt);

        if (!verify_password(password, stored_hash)) {
            return error_response(401, "Incorrect username or password.");
        }

        // Create a new session token and remember which user it belongs to.
        std::string token = generate_session_token();
        {
            std::lock_guard<std::mutex> lock(g_sessions_mutex);
            g_sessions[token] = user_id;
        }

        crow::json::wvalue result;
        result["message"] = "Logged in successfully.";
        result["username"] = username;
        crow::response res(200, result);
        res.set_header("Content-Type", "application/json");
        res.set_header("Set-Cookie", "session_token=" + token + "; Path=/; HttpOnly; SameSite=Lax");
        return res;
    });

    CROW_ROUTE(app, "/api/logout").methods("POST"_method)
    ([](const crow::request& req) {
        std::string token = get_cookie_value(req, "session_token");
        if (!token.empty()) {
            std::lock_guard<std::mutex> lock(g_sessions_mutex);
            g_sessions.erase(token);
        }

        crow::json::wvalue result;
        result["message"] = "Logged out successfully.";
        crow::response res(200, result);
        res.set_header("Content-Type", "application/json");
        // Clear the cookie by expiring it immediately.
        res.set_header("Set-Cookie", "session_token=; Path=/; HttpOnly; Max-Age=0");
        return res;
    });

    CROW_ROUTE(app, "/api/check-auth").methods("GET"_method)
    ([db](const crow::request& req) {
        int user_id = get_logged_in_user_id(req);
        crow::json::wvalue result;

        if (user_id == -1) {
            result["logged_in"] = false;
            return crow::response(200, result);
        }

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "SELECT username FROM users WHERE id = ?", -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, user_id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result["logged_in"] = true;
            result["username"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        } else {
            result["logged_in"] = false;
        }
        sqlite3_finalize(stmt);

        return crow::response(200, result);
    });

    // -------------------------------------------------------------------
    // OTHER ROUTES
    // -------------------------------------------------------------------

    CROW_ROUTE(app, "/api/categories").methods("GET"_method)
    ([]() {
        crow::json::wvalue result;
        result["categories"] = CATEGORIES;
        return crow::response(200, result);
    });

    // -------------------------------------------------------------------
    // EXPENSE ROUTES (all require the user to be logged in)
    // -------------------------------------------------------------------

    CROW_ROUTE(app, "/api/expenses").methods("GET"_method)
    ([db](const crow::request& req) {
        int user_id = get_logged_in_user_id(req);
        if (user_id == -1) return error_response(401, "You must be logged in to do that.");

        std::string category = req.url_params.get("category") ? req.url_params.get("category") : "";

        sqlite3_stmt* stmt;
        if (!category.empty() && category != "All") {
            sqlite3_prepare_v2(db,
                "SELECT id, title, amount, category, date FROM expenses "
                "WHERE user_id = ? AND category = ? ORDER BY date DESC, id DESC",
                -1, &stmt, nullptr);
            sqlite3_bind_int(stmt, 1, user_id);
            sqlite3_bind_text(stmt, 2, category.c_str(), -1, SQLITE_TRANSIENT);
        } else {
            sqlite3_prepare_v2(db,
                "SELECT id, title, amount, category, date FROM expenses "
                "WHERE user_id = ? ORDER BY date DESC, id DESC",
                -1, &stmt, nullptr);
            sqlite3_bind_int(stmt, 1, user_id);
        }

        crow::json::wvalue::list expenses_list;
        double total = 0.0;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            crow::json::wvalue expense;
            expense["id"] = sqlite3_column_int(stmt, 0);
            expense["title"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            double amount = sqlite3_column_double(stmt, 2);
            expense["amount"] = amount;
            expense["category"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            expense["date"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            total += amount;
            expenses_list.push_back(std::move(expense));
        }
        sqlite3_finalize(stmt);

        crow::json::wvalue result;
        result["expenses"] = std::move(expenses_list);
        result["total"] = std::round(total * 100.0) / 100.0;
        return crow::response(200, result);
    });

    CROW_ROUTE(app, "/api/expenses").methods("POST"_method)
    ([db](const crow::request& req) {
        int user_id = get_logged_in_user_id(req);
        if (user_id == -1) return error_response(401, "You must be logged in to do that.");

        auto body = crow::json::load(req.body);
        if (!body) return error_response(400, "Invalid JSON body.");

        std::string title = body.has("title") ? body["title"].s() : "";
        std::string category = body.has("category") ? body["category"].s() : "";
        std::string date = body.has("date") ? body["date"].s() : "";

        if (title.empty() || category.empty() || date.empty()) {
            return error_response(400, "Title, category, and date are all required.");
        }
        if (!body.has("amount")) {
            return error_response(400, "Amount must be a number.");
        }

        double amount = body["amount"].d();
        if (amount <= 0) {
            return error_response(400, "Amount must be greater than zero.");
        }

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db,
            "INSERT INTO expenses (user_id, title, amount, category, date) VALUES (?, ?, ?, ?, ?)",
            -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_text(stmt, 2, title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 3, amount);
        sqlite3_bind_text(stmt, 4, category.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        int new_id = static_cast<int>(sqlite3_last_insert_rowid(db));
        sqlite3_finalize(stmt);

        crow::json::wvalue expense;
        expense["id"] = new_id;
        expense["title"] = title;
        expense["amount"] = amount;
        expense["category"] = category;
        expense["date"] = date;

        crow::json::wvalue result;
        result["message"] = "Expense added.";
        result["expense"] = std::move(expense);

        crow::response res(201, result);
        res.set_header("Content-Type", "application/json");
        return res;
    });

    CROW_ROUTE(app, "/api/expenses/<int>").methods("DELETE"_method)
    ([db](const crow::request& req, int expense_id) {
        int user_id = get_logged_in_user_id(req);
        if (user_id == -1) return error_response(401, "You must be logged in to do that.");

        // Make sure the expense exists AND belongs to this user before deleting.
        sqlite3_stmt* check_stmt;
        sqlite3_prepare_v2(db, "SELECT id FROM expenses WHERE id = ? AND user_id = ?", -1, &check_stmt, nullptr);
        sqlite3_bind_int(check_stmt, 1, expense_id);
        sqlite3_bind_int(check_stmt, 2, user_id);
        bool found = (sqlite3_step(check_stmt) == SQLITE_ROW);
        sqlite3_finalize(check_stmt);

        if (!found) {
            return error_response(404, "Expense not found.");
        }

        sqlite3_stmt* delete_stmt;
        sqlite3_prepare_v2(db, "DELETE FROM expenses WHERE id = ?", -1, &delete_stmt, nullptr);
        sqlite3_bind_int(delete_stmt, 1, expense_id);
        sqlite3_step(delete_stmt);
        sqlite3_finalize(delete_stmt);

        crow::json::wvalue result;
        result["message"] = "Expense deleted.";
        return crow::response(200, result);
    });

    std::cout << "Expense tracker server running at http://localhost:5000" << std::endl;
    app.port(5000).multithreaded().run();

    sqlite3_close(db);
    return 0;
}
