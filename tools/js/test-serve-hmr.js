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

async function overlayInfo(page) {
  return page.evaluate(() => {
    const d = [...document.documentElement.children].find(
      (e) => e.tagName === "DIV" && e.getAttribute("dir"),
    );
    if (!d || d.style.overflow !== "auto") return null;
    const kids = [...d.querySelectorAll("div[dir]")].map((e) =>
      e.getAttribute("dir"),
    );
    return {
      dir: d.getAttribute("dir"),
      lang: d.getAttribute("lang"),
      text: d.textContent,
      ltrLines: kids.filter((k) => k === "ltr").length,
      autoLines: kids.filter((k) => k === "auto").length,
    };
  });
}

async function overlayWith(page, needle, ms) {
  return until(async () => {
    const o = await overlayInfo(page);
    return o && o.text.indexOf(needle) >= 0 ? o : null;
  }, ms);
}

async function listening(port) {
  return until(async () => {
    const r = await fetch("http://127.0.0.1:" + port + "/__salam_hmr");
    return r.status === 200;
  }, 60000);
}

async function mustListen(port, name, log) {
  if (await listening(port)) return;
  const text = fs.existsSync(log)
    ? fs.readFileSync(log, "utf8").slice(-3000)
    : "";
  throw new Error(name + " did not start on port " + port + "\n" + text);
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
    const eno = await overlayInfo(page);
    check(
      "an English overlay is ltr and English",
      !!eno && eno.dir === "ltr" && eno.lang === "en",
      JSON.stringify(eno && { dir: eno.dir, lang: eno.lang }),
    );
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
    await mustListen(port, "the restarted dev server", log);
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
    await mustListen(port2, "the built folder server", log);
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

  const faDir = path.join(dir, "fa");
  fs.mkdirSync(faDir);
  const faSrc = path.join(faDir, "app.salam");
  const faGood =
    "صفحه:\n    عنوان = «سلام»\n    جعبه:\n        محتوا = «سلام دنیا»\n    پایان\nپایان\n";
  fs.writeFileSync(faSrc, faGood);
  const port4 = port + 3;
  const server4 = start(["serve", faSrc], dir, port4, log);
  try {
    await mustListen(port4, "the Persian dev server", log);
    await page.goto("http://127.0.0.1:" + port4 + "/");
    fs.writeFileSync(faSrc, "صفحه:\n    عنوان = \n");
    const fao = await overlayWith(page, "ساخت ناموفق بود", 60000);
    check("a Persian overlay has Persian text", !!fao);
    check(
      "a Persian overlay is rtl and fa",
      !!fao && fao.dir === "rtl" && fao.lang === "fa",
      JSON.stringify(fao && { dir: fao.dir, lang: fao.lang }),
    );
    check(
      "a Persian overlay keeps file lines ltr and message lines automatic",
      !!fao && fao.ltrLines > 0 && fao.autoLines > 0,
      JSON.stringify(fao && { l: fao.ltrLines, a: fao.autoLines }),
    );
    check(
      "the diagnostics inside it are Persian",
      !!fao && fao.text.indexOf("خطا") >= 0,
    );
    fs.writeFileSync(faSrc, faGood);
    const faok = await until(
      async () => (await overlayInfo(page)) === null,
      60000,
    );
    check("a Persian overlay goes away after a good save", !!faok);
  } finally {
    await server4.stop();
  }

  const ssrCases = [
    {
      lang: "en",
      title: "Build failed",
      dir: "ltr",
      page: "/about",
      ok: "We sell things.",
      more: "We sell more things.",
      best: "We sell the best things.",
      brk: ["heading:", "heading!!:"],
    },
    {
      lang: "fa",
      title: "ساخت ناموفق بود",
      dir: "rtl",
      page: "/about",
      ok: "ما چیز می‌فروشیم.",
      more: "ما چیز بیشتر می‌فروشیم.",
      best: "ما بهترین چیزها را می‌فروشیم.",
      brk: ["سرتیتر:", "سرتیتر!!:"],
    },
  ];
  for (const C of ssrCases) {
    const ssrSrc = path.resolve(
      __dirname,
      "../../tests/" + C.lang + "/ssr/site",
    );
    if (!fs.existsSync(ssrSrc) || process.platform === "win32") continue;
    const ssr = path.join(dir, "ssr-" + C.lang);
    fs.cpSync(ssrSrc, ssr, { recursive: true });
    const about = path.join(ssr, "pages", "about.salam");
    const port3 = port + 10 + (C.lang === "fa" ? 1 : 0);
    const server3 = start(["serve", ssr, "--render"], dir, port3, log);
    try {
      const up = await until(async () => {
        const r = await fetch("http://127.0.0.1:" + port3 + C.page);
        return r.status === 200;
      }, 240000);
      check("the " + C.lang + " server rendered project starts", !!up);
      await page.goto("http://127.0.0.1:" + port3 + C.page);
      await page.evaluate(() => {
        window.__marker = "alive";
      });
      check(
        "server rendered text is shown (" + C.lang + ")",
        (await page.textContent("body")).indexOf(C.ok) >= 0,
      );
      fs.writeFileSync(
        about,
        fs.readFileSync(about, "utf8").replace(C.ok, C.more),
      );
      const fresh = await until(
        async () => (await page.textContent("body")).indexOf(C.more) >= 0,
        240000,
      );
      check("a rebuilt server reloads the page (" + C.lang + ")", !!fresh);
      check(
        "the page really reloaded (" + C.lang + ")",
        (await page.evaluate(() => window.__marker)) === undefined,
      );
      const good = fs.readFileSync(about, "utf8");
      fs.writeFileSync(about, good.replace(C.brk[0], C.brk[1]));
      const shown = await overlayWith(page, C.title, 240000);
      check(
        "a server build error shows over the page (" + C.lang + ")",
        !!shown,
      );
      check(
        "the SSR overlay is " + C.dir + " and " + C.lang,
        !!shown && shown.dir === C.dir && shown.lang === C.lang,
        JSON.stringify(shown && { dir: shown.dir, lang: shown.lang }),
      );
      check(
        "the old pages stay up (" + C.lang + ")",
        (await page.textContent("body")).indexOf(C.more) >= 0,
      );
      fs.writeFileSync(about, good.replace(C.more, C.best));
      const back = await until(
        async () => (await page.textContent("body")).indexOf(C.best) >= 0,
        240000,
      );
      check("the next good save recovers (" + C.lang + ")", !!back);
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
