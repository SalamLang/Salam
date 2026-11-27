const fs = require("fs");
const path = require("path");
const { spawn } = require("child_process");
const { chromium } = require("playwright-core");

const dist = process.argv[2];
if (!dist) {
  console.error("usage: node test-ssr-state.js path/to/dist");
  process.exit(2);
}
const fa = process.env.SALAM_TEST_LANG === "fa";
const valueWord = fa ? "مقدار" : "value";
const againWord = fa ? "دوباره" : "again";

function browserPath() {
  const list = [
    process.env.CHROME,
    "/usr/bin/google-chrome",
    "/usr/bin/chromium",
    "/usr/bin/chromium-browser",
  ];
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
    console.log(
      "FAIL " + name + (detail !== undefined ? " (" + detail + ")" : ""),
    );
  }
}

function startServer() {
  const child = spawn(path.join(dist, "server"), [], {
    env: { ...process.env, PORT: "0", HOST: "127.0.0.1" },
    stdio: ["ignore", "pipe", "inherit"],
  });
  return new Promise((resolve, reject) => {
    let out = "";
    const timer = setTimeout(
      () => reject(new Error("server did not start: " + out)),
      15000,
    );
    child.stdout.on("data", (d) => {
      out += d;
      const m = out.match(/listening on (http:\/\/127\.0\.0\.1:\d+)/);
      if (m) {
        clearTimeout(timer);
        resolve({ child, base: m[1], early: out.slice(0, m.index) });
      }
    });
  });
}

async function open(browser, url) {
  const page = await browser.newPage();
  const seen = { calls: [], logs: [] };
  page.on("pageerror", (e) => seen.logs.push("error: " + e.message));
  page.on("console", (m) => {
    if (!m.text().startsWith("Failed to load resource"))
      seen.logs.push(m.type() + ": " + m.text());
  });
  page.on("request", (r) => {
    const m = r.url().match(/\/_salam\/fn\/(.*)$/);
    if (m) seen.calls.push(m[1]);
  });
  await page.goto(url);
  await page.waitForTimeout(300);
  return { page, seen };
}

async function click(tab, selector) {
  tab.seen.calls.length = 0;
  await tab.page.click(selector);
  await tab.page.waitForTimeout(300);
  return tab.seen.calls.slice().sort().join(",");
}

const text = async (tab, selector) =>
  (await tab.page.textContent(selector)) || "";
const number = async (tab) =>
  Number((await text(tab, "#value")).slice(valueWord.length + 1));

(async () => {
  const server = await startServer();
  let browser;
  try {
    check(
      "no server function runs at startup",
      !server.early.includes(" ran"),
      server.early,
    );
    browser = await chromium.launch({ executablePath: browserPath() });

    const home = await open(browser, server.base + "/");
    check(
      "loading a page calls nothing",
      home.seen.calls.length === 0,
      home.seen.calls.join(","),
    );
    const first = await number(home);
    check("the rendered value is shown", first >= 1, first);

    let before = first;
    for (let i = 1; i <= 2; i++) {
      const calls = await click(home, "#reload");
      check("reload " + i + " sends one request", calls === "Next", calls);
      const now = await number(home);
      check(
        "reload " + i + " shows a newer value",
        now > before,
        before + " -> " + now,
      );
      before = now;
    }

    let calls = await click(home, "#to-other");
    check(
      "entering a route calls what that route needs",
      calls === "Details,Names,Other",
      calls,
    );
    check(
      "the entered route shows its state",
      (await text(home, "#info")) === "BOX 3 2" &&
        (await text(home, "#again")) === againWord + " 42",
      (await text(home, "#info")) + " / " + (await text(home, "#again")),
    );
    calls = await click(home, "#again");
    check(
      "a handler on the new route sends one request",
      calls === "Other",
      calls,
    );
    check(
      "the handler result is shown",
      (await text(home, "#again")) === againWord + " 82",
      await text(home, "#again"),
    );

    calls = await click(home, "#to-home");
    check("going back loads fresh state", calls === "Next", calls);
    const back = await number(home);
    check(
      "the old value is not replayed",
      back > before,
      before + " -> " + back,
    );
    check(
      "no errors or warnings",
      home.seen.logs.length === 0,
      home.seen.logs.join(" | "),
    );

    const other = await open(browser, server.base + "/other");
    check(
      "a direct load of a route calls nothing",
      other.seen.calls.length === 0,
      other.seen.calls.join(","),
    );
    check(
      "the server rendered route state is kept",
      (await text(other, "#info")) === "BOX 3 2" &&
        (await text(other, "#again")) === againWord + " 42",
      (await text(other, "#info")) + " / " + (await text(other, "#again")),
    );
    calls = await click(other, "#add");
    check(
      "the hydrated list can be changed",
      calls === "" && (await text(other, "#info")) === "BOX 4 3",
      calls + " / " + (await text(other, "#info")),
    );
    calls = await click(other, "#again");
    check(
      "a handler after hydration sends one request",
      calls === "Other",
      calls,
    );
    check(
      "no errors or warnings after hydration",
      other.seen.logs.length === 0,
      other.seen.logs.join(" | "),
    );
  } finally {
    if (browser) await browser.close();
    server.child.kill();
  }
  console.log(failures + " check(s) failed");
  process.exit(failures ? 1 : 0);
})().catch((e) => {
  console.error(e);
  process.exit(1);
});
