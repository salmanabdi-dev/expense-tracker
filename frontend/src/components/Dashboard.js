// Dashboard.js
// ------------
// The main screen a user sees once logged in. It:
//   - Fetches the category list and expenses from the backend
//   - Re-fetches expenses whenever the category filter changes
//   - Passes data and callback functions down to the smaller components

import React, { useEffect, useState, useCallback } from "react";
import { getCategories, getExpenses, deleteExpense, logoutUser } from "../api";
import ExpenseForm from "./ExpenseForm";
import ExpenseList from "./ExpenseList";
import ExpenseFilter from "./ExpenseFilter";
import TotalSpending from "./TotalSpending";

function Dashboard({ username, onLogout }) {
  const [categories, setCategories] = useState([]);
  const [expenses, setExpenses] = useState([]);
  const [total, setTotal] = useState(0);
  const [selectedCategory, setSelectedCategory] = useState("All");
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState("");

  // useCallback so this function has a stable identity across renders,
  // which keeps the useEffect below from re-running unnecessarily.
  const loadExpenses = useCallback(async (category) => {
    setLoading(true);
    setError("");
    try {
      const data = await getExpenses(category);
      setExpenses(data.expenses);
      setTotal(data.total);
    } catch (err) {
      setError(err.message);
    } finally {
      setLoading(false);
    }
  }, []);

  // On first load: fetch the categories once, then fetch expenses.
  useEffect(() => {
    getCategories()
      .then((data) => setCategories(data.categories))
      .catch(() => setCategories([]));
  }, []);

  // Whenever the selected category changes, re-fetch the expense list.
  useEffect(() => {
    loadExpenses(selectedCategory);
  }, [selectedCategory, loadExpenses]);

  async function handleDelete(id) {
    try {
      await deleteExpense(id);
      loadExpenses(selectedCategory); // refresh the list after deleting
    } catch (err) {
      setError(err.message);
    }
  }

  async function handleLogout() {
    await logoutUser();
    onLogout();
  }

  return (
    <div className="dashboard">
      <header className="dashboard-header">
        <h1>Expense Tracker</h1>
        <div className="header-right">
          <span>Hi, {username}!</span>
          <button type="button" className="logout-button" onClick={handleLogout}>
            Log Out
          </button>
        </div>
      </header>

      <div className="dashboard-grid">
        <div className="dashboard-left">
          <ExpenseForm
            categories={categories}
            onExpenseAdded={() => loadExpenses(selectedCategory)}
          />
        </div>

        <div className="dashboard-right">
          <TotalSpending total={total} />
          <ExpenseFilter
            categories={categories}
            selectedCategory={selectedCategory}
            onChange={setSelectedCategory}
          />

          {error && <p className="error-text">{error}</p>}
          {loading ? (
            <p className="loading-message">Loading expenses...</p>
          ) : (
            <ExpenseList expenses={expenses} onDelete={handleDelete} />
          )}
        </div>
      </div>
    </div>
  );
}

export default Dashboard;
