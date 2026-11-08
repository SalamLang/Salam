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

createRoot(document.getElementById("root") as HTMLElement).render(
  <StrictMode>
    <App />
    <ConfirmHost />
    <ToastHost />
  </StrictMode>,
);
