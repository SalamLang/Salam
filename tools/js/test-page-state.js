"use strict";
const fs = require("fs");
const path = require("path");
const { chromium } = require("playwright-core");

const file = process.argv[2];
if (!file) {
  console.error("usage: node test-page-state.js page.html");
  console.error("page.html is tests/en/page/components/component_state_list.salam built with salam web");
  console.error("the browser is $CHROME, or Google Chrome or Chromium from the usual places");
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

(async () => {
  const browser = await chromium.launch({ executablePath: browserPath() });
  const page = await browser.newPage();
  const errors = [];
  page.on("pageerror", (e) => errors.push(e.message));
  await page.goto("file://" + path.resolve(file));
  await page.waitForTimeout(100);
  const text = (sel) => page.$eval(sel, (e) => e.textContent).catch(() => null);
  const items = () => page.$$eval("#todo-list li", (l) => l.map((e) => e.textContent).join(","));
  const runs = () => page.evaluate(() => window.__salam_runs);

  check("no script errors on load", errors.length === 0, errors.join(" | "));
  const ready = (await page.$("#todo-add")) && (await page.$("#A-btn")) && (await page.$("#inner"));
  check("the page mounts its components", !!ready);
  if (!ready) {
    await browser.close();
    console.log(failures + " check(s) failed");
    process.exit(1);
  }

  const runs0 = await runs();
  await page.click("#todo-add");
  await page.click("#todo-add");
  check("two clicks add two list items", (await items()) === "t0,t1", await items());
  check("a state write re-runs a scope", (await runs()) > runs0);
  await page.click("#todo-rot");
  check("rotate moves the last item to the front", (await items()) === "t1,t0", await items());

  check("the toggle body starts hidden", (await text("#A-body")) === null);
  await page.click("#A-btn");
  check("the toggle shows its body", (await text("#A-body")) === "Body of A", await text("#A-body"));
  await page.click("#A-btn");
  check("the toggle hides its body again", (await text("#A-body")) === null);

  await page.click("#inner");
  await page.click("#inner");
  check("the inner counter counts", (await text("#inner")) === "inner 2", await text("#inner"));
  const before = await runs();
  await page.click("#outer-bump");
  check("the outer counter counts", (await text("#outer-bump")) === "outer 1", await text("#outer-bump"));
  check("the inner counter keeps its value", (await text("#inner")) === "inner 2", await text("#inner"));
  check("one click renders once", (await runs()) - before <= 2, (await runs()) - before);

  await page.click("#outer-toggle");
  check("hiding the inner component removes it", (await text("#inner")) === null);
  await page.click("#outer-toggle");
  check("showing it again starts it fresh", (await text("#inner")) === "inner 0", await text("#inner"));

  check("no script errors after clicks", errors.length === 0, errors.join(" | "));
  await browser.close();
  console.log(failures === 0 ? "page state ok" : failures + " check(s) failed");
  process.exit(failures === 0 ? 0 : 1);
})();
