import { spawn } from "node:child_process";
import { mkdirSync, readFileSync, writeFileSync } from "node:fs";

const PORT = Number(process.env.SALAM_BROWSER_PORT) || 9333;
const PROFILE = process.env.SALAM_BROWSER_PROFILE || "/tmp/salam-browser";
const STATE = `${PROFILE}-state.json`;
const MAX_IMAGE = 300000;
const MAX_TEXT = 20000;
const CALL_MS = 30000;

const LOGGER = `(() => {
  if (window.__salamLog) return;
  const log = (window.__salamLog = []);
  const net = (window.__salamNet = []);
  const keep = (list, line) => { list.push(line); if (list.length > 300) list.shift(); };
  const show = (x) => { try { return x instanceof Error ? x.stack || x.message : typeof x === "object" && x !== null ? JSON.stringify(x) : String(x); } catch { return String(x); } };
  const push = (level, text) => keep(log, level + ": " + String(text).slice(0, 1000));
  for (const level of ["log", "info", "debug", "warn", "error"]) {
    const orig = console[level].bind(console);
    console[level] = (...a) => { push(level === "warn" ? "warn" : level, a.map(show).join(" ")); orig(...a); };
  }
  addEventListener("error", (e) => {
    const t = e.target;
    if (t && t !== window && (t.src || t.href)) push("failed", (t.tagName || "").toLowerCase() + " " + (t.src || t.href));
    else push("exception", (e.message || "error") + (e.filename ? " at " + e.filename + ":" + e.lineno : ""));
  }, true);
  addEventListener("unhandledrejection", (e) => push("exception", "unhandled rejection: " + show(e.reason)));
  addEventListener("securitypolicyviolation", (e) => push("error", "CSP blocked " + e.blockedURI + " (" + e.violatedDirective + ")"));
  const seenUrls = new Set();
  const note = (entry) => {
    if (seenUrls.has(entry.name + entry.startTime)) return;
    seenUrls.add(entry.name + entry.startTime);
    keep(net, "GET " + (entry.responseStatus || "-") + " " + entry.initiatorType + " " + entry.name.slice(0, 200) + " " + Math.round(entry.duration) + "ms");
  };
  try { new PerformanceObserver((l) => l.getEntries().forEach(note)).observe({ type: "resource", buffered: true }); } catch {}
  const origFetch = window.fetch && window.fetch.bind(window);
  if (origFetch) window.fetch = async (input, init) => {
    const url = String(input && input.url ? input.url : input).slice(0, 200);
    const method = (init && init.method) || (input && input.method) || "GET";
    try { const r = await origFetch(input, init); if (r.status >= 400) push("failed", r.status + " " + method + " " + url); return r; }
    catch (err) { keep(net, method + " failed fetch " + url + " " + show(err)); push("failed", method + " " + url + ": " + show(err)); throw err; }
  };
  const open = XMLHttpRequest.prototype.open;
  XMLHttpRequest.prototype.open = function (method, url, ...rest) {
    this.addEventListener("loadend", () => { keep(net, method + " " + (this.status || "failed") + " xhr " + String(url).slice(0, 200)); if (!this.status || this.status >= 400) push("failed", (this.status || "network error") + " " + method + " " + url); });
    return open.call(this, method, url, ...rest);
  };
})()`;

const OUTLINE = `(() => {
  const clean = (s) => String(s || "").replace(/\\s+/g, " ").trim();
  const seen = (el) => { const r = el.getBoundingClientRect(); const st = getComputedStyle(el); return r.width > 0 && r.height > 0 && st.visibility !== "hidden" && st.display !== "none"; };
  const pick = (sel, fn, n) => Array.from(document.querySelectorAll(sel)).filter(seen).slice(0, n).map(fn).filter(Boolean);
  return {
    text: clean(document.body ? document.body.innerText : "").slice(0, ${MAX_TEXT}),
    headings: pick("h1,h2,h3", (h) => h.tagName.toLowerCase() + " " + clean(h.innerText), 40),
    links: pick("a[href]", (a) => clean(a.innerText || a.getAttribute("aria-label")) + " -> " + a.href, 60),
    buttons: pick("button,[role=button],input[type=submit],input[type=button]", (b) => clean(b.innerText || b.value || b.getAttribute("aria-label")) || "(unlabeled button)", 40),
    inputs: pick("input:not([type=hidden]),textarea,select", (i) => [i.tagName.toLowerCase(), i.type, i.name && "name=" + i.name, i.id && "#" + i.id, i.placeholder && "placeholder=" + clean(i.placeholder)].filter(Boolean).join(" "), 40),
    broken: Array.from(document.images).filter((im) => im.complete && im.naturalWidth === 0 && im.src).slice(0, 20).map((im) => im.src),
    noalt: Array.from(document.images).filter((im) => !im.hasAttribute("alt")).length,
  };
})()`;

const KEYS = {
  Enter: [13, "\r"],
  Tab: [9, ""],
  Escape: [27, ""],
  Backspace: [8, ""],
  Delete: [46, ""],
  ArrowUp: [38, ""],
  ArrowDown: [40, ""],
  ArrowLeft: [37, ""],
  ArrowRight: [39, ""],
  Home: [36, ""],
  End: [35, ""],
  PageUp: [33, ""],
  PageDown: [34, ""],
  Space: [32, " "],
};

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

function finish(output, image) {
  process.stdout.write(JSON.stringify({ output, image: image || "", image_mime: image ? "image/jpeg" : "" }));
  process.exit(0);
}

function readState() {
  try {
    return JSON.parse(readFileSync(STATE, "utf8"));
  } catch {
    return { width: 1280, height: 800 };
  }
}

async function json(path, method = "GET") {
  try {
    const r = await fetch(`http://127.0.0.1:${PORT}${path}`, { method });
    return r.ok ? await r.json() : null;
  } catch {
    return null;
  }
}

async function ensureBrowser(state) {
  if (await json("/json/version")) return true;
  mkdirSync(PROFILE, { recursive: true });
  const bin = process.env.SALAM_BROWSER || "chromium";
  const child = spawn(
    bin,
    [
      "--headless=new",
      "--no-sandbox",
      "--disable-gpu",
      "--disable-dev-shm-usage",
      "--no-first-run",
      "--no-default-browser-check",
      "--disable-extensions",
      "--hide-scrollbars",
      "--mute-audio",
      `--remote-debugging-port=${PORT}`,
      `--user-data-dir=${PROFILE}`,
      `--window-size=${state.width},${state.height}`,
      "about:blank",
    ],
    { detached: true, stdio: "ignore" },
  );
  child.on("error", () => {});
  child.unref();
  for (let i = 0; i < 75; i += 1) {
    await sleep(200);
    if (await json("/json/version")) return true;
  }
  return false;
}

class Cdp {
  constructor(url) {
    this.url = url;
    this.next = 1;
    this.pending = new Map();
    this.handlers = new Map();
  }
  open() {
    return new Promise((resolve, reject) => {
      this.ws = new WebSocket(this.url);
      this.ws.onopen = () => resolve();
      this.ws.onerror = () => reject(new Error("could not connect to the browser"));
      this.ws.onmessage = (ev) => {
        const m = JSON.parse(typeof ev.data === "string" ? ev.data : ev.data.toString());
        if (m.id && this.pending.has(m.id)) {
          const { ok, fail } = this.pending.get(m.id);
          this.pending.delete(m.id);
          if (m.error) fail(new Error(m.error.message));
          else ok(m.result);
        } else if (m.method) {
          for (const fn of this.handlers.get(m.method) ?? []) fn(m.params);
        }
      };
    });
  }
  send(method, params = {}) {
    const id = this.next++;
    this.ws.send(JSON.stringify({ id, method, params }));
    return new Promise((ok, fail) => {
      this.pending.set(id, { ok, fail });
      setTimeout(() => {
        if (this.pending.delete(id)) fail(new Error(`${method} timed out`));
      }, CALL_MS);
    });
  }
  on(method, fn) {
    if (!this.handlers.has(method)) this.handlers.set(method, []);
    this.handlers.get(method).push(fn);
  }
  once(method, ms) {
    return new Promise((resolve) => {
      let done = false;
      this.on(method, (p) => {
        if (!done) {
          done = true;
          resolve(p);
        }
      });
      setTimeout(() => {
        if (!done) {
          done = true;
          resolve(null);
        }
      }, ms);
    });
  }
}

async function evaluate(cdp, expression) {
  const r = await cdp.send("Runtime.evaluate", { expression, awaitPromise: true, returnByValue: true });
  if (r.exceptionDetails) throw new Error(r.exceptionDetails.exception?.description || r.exceptionDetails.text || "the script threw");
  return r.result?.value;
}

async function shot(cdp, args, state) {
  let clip;
  if (args.selector) {
    const rect = await evaluate(cdp, `(() => { const el = document.querySelector(${JSON.stringify(args.selector)}); if (!el) return null; el.scrollIntoView({block: "center"}); const r = el.getBoundingClientRect(); return {x: r.left + scrollX, y: r.top + scrollY, width: Math.max(1, r.width), height: Math.max(1, Math.min(r.height, 8000))}; })()`);
    if (!rect) throw new Error(`no element matches ${args.selector}`);
    clip = { ...rect, scale: 1 };
  } else if (args.full_page) {
    const m = await cdp.send("Page.getLayoutMetrics");
    const size = m.cssContentSize || m.contentSize;
    clip = { x: 0, y: 0, width: Math.min(size.width, 4000), height: Math.min(size.height, 8000), scale: 1 };
  }
  for (let round = 0; round < 3; round += 1) {
    for (const quality of [70, 50, 35, 25]) {
      const r = await cdp.send("Page.captureScreenshot", { format: "jpeg", quality, ...(clip ? { clip, captureBeyondViewport: true } : {}) });
      if (r.data.length <= MAX_IMAGE) {
        const w = clip ? Math.round(clip.width) : state.width;
        const h = clip ? Math.round(clip.height) : state.height;
        return { data: r.data, note: `Screenshot attached (${w}x${h}${round > 0 ? ", cut to fit" : ""}).` };
      }
    }
    if (!clip || clip.height <= 1000) break;
    clip.height = Math.round(clip.height / 2);
  }
  return { data: "", note: "The screenshot was too large to send; use text to read the page." };
}

async function main() {
  const args = JSON.parse(readFileSync(0, "utf8") || "{}");
  const action = String(args.action || "");
  const state = readState();
  if (args.width > 0 && args.height > 0) {
    state.width = Math.min(Math.max(Math.round(args.width), 200), 3840);
    state.height = Math.min(Math.max(Math.round(args.height), 200), 3840);
    writeFileSync(STATE, JSON.stringify(state));
  }
  if (action === "close") {
    const v = await json("/json/version");
    if (!v) finish("No browser was open.");
    const b = new Cdp(v.webSocketDebuggerUrl);
    await b.open();
    await b.send("Browser.close").catch(() => {});
    finish("The browser is closed.");
  }
  if (!(await ensureBrowser(state))) finish("Chromium did not start in the sandbox.");
  let pages = ((await json("/json/list")) || []).filter((t) => t.type === "page");
  if (pages.length === 0) pages = [await json("/json/new?about:blank", "PUT")];
  const cdp = new Cdp(pages[0].webSocketDebuggerUrl);
  await cdp.open();
  const problems = [];
  let status = 0;
  let live = false;
  cdp.on("Runtime.exceptionThrown", (p) => live && problems.push(`exception: ${p.exceptionDetails.exception?.description?.split("\n")[0] || p.exceptionDetails.text}`));
  cdp.on("Runtime.consoleAPICalled", (p) => {
    if (live && (p.type === "error" || p.type === "warning")) problems.push(`${p.type === "warning" ? "warn" : "error"}: ${p.args.map((a) => a.value ?? a.description ?? "").join(" ").slice(0, 500)}`);
  });
  cdp.on("Network.responseReceived", (p) => {
    if (p.type === "Document" && !status) status = p.response.status;
    if (p.response.status >= 400) problems.push(`failed: ${p.response.status} ${p.response.url}`);
  });
  cdp.on("Network.loadingFailed", (p) => {
    if (!p.canceled) problems.push(`failed: ${p.errorText} (${p.type})`);
  });
  cdp.on("Log.entryAdded", (p) => {
    if (live && (p.entry.level === "error" || p.entry.level === "warning")) problems.push(`${p.entry.level === "warning" ? "warn" : "error"} (${p.entry.source}): ${String(p.entry.text).slice(0, 500)}`);
  });
  await cdp.send("Page.enable");
  await cdp.send("Runtime.enable");
  live = true;
  await cdp.send("Network.enable");
  await cdp.send("Log.enable");
  await cdp.send("Page.addScriptToEvaluateOnNewDocument", { source: LOGGER });
  const mobile = state.width <= 500;
  await cdp.send("Emulation.setDeviceMetricsOverride", { width: state.width, height: state.height, deviceScaleFactor: 1, mobile });
  await cdp.send("Emulation.setTouchEmulationEnabled", { enabled: mobile });
  if (mobile) await cdp.send("Emulation.setUserAgentOverride", { userAgent: "Mozilla/5.0 (Linux; Android 14; Pixel 8) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130.0 Mobile Safari/537.36" });
  const settle = async (ms) => {
    await sleep(Math.min(Math.max(Number(args.wait_ms) || ms, 0), 15000));
    await evaluate(cdp, LOGGER).catch(() => {});
  };
  let body = "";
  let wantShot = false;
  switch (action) {
    case "open": {
      if (!args.url) finish("open needs a url.");
      const loaded = cdp.once("Page.loadEventFired", 20000);
      const nav = await cdp.send("Page.navigate", { url: String(args.url) });
      if (nav.errorText) finish(`Could not open ${args.url}: ${nav.errorText}`);
      await loaded;
      await settle(1500);
      wantShot = true;
      break;
    }
    case "screenshot":
      wantShot = true;
      break;
    case "click": {
      let x = Number(args.x);
      let y = Number(args.y);
      if (args.selector) {
        const c = await evaluate(cdp, `(() => { const el = document.querySelector(${JSON.stringify(args.selector)}); if (!el) return null; el.scrollIntoView({block: "center"}); const r = el.getBoundingClientRect(); return {x: r.left + r.width / 2, y: r.top + r.height / 2}; })()`);
        if (!c) finish(`No element matches ${args.selector}.`);
        x = c.x;
        y = c.y;
      }
      if (!Number.isFinite(x) || !Number.isFinite(y)) finish("click needs a selector, or x and y.");
      await cdp.send("Input.dispatchMouseEvent", { type: "mouseMoved", x, y });
      await cdp.send("Input.dispatchMouseEvent", { type: "mousePressed", x, y, button: "left", clickCount: 1 });
      await cdp.send("Input.dispatchMouseEvent", { type: "mouseReleased", x, y, button: "left", clickCount: 1 });
      await settle(1000);
      wantShot = true;
      break;
    }
    case "type":
    case "press": {
      if (action === "type") {
        if (args.selector) {
          const ok = await evaluate(cdp, `(() => { const el = document.querySelector(${JSON.stringify(args.selector)}); if (!el) return false; el.scrollIntoView({block: "center"}); el.focus(); return true; })()`);
          if (!ok) finish(`No element matches ${args.selector}.`);
        }
        await cdp.send("Input.insertText", { text: String(args.text ?? "") });
      }
      const key = String(args.key || "");
      if (key) {
        const [code, text] = KEYS[key] ?? [key.length === 1 ? key.toUpperCase().charCodeAt(0) : 0, key.length === 1 ? key : ""];
        await cdp.send("Input.dispatchKeyEvent", { type: "keyDown", key: key === "Space" ? " " : key, code: key, windowsVirtualKeyCode: code, text });
        await cdp.send("Input.dispatchKeyEvent", { type: "keyUp", key: key === "Space" ? " " : key, code: key, windowsVirtualKeyCode: code });
      } else if (action === "press") {
        finish("press needs a key, such as Enter.");
      }
      await settle(800);
      wantShot = true;
      break;
    }
    case "scroll": {
      if (args.selector) {
        const ok = await evaluate(cdp, `(() => { const el = document.querySelector(${JSON.stringify(args.selector)}); if (!el) return false; el.scrollIntoView({block: "center"}); return true; })()`);
        if (!ok) finish(`No element matches ${args.selector}.`);
      } else {
        await evaluate(cdp, `scrollBy(0, ${Number(args.y) || 0} || innerHeight)`);
      }
      await settle(500);
      wantShot = true;
      break;
    }
    case "eval": {
      if (!args.script) finish("eval needs a script.");
      try {
        const v = await evaluate(cdp, String(args.script));
        body = `Result: ${JSON.stringify(v ?? null, null, 2).slice(0, MAX_TEXT)}`;
      } catch (e) {
        body = `The script threw: ${e.message}`;
      }
      break;
    }
    case "text": {
      const o = await evaluate(cdp, OUTLINE);
      const part = (label, list) => (list.length ? `\n\n${label}:\n- ${list.join("\n- ")}` : "");
      body = `${o.text}${part("Headings", o.headings)}${part("Links", o.links)}${part("Buttons", o.buttons)}${part("Inputs", o.inputs)}${part("Broken images", o.broken)}${o.noalt ? `\n\n${o.noalt} images have no alt text.` : ""}`;
      break;
    }
    case "console": {
      const log = (await evaluate(cdp, "(() => { const l = (window.__salamLog || []).slice(); if (window.__salamLog) window.__salamLog.length = 0; return l; })()").catch(() => [])) || [];
      await evaluate(cdp, LOGGER).catch(() => {});
      body = log.length ? `Console since the last check (every level):\n- ${log.join("\n- ")}` : "Nothing in the console since the last check.";
      break;
    }
    case "network": {
      const list = (await evaluate(cdp, "(() => { const l = (window.__salamNet || []).slice(); if (window.__salamNet) window.__salamNet.length = 0; return l; })()").catch(() => [])) || [];
      await evaluate(cdp, LOGGER).catch(() => {});
      body = list.length ? `Requests since the last check (method, status, type, url, time):\n- ${list.join("\n- ")}` : "No requests since the last check.";
      break;
    }
    case "hover": {
      let x = Number(args.x);
      let y = Number(args.y);
      if (args.selector) {
        const c = await evaluate(cdp, `(() => { const el = document.querySelector(${JSON.stringify(args.selector)}); if (!el) return null; el.scrollIntoView({block: "center"}); const r = el.getBoundingClientRect(); return {x: r.left + r.width / 2, y: r.top + r.height / 2}; })()`);
        if (!c) finish(`No element matches ${args.selector}.`);
        x = c.x;
        y = c.y;
      }
      if (!Number.isFinite(x) || !Number.isFinite(y)) finish("hover needs a selector, or x and y.");
      await cdp.send("Input.dispatchMouseEvent", { type: "mouseMoved", x, y });
      await settle(500);
      wantShot = true;
      break;
    }
    case "select": {
      if (!args.selector) finish("select needs the selector of a <select> and the option in text.");
      const picked = await evaluate(cdp, `(() => { const el = document.querySelector(${JSON.stringify(args.selector)}); if (!el || !el.options) return "missing"; const want = ${JSON.stringify(String(args.text ?? ""))}; const opt = Array.from(el.options).find((o) => o.value === want || o.text.trim() === want); if (!opt) return "no option " + want + "; options: " + Array.from(el.options).map((o) => o.text.trim()).join(", "); el.value = opt.value; el.dispatchEvent(new Event("input", {bubbles: true})); el.dispatchEvent(new Event("change", {bubbles: true})); return "selected " + opt.text.trim(); })()`);
      if (picked === "missing") finish(`No <select> matches ${args.selector}.`);
      body = picked;
      await settle(500);
      wantShot = true;
      break;
    }
    case "wait": {
      if (!args.selector && !args.text) finish("wait needs a selector or a text to wait for.");
      const limit = Math.min(Math.max(Number(args.wait_ms) || 10000, 100), 30000);
      const probe = args.selector ? `(() => { const el = document.querySelector(${JSON.stringify(args.selector)}); if (!el) return false; const r = el.getBoundingClientRect(); return r.width > 0 && r.height > 0; })()` : `(document.body ? document.body.innerText : "").includes(${JSON.stringify(String(args.text))})`;
      const began = Date.now();
      let found = false;
      while (!found && Date.now() - began < limit) {
        found = await evaluate(cdp, probe).catch(() => false);
        if (!found) await sleep(200);
      }
      body = found ? `Found ${args.selector || JSON.stringify(args.text)} after ${Date.now() - began} ms.` : `Still not there after ${limit} ms: ${args.selector || JSON.stringify(args.text)}.`;
      wantShot = !found;
      break;
    }
    case "reload": {
      const loaded = cdp.once("Page.loadEventFired", 20000);
      await cdp.send("Page.reload", { ignoreCache: true });
      await loaded;
      await settle(1000);
      wantShot = true;
      break;
    }
    case "back": {
      const loaded = cdp.once("Page.loadEventFired", 10000);
      await evaluate(cdp, "history.back()");
      await loaded;
      await settle(800);
      wantShot = true;
      break;
    }
    default:
      finish(`Unknown browser action ${action}. Use open, screenshot, click, hover, type, press, select, scroll, wait, eval, text, console, network, reload, back or close.`);
  }
  const info = await evaluate(cdp, "({url: location.href, title: document.title})").catch(() => ({ url: "", title: "" }));
  let head = `${info.url}${status ? ` (${status})` : ""} "${info.title}"`;
  let image = "";
  if (wantShot) {
    const s = await shot(cdp, action === "screenshot" ? args : {}, state);
    image = s.data;
    head += `\n${s.note}`;
  }
  const unique = [...new Set(problems)].slice(0, 30);
  const tail = unique.length ? `\n\nProblems seen:\n- ${unique.join("\n- ")}` : "";
  finish(`${head}${body ? `\n\n${body}` : ""}${tail}`, image);
}

setTimeout(() => finish("The browser did not answer in time."), 75000).unref();
main().catch((e) => finish(`The browser failed: ${e.message}`));
