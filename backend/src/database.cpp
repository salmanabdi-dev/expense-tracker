// database.cpp
// -------------
// Implementation of the database helper functions declared in database.hpp.

#include "database.hpp"
#include <iostream>
#include <stdexcept>

sqlite3* open_database(const std::string& path) {
    sqlite3* db = nullptr;
    int result = sqlite3_open(path.c_str(), &db);

    if (result != SQLITE_OK) {
        std::string message = "Could not open database: ";
        message += sqlite3_errmsg(db);
        throw std::runtime_error(message);
    }

    return db;
}

void init_database(sqlite3* db) {
    // Each registered user has a unique username and a HASHED password.
    // We never store plain-text passwords - see auth.hpp for hashing.
    const char* create_users_table =
        "CREATE TABLE IF NOT EXISTS users ("
        "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   username TEXT UNIQUE NOT NULL,"
        "   password_hash TEXT NOT NULL"
        ");";

    // Each expense belongs to exactly one user (user_id references users.id).
    const char* create_expenses_table =
        "CREATE TABLE IF NOT EXISTS expenses ("
        "   id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "   user_id INTEGER NOT NULL,"
        "   title TEXT NOT NULL,"
        "   amount REAL NOT NULL,"
        "   category TEXT NOT NULL,"
        "   date TEXT NOT NULL,"
        "   FOREIGN KEY (user_id) REFERENCES users(id)"
        ");";

    char* error_message = nullptr;

    if (sqlite3_exec(db, create_users_table, nullptr, nullptr, &error_message) != SQLITE_OK) {
        std::string message = "Failed to create users table: ";
        message += error_message;
        sqlite3_free(error_message);
        throw std::runtime_error(message);
    }

    if (sqlite3_exec(db, create_expenses_table, nullptr, nullptr, &error_message) != SQLITE_OK) {
        std::string message = "Failed to create expenses table: ";
        message += error_message;
        sqlite3_free(error_message);
        throw std::runtime_error(message);
    }

    std::cout << "Database ready (tables created if they didn't already exist)." << std::endl;
}
