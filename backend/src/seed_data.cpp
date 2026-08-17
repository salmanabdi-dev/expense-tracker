// seed_data.cpp
// -------------
// A small standalone program that fills the database with a demo user and
// a handful of example expenses, so you can try out the app immediately
// without manually registering and typing in data.
//
// Build it as part of the normal CMake build, then run the resulting
// "seed_data" program once:
//   ./seed_data          (Linux/macOS)
//   seed_data.exe         (Windows)
//
// Demo login:
//   username: demo
//   password: demo1234

#include "database.hpp"
#include "auth.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <tuple>

const std::string DEMO_USERNAME = "demo";
const std::string DEMO_PASSWORD = "demo1234";

// title, amount, category, date
const std::vector<std::tuple<std::string, double, std::string, std::string>> SAMPLE_EXPENSES = {
    {"Groceries", 54.32, "Food", "2026-07-01"},
    {"Bus pass", 45.00, "Transport", "2026-07-02"},
    {"Movie night", 22.50, "Entertainment", "2026-07-05"},
    {"Electricity bill", 88.10, "Bills", "2026-07-07"},
    {"New running shoes", 65.99, "Shopping", "2026-07-10"},
    {"Dentist appointment", 120.00, "Health", "2026-07-12"},
    {"Coffee with a friend", 8.75, "Food", "2026-07-14"},
    {"Streaming subscription", 15.99, "Entertainment", "2026-07-15"},
};

int main() {
    sqlite3* db = open_database("expenses.db");
    init_database(db);

    // Check if the demo user already exists.
    int user_id = -1;
    sqlite3_stmt* check_stmt;
    sqlite3_prepare_v2(db, "SELECT id FROM users WHERE username = ?", -1, &check_stmt, nullptr);
    sqlite3_bind_text(check_stmt, 1, DEMO_USERNAME.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(check_stmt) == SQLITE_ROW) {
        user_id = sqlite3_column_int(check_stmt, 0);
        std::cout << "Demo user '" << DEMO_USERNAME << "' already exists (id=" << user_id << ")." << std::endl;
    }
    sqlite3_finalize(check_stmt);

    if (user_id == -1) {
        std::string password_hash = hash_password(DEMO_PASSWORD);

        sqlite3_stmt* insert_stmt;
        sqlite3_prepare_v2(db, "INSERT INTO users (username, password_hash) VALUES (?, ?)", -1, &insert_stmt, nullptr);
        sqlite3_bind_text(insert_stmt, 1, DEMO_USERNAME.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insert_stmt, 2, password_hash.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(insert_stmt);
        sqlite3_finalize(insert_stmt);

        user_id = static_cast<int>(sqlite3_last_insert_rowid(db));
        std::cout << "Created demo user '" << DEMO_USERNAME << "' (id=" << user_id << ")." << std::endl;
    }

    // Only add sample expenses if this user doesn't have any yet, so
    // running this program twice doesn't create duplicates.
    sqlite3_stmt* count_stmt;
    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM expenses WHERE user_id = ?", -1, &count_stmt, nullptr);
    sqlite3_bind_int(count_stmt, 1, user_id);
    sqlite3_step(count_stmt);
    int existing_count = sqlite3_column_int(count_stmt, 0);
    sqlite3_finalize(count_stmt);

    if (existing_count > 0) {
        std::cout << "Sample expenses already exist for this user - skipping." << std::endl;
    } else {
        for (const auto& [title, amount, category, date] : SAMPLE_EXPENSES) {
            sqlite3_stmt* insert_stmt;
            sqlite3_prepare_v2(db,
                "INSERT INTO expenses (user_id, title, amount, category, date) VALUES (?, ?, ?, ?, ?)",
                -1, &insert_stmt, nullptr);
            sqlite3_bind_int(insert_stmt, 1, user_id);
            sqlite3_bind_text(insert_stmt, 2, title.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(insert_stmt, 3, amount);
            sqlite3_bind_text(insert_stmt, 4, category.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(insert_stmt, 5, date.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(insert_stmt);
            sqlite3_finalize(insert_stmt);
        }
        std::cout << "Added " << SAMPLE_EXPENSES.size() << " sample expenses." << std::endl;
    }

    sqlite3_close(db);

    std::cout << "\nDone! You can now log in with:" << std::endl;
    std::cout << "  username: " << DEMO_USERNAME << std::endl;
    std::cout << "  password: " << DEMO_PASSWORD << std::endl;

    return 0;
}
