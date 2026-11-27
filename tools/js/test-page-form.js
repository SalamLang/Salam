const fs = require("fs");
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

async function run(page, frame, label) {
  const text = (sel) => frame.$eval(sel, (e) => e.textContent).catch(() => null);
  const stayed = () =>
    frame.evaluate(() => window.marker === 1).catch(() => false);
  await frame.waitForSelector("#a-send");
  await frame.evaluate(() => {
    window.marker = 1;
  });

  await frame.click("#a-send");
  await page.waitForTimeout(300);
  check(label + "a submit button runs the handler", (await text("#a-out")) === "typed handled", await text("#a-out"));
  check(label + "the other instance is untouched", (await text("#b-out")) === "typed ", await text("#b-out"));
  check(label + "the page does not reload", await stayed());

  await frame.press("#b-form input", "Enter");
  await page.waitForTimeout(300);
  check(label + "Enter in a field runs the handler", (await text("#b-out")) === "typed handled", await text("#b-out"));
  check(label + "Enter does not reload", await stayed());

  await frame.click("#add");
  await frame.click("#y-send");
  await page.waitForTimeout(300);
  check(label + "a form in a list row runs its own handler", (await text("#y-out")) === "typed handled", await text("#y-out"));
  check(label + "the other row is untouched", (await text("#x-out")) === "typed ", await text("#x-out"));
  check(label + "a list row does not reload", await stayed());

  await frame.click("#named-send");
  await page.waitForTimeout(300);
  check(label + "a named handler does not reload", await stayed());

  await frame.click("#native-send");
  await page.waitForTimeout(700);
  check(label + "a form without a handler still submits", !(await stayed()));
}

(async () => {
  const browser = await chromium.launch({ executablePath: browserPath() });
  const errors = [];
  const open = async () => {
    const page = await browser.newPage();
    page.on("pageerror", (e) => errors.push(e.message));
    return page;
  };

  const url = process.argv[2].startsWith("http")
    ? process.argv[2]
    : "file://" + path.resolve(file);
  const page = await open();
  await page.goto(url);
  await run(page, page.mainFrame(), "");

  if (!url.startsWith("http")) {
    const framed = await open();
    await framed.setContent(
      '<iframe sandbox="allow-scripts allow-forms" style="width:700px;height:700px"></iframe>',
    );
    await framed.$eval(
      "iframe",
      (el, html) => {
        el.srcdoc = html;
      },
      fs.readFileSync(path.resolve(file), "utf8"),
    );
    const handle = await framed.waitForSelector("iframe");
    await framed.waitForTimeout(300);
    await run(framed, await handle.contentFrame(), "sandboxed frame: ");
  }

  check("no script errors", errors.length === 0, errors.join(" | "));
  await browser.close();
  console.log(failures === 0 ? "page form ok" : failures + " check(s) failed");
  process.exit(failures === 0 ? 0 : 1);
})();
