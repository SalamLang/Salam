const fs = require("fs");
const os = require("os");
const path = require("path");
const { spawn } = require("child_process");
const { chromium } = require("playwright-core");

const salam = process.argv[2];
if (!salam) {
  console.error("usage: node test-serve-hmr.js /path/to/salam");
  console.error(
    "the browser is $CHROME, or Chrome or Chromium from the usual places",
  );
  process.exit(2);
}

function browserOptions() {
  const list = [
    process.env.CHROME,
    "/usr/bin/google-chrome",
    "/usr/bin/chromium",
    "/usr/bin/chromium-browser",
  ];
  for (const p of list) {
    if (p && fs.existsSync(p)) return { executablePath: p };
  }
  return {};
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

const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

async function until(fn, ms) {
  const end = Date.now() + (ms || 20000);
  for (;;) {
    let v = null;
    try {
      v = await fn();
    } catch (e) {
      v = null;
    }
    if (v) return v;
    if (Date.now() > end) return null;
    await sleep(100);
  }
}

function appSource(version, name, initial, extra) {
  const rows = [];
  for (let i = 0; i < 60; i++)
    rows.push(
      '            paragraph:\n                content = "row ' +
        i +
        '"\n            end',
    );
  return [
    "mut " + name + " := " + initial,
    "",
    "page:",
    '    title = "Hot"',
    "    box:",
    "        paragraph:",
    '            id = "label"',
    '            content = "Count " + ' + name,
    "        end",
    "        button:",
    '            id = "inc"',
    "            click = (): " + name + "++ end",
    '            content = "Add"',
    "        end",
    "        paragraph:",
    '            id = "ver"',
    '            content = "' + version + '"',
    extra || "",
    "        end",
    rows.join("\n"),
    "    end",
    "end",
    "",
  ].join("\n");
}

function start(args, cwd, port, log) {
  const out = fs.openSync(log, "a");
  const child = spawn(salam, args.concat(["--port=" + port]), {
    cwd,
    detached: true,
    stdio: ["ignore", out, out],
  });
  const exited = new Promise((resolve) => child.on("exit", resolve));
  child.stop = async () => {
    try {
      process.kill(-child.pid);
    } catch (e) {
      child.kill();
    }
    await Promise.race([exited, sleep(10000)]);
  };
  return child;
}

async function listening(port) {
  return until(async () => {
    const r = await fetch("http://127.0.0.1:" + port + "/__salam_hmr");
    return r.status === 200;
  }, 60000);
}

(async () => {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), "salam-hmr-"));
  const site = path.join(dir, "site");
  fs.mkdirSync(site);
  const src = path.join(site, "app.salam");
  fs.writeFileSync(src, appSource("v1", "count", "0"));
  const port = 20000 + Math.floor(Math.random() * 20000);
  const log = path.join(dir, "serve.log");
  let server = start(["serve", src], dir, port, log);
  let browser = null;
  try {
    browser = await chromium.launch(browserOptions());
  } catch (e) {
    await server.stop();
    throw e;
  }
  const page = await browser.newPage({ viewport: { width: 800, height: 400 } });
  const errors = [];
  page.on("pageerror", (e) => errors.push(e.message));
  try {
    check("dev server answers", !!(await listening(port)));
    await page.goto("http://127.0.0.1:" + port + "/");
    check("page renders", !!(await until(() => page.$("#label"), 10000)));
    await page.click("#inc");
    await page.click("#inc");
    check(
      "state changes in the page",
      (await page.textContent("#label")) === "Count 2",
    );
    await page.evaluate(() => {
      window.__marker = "alive";
    });

    fs.writeFileSync(src, appSource("v2", "count", "0"));
    const swapped = await until(
      async () => (await page.textContent("#ver")) === "v2",
      60000,
    );
    check("a source edit shows up", !!swapped);
    check(
      "the page was updated in place",
      (await page.evaluate(() => window.__marker)) === "alive",
    );
    check(
      "state survives the update",
      (await page.textContent("#label")) === "Count 2",
      await page.textContent("#label"),
    );
    await page.click("#inc");
    check(
      "the new code handles clicks",
      (await page.textContent("#label")) === "Count 3",
    );

    await page.evaluate(() => {
      window.__before = document.getElementById("inc");
    });
    fs.writeFileSync(
      src,
      appSource("v2", "count", "0", '            color = "rgb(200, 0, 0)"'),
    );
    const styled = await until(
      async () =>
        (await page.$eval("#ver", (e) => getComputedStyle(e).color)) ===
        "rgb(200, 0, 0)",
      60000,
    );
    check("a style edit applies", !!styled);
    check(
      "a style edit keeps the state",
      (await page.textContent("#label")) === "Count 3",
    );

    fs.writeFileSync(src, appSource("v3", "total", "5"));
    const reshaped = await until(
      async () => (await page.textContent("#ver")) === "v3",
      60000,
    );
    check("a renamed state still updates", !!reshaped);
    check(
      "a renamed state starts fresh",
      (await page.textContent("#label")) === "Count 5",
      await page.textContent("#label"),
    );

    fs.writeFileSync(src, "page:\n    title = \n");
    const overlay = await until(async () => {
      const t = await page.evaluate(() => {
        const d = [...document.documentElement.children].find(
          (e) => e.tagName === "DIV",
        );
        return d ? d.textContent : "";
      });
      return t.indexOf("Build failed") >= 0 ? t : null;
    }, 60000);
    check("a build error shows over the page", !!overlay);
    check(
      "the page underneath is kept",
      (await page.textContent("#ver")) === "v3",
    );
    fs.writeFileSync(src, appSource("v4", "count", "0"));
    const cleared = await until(
      async () => (await page.textContent("#ver")) === "v4",
      60000,
    );
    check("the next good save recovers", !!cleared);
    const gone = await page.evaluate(
      () =>
        ![...document.documentElement.children].some(
          (e) =>
            e.tagName === "DIV" && e.textContent.indexOf("Build failed") >= 0,
        ),
    );
    check("the overlay is gone", gone);
    check(
      "recovery did not reload the page",
      (await page.evaluate(() => window.__marker)) === "alive",
    );

    await page.evaluate(() => window.scrollTo(0, 600));
    const before = await page.evaluate(() => window.scrollY);
    await server.stop();
    server = start(["serve", src], dir, port, log);
    await listening(port);
    const reloaded = await until(
      async () => (await page.evaluate(() => window.__marker)) === undefined,
      60000,
    );
    check("a restarted server reloads the page", !!reloaded);
    await until(() => page.$("#label"), 10000);
    await sleep(500);
    const after = await page.evaluate(() => window.scrollY);
    check(
      "scroll position survives the reload",
      before > 0 && Math.abs(after - before) < 5,
      before + " -> " + after,
    );
    check("no script errors", errors.length === 0, errors.join(" | "));
  } finally {
    await server.stop();
  }

  const dist = path.join(dir, "dist");
  fs.mkdirSync(dist);
  fs.writeFileSync(path.join(dist, "s.css"), "#t{color:rgb(255,0,0)}");
  fs.writeFileSync(
    path.join(dist, "index.html"),
    '<!doctype html><html><head><link rel="stylesheet" href="s.css"></head><body><p id="t">hello</p><div style="height:3000px"></div></body></html>',
  );
  const port2 = port + 1;
  const server2 = start(["serve", dist, "--watch"], dir, port2, log);
  try {
    await listening(port2);
    await page.goto("http://127.0.0.1:" + port2 + "/");
    await page.evaluate(() => {
      window.__marker = "alive";
    });
    check("built folder is served", (await page.textContent("#t")) === "hello");
    fs.writeFileSync(path.join(dist, "s.css"), "#t{color:rgb(0,0,255)}");
    const blue = await until(
      async () =>
        (await page.$eval("#t", (e) => getComputedStyle(e).color)) ===
        "rgb(0, 0, 255)",
      20000,
    );
    check("a stylesheet change applies", !!blue);
    check(
      "without reloading",
      (await page.evaluate(() => window.__marker)) === "alive",
    );
    await page.evaluate(() => window.scrollTo(0, 800));
    fs.writeFileSync(
      path.join(dist, "index.html"),
      '<!doctype html><html><head><link rel="stylesheet" href="s.css"></head><body><p id="t">changed</p><div style="height:3000px"></div></body></html>',
    );
    const changed = await until(
      async () => (await page.textContent("#t")) === "changed",
      20000,
    );
    check("an html change reloads", !!changed);
    await sleep(500);
    const y = await page.evaluate(() => window.scrollY);
    check("the reload keeps the scroll position", Math.abs(y - 800) < 5, y);
  } finally {
    await server2.stop();
  }

  const ssrSrc = path.resolve(__dirname, "../../tests/en/ssr/site");
  if (fs.existsSync(ssrSrc) && process.platform !== "win32") {
    const ssr = path.join(dir, "ssr");
    fs.cpSync(ssrSrc, ssr, { recursive: true });
    const about = path.join(ssr, "pages", "about.salam");
    const port3 = port + 2;
    const server3 = start(["serve", ssr, "--render"], dir, port3, log);
    try {
      const up = await until(async () => {
        const r = await fetch("http://127.0.0.1:" + port3 + "/about");
        return r.status === 200;
      }, 240000);
      check("the server rendered project starts", !!up);
      await page.goto("http://127.0.0.1:" + port3 + "/about");
      await page.evaluate(() => {
        window.__marker = "alive";
      });
      check(
        "server rendered text is shown",
        (await page.textContent("body")).indexOf("We sell things.") >= 0,
      );
      fs.writeFileSync(
        about,
        fs
          .readFileSync(about, "utf8")
          .replace("We sell things.", "We sell more things."),
      );
      const fresh = await until(
        async () =>
          (await page.textContent("body")).indexOf("We sell more things.") >= 0,
        240000,
      );
      check("a rebuilt server reloads the page", !!fresh);
      check(
        "the page really reloaded",
        (await page.evaluate(() => window.__marker)) === undefined,
      );
      const good = fs.readFileSync(about, "utf8");
      fs.writeFileSync(about, good.replace("heading:", "heading!!:"));
      const shown = await until(async () => {
        const t = await page.evaluate(() => {
          const d = [...document.documentElement.children].find(
            (e) => e.tagName === "DIV",
          );
          return d ? d.textContent : "";
        });
        return t.indexOf("Build failed") >= 0 ? t : null;
      }, 240000);
      check("a server build error shows over the page", !!shown);
      check(
        "the old pages stay up",
        (await page.textContent("body")).indexOf("We sell more things.") >= 0,
      );
      fs.writeFileSync(
        about,
        good.replace("We sell more things.", "We sell the best things."),
      );
      const back = await until(
        async () =>
          (await page.textContent("body")).indexOf(
            "We sell the best things.",
          ) >= 0,
        240000,
      );
      check("the next good save recovers", !!back);
    } finally {
      await server3.stop();
    }
  }
  await browser.close();
  fs.rmSync(dir, { recursive: true, force: true });
  process.exit(failures === 0 ? 0 : 1);
})().catch((e) => {
  console.log("FAIL " + e.message);
  process.exit(1);
});
