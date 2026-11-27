const fs = require("fs");
const http = require("http");
const path = require("path");
const { spawn } = require("child_process");
const { chromium } = require("playwright-core");

const dist = process.argv[2];
if (!dist) {
  console.error("usage: node test-ssr-dashboard.js path/to/dist");
  process.exit(2);
}
const fa = process.env.SALAM_TEST_LANG === "fa";
const shipped = fa ? "ارسال‌شده ثبت شد" : "Marked as shipped";
const order = fa
  ? { شماره: 7, مشتری: "Sara", مبلغ: 12, وضعیت: "pending" }
  : { id: 7, customer: "Sara", total: 12, status: "pending" };

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

const shipCalls = [];
const api = http.createServer((req, res) => {
  const url = req.url.split("?")[0];
  let value = {};
  if (url === "/orders/7") value = order;
  if (url === "/orders/7/ship") shipCalls.push(req.method);
  res.setHeader("Content-Type", "application/json");
  res.end(JSON.stringify(value));
});

function startServer(apiPort) {
  const child = spawn(path.join(dist, "server"), [], {
    env: {
      ...process.env,
      PORT: "0",
      HOST: "127.0.0.1",
      DESK_API: "http://127.0.0.1:" + apiPort,
      DESK_TOKEN: "secret",
    },
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
        resolve({ child, base: m[1] });
      }
    });
  });
}

(async () => {
  await new Promise((r) => api.listen(0, "127.0.0.1", r));
  const bundle = fs
    .readdirSync(path.join(dist, "public"))
    .filter((f) => f.endsWith(".js"))
    .map((f) => fs.readFileSync(path.join(dist, "public", f), "utf8"))
    .join("");
  check("the bundle has no thread primitives", !bundle.includes("pthread"));
  const server = await startServer(api.address().port);
  let browser;
  try {
    browser = await chromium.launch({ executablePath: browserPath() });
    const page = await browser.newPage();
    const errors = [];
    page.on("pageerror", (e) => errors.push(e.message));
    await page.goto(server.base + "/order/7");
    await page.waitForSelector("#ship-btn");
    check("the bundle loads", errors.length === 0, errors.join(" | "));
    await page.click("#ship-btn");
    await page.waitForTimeout(500);
    const msg = (await page.textContent("#msg")) || "";
    check("a click handler runs", msg === shipped, msg);
    check(
      "the server function reached the API",
      shipCalls.length === 1 && shipCalls[0] === "POST",
      shipCalls.join(","),
    );
    check("no errors after the click", errors.length === 0, errors.join(" | "));
  } finally {
    if (browser) await browser.close();
    server.child.kill();
    api.close();
  }
  console.log(failures + " check(s) failed");
  process.exit(failures ? 1 : 0);
})().catch((e) => {
  console.error(e);
  process.exit(1);
});
