const fs = require("fs");
const { chromium } = require("playwright-core");

const start = process.argv[2];
const mode = process.argv[3] || "history";
if (!start) {
  console.error("usage: node test-page-router.js URL [history|hash]");
  console.error(
    "URL is the served tests/en/page/pages/routes_history.salam (salam serve, history mode)",
  );
  console.error("or file:///.../routes_hash.html (hash mode)");
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
    if (m.type() === "error" && !/Failed to load resource/.test(m.text()))
      errors.push(m.text());
  });
  const base = start.replace(/\/$/, "");
  const url = (p) => (mode === "hash" ? start + "#" + p : base + p);
  const where = () =>
    page.evaluate(
      (m) => (m === "hash" ? location.hash.slice(1) : location.pathname),
      mode,
    );
  const text = (sel) => page.$eval(sel, (e) => e.textContent).catch(() => null);
  const settle = () => page.waitForTimeout(60);

  await page.goto(url("/users/2"));
  await settle();
  check("no script errors on load", errors.length === 0, errors.join(" | "));
  check(
    "a deep link opens its route",
    (await text("h1")) === "User 2",
    await text("h1"),
  );

  await page.click("#next");
  await settle();
  check(
    "go() shows the next route",
    (await text("h1")) === "User 3",
    await text("h1"),
  );
  check(
    "go() writes the address",
    (await where()) === "/users/3",
    await where(),
  );

  await page.goBack();
  await settle();
  check(
    "back returns to the previous route",
    (await text("h1")) === "User 2",
    await text("h1"),
  );

  await page.click("nav a >> nth=0");
  await settle();
  check(
    "a link opens its route without a reload",
    (await text("h1")) === "Home",
    await text("h1"),
  );
  check(
    "a route title sets the document title",
    (await page.title()) === "Home",
    await page.title(),
  );
  check(
    "the link to the current route is marked",
    (await page.$eval("nav a >> nth=0", (e) =>
      e.getAttribute("aria-current"),
    )) === "page",
  );

  await page.click("nav a >> nth=2");
  await settle();
  check("a guard redirects", (await where()) === "/login", await where());
  check("the redirect target shows", !!(await page.$("#login")));

  await page.click("#login");
  await settle();
  check(
    "replace() after sign in opens the guarded route",
    (await text("p")) === "Admin area",
    await text("p"),
  );
  check(
    "replace() writes the address",
    (await where()) === "/admin",
    await where(),
  );

  await page.goto(url("/users/abc"));
  await settle();
  check(
    "a parameter that is not an int falls to *",
    (await text("p")) === "Nothing at /users/abc",
    await text("p"),
  );

  await page.goto(url("/no/such/page"));
  await settle();
  check(
    "an unknown path shows the * route",
    (await text("p")) === "Nothing at /no/such/page",
    await text("p"),
  );

  check(
    "no script errors after navigation",
    errors.length === 0,
    errors.join(" | "),
  );
  await browser.close();
  console.log(
    failures === 0 ? "page router ok" : failures + " check(s) failed",
  );
  process.exit(failures === 0 ? 0 : 1);
})();
