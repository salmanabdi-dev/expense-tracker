// api.js
// ------
// Every network call to our backend lives in this ONE file. Keeping all
// fetch() calls here means the rest of the app (components) never has to
// think about URLs, headers, or error handling more than once.
//
// `credentials: "include"` is important - it tells the browser to send
// our session cookie with every request, which is how the backend knows
// who is logged in.

const BASE_URL = "http://localhost:5000/api";

// A small helper that wraps fetch() and turns a non-OK response into a
// thrown error with a readable message (the backend's own error message
// when available).
async function request(path, options = {}) {
  const response = await fetch(`${BASE_URL}${path}`, {
    credentials: "include",
    headers: { "Content-Type": "application/json" },
    ...options,
  });

  const data = await response.json().catch(() => ({}));

  if (!response.ok) {
    throw new Error(data.error || "Something went wrong. Please try again.");
  }

  return data;
}

export function registerUser(username, password) {
  return request("/register", {
    method: "POST",
    body: JSON.stringify({ username, password }),
  });
}

export function loginUser(username, password) {
  return request("/login", {
    method: "POST",
    body: JSON.stringify({ username, password }),
  });
}

export function logoutUser() {
  return request("/logout", { method: "POST" });
}

export function checkAuth() {
  return request("/check-auth");
}

export function getCategories() {
  return request("/categories");
}

export function getExpenses(category) {
  const query = category && category !== "All" ? `?category=${encodeURIComponent(category)}` : "";
  return request(`/expenses${query}`);
}

export function addExpense(expense) {
  return request("/expenses", {
    method: "POST",
    body: JSON.stringify(expense),
  });
}

export function deleteExpense(id) {
  return request(`/expenses/${id}`, { method: "DELETE" });
}
