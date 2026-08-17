// ExpenseFilter.js
// ----------------
// A dropdown that lets the user filter expenses by category. "All" shows
// everything. Selecting a category tells the parent (Dashboard.js) to
// re-fetch expenses filtered to just that category.

import React from "react";

function ExpenseFilter({ categories, selectedCategory, onChange }) {
  return (
    <div className="filter-row">
      <label htmlFor="category-filter">Filter by category:</label>
      <select
        id="category-filter"
        value={selectedCategory}
        onChange={(e) => onChange(e.target.value)}
      >
        <option value="All">All</option>
        {categories.map((cat) => (
          <option key={cat} value={cat}>
            {cat}
          </option>
        ))}
      </select>
    </div>
  );
}

export default ExpenseFilter;
