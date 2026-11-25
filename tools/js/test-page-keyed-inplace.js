const fs = require("fs");
const path = require("path");
const { chromium } = require("playwright-core");

const file = process.argv[2];
if (!file) {
  console.error("usage: node test-page-keyed-inplace.js page.html");
  console.error(
    "page.html is tests/en/page/components/keyed_inplace.salam built with salam web",
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
    page.$$eval("#rows > li", (l) =>
      l.map((e) => e.getAttribute("data-key")).join(","),
    );
  const mark = () =>
    page.$$eval("#rows li", (l) =>
      l.forEach((e) => {
        e.__k = e.getAttribute("data-key");
      }),
    );
  const kept = () =>
    page.$$eval(
      "#rows > li",
      (l) => l.filter((e) => e.__k === e.getAttribute("data-key")).length,
    );
  const cls = () =>
    page.$$eval("#rows > li", (l) => l.map((e) => e.className).join(","));
  const words = (n) =>
    page.$$eval("#row-" + n + " li", (l) =>
      l.map((e) => e.textContent).join(","),
    );

  await page.click("#fill");
  check(
    "fill renders five rows",
    (await order()) === "a,b,c,d,e",
    await order(),
  );
  await mark();
  await page.fill("#in-b", "typed b");

  await page.click("#pick-b");
  check(
    "pick marks one row",
    (await cls()) === "off,on,off,off,off",
    await cls(),
  );
  check("pick keeps every row element", (await kept()) === 5, await kept());
  check(
    "pick keeps a typed value",
    (await page.$eval("#in-b", (e) => e.value)) === "typed b",
  );

  await page.click("#pick-d");
  check(
    "pick moves the mark",
    (await cls()) === "off,off,off,on,off",
    await cls(),
  );
  check(
    "a second pick keeps every row element",
    (await kept()) === 5,
    await kept(),
  );
  const inplace = await page.evaluate(() => window.__salam_inplace);
  check("picks patch rows in place", inplace >= 2, inplace);

  const inner = await page.$eval("#row-c li", (e) => {
    e.__w = 1;
    return e.textContent;
  });
  await page.click("#rename-c");
  check(
    "rename changes the label",
    (await page.$eval("#label-c", (e) => e.textContent)) === "c two words",
  );
  check(
    "rename grows the inner list",
    (await words("c")) === "c,two,words",
    await words("c"),
  );
  check(
    "rename keeps the inner row it had",
    inner === "c" && (await page.$eval("#row-c li", (e) => e.__w === 1)),
  );
  check("rename keeps every row element", (await kept()) === 5, await kept());

  await page.fill("#in-d", "typed d");
  await page.click("#rename-d");
  check(
    "a row with a typed value is patched",
    (await page.$eval("#label-d", (e) => e.textContent)) === "d two",
  );
  check(
    "the typed value survives the patch",
    (await page.$eval("#in-d", (e) => e.value)) === "typed d",
    await page.$eval("#in-d", (e) => e.value),
  );

  await page.click("#drop");
  check(
    "drop removes the second row",
    (await order()) === "a,c,d,e",
    await order(),
  );
  check("drop keeps the other rows", (await kept()) === 4, await kept());
  await page.click("#pick-e");
  check("pick after a drop", (await cls()) === "off,off,off,on", await cls());
  check("pick after a drop keeps the rows", (await kept()) === 4, await kept());
  check(
    "the picked handler reads its own row",
    (await page.$eval("#picked", (e) => e.textContent)) === "picked e",
  );

  const before = await page.evaluate(() => window.__salam_inplace);
  await page.fill("#in-c", "typed c");
  await page.click("#rev");
  check("reverse flips the rows", (await order()) === "e,d,c,a", await order());
  check("reverse keeps every row element", (await kept()) === 4, await kept());
  check(
    "a typed value moves with its row",
    (await page.$eval("#in-c", (e) => e.value)) === "typed c",
  );
  await page.click("#ins");
  check("insert adds a row", (await order()) === "e,x,d,c,a", await order());
  check("insert keeps the other rows", (await kept()) === 4, await kept());
  check(
    "the new row has its label",
    (await page.$eval("#label-x", (e) => e.textContent)) === "x one",
  );
  await page.click("#pick-x");
  check(
    "a new row handler works",
    (await page.$eval("#picked", (e) => e.textContent)) === "picked x",
  );
  check(
    "the new row is marked",
    (await cls()) === "off,on,off,off,off",
    await cls(),
  );
  await page.click("#drop");
  check("drop after insert", (await order()) === "e,d,c,a", await order());
  check("drop after insert keeps the rows", (await kept()) === 4, await kept());
  const moved = await page.evaluate(() => window.__salam_inplace);
  check(
    "moves and inserts skip the full render",
    moved - before >= 4,
    moved - before,
  );

  check("no page errors", errors.length === 0, errors.join(" | "));
  await browser.close();
  process.exit(failures ? 1 : 0);
})();
