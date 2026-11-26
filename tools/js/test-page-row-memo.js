const fs = require("fs");
const path = require("path");
const { chromium } = require("playwright-core");

const file = process.argv[2];
if (!file) {
  console.error("usage: node test-page-row-memo.js page.html");
  console.error(
    "page.html is tests/en/page/components/row_memo.salam built with salam web",
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
  const cells = () =>
    page.$$eval("#cells li", (l) =>
      l.map((e) => e.textContent.replace("go", "") + ":" + e.className),
    );
  const text = async () => (await cells()).join(",");

  await page.click("#fill");
  check(
    "fill renders six rows",
    (await text()) === "a-x:off,b-x:off,c-x:off,d-x:off,e-x:off,f-x:off",
    await text(),
  );
  await page.click("#go-c");
  check(
    "choosing marks one row",
    (await text()) === "a-x:off,b-x:off,c-x:on,d-x:off,e-x:off,f-x:off",
    await text(),
  );
  await page.click("#suffix");
  check(
    "a global read by every row updates every row",
    (await text()) === "a-xy:off,b-xy:off,c-xy:on,d-xy:off,e-xy:off,f-xy:off",
    await text(),
  );
  await page.click("#dup");
  check(
    "a duplicate key renders both rows",
    (await text()) ===
      "a-xy:off,b-xy:off,b-xy:off,c-xy:on,d-xy:off,e-xy:off,f-xy:off",
    await text(),
  );
  await page.click("#suffix");
  check(
    "duplicate rows follow a global too",
    (await text()) ===
      "a-xyy:off,b-xyy:off,b-xyy:off,c-xyy:on,d-xyy:off,e-xyy:off,f-xyy:off",
    await text(),
  );
  await page.click("#fill");
  await page.click("#many");
  check(
    "removing several rows at once",
    (await text()) === "d-xyy:off,e-xyy:off,f-xyy:off",
    await text(),
  );
  await page.click("#go-f");
  check(
    "a handler works after a bulk removal",
    (await page.$eval("#chosen", (e) => e.textContent)) === "chosen f",
  );
  await page.click("#clear");
  check("clear empties the list", (await text()) === "", await text());
  await page.click("#fill");
  check(
    "the same keys come back fresh",
    (await text()) ===
      "a-xyy:off,b-xyy:off,c-xyy:off,d-xyy:off,e-xyy:off,f-xyy:on",
    await text(),
  );
  await page.click("#go-a");
  check(
    "a handler on a recreated row reads its own row",
    (await page.$eval("#chosen", (e) => e.textContent)) === "chosen a",
  );

  check("no page errors", errors.length === 0, errors.join(" | "));
  await browser.close();
  process.exit(failures ? 1 : 0);
})();
