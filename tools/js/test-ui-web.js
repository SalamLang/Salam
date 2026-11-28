const fs = require("node:fs");
const path = require("node:path");
const { chromium } = require("playwright-core");

const [html, source] = process.argv.slice(2);
if (!html || !source) {
  console.error("usage: node test-ui-web.js <page.html> <test.salam>");
  process.exit(2);
}

const base = source.replace(/\.salam$/, "");
const steps = fs
  .readFileSync(`${base}.script`, "utf8")
  .split("\n")
  .map((s) => s.trim())
  .filter(Boolean);
const web = `${base}.web.out`;
const expected = fs
  .readFileSync(fs.existsSync(web) ? web : `${base}.out`, "utf8")
  .replace(/\n+$/, "");

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

(async () => {
  const browser = await chromium.launch({ executablePath: browserPath() });
  const page = await browser.newPage({
    viewport: { width: 1024, height: 768 },
  });
  page.setDefaultTimeout(5000);
  const lines = [];
  const errors = [];
  page.on("console", (m) => lines.push(m.text()));
  page.on("pageerror", (e) => errors.push(e.message));
  await page.goto(`file://${path.resolve(html)}`);
  const settle = () =>
    page.evaluate(
      () => new Promise((r) => requestAnimationFrame(() => setTimeout(r, 0))),
    );
  await settle();
  for (const step of steps) {
    const cut = step.indexOf(" ");
    const cmd = cut < 0 ? step : step.slice(0, cut);
    const arg = cut < 0 ? "" : step.slice(cut + 1);
    if (cmd === "quit") break;
    if (cmd === "click") await page.click(arg);
    else if (cmd === "type") await page.keyboard.type(arg);
    else if (cmd === "key") await page.keyboard.press(arg);
    else throw new Error(`unsupported script step: ${step}`);
    await settle();
  }
  await browser.close();
  const got = lines.join("\n").replace(/\n+$/, "");
  if (errors.length > 0 || got !== expected) {
    console.error(`FAIL ${source}`);
    for (const e of errors) console.error(`page error: ${e}`);
    console.error(`--- expected\n${expected}\n--- got\n${got}`);
    process.exit(1);
  }
  console.log(`ok ${source}`);
})().catch((e) => {
  console.error(`FAIL ${source}\n${e.message}`);
  process.exit(1);
});
