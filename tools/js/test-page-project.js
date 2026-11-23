const fs = require("fs");
const os = require("os");
const path = require("path");
const http = require("http");
const { execFileSync } = require("child_process");
const { chromium } = require("playwright-core");

const salam = process.argv[2];
const name = process.argv[3];
const langs = (process.argv[4] || "en,fa")
  .split(",")
  .filter((l) =>
    require("fs").existsSync(
      require("path").join(
        __dirname,
        "..",
        "..",
        "tests",
        l,
        "page",
        "projects",
        name || "",
        "main.salam",
      ),
    ),
  );
if (!salam || !name) {
  console.error("usage: node test-page-project.js SALAM NAME [en,fa]");
  console.error("NAME is a folder of tests/LANG/page/projects/");
  console.error("the steps live in tools/js/page-projects/NAME.js");
  process.exit(2);
}

function browserPath() {
  const list = [
    process.env.CHROME,
    "/usr/bin/google-chrome",
    "/usr/bin/chromium",
    "/usr/bin/chromium-browser",
    "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
  ];
  for (const p of list) {
    if (p && fs.existsSync(p)) return p;
  }
  console.error("no browser found; set CHROME to a Chrome or Chromium binary");
  process.exit(2);
}

const root = path.resolve(__dirname, "..", "..");
const steps = require(path.join(__dirname, "page-projects", name + ".js"));
let failures = 0;
let total = 0;

(async () => {
  const browser = await chromium.launch({ executablePath: browserPath() });
  for (const lang of langs) {
    const dir = fs.mkdtempSync(path.join(os.tmpdir(), "salam-proj-"));
    const html = path.join(dir, "index.html");
    execFileSync(
      salam,
      [
        "page",
        "build",
        path.join(root, "tests", lang, "page", "projects", name, "main.salam"),
        "--inline",
        "--no-minify",
        "--output=" + html,
        "--no-color",
        "--log-level=error",
        "--lang=" + lang,
      ],
      { stdio: "pipe" },
    );
    const body = fs.readFileSync(html);
    const server = http.createServer((req, res) => {
      res.setHeader("content-type", "text/html; charset=utf-8");
      res.end(body);
    });
    await new Promise((r) => server.listen(0, "127.0.0.1", r));
    const base = "http://127.0.0.1:" + server.address().port;
    const hash =
      /name="salam-router" content="hash"|__salam_rt_hash = true/.test(
        body.toString(),
      );
    const ctx = await browser.newContext();
    const page = await ctx.newPage();
    const errors = [];
    page.on("pageerror", (e) => errors.push(e.message));
    page.on("console", (m) => {
      if (m.type() === "error" && !/Failed to load resource/.test(m.text()))
        errors.push(m.text());
    });
    const t = {
      lang,
      page,
      check(label, ok, detail) {
        total++;
        if (ok) {
          console.log("PASS " + lang + " " + name + ": " + label);
        } else {
          failures++;
          console.log(
            "FAIL " +
              lang +
              " " +
              name +
              ": " +
              label +
              (detail !== undefined ? " (" + detail + ")" : ""),
          );
        }
      },
      async goto(p) {
        await page.goto(hash ? base + "/#" + p : base + p);
        await page.waitForTimeout(60);
      },
      async where() {
        return page.evaluate(
          (h) => (h ? location.hash.slice(1) : location.pathname),
          hash,
        );
      },
      async text(sel) {
        return page.$eval(sel, (e) => e.textContent.trim()).catch(() => null);
      },
      async texts(sel) {
        return page.$$eval(sel, (l) => l.map((e) => e.textContent.trim()));
      },
      async count(sel) {
        return (await page.$$(sel)).length;
      },
      async click(sel) {
        await page.click(sel);
        await page.waitForTimeout(30);
      },
      async dblclick(sel) {
        await page.dblclick(sel);
        await page.waitForTimeout(30);
      },
      async fill(sel, v) {
        await page.fill(sel, v);
        await page.waitForTimeout(30);
      },
      async press(sel, k) {
        await page.press(sel, k);
        await page.waitForTimeout(30);
      },
      async until(read, ok, ms) {
        const stop = Date.now() + (ms || 3000);
        for (;;) {
          const v = await read();
          if (ok(v) || Date.now() > stop) return v;
          await new Promise((r) => setTimeout(r, 25));
        }
      },
      async sleep(ms) {
        await page.waitForTimeout(ms);
      },
      errors,
    };
    try {
      await steps(t);
      t.check(
        "no script errors",
        errors.length === 0,
        [...new Set(errors)].slice(0, 3).join(" | "),
      );
    } catch (e) {
      failures++;
      total++;
      console.log("FAIL " + lang + " " + name + ": " + (e && e.message));
    }
    await ctx.close();
    server.close();
    fs.rmSync(dir, { recursive: true, force: true });
  }
  await browser.close();
  console.log(
    failures === 0
      ? name + ": " + total + " checks ok"
      : name + ": " + failures + " check(s) failed",
  );
  process.exit(failures === 0 ? 0 : 1);
})();
