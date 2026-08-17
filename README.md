# Personal Expense Tracker

A full-stack personal expense tracker built with React, C++, Crow, and SQLite.

- **Frontend:** React
- **Backend:** C++ with the Crow web framework
- **Database:** SQLite

## Features

- Register and log in with password hashing
- Add expenses with a title, amount, category, and date
- View saved expenses
- Delete expenses
- Filter expenses by category
- View total spending
- Session-based authentication
- Responsive interface for desktop and mobile

---

## 1. Project Structure

```text
expense-tracker/
├── backend/
│   ├── CMakeLists.txt
│   └── src/
│       ├── main.cpp
│       ├── database.hpp
│       ├── database.cpp
│       ├── auth.hpp
│       └── seed_data.cpp
│
└── frontend/
    ├── package.json
    ├── public/
    │   └── index.html
    └── src/
        ├── index.js
        ├── App.js
        ├── App.css
        ├── api.js
        └── components/
            ├── Login.js
            ├── Register.js
            ├── Dashboard.js
            ├── ExpenseForm.js
            ├── ExpenseList.js
            ├── ExpenseFilter.js
            └── TotalSpending.js
```

---

## 2. Requirements

### Backend

- C++17 compiler
- CMake 3.14+
- SQLite3 development libraries
- Standalone Asio
- Internet connection during the first build so CMake can download Crow

For Windows with MSYS2 UCRT64:

```bash
pacman -S mingw-w64-ucrt-x86_64-sqlite3
pacman -S mingw-w64-ucrt-x86_64-asio
```

### Frontend

- Node.js
- npm

---

## 3. Setup & Run — Backend

### Windows with MSYS2 / MinGW

```powershell
cd backend
mkdir build
cd build
cmake -G "MinGW Makefiles" -DASIO_INCLUDE_DIR="C:/msys64/ucrt64/include" ..
cmake --build .
```

### Other Platforms

With the required dependencies installed:

```bash
cd backend
mkdir build
cd build
cmake ..
cmake --build .
```

The build creates two programs:

- `expense_server` — runs the API server
- `seed_data` — creates the demo account and sample expenses

### Sample Data

On Windows:

```powershell
.\seed_data.exe
```

On macOS/Linux:

```bash
./seed_data
```

This creates a demo account and 8 sample expenses.

**Demo Account**

```text
Username: demo
Password: demo1234
```

### Start the Backend

Windows:

```powershell
.\expense_server.exe
```

macOS/Linux:

```bash
./expense_server
```

The backend API runs on port `5000`. The SQLite database is stored as `expenses.db` in the build directory.

---

## 4. Setup & Run — Frontend

Open another terminal while keeping the backend running:

```bash
cd frontend
npm install
npm start
```

The React development server starts the application locally.

Frontend API requests use the `/api` path and are forwarded to the C++ backend through the proxy configured in `package.json`.

---

## 5. Usage

1. Log in using the demo account or register a new account.
2. Add an expense with a title, amount, category, and date.
3. View your saved expenses on the dashboard.
4. Filter expenses by category.
5. View the total spending for the currently displayed expenses.
6. Delete expenses using the Delete button.

Each account only has access to its own expenses.

---

## 6. Authentication

When a user registers, their password is combined with a randomly generated salt and hashed before being stored in SQLite. Plain-text passwords are not stored.

When a user logs in successfully, the backend generates a random session token and associates it with that user. The token is sent to the browser as a cookie and included with future API requests.

Protected routes use the session token to determine which user is making the request. Logging out removes the active session.

The authentication system is designed for the scope of this project. A production application would typically use persistent session storage and a dedicated password-hashing algorithm such as bcrypt or Argon2.

---

## 7. File Reference

### Backend

- **`CMakeLists.txt`** — Configures the C++ build, downloads Crow, and links the required libraries.
- **`src/database.hpp` / `database.cpp`** — Handles the SQLite connection and database tables.
- **`src/auth.hpp`** — Contains password hashing, verification, salt generation, and session-token utilities.
- **`src/main.cpp`** — Defines the API routes, handles requests, and manages user sessions.
- **`src/seed_data.cpp`** — Creates the demo account and sample expense data.

### Frontend

- **`public/index.html`** — HTML entry page for the React application.
- **`src/index.js`** — React entry point.
- **`src/App.js`** — Controls the authentication and dashboard views.
- **`src/App.css`** — Contains the application's styling.
- **`src/api.js`** — Handles communication between the frontend and backend.
- **`src/components/Login.js`** — Login form.
- **`src/components/Register.js`** — Registration form.
- **`src/components/Dashboard.js`** — Main expense dashboard.
- **`src/components/ExpenseForm.js`** — Form for adding expenses.
- **`src/components/ExpenseList.js`** — Displays and deletes expenses.
- **`src/components/ExpenseFilter.js`** — Category filtering.
- **`src/components/TotalSpending.js`** — Displays total spending.

---

## 8. Common Issues

- **CMake cannot find SQLite3** — Make sure the SQLite3 development package is installed for your compiler.
- **CMake cannot find Asio** — On MSYS2 UCRT64, install `mingw-w64-ucrt-x86_64-asio` and provide the Asio include directory when running CMake.
- **Frontend cannot reach the backend** — Make sure `expense_server` is running before using the frontend.
- **Demo login does not work** — Run `seed_data` to create the demo account before logging in.
- **First CMake configuration takes longer** — CMake downloads the Crow framework during the initial build.

---

## Future Improvements

- Add support for editing existing expenses
- Add monthly spending charts and visualizations
- Export expenses to CSV
- Add customizable budget limits and spending alerts