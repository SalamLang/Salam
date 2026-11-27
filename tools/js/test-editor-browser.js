const fs = require("fs");
const http = require("http");
const path = require("path");
const { chromium } = require("playwright-core");

const root = path.resolve(process.argv[2] || "editor");
const shots = process.argv[3] || "";
const wantWasm = process.env.EDITOR_NO_WASM !== "1";

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
  ".html": "text/html",
  ".js": "text/javascript",
  ".wasm": "application/wasm",
  ".svg": "image/svg+xml",
  ".woff2": "font/woff2",
  ".json": "application/json",
  ".webmanifest": "application/manifest+json",
};
const server = http.createServer((req, res) => {
  let p = decodeURIComponent(req.url.split("?")[0]);
  if (p.endsWith("/")) p += "index.html";
  const f = path.join(root, path.normalize(p));
  if (
    !f.startsWith(root) ||
    !fs.existsSync(f) ||
    fs.statSync(f).isDirectory()
  ) {
    res.writeHead(404);
    res.end();
    return;
  }
  res.writeHead(200, {
    "Content-Type": types[path.extname(f)] || "application/octet-stream",
  });
  fs.createReadStream(f).pipe(res);
});

let failures = 0;
function check(name, ok, detail) {
  if (ok) {
    console.log("PASS " + name);
  } else {
    failures++;
    console.log(
      "FAIL " +
        name +
        (detail !== undefined ? " (" + String(detail).slice(0, 300) + ")" : ""),
    );
  }
}

async function setCode(page, text) {
  await page.evaluate((t) => {
    const c = document.querySelector("#code");
    c.value = t;
    c.dispatchEvent(new InputEvent("input", { inputType: "insertFromPaste" }));
    c.scrollTop = 0;
    c.scrollLeft = 0;
  }, text);
  await page.waitForTimeout(150);
}

server.listen(0, "127.0.0.1", async () => {
  const url = "http://127.0.0.1:" + server.address().port + "/";
  const browser = await chromium.launch({ executablePath: browserPath() });
  const page = await browser.newPage({
    viewport: { width: 1280, height: 800 },
  });
  const errors = [];
  page.on("pageerror", (e) => errors.push(e.message));
  await page.goto(url, { waitUntil: "load" });
  const shot = async (n) => {
    if (shots) await page.screenshot({ path: path.join(shots, n + ".png") });
  };

  if (wantWasm) {
    const ready = await page
      .waitForFunction(
        () =>
          document.querySelector("#run") &&
          !document.querySelector("#run").disabled &&
          /ok|err/.test(document.querySelector("#status").className),
        null,
        { timeout: 120000 },
      )
      .then(
        () => true,
        () => false,
      );
    check("the compiler loads", ready, await page.textContent("#status"));
    check(
      "the first example runs",
      (await page.textContent("#output")).indexOf("Hello, Salam") >= 0,
      await page.textContent("#output"),
    );
  }
  if (await page.$eval("#autorun", (e) => e.classList.contains("on")))
    await page.click("#autorun");

  const box = await page.$eval("#edwrap", (e) => {
    const r = e.getBoundingClientRect();
    return { x: r.x, y: r.y, w: r.width, h: r.height };
  });
  const code = await page.$eval("#code", (c) => ({
    h: c.getBoundingClientRect().height,
    parent: c.parentElement.id,
  }));
  check(
    "the code area fills the editor",
    Math.abs(code.h - box.h) < 2,
    code.h + " vs " + box.h,
  );
  check(
    "the code area sits in the editor wrap",
    code.parent === "edwrap",
    code.parent,
  );

  const rowBox = (n) =>
    page.evaluate((k) => {
      const r = document
        .querySelectorAll(".ln-row")
        [k - 1].getBoundingClientRect();
      return { y: r.y, h: r.height };
    }, n);
  const clickLine = async (n, dx) => {
    const r = await rowBox(n);
    const x = await page.$eval("#edwrap", (e) => e.getBoundingClientRect().x);
    await page.mouse.click(x + dx, r.y + r.h / 2);
  };
  const caretLine = () =>
    page.evaluate(() => {
      const c = document.querySelector("#code");
      return c.value.slice(0, c.selectionStart).split("\n").length;
    });

  await setCode(
    page,
    'func main:\n    println "one"\n    println "two"\n    println "three"\nend\n',
  );
  await clickLine(3, 300);
  check(
    "a click on line 3 puts the caret on line 3",
    (await caretLine()) === 3,
    await caretLine(),
  );
  await page.keyboard.type("Q");
  check(
    "typing goes where the click was",
    (await page.$eval("#code", (c) => c.value.split("\n")[2])).indexOf("Q") >=
      0,
  );
  await page.evaluate(() => document.activeElement.blur());
  await page.mouse.click(box.x + box.w / 2, box.y + box.h - 30);
  check(
    "a click below the code focuses the editor",
    (await page.evaluate(() => document.activeElement.id)) === "code",
  );

  let long = "func main:\n";
  for (let i = 1; i <= 1200; i++) {
    long +=
      i % 7 === 0
        ? "    // a comment long enough to run past the right edge of the editor and scroll it sideways " +
          i +
          "\n"
        : "    x" + i + " := " + i + "\n";
  }
  long += "end\n";
  await setCode(page, long);
  const rows = await page.$$eval(".ln-row", (l) => l.length);
  const lines = await page.$eval("#code", (c) => c.value.split("\n").length);
  check("one line number per line", rows === lines, rows + " vs " + lines);
  const fit = await page.evaluate(() => {
    const r = Array.from(document.querySelectorAll(".ln-row")).pop();
    return r.scrollWidth <= r.clientWidth;
  });
  check("four digit line numbers fit their column", fit);

  await page.mouse.move(box.x + box.w / 2, box.y + box.h / 2);
  for (let i = 0; i < 30; i++) await page.mouse.wheel(0, 2000);
  await page.waitForTimeout(200);
  const sc = await page.evaluate(() => {
    const c = document.querySelector("#code");
    return {
      top: c.scrollTop,
      hl: document.querySelector("#hl").scrollTop,
      tr: document.querySelector("#ln-inner").style.transform,
    };
  });
  check("the wheel scrolls the code", sc.top > 1000, sc.top);
  check(
    "the highlight follows the scroll",
    Math.abs(sc.top - sc.hl) < 1,
    sc.top + " vs " + sc.hl,
  );
  check(
    "the line numbers follow the scroll",
    sc.tr === "translateY(-" + sc.top + "px)",
    sc.tr,
  );
  const near = rows - 5;
  await clickLine(near, 300);
  check(
    "a click after scrolling lands on its line",
    (await caretLine()) === near,
    (await caretLine()) + " vs " + near,
  );
  await shot("scrolled");
  await page.mouse.move(box.x + box.w / 2, box.y + box.h / 2);
  for (let i = 0; i < 10; i++) await page.mouse.wheel(3000, 0);
  await page.waitForTimeout(200);
  const sx = await page.evaluate(() => [
    document.querySelector("#code").scrollLeft,
    document.querySelector("#hl").scrollLeft,
  ]);
  check(
    "the highlight follows a sideways scroll",
    sx[0] > 0 && Math.abs(sx[0] - sx[1]) < 1,
    sx.join(" vs "),
  );

  await page.click('#lang-seg [data-value="fa"]');
  await setCode(page, 'تابع ریشه:\n    سرچاپ "یک"\n    سرچاپ "دو"\nپایان\n');
  await clickLine(3, 300);
  check(
    "right to left: a click on line 3 puts the caret on line 3",
    (await caretLine()) === 3,
    await caretLine(),
  );
  await shot("persian");
  await page.click('#lang-seg [data-value="en"]');

  if (wantWasm) {
    await setCode(
      page,
      'func main:\n    repeat 3 in i:\n        println "line", i\n    end\nend\n',
    );
    await page.click("#run");
    await page.waitForFunction(
      () => /ok|err/.test(document.querySelector("#status").className),
      null,
      { timeout: 120000 },
    );
    check(
      "Run prints the output",
      (await page.textContent("#output")).indexOf("line 2") >= 0,
      await page.textContent("#output"),
    );
    await page.click('#mode-seg [data-value="layout"]');
    await page.waitForFunction(
      () => /ok|err/.test(document.querySelector("#status").className),
      null,
      { timeout: 120000 },
    );
    await setCode(
      page,
      'page:\n    title = "Test"\n    box:\n        background = "blue"\n        color = "white"\n        paragraph: content = "Hello page" end\n    end\nend\n',
    );
    await page.click("#run");
    await page.waitForFunction(
      () => /ok|err/.test(document.querySelector("#status").className),
      null,
      { timeout: 120000 },
    );
    const doc = await page.$eval("#preview", (f) => f.srcdoc || "");
    check(
      "Layout mode builds a page with colour names",
      doc.indexOf("Hello page") >= 0 && /#0000ff/i.test(doc),
      doc.slice(0, 300),
    );
    await shot("layout");

    const settled = () =>
      page.waitForFunction(
        () => /ok|err/.test(document.querySelector("#status").className),
        null,
        { timeout: 120000 },
      );
    const previewShot = async () => {
      const shown = await page
        .frameLocator("#preview")
        .locator("text=Hello page")
        .waitFor({ timeout: 10000 })
        .then(
          () => true,
          () => false,
        );
      await page.waitForTimeout(300);
      return { shown, png: await page.locator("#preview").screenshot() };
    };
    const first = await previewShot();
    for (const tab of ["html", "css"]) {
      await page.click('#view-seg [data-value="' + tab + '"]');
      await settled();
      await page.click('#view-seg [data-value="run"]');
      await settled();
      const again = await previewShot();
      check(
        "the preview shows the page again after the " +
          tab.toUpperCase() +
          " tab",
        first.shown && again.shown,
      );
      check(
        "the preview paints the same after the " + tab.toUpperCase() + " tab",
        Buffer.compare(first.png, again.png) === 0,
      );
    }
    await shot("layout-again");

    const pages = {
      en: 'page:\n    title = "Tabs"\n    style: media = "screen" content = "p { margin: 0 }" end\n    style: content = ".x { color: red }" end\n    script: content = "window.tabs = 1" end\n    paragraph: content = "Hello tabs" end\nend\n',
      fa: 'صفحه:\n    عنوان = "زبانه"\n    پاراگراف: محتوا = "سلام دنیا" پایان\nپایان\n',
    };
    for (const [lang, src] of Object.entries(pages)) {
      await page.click('#lang-seg [data-value="' + lang + '"]');
      await setCode(page, src);
      await page.click("#run");
      await settled();
      for (const tab of ["html", "css", "js", "run"]) {
        const got = await page.evaluate((t) => {
          document.querySelector('#view-seg [data-value="' + t + '"]').click();
          return {
            status: document.querySelector("#status").className,
            text: document.querySelector("#output").textContent,
          };
        }, tab);
        check(
          "the " + lang + " " + tab + " tab is ready after one run",
          /ok/.test(got.status),
          got.status,
        );
        if (tab === "run") continue;
        const want = await page.evaluate(
          ([s, l, t]) =>
            window.Module.cwrap("salam_web_emit", "string", [
              "string",
              "string",
              "string",
            ])(s, l, t),
          [src, lang, tab],
        );
        check(
          "the " + lang + " " + tab + " tab matches the compiler",
          got.text === want,
          got.text.length + " vs " + want.length + ": " + got.text,
        );
      }
    }
    await page.click('#lang-seg [data-value="en"]');
    await setCode(page, pages.en);
    await page.click("#run");
    await settled();
    await page.click('#view-seg [data-value="run"]');

    check(
      "programs are compiled in a worker",
      page.workers().length === 1,
      page.workers().length,
    );
    await page.click('#mode-seg [data-value="app"]');
    await settled();
    await page.click("#autorun");
    const printed = (text) =>
      page
        .waitForFunction(
          (t) => document.querySelector("#output").textContent.includes(t),
          text,
          { timeout: 30000 },
        )
        .then(
          () => true,
          () => false,
        );
    for (const word of ["first", "second", "third"]) {
      await setCode(page, 'func main:\n    println "' + word + '"\n end\n');
    }
    check(
      "typing ends on the output of the last edit",
      await printed("third"),
      await page.textContent("#output"),
    );
    await setCode(
      page,
      "func main:\n    mut n := 0\n    until true:\n        n++\n    end\n    println n\nend\n",
    );
    await page.waitForFunction(
      () => /busy/.test(document.querySelector("#status").className),
      null,
      { timeout: 30000 },
    );
    const started = Date.now();
    await page.evaluate(() => document.querySelector("#code").focus());
    check(
      "the page answers while a program is running",
      Date.now() - started < 1000,
      Date.now() - started,
    );
    await setCode(page, 'func main:\n    println "after the loop"\nend\n');
    check(
      "an edit made during a run is compiled afterwards",
      await printed("after the loop"),
      await page.textContent("#output"),
    );
    await page.click("#autorun");
    await page.click('#mode-seg [data-value="layout"]');
    await page.click('#view-seg [data-value="run"]');

    const g = await page.$eval("#gutter", (e) => {
      const r = e.getBoundingClientRect();
      return { x: r.x + r.width / 2, y: r.y + r.height / 2 };
    });
    const pv = await page.$eval("#preview", (e) => {
      const r = e.getBoundingClientRect();
      return { x: r.x + r.width / 2, y: r.y + r.height / 2 };
    });
    const cols = () =>
      page.$eval("#main-split", (e) => e.style.gridTemplateColumns);
    const dragClasses = () =>
      page.evaluate(
        () =>
          document.querySelector("#gutter").classList.contains("dragging") ||
          document.querySelector("#main-split").classList.contains("resizing"),
      );
    const before = await cols();
    await page.mouse.move(g.x, g.y);
    await page.mouse.down();
    await page.mouse.move(g.x - 80, g.y, { steps: 4 });
    const during = await cols();
    await page.mouse.move(pv.x, pv.y, { steps: 4 });
    await page.mouse.up();
    check(
      "the drag classes clear on a release over the preview",
      !(await dragClasses()),
    );
    const released = await cols();
    await page.mouse.move(box.x + 100, box.y + 100, { steps: 4 });
    const later = await cols();
    check(
      "the gutter resizes while dragged",
      during !== "" && during !== before,
      before + " then " + during,
    );
    check(
      "the gutter stops on a release over the preview",
      later === released,
      released + " then " + later,
    );
    await page.$eval("#main-split", (e) => (e.style.gridTemplateColumns = ""));
  }

  await page.setViewportSize({ width: 390, height: 844 });
  await page.waitForTimeout(200);
  await setCode(
    page,
    "func main:\n    println 1\n    println 2\n    println 3\nend\n",
  );
  await clickLine(4, 150);
  check(
    "phone width: a click on line 4 puts the caret on line 4",
    (await caretLine()) === 4,
    await caretLine(),
  );
  await shot("phone");

  check("no script errors", errors.length === 0, errors.join(" | "));
  await browser.close();
  server.close();
  console.log(
    failures === 0 ? "editor in the browser ok" : failures + " check(s) failed",
  );
  process.exit(failures === 0 ? 0 : 1);
});
