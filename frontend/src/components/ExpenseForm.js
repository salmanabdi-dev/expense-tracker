// ExpenseForm.js
// --------------
// A form for adding a new expense. It calls the addExpense API and then
// tells the parent component (Dashboard.js) to refresh the expense list.

import React, { useState } from "react";
import { addExpense } from "../api";

function ExpenseForm({ categories, onExpenseAdded }) {
  const today = new Date().toISOString().slice(0, 10); // e.g. "2026-07-21"

  const [title, setTitle] = useState("");
  const [amount, setAmount] = useState("");
  const [category, setCategory] = useState(categories[0] || "");
  const [date, setDate] = useState(today);
  const [error, setError] = useState("");
  const [submitting, setSubmitting] = useState(false);

  async function handleSubmit(event) {
    event.preventDefault();
    setError("");
    setSubmitting(true);

    try {
      await addExpense({
  title,
  amount: Number(amount),
  category,
  date,
});
      // Reset the form after a successful add.
      setTitle("");
      setAmount("");
      setDate(today);
      onExpenseAdded();
    } catch (err) {
      setError(err.message);
    } finally {
      setSubmitting(false);
    }
  }

  return (
    <div className="card">
      <h3>Add Expense</h3>
      <form className="expense-form" onSubmit={handleSubmit}>
        <div className="form-row">
          <label htmlFor="expense-title">Title</label>
          <input
            id="expense-title"
            type="text"
            placeholder="e.g. Groceries"
            value={title}
            onChange={(e) => setTitle(e.target.value)}
            required
          />
        </div>

        <div className="form-row">
          <label htmlFor="expense-amount">Amount ($)</label>
          <input
            id="expense-amount"
            type="number"
            step="0.01"
            min="0.01"
            placeholder="0.00"
            value={amount}
            onChange={(e) => setAmount(e.target.value)}
            required
          />
        </div>

        <div className="form-row">
          <label htmlFor="expense-category">Category</label>
          <select
            id="expense-category"
            value={category}
            onChange={(e) => setCategory(e.target.value)}
          >
            {categories.map((cat) => (
              <option key={cat} value={cat}>
                {cat}
              </option>
            ))}
          </select>
        </div>

        <div className="form-row">
          <label htmlFor="expense-date">Date</label>
          <input
            id="expense-date"
            type="date"
            value={date}
            onChange={(e) => setDate(e.target.value)}
            required
          />
        </div>

        {error && <p className="error-text">{error}</p>}

        <button type="submit" disabled={submitting}>
          {submitting ? "Adding..." : "Add Expense"}
        </button>
      </form>
    </div>
  );
}

export default ExpenseForm;
