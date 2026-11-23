const fs = require("fs");
const http = require("http");
const path = require("path");
const { chromium } = require("playwright-core");

const htmlFile = process.argv[2];
const scenarioFile = process.argv[3];
const mode = process.argv[4] || "hash";
if (!htmlFile || !scenarioFile) {
  console.error(
    "usage: node test-page-project.js page.html scenario.js [hash|history]",
  );
  console.error("page.html is a project built with salam page build --inline");
  console.error("scenario.js exports async function (t) that drives the page");
  process.exit(2);
}

function browserPath() {
  const list = [
    process.env.CHROME,
    "/usr/bin/google-chrome",
    "/usr/bin/chromium",
    "/usr/bin/chromium-browser",
    "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
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

function serve(html) {
  return new Promise((resolve) => {
    const server = http.createServer((req, res) => {
      res.setHeader("content-type", "text/html; charset=utf-8");
      res.end(html);
    });
    server.listen(0, "127.0.0.1", () => resolve(server));
  });
}

(async () => {
  const html = fs.readFileSync(htmlFile);
  const server = await serve(html);
  const base = "http://127.0.0.1:" + server.address().port;
  const browser = await chromium.launch({ executablePath: browserPath() });
  const page = await browser.newPage();
  const errors = [];
  page.on("pageerror", (e) => errors.push(e.message));
  page.on("console", (m) => {
    if (m.type() === "error" && !/Failed to load resource/.test(m.text()))
      errors.push(m.text());
  });
  const until = async (read, ok) => {
    const stop = Date.now() + 3000;
    for (;;) {
      const v = await read();
      if (ok(v) || Date.now() > stop) return v;
      await new Promise((r) => setTimeout(r, 20));
    }
  };
  const t = {
    page,
    mode,
    errors,
    goto: async (p) => {
      await page.goto(mode === "hash" ? base + "/#" + p : base + p);
      await page.waitForTimeout(60);
    },
    where: () =>
      page.evaluate(
        (m) => (m === "hash" ? location.hash.slice(1) : location.pathname),
        mode,
      ),
    text: (sel) => page.$eval(sel, (e) => e.textContent).catch(() => null),
    texts: (sel) =>
      page.$$eval(sel, (l) => l.map((e) => e.textContent)).catch(() => []),
    count: (sel) => page.$$(sel).then((l) => l.length),
    attr: (sel, name) =>
      page.$eval(sel, (e, n) => e.getAttribute(n), name).catch(() => null),
    value: (sel) => page.$eval(sel, (e) => e.value).catch(() => null),
    click: async (sel) => {
      await page.click(sel);
      await page.waitForTimeout(40);
    },
    fill: async (sel, v) => {
      await page.fill(sel, v);
      await page.waitForTimeout(40);
    },
    check: async (sel) => {
      await page.check(sel);
      await page.waitForTimeout(40);
    },
    uncheck: async (sel) => {
      await page.uncheck(sel);
      await page.waitForTimeout(40);
    },
    waitText: (sel, want) =>
      until(
        () => t.text(sel),
        (v) => v !== null && v.includes(want),
      ),
    waitCount: (sel, want) =>
      until(
        () => t.count(sel),
        (v) => v === want,
      ),
    waitPath: (want) =>
      until(
        () => t.where(),
        (v) => v === want,
      ),
    ok: check,
    eq: (name, got, want) =>
      check(
        name,
        got === want,
        "got " + JSON.stringify(got) + ", want " + JSON.stringify(want),
      ),
    has: (name, got, part) =>
      check(
        name,
        typeof got === "string" && got.includes(part),
        "got " +
          JSON.stringify(got) +
          ", want it to contain " +
          JSON.stringify(part),
      ),
    lacks: (name, got, part) =>
      check(
        name,
        typeof got === "string" && !got.includes(part),
        "got " +
          JSON.stringify(got) +
          ", want it without " +
          JSON.stringify(part),
      ),
  };
  try {
    await require(path.resolve(scenarioFile))(t);
    check("no script errors", errors.length === 0, errors.join(" | "));
  } catch (e) {
    check("scenario ran to the end", false, e && e.message);
  }
  await browser.close();
  server.close();
  console.log(failures === 0 ? "ALL PASS" : failures + " FAILED");
  process.exit(failures === 0 ? 0 : 1);
})();
