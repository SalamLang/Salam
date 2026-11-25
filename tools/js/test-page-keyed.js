const fs = require("fs");
const path = require("path");
const { chromium } = require("playwright-core");

const file = process.argv[2];
if (!file) {
  console.error("usage: node test-page-keyed.js page.html");
  console.error(
    "page.html is tests/en/page/components/keyed_moves.salam built with salam build --platform=web",
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

(async () => {
  const browser = await chromium.launch({ executablePath: browserPath() });
  const page = await browser.newPage();
  const errors = [];
  page.on("pageerror", (e) => errors.push(e.message));
  page.on("console", (m) => {
    if (m.type() === "error") errors.push(m.text());
  });
  await page.goto("file://" + path.resolve(file));
  await page.waitForTimeout(100);
  const order = () =>
    page.$$eval("#rows li", (l) =>
      l.map((e) => e.getAttribute("data-key")).join(","),
    );
  const mark = () =>
    page.$$eval("#rows li", (l) =>
      l.forEach((e) => {
        e.__k = e.getAttribute("data-key");
      }),
    );
  const kept = () =>
    page.$$eval("#rows li", (l) =>
      l.every(
        (e) => e.__k === undefined || e.__k === e.getAttribute("data-key"),
      ),
    );
  const marked = () =>
    page.$$eval("#rows li", (l) => l.filter((e) => e.__k !== undefined).length);

  await page.click("#fill");
  check(
    "fill renders five rows",
    (await order()) === "a,b,c,d,e",
    await order(),
  );
  await mark();
  await page.fill("#in-b", "typed b");
  await page.click("#fill");
  check(
    "a render with no change keeps every row",
    (await marked()) === 5,
    await marked(),
  );

  await page.click("#swap");
  check("swap moves the ends", (await order()) === "e,b,c,d,a", await order());
  check(
    "swap keeps each row element",
    (await kept()) && (await marked()) === 5,
  );

  await page.click("#drop");
  check(
    "drop removes the second row",
    (await order()) === "e,c,d,a",
    await order(),
  );
  check(
    "drop keeps the other row elements",
    (await kept()) && (await marked()) === 4,
  );

  await page.click("#rev");
  check("reverse flips the rows", (await order()) === "a,d,c,e", await order());
  check(
    "reverse keeps each row element",
    (await kept()) && (await marked()) === 4,
  );

  await page.click("#add");
  check("add appends a row", (await order()) === "a,d,c,e,f", await order());
  check(
    "add keeps the old row elements",
    (await kept()) && (await marked()) === 4,
  );

  await page.click("#fill");
  check(
    "fill again restores the rows",
    (await order()) === "a,b,c,d,e",
    await order(),
  );
  check(
    "an untouched input stays empty",
    (await page.$eval("#in-c", (e) => e.value)) === "",
  );

  await page.click("#fill");
  await page.fill("#in-d", "typed d");
  await page.click("#swap");
  await page.click("#rev");
  check(
    "a typed value moves with its row",
    (await page.$eval("#in-d", (e) => e.value)) === "typed d",
    await page.$eval("#in-d", (e) => e.value),
  );

  check("no script errors", errors.length === 0, errors.join(" | "));
  await browser.close();
  console.log(
    failures === 0 ? "keyed moves ok" : failures + " check(s) failed",
  );
  process.exit(failures === 0 ? 0 : 1);
})();
