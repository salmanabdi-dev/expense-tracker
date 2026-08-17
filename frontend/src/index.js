// index.js
// --------
// This is the very first file that runs. Its only job is to render the
// main <App /> component into the "root" div from public/index.html.

import React from "react";
import ReactDOM from "react-dom/client";
import "./index.css";
import App from "./App";

const root = ReactDOM.createRoot(document.getElementById("root"));
root.render(
  <React.StrictMode>
    <App />
  </React.StrictMode>
);
