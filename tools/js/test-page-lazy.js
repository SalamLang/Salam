const fs = require("fs");
const http = require("http");
const path = require("path");
const { chromium } = require("playwright-core");

const root = process.argv[2];
const lang = process.argv[3] || "en";
const prefix = (process.argv[4] || "").replace(/\/$/, "");
if (!root) {
  console.error("usage: node test-page-lazy.js PUBLIC_DIR [en|fa] [BASE]");
  console.error(
    "PUBLIC_DIR is the build of tests/<lang>/ssr/lazy, served the way a static host does,",
  );
  console.error("or the http URL of its generated server");
  process.exit(2);
}

const words = {
  en: {
    guide: "/guide-é",
    home: "home page words",
    about: "about page words & <signs>",
    intro: "intro page words",
    setup: "setup page words",
    menu: "docs menu words",
    guideText: "guide page words",
    counter: "counter page words",
    add: "Add ",
    user: "User 7",
    missing: "missing page words",
    jump: "jump page words",
    title: {
      home: "Home",
      about: "About",
      intro: "Intro",
      counter: "Counter",
    },
  },
  fa: {
    guide: "/راهنما",
    home: "واژه‌های خانه",
    about: "واژه‌های درباره & <نشانه>",
    intro: "واژه‌های آغاز",
    setup: "واژه‌های نصب",
    menu: "فهرست مستندات",
    guideText: "واژه‌های راهنما",
    counter: "واژه‌های شمارنده",
    add: "افزودن ",
    user: "کاربر 7",
    missing: "واژه‌های گمشده",
    jump: "واژه‌های پرش",
    title: {
      home: "خانه",
      about: "درباره",
      intro: "آغاز",
      counter: "شمارنده",
    },
  },
}[lang];

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

const types = {
  ".html": "text/html; charset=utf-8",
  ".js": "text/javascript",
  ".css": "text/css",
};
const requests = [];
let broken = "";
let delay = {};

function forward(req, res, p) {
  const answer = (up) => {
    res.writeHead(up.statusCode, up.headers);
    up.pipe(res);
  };
  const to = root.replace(/\/$/, "") + req.url;
  setTimeout(() => http.get(to, answer), delay[p] || 0);
}

function serve(req, res) {
  let p = decodeURIComponent(req.url.split("?")[0]);
  requests.push(p);
  if (root.startsWith("http://")) {
    if (broken && p === broken) {
      res.writeHead(404);
      return res.end("gone");
    }
    return forward(req, res, p);
  }
  const send = (code, file) => {
    res.writeHead(code, {
      "content-type": types[path.extname(file)] || "text/plain",
    });
    res.end(fs.readFileSync(file));
  };
  const lost = () => send(404, path.join(root, "404.html"));
  if (prefix) {
    if (p !== prefix && !p.startsWith(prefix + "/")) return lost();
    p = p.slice(prefix.length) || "/";
  }
  if (broken && p === broken) return lost();
  let file = path.join(root, p);
  if (fs.existsSync(file) && fs.statSync(file).isDirectory()) {
    if (!p.endsWith("/")) {
      res.writeHead(301, { location: encodeURI(prefix + p + "/") });
      return res.end();
    }
    file = path.join(file, "index.html");
  }
  if (!fs.existsSync(file)) return lost();
  const wait = delay[p] || 0;
  setTimeout(() => send(200, file), wait);
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
  const server = http.createServer(serve);
  await new Promise((r) => server.listen(0, "127.0.0.1", r));
  const origin = "http://127.0.0.1:" + server.address().port + prefix;
  const browser = await chromium.launch({ executablePath: browserPath() });
  const page = await browser.newPage();
  const errors = [];
  page.on("pageerror", (e) => errors.push(e.message));
  page.on("console", (m) => {
    if (m.type() === "error" && !/Failed to load resource/.test(m.text()))
      errors.push(m.text());
  });
  const body = () => page.evaluate(() => document.body.innerText);
  const where = () =>
    page.evaluate(
      (b) => decodeURIComponent(location.pathname).slice(b.length),
      prefix,
    );
  const until = async (read, ok) => {
    const stop = Date.now() + 4000;
    for (;;) {
      const v = await read().catch(() => "");
      if (ok(v) || Date.now() > stop) return v;
      await new Promise((r) => setTimeout(r, 20));
    }
  };
  const shows = async (name, want, gone) => {
    const v = await until(
      body,
      (t) => t.includes(want) && (!gone || !t.includes(gone)),
    );
    check(name, v.includes(want) && (!gone || !v.includes(gone)), v);
  };
  const title = async (name, want) => {
    const v = await until(
      () => page.title(),
      (t) => t === want,
    );
    check(name, v === want, v);
  };
  const marked = () => page.evaluate(() => window.__stay === 1);
  const click = (id) => page.click("#" + id);

  await page.goto(origin + "/");
  await shows("the home page shows its text", words.home);
  await page.evaluate(() => {
    window.__stay = 1;
  });
  requests.length = 0;
  await click("to-about");
  await shows(
    "a link loads a page that is not in the script",
    words.about,
    words.home,
  );
  await title("the title follows the page", words.title.about);
  await click("to-intro");
  await shows(
    "a page in a folder with a layout loads",
    words.intro,
    words.about,
  );
  await shows("the layout comes with it", words.menu);
  await click("to-setup");
  await shows("a second page of the layout loads", words.setup, words.intro);
  await click("to-guide");
  await shows(
    "a page with a route setting loads",
    words.guideText,
    words.setup,
  );
  check("the address follows", (await where()) === words.guide, await where());
  await click("to-counter");
  await shows(
    "a page with state is drawn by the script",
    words.counter,
    words.guideText,
  );
  await click("add");
  await shows("its handler runs", words.add + "1");
  await click("to-user");
  await shows(
    "a page with a parameter is drawn by the script",
    words.user,
    words.counter,
  );
  await click("to-jump");
  await shows("a page with only a handler is loaded", words.jump, words.user);
  await click("go-about");
  await shows("its handler runs after loading", words.about, words.jump);
  await click("to-missing");
  await shows(
    "an unknown path shows the fallback page",
    words.missing,
    words.about,
  );
  await click("to-home");
  await shows("the home page comes back", words.home, words.missing);
  check("navigation never reloaded the document", await marked());

  requests.length = 0;
  const back = async (name, want, gone) => {
    await page.goBack();
    await shows(name, want, gone);
  };
  await back("back shows the fallback again", words.missing, words.home);
  await back(
    "back goes to the page the handler opened",
    words.about,
    words.missing,
  );
  await back("back reaches the handler page", words.jump, words.about);
  await back("back reaches the parameter page", words.user, words.jump);
  await back("back reaches the state page", words.counter, words.user);
  await back("back reaches a loaded page", words.guideText, words.counter);
  await back("back reaches the layout page", words.setup, words.guideText);
  await back("back reaches the earlier layout page", words.intro, words.setup);
  await back("back reaches the first loaded page", words.about, words.intro);
  await title("back restores the title", words.title.about);
  await page.goForward();
  await shows("forward shows the next page", words.intro, words.about);
  check(
    "visited pages are not fetched again",
    !requests.some((r) => /about|docs/.test(r)),
    requests.join(" "),
  );
  check("history never reloaded the document", await marked());

  requests.length = 0;
  await page.goto(origin + "/about/");
  await shows("a direct visit shows the page", words.about);
  await page.waitForTimeout(300);
  check(
    "a direct visit fetches nothing more",
    requests.filter((r) => r.includes("about")).length ===
      (root.startsWith("http://") ? 2 : 1),
    requests.join(" "),
  );
  await click("to-home");
  await shows("leaving it works", words.home, words.about);
  requests.length = 0;
  await page.goBack();
  await shows("coming back to it works", words.about, words.home);
  check(
    "the first page is kept",
    !requests.some((r) => r.includes("about")),
    requests.join(" "),
  );

  await page.goto(origin + words.guide);
  await shows("a direct visit to a route setting path works", words.guideText);
  await page.goto(origin + "/docs/setup");
  await shows("a direct visit without the last slash works", words.setup);
  await click("to-intro");
  await shows("navigation from there works", words.intro, words.setup);
  await page.goto(origin + "/user/7");
  await shows("a deep link served by the fallback file works", words.user);
  await click("to-about");
  await shows(
    "navigation from the fallback file works",
    words.about,
    words.user,
  );

  delay = { "/docs/intro/": 700 };
  await page.goto(origin + "/");
  await click("to-intro");
  await click("to-about");
  await shows("a newer page wins", words.about);
  await page.waitForTimeout(1000);
  await shows("a slow answer does not replace it", words.about, words.intro);
  await click("to-intro");
  await shows(
    "the slow page is shown when asked for",
    words.intro,
    words.about,
  );
  delay = {};
  check("no script errors", errors.length === 0, errors.join(" | "));

  await page.goto(origin + "/");
  await page.evaluate(() => {
    window.__stay = 1;
  });
  broken = "/docs/setup/";
  requests.length = 0;
  await click("to-setup");
  await until(marked, (v) => v === false);
  check("a page that cannot be loaded reloads the document", !(await marked()));
  await page.waitForTimeout(1000);
  const tries = requests.filter((r) => r.includes("/docs/setup")).length;
  check("it does not reload for ever", tries > 0 && tries <= 4, tries);
  broken = "";

  await browser.close();
  server.close();
  process.exit(failures ? 1 : 0);
})();
