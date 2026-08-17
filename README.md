# Personal Expense Tracker

A simple, beginner-friendly full-stack expense tracker.

- **Frontend:** React
- **Backend:** C++ (using the [Crow](https://github.com/CrowCpp/Crow) web framework)
- **Database:** SQLite

Features:
- Register and log in (passwords are hashed, never stored as plain text)
- Add an expense with a title, amount, category, and date
- View all your expenses
- Delete expenses
- Filter expenses by category
- See your total spending at a glance
- Clean, responsive interface that works on mobile and desktop

---

## 1. Project Structure

```
expense-tracker/
├── backend/                 # C++ API server
│   ├── CMakeLists.txt        # Build instructions (also downloads Crow for you)
│   └── src/
│       ├── main.cpp          # All API routes (register, login, expenses, etc.)
│       ├── database.hpp/.cpp # SQLite connection + table creation
│       ├── auth.hpp          # Password hashing + session token helpers
│       └── seed_data.cpp     # One-off program that adds demo/sample data
│
└── frontend/                 # React application
    ├── public/
    │   └── index.html        # The single HTML page React renders into
    └── src/
        ├── index.js          # React entry point
        ├── App.js            # Decides: show login/register, or the dashboard
        ├── App.css           # All styling for the app
        ├── api.js            # Every fetch() call to the backend, in one place
        └── components/
            ├── Login.js          # Login form
            ├── Register.js       # Registration form
            ├── Dashboard.js      # Main logged-in screen, ties everything together
            ├── ExpenseForm.js    # Form to add a new expense
            ├── ExpenseList.js    # Table of expenses with delete buttons
            ├── ExpenseFilter.js  # Category filter dropdown
            └── TotalSpending.js  # Total spending banner
```

---

## 2. Requirements

**Backend (C++):**
- A C++17 compiler (g++, clang, or MSVC)
- CMake 3.14+
- SQLite3 development headers:
  - **Ubuntu/Debian:** `sudo apt install libsqlite3-dev`
  - **macOS (Homebrew):** `brew install sqlite3`
  - **Windows:** easiest via [vcpkg](https://vcpkg.io): `vcpkg install sqlite3`
- An internet connection the *first* time you build (CMake automatically
  downloads the Crow framework for you — you don't need to install it by hand)

**Frontend (React):**
- [Node.js](https://nodejs.org/) (version 16 or newer) and npm

---

## 3. Setup & Run — Backend

```bash
cd backend
mkdir build
cd build
cmake ..
cmake --build .
```

This produces two programs inside the `build` folder: `expense_server` and `seed_data`.

**Add sample data (optional, but recommended for trying the app out):**

```bash
./seed_data
```

This creates a demo account and 8 sample expenses:
- **username:** `demo`
- **password:** `demo1234`

**Start the server:**

```bash
./expense_server
```

You should see:
```
Database ready (tables created if they didn't already exist).
Expense tracker server running at http://localhost:5000
```

Leave this running — it's your API server. A file called `expenses.db` will
appear in the `build` folder; that's your SQLite database.

> On Windows, the built executables will be named `expense_server.exe` and
> `seed_data.exe`, likely inside a `Debug` or `Release` subfolder depending
> on your generator.

---

## 4. Setup & Run — Frontend

Open a **new** terminal (leave the backend running in the first one):

```bash
cd frontend
npm install
npm start
```

This opens the app in your browser at **http://localhost:3000**. It talks to
the backend at `http://localhost:5000` automatically (see `src/api.js`).

---

## 5. Trying It Out

1. Go to http://localhost:3000
2. Log in with the demo account (`demo` / `demo1234`) — or click "Register
   here" to create your own account
3. Add an expense using the form on the left
4. Use the category dropdown to filter your expenses
5. Watch the "Total Spending" banner update to match the filter
6. Delete an expense with the "Delete" button on its row

---

## 6. How Authentication Works (in plain terms)

1. You register with a username and password. The backend hashes your
   password (mixed with a random "salt") before saving it — the real
   password is never stored anywhere.
2. When you log in, the backend checks your password against the stored
   hash. If it matches, it creates a random "session token" and remembers
   it in memory, then sends it to your browser as a cookie.
3. Every request your browser makes afterward automatically includes that
   cookie, so the backend knows which user is asking without you having to
   log in again on every page load.
4. Logging out simply forgets that token.

This is a simplified approach meant for learning. A production app would
typically use a proper session store (like Redis) and a slower, purpose-built
password hashing algorithm (like bcrypt or Argon2) instead of SHA-256.

---

## 7. Folder & File Purpose Reference

### Backend (`backend/`)
- **`CMakeLists.txt`** — Tells CMake how to build the project. It
  automatically downloads Crow (the web framework) so you don't need to
  install it separately, and links against your system's SQLite3 library.
- **`src/database.hpp` / `database.cpp`** — Opens the SQLite database file
  and creates the `users` and `expenses` tables if they don't exist yet.
- **`src/auth.hpp`** — Contains a from-scratch SHA-256 implementation plus
  helper functions for hashing/verifying passwords and generating random
  session tokens.
- **`src/main.cpp`** — The heart of the backend. Defines every API route
  (`/api/register`, `/api/login`, `/api/expenses`, etc.), reads/writes to
  the database, and manages login sessions.
- **`src/seed_data.cpp`** — A separate small program (not part of the
  server) that inserts a demo user and sample expenses into the database,
  so you have something to look at immediately.

### Frontend (`frontend/`)
- **`public/index.html`** — The one and only HTML page. React takes over
  the `<div id="root">` inside it and renders everything else.
- **`src/index.js`** — The entry point; tells React to render `<App />`.
- **`src/App.js`** — The "traffic controller" of the frontend. Checks if
  you're logged in and shows either the Login/Register screen or the
  Dashboard.
- **`src/App.css`** — All the visual styling, written in plain CSS with
  Flexbox/Grid for a responsive, mobile-friendly layout.
- **`src/api.js`** — Every network call to the backend lives here. If you
  ever need to change the backend's URL or how errors are handled, this is
  the only file you need to touch.
- **`src/components/Login.js`** — The login form.
- **`src/components/Register.js`** — The registration form.
- **`src/components/Dashboard.js`** — The main screen after logging in. It
  fetches categories/expenses and passes data down to the smaller
  components below.
- **`src/components/ExpenseForm.js`** — The "add a new expense" form.
- **`src/components/ExpenseList.js`** — Renders the table of expenses and
  their delete buttons.
- **`src/components/ExpenseFilter.js`** — The category filter dropdown.
- **`src/components/TotalSpending.js`** — The banner showing the total of
  whatever expenses are currently displayed.

---

## 8. Common Issues

- **"Could not find SQLite3"** during `cmake ..` — install the SQLite3
  development package for your OS (see Requirements above), then re-run
  `cmake ..`.
- **Frontend can't reach the backend / CORS errors** — make sure
  `expense_server` is running on port 5000 *before* you use the app, and
  that you're accessing the frontend at `http://localhost:3000` (not `127.0.0.1`,
  since the backend is configured to allow `localhost` specifically).
- **First `cmake ..` takes a while** — that's expected the first time,
  since it's downloading the Crow framework. Later builds are fast.

---

## 9. Ideas for Extending This Project (optional)

- Edit an existing expense (not just add/delete)
- Monthly spending charts
- Export expenses to CSV
- Budget limits with alerts when you're close to going over

Enjoy building on top of this project!
