// ExpenseList.js
// --------------
// Displays all the expenses that were fetched from the backend, in a
// simple table, with a delete button on each row.

import React from "react";

function ExpenseList({ expenses, onDelete }) {
  if (expenses.length === 0) {
    return (
      <div className="card">
        <h3>Your Expenses</h3>
        <p className="empty-message">No expenses to show yet. Add one above!</p>
      </div>
    );
  }

  return (
    <div className="card">
      <h3>Your Expenses</h3>
      <div className="table-wrapper">
        <table className="expense-table">
          <thead>
            <tr>
              <th>Title</th>
              <th>Category</th>
              <th>Date</th>
              <th>Amount</th>
              <th></th>
            </tr>
          </thead>
          <tbody>
            {expenses.map((expense) => (
              <tr key={expense.id}>
                <td>{expense.title}</td>
                <td>
                  <span className="category-badge">{expense.category}</span>
                </td>
                <td>{expense.date}</td>
                <td className="amount-cell">${expense.amount.toFixed(2)}</td>
                <td>
                  <button
                    type="button"
                    className="delete-button"
                    onClick={() => onDelete(expense.id)}
                    aria-label={`Delete ${expense.title}`}
                  >
                    Delete
                  </button>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
}

export default ExpenseList;
