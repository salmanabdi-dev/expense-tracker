# CMakeLists.txt
# ---------------
# This file tells CMake how to build our C++ backend.
#
# It automatically downloads the Crow web framework for you (via
# FetchContent), so you don't need to install it by hand. You DO need
# SQLite3's development headers installed on your system already:
#   - Ubuntu/Debian: sudo apt install libsqlite3-dev
#   - macOS (Homebrew): brew install sqlite3
#   - Windows: easiest via vcpkg -> vcpkg install sqlite3
#
# Two programs get built:
#   1. expense_server -> the actual API server
#   2. seed_data       -> a one-off tool that fills the database with a
#                          demo user and sample expenses

cmake_minimum_required(VERSION 3.14)
project(expense_tracker_backend CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Download and make the Crow web framework available ---
# NOTE: v1.3.0 (not v1.2.0) is required here. Earlier releases still had
# some legacy asio::io_service code paths that fail to compile against
# newer standalone Asio releases (Asio removed/renamed that API). v1.3.0's
# CI is explicitly tested against Boost::asio AND standalone Asio, on both
# clang and gcc, so it's the safer minimum version to pin to.
include(FetchContent)
FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.3.3
)
FetchContent_MakeAvailable(Crow)

# --- Find SQLite3 (must be installed on your system, see above) ---
find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)

# --- Main API server ---
add_executable(expense_server
    src/main.cpp
    src/database.cpp
)
target_include_directories(expense_server PRIVATE src)
target_link_libraries(expense_server PRIVATE Crow::Crow SQLite::SQLite3 Threads::Threads ws2_32 mswsock)


# --- Sample data seeding tool ---
add_executable(seed_data
    src/seed_data.cpp
    src/database.cpp
)
target_include_directories(seed_data PRIVATE src)
target_link_libraries(seed_data PRIVATE SQLite::SQLite3)
