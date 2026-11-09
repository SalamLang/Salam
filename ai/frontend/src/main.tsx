import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import { App } from "./App";
import { ConfirmHost } from "./Confirm";
import { ToastHost } from "./Toasts";
import { applyDocumentLang } from "./i18n";
import { installLinkHandler, isDesktop } from "./desktop";
import "./styles.css";

applyDocumentLang();
installLinkHandler();

document.addEventListener("contextmenu", (e) => {
  const el = e.target;
  if (!(el instanceof HTMLImageElement)) return;
  if (isDesktop || !el.hasAttribute("data-user-image")) e.preventDefault();
});
document.addEventListener("dragstart", (e) => {
  if (e.target instanceof HTMLImageElement) e.preventDefault();
});

function labelCells(root: ParentNode) {
  root.querySelectorAll<HTMLTableElement>("table.data-table").forEach((table) => {
    const heads = Array.from(table.querySelectorAll("thead th")).map((h) => (h.textContent || "").trim());
    table.classList.toggle("wide", heads.length > 9);
    table.querySelectorAll("tbody tr").forEach((row) => {
      Array.from(row.children).forEach((cell, i) => {
        const label = heads[i];
        if (label && cell.getAttribute("data-label") !== label) cell.setAttribute("data-label", label);
      });
    });
  });
}

let labelTimer = 0;
new MutationObserver(() => {
  cancelAnimationFrame(labelTimer);
  labelTimer = requestAnimationFrame(() => labelCells(document));
}).observe(document.body, { childList: true, subtree: true });

createRoot(document.getElementById("root") as HTMLElement).render(
  <StrictMode>
    <App />
    <ConfirmHost />
    <ToastHost />
  </StrictMode>,
);
