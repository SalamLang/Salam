const fs = require("fs");
const http = require("http");
const path = require("path");
const { chromium } = require("playwright-core");

const file = process.argv[2];
if (!file) {
  console.error("usage: node test-page-form.js page.html");
  console.error(
    "page.html is tests/en/page/components/form_submit.salam built with salam build --platform=web",
  );
  console.error(
    "the browser is $CHROME, or Google Chrome or Chromium from the usual places",
  );
  process.exit(2);
}

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

function serve(html) {
  const frameHost =
    '<!doctype html><iframe sandbox="allow-scripts allow-forms" src="/" style="width:700px;height:700px"></iframe>';
  const server = http.createServer((req, res) => {
    const name = req.url.split("?")[0];
    let body = "<!doctype html><p>submitted</p>";
    if (name === "/") body = html;
    if (name === "/frame") body = frameHost;
    res.writeHead(200, { "content-type": "text/html; charset=utf-8" });
    res.end(body);
  });
  return new Promise((resolve) => {
    server.listen(0, "127.0.0.1", () => resolve(server));
  });
}

async function run(page, frame, label) {
  const text = (sel) =>
    frame.$eval(sel, (e) => e.textContent).catch(() => null);
  const stayed = () =>
    frame.evaluate(() => window.marker === 1).catch(() => false);
  const expect = async (name, sel, want) => {
    const got = await text(sel);
    check(label + name, got === want, got);
  };
  await frame.waitForSelector("#a-send");
  await frame.evaluate(() => {
    window.marker = 1;
  });

  await frame.click("#a-send");
  await page.waitForTimeout(300);
  await expect("a submit button runs the handler", "#a-out", "typed handled");
  await expect("the other instance is untouched", "#b-out", "typed ");
  check(label + "the page does not reload", await stayed());

  await frame.press("#b-form input", "Enter");
  await page.waitForTimeout(300);
  await expect("Enter in a field runs the handler", "#b-out", "typed handled");
  check(label + "Enter does not reload", await stayed());

  await frame.click("#add");
  await frame.click("#y-send");
  await page.waitForTimeout(300);
  await expect(
    "a form in a list row runs its own handler",
    "#y-out",
    "typed handled",
  );
  await expect("the other row is untouched", "#x-out", "typed ");
  check(label + "a list row does not reload", await stayed());

  await frame.click("#named-send");
  await page.waitForTimeout(300);
  check(label + "a named handler does not reload", await stayed());

  await frame.click("#native-send");
  await page.waitForTimeout(700);
  check(label + "a form without a handler still submits", !(await stayed()));
}

async function scenario(url, framed, errors) {
  const browser = await chromium.launch({ executablePath: browserPath() });
  const label = framed ? "sandboxed frame: " : "";
  try {
    const page = await browser.newPage();
    page.on("pageerror", (e) => errors.push(e.message));
    page.on("crash", () => errors.push(label + "the page crashed"));
    await page.goto(url);
    let target = page.mainFrame();
    if (framed) {
      const handle = await page.waitForSelector("iframe");
      target = await handle.contentFrame();
    }
    await run(page, target, label);
  } finally {
    await browser.close();
  }
}

(async () => {
  const server = await serve(fs.readFileSync(path.resolve(file), "utf8"));
  const base = "http://127.0.0.1:" + server.address().port;
  const errors = [];
  try {
    await scenario(base + "/", false, errors);
    await scenario(base + "/frame", true, errors);
  } catch (e) {
    check("the browser run finishes", false, e.message);
  }
  server.close();

  check("no script errors", errors.length === 0, errors.join(" | "));
  console.log(failures === 0 ? "page form ok" : failures + " check(s) failed");
  process.exit(failures === 0 ? 0 : 1);
})();
