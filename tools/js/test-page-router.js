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
  const until = async (read, ok) => {
    const stop = Date.now() + 3000;
    for (;;) {
      const v = await read();
      if (ok(v) || Date.now() > stop) return v;
      await new Promise((r) => setTimeout(r, 20));
    }
  };
  const h1Is = (want) =>
    until(
      () => text("h1"),
      (v) => v === want,
    );
  const pIs = (want) =>
    until(
      () => text("p"),
      (v) => v === want,
    );
  const whereIs = (want) => until(where, (v) => v === want);

  await page.goto(url("/users/2"));
  let h = await h1Is("User 2");
  check("no script errors on load", errors.length === 0, errors.join(" | "));
  check("a deep link opens its route", h === "User 2", h);

  await page.click("#next");
  h = await h1Is("User 3");
  check("go() shows the next route", h === "User 3", h);
  let w = await whereIs("/users/3");
  check("go() writes the address", w === "/users/3", w);

  await page.goBack();
  h = await h1Is("User 2");
  check("back returns to the previous route", h === "User 2", h);

  await page.evaluate(() => {
    window.__salamRouterMark = 1;
  });
  await page.click("nav a >> nth=0");
  h = await h1Is("Home");
  check("a link opens its route", h === "Home", h);
  check(
    "a link keeps the document (no reload)",
    await page.evaluate(() => window.__salamRouterMark === 1),
  );
  const title = await until(
    () => page.title(),
    (v) => v === "Home",
  );
  check("a route title sets the document title", title === "Home", title);
  const cur = await until(
    () => page.$eval("nav a >> nth=0", (e) => e.getAttribute("aria-current")),
    (v) => v === "page",
  );
  check("the link to the current route is marked", cur === "page", cur);

  if (await page.$("#plain")) {
    await page.evaluate(() => {
      window.__salamRouterMark = 2;
    });
    await page.click("#plain");
    h = await h1Is("User 7");
    check(
      "a url link loads the page from the server",
      h === "User 7" &&
        (await page.evaluate(() => window.__salamRouterMark !== 2)),
      h,
    );
  }

  await page.click("nav a >> nth=2");
  w = await whereIs("/login");
  check("a guard redirects", w === "/login", w);
  check("the redirect target shows", !!(await page.$("#login")));

  await page.click("#login");
  const admin = await pIs("Admin area");
  check(
    "replace() after sign in opens the guarded route",
    admin === "Admin area",
    admin,
  );
  w = await whereIs("/admin");
  check("replace() writes the address", w === "/admin", w);

  await page.click("#signout");
  w = await whereIs("/login");
  check("signing out leaves the guarded route", w === "/login", w);
  check("the sign in screen shows again", !!(await page.$("#login")));

  if (mode === "history") {
    await page.goto(url("/search?q=salam"));
    h = await h1Is("Home");
    check("a guard that reads the query shows its route", h === "Home", h);
    await page.goto(url("/search?q=blocked"));
    w = await whereIs("/login");
    check("a guard that reads the query redirects", w === "/login", w);
  }

  await page.goto(url("/users/abc"));
  let p = await pIs("Nothing at /users/abc");
  check(
    "a parameter that is not an int falls to *",
    p === "Nothing at /users/abc",
    p,
  );

  await page.goto(url("/users/99999999999"));
  p = await pIs("Nothing at /users/99999999999");
  check(
    "a parameter too big for an int falls to *",
    p === "Nothing at /users/99999999999",
    p,
  );

  await page.goto(url("/no/such/page"));
  p = await pIs("Nothing at /no/such/page");
  check(
    "an unknown path shows the * route",
    p === "Nothing at /no/such/page",
    p,
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
