// App.js
// ------
// The top-level component. It decides which screen to show:
//   - A loading message while we check if the user is already logged in
//   - The Login or Register screen if they're not logged in
//   - The Dashboard if they are

import React, { useEffect, useState } from "react";
import { checkAuth } from "./api";
import Login from "./components/Login";
import Register from "./components/Register";
import Dashboard from "./components/Dashboard";
import "./App.css";

function App() {
  const [checkingAuth, setCheckingAuth] = useState(true);
  const [username, setUsername] = useState(null); // null = not logged in
  const [showRegister, setShowRegister] = useState(false);

  // On page load, ask the backend if we already have a valid session
  // (e.g. because the user refreshed the page).
  useEffect(() => {
    checkAuth()
      .then((data) => {
        if (data.logged_in) {
          setUsername(data.username);
        }
      })
      .finally(() => setCheckingAuth(false));
  }, []);

  if (checkingAuth) {
    return (
      <div className="centered-screen">
        <p>Loading...</p>
      </div>
    );
  }

  if (username) {
    return (
      <Dashboard
        username={username}
        onLogout={() => setUsername(null)}
      />
    );
  }

  return (
    <div className="centered-screen">
      {showRegister ? (
        <Register onSwitchToLogin={() => setShowRegister(false)} />
      ) : (
        <Login
          onLoginSuccess={(name) => setUsername(name)}
          onSwitchToRegister={() => setShowRegister(true)}
        />
      )}
    </div>
  );
}

export default App;
