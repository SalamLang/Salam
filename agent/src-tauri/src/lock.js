(() => {
  if (window.__salamLocked) return;
  window.__salamLocked = true;

  const READABLE = ".msg-body, .bubble";
  const EDITABLE = "input, textarea, select, [contenteditable]:not([contenteditable='false'])";

  const style = document.createElement("style");
  style.textContent =
    "html, html * { -webkit-user-select: none; user-select: none; -webkit-user-drag: none; -webkit-touch-callout: none; }" +
    "html .msg-body, html .msg-body *, html .bubble, html .bubble *, html input, html textarea, html [contenteditable]:not([contenteditable='false']), html [contenteditable]:not([contenteditable='false']) * { -webkit-user-select: text; user-select: text; }" +
    "html img, html svg, html a { -webkit-user-drag: none; }";
  document.documentElement.appendChild(style);

  const element = (node) => (node && node.nodeType === 1 ? node : node && node.parentElement);
  const within = (node, selector) => {
    const el = element(node);
    return !!(el && el.closest && el.closest(selector));
  };

  const stop = (e) => {
    e.preventDefault();
    e.stopImmediatePropagation();
  };

  window.addEventListener("contextmenu", stop, true);
  window.addEventListener("dragstart", stop, true);
  window.addEventListener(
    "selectstart",
    (e) => {
      if (!within(e.target, READABLE) && !within(e.target, EDITABLE)) e.preventDefault();
    },
    true,
  );

  const blocked = (e) => {
    const mod = e.ctrlKey || e.metaKey;
    const code = e.code;
    if (code === "F12" || code === "F5" || code === "F7") return true;
    if (e.altKey && (code === "ArrowLeft" || code === "ArrowRight" || code === "Home")) return true;
    if (!mod) return false;
    if (code === "KeyA") return !within(e.target, EDITABLE);
    if (e.shiftKey && (code === "KeyI" || code === "KeyJ" || code === "KeyC" || code === "KeyK" || code === "KeyR" || code === "KeyP")) return true;
    if (e.altKey && (code === "KeyI" || code === "KeyJ" || code === "KeyC" || code === "KeyU")) return true;
    return code === "KeyU" || code === "KeyS" || code === "KeyP" || code === "KeyR" || code === "KeyG" || code === "KeyO";
  };

  window.addEventListener(
    "keydown",
    (e) => {
      if (blocked(e)) stop(e);
    },
    true,
  );

  window.addEventListener(
    "keyup",
    (e) => {
      if (e.code === "F12") stop(e);
    },
    true,
  );
})();
