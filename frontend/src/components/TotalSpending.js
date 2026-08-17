// TotalSpending.js
// ----------------
// A small banner showing the total of whatever expenses are currently
// displayed (this respects the active category filter, since the total
// comes from the same API call as the filtered expense list).

import React from "react";

function TotalSpending({ total }) {
  return (
    <div className="total-banner">
      <span>Total Spending</span>
      <strong>${total.toFixed(2)}</strong>
    </div>
  );
}

export default TotalSpending;
