// database.hpp
// -------------
// Declares small helper functions for opening our SQLite database and
// creating its tables. See database.cpp for the actual implementation.
//
// We talk to SQLite using its plain C API (sqlite3.h). It's a little more
// verbose than using a wrapper library, but it's the most transparent way
// for a beginner to see exactly what's happening: prepare a statement,
// bind values into it, step through results, then finalize it.

#pragma once

#include <sqlite3.h>
#include <string>

// Opens (and creates, if missing) the SQLite database file at the given
// path. Returns a raw sqlite3* handle - the caller is responsible for
// eventually calling sqlite3_close() on it.
sqlite3* open_database(const std::string& path);

// Creates the "users" and "expenses" tables if they don't already exist.
// Safe to call every time the program starts.
void init_database(sqlite3* db);
