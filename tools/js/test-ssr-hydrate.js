"use strict";
const fs = require("fs");
const path = require("path");
const { chromium } = require("playwright-core");

const base = process.argv[2];
const publicDir = process.argv[3];
if (!base || !publicDir) {
  console.error("usage: node test-ssr-hydrate.js http://127.0.0.1:PORT path/to/dist/public");
  console.error("the server is the one built by 'salam page build --ssr' from tests/en/ssr/site");
  process.exit(2);
}

function browserPath() {
  const list = [process.env.CHROME, "/usr/bin/google-chrome", "/usr/bin/chromium", "/usr/bin/chromium-browser"];
  for (const p of list) {
    if (p && fs.existsSync(p)) return p;
  }
  console.error("no browser found; set CHROME to a Chrome or Chromium binary");
  process.exit(2);
}

let failures = 0;
function check(name, ok, detail) {
  if (ok) {
    console.log("PASS " + name);
  } else {
    failures++;
    console.log("FAIL " + name + (detail !== undefined ? " (" + detail + ")" : ""));
  }
}

const watch = () => {
  window.__removed = 0;
  window.__ids = {};
  new MutationObserver((list) => {
    for (const m of list) {
      for (const n of m.removedNodes) {
        if (n.nodeType === 1) window.__removed++;
      }
    }
  }).observe(document, { childList: true, subtree: true });
};

const body = (p) => p.evaluate(() => document.body.innerHTML.replace(/<script[^>]*>[\s\S]*?<\/script>/g, ""));

async function open(browser, mode) {
  const page = await browser.newPage();
  const logs = [];
  page.on("pageerror", (e) => logs.push("error: " + e.message));
  page.on("console", (m) => {
    if (!m.text().startsWith("Failed to load resource")) logs.push(m.type() + ": " + m.text());
  });
  await page.addInitScript(watch);
  if (mode) {
    await page.route(base + "/counter", async (route) => {
      const res = await route.fetch();
      await route.fulfill({ response: res, body: mode(await res.text()) });
    });
  }
  await page.goto(base + "/counter");
  await page.waitForSelector("#count");
  await page.waitForTimeout(150);
  return { page, logs };
}

(async () => {
  const browser = await chromium.launch({ executablePath: browserPath() });
  const served = await (await fetch(base + "/counter")).text();
  check("the served HTML already has the page", served.includes('id="count"') && served.includes("Count 0"));
  check("the served HTML carries the state", served.includes('id="__salam"'));

  const a = await open(browser, null);
  check("hydration keeps the server nodes", (await a.page.evaluate(() => window.__removed)) === 0, await a.page.evaluate(() => window.__removed));
  check("hydration prints no warning or error", a.logs.length === 0, a.logs.join(" | "));
  check("the state is read", (await a.page.evaluate(() => window.__salam_state && window.__salam_state.status)) === 200);
  await a.page.click("#inc");
  await a.page.click("#inc");
  check("a click works after hydration", (await a.page.textContent("#count")) === "Count 2", await a.page.textContent("#count"));
  const withCalls = await open(browser, (t) => t.replace('"calls":{}', '"calls":{"F:162c1995:0":7}'));
  const key = await withCalls.page.evaluate(() => {
    const k = window.__salam_call_key("F", '{"id":3}');
    const k2 = window.__salam_call_key("F", '{"id":3}');
    return [k, window.__salam_call_cache_get(k), window.__salam_call_cache_get(k2)];
  });
  check("the call cache hook answers by key", key[0] === "F:162c1995:0" && key[1] === 7 && key[2] == null, JSON.stringify(key));

  const fresh = await open(browser, null);
  const hydrated = await body(fresh.page);
  const shell = fs.readFileSync(path.join(publicDir, ".shell.html"), "utf8");
  const only = await open(browser, () => shell);
  const rendered = await body(only.page);
  check("server HTML plus bundle equals a client-only render", hydrated === rendered, hydrated.slice(0, 200) + " <> " + rendered.slice(0, 200));
  check("the client-only render prints no hydration warning", !only.logs.some((l) => l.includes("hydration")));

  const bad = await open(browser, (t) => t.replace("Count 0", "Count 9"));
  check("a differing scope warns", bad.logs.some((l) => l.includes("hydration mismatch")), bad.logs.join(" | "));
  check("a differing scope is rendered again from state", (await bad.page.textContent("#count")) === "Count 0");
  check("a mismatch is not an error", !bad.logs.some((l) => l.startsWith("error")));
  await bad.page.click("#inc");
  check("a click works after a mismatch", (await bad.page.textContent("#count")) === "Count 1");

  await browser.close();
  console.log(failures + " check(s) failed");
  process.exit(failures ? 1 : 0);
})();
