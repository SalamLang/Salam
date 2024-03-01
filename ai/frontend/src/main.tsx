import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import { App } from "./App";
import { ConfirmHost } from "./Confirm";
import { ToastHost } from "./Toasts";
import { applyDocumentLang } from "./i18n";
import "./styles.css";

applyDocumentLang();

createRoot(document.getElementById("root") as HTMLElement).render(
  <StrictMode>
    <App />
    <ConfirmHost />
    <ToastHost />
  </StrictMode>,
);
