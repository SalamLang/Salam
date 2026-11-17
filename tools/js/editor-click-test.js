const fs = require("fs");
const { JSDOM, VirtualConsole } = require("jsdom");

const file = process.argv[2];
if (!file) {
  console.error("usage: node editor-click-test.js editor/index.html");
  process.exit(2);
}

const errors = [];
const virtual = new VirtualConsole();
virtual.on("jsdomError", (e) => errors.push(String(e.message).slice(0, 200)));

const dom = new JSDOM(fs.readFileSync(file, "utf8"), {
  runScripts: "dangerously",
  pretendToBeVisual: true,
  virtualConsole: virtual,
  url: "http://localhost/",
  beforeParse(w) {
    w.matchMedia = (q) => ({
      matches: false,
      media: q,
      addEventListener() {},
      removeEventListener() {},
      addListener() {},
      removeListener() {},
    });
  },
});
const d = dom.window.document;
const q = (s) => d.querySelector(s);

let failed = 0;
function check(name, ok) {
  console.log((ok ? "ok   " : "FAIL ") + name);
  if (!ok) failed++;
}

setTimeout(() => {
  check("page script ran without errors", errors.length === 0);
  const active = (sel) => [...d.querySelectorAll(sel)].map((b) => b.classList.contains("active"));

  check("mode starts on App", active("#mode-seg .seg-btn").join() === "true,false");
  q('#mode-seg [data-value="layout"]').click();
  check("mode button switches to Layout", active("#mode-seg .seg-btn").join() === "false,true");
  check("view tabs for layout appear", !q('#view-seg [data-value="html"]').hidden);
  q('#mode-seg [data-value="app"]').click();

  q('#lang-seg [data-value="fa"]').click();
  check("fa switches to rtl", d.documentElement.dir === "rtl" && d.documentElement.lang === "fa");
  check("fa marks the Persian button", active("#lang-seg .seg-btn").join() === "false,true");
  q('#lang-seg [data-value="en"]').click();
  check("en switches back to ltr", d.documentElement.dir === "ltr");

  const before = q(".theme-lbl").textContent;
  q("#theme").click();
  check("theme button changes the theme", q(".theme-lbl").textContent !== before);

  const on = q("#autorun").classList.contains("on");
  q("#autorun").click();
  check("auto-run toggles", q("#autorun").classList.contains("on") !== on);

  q("#dd-button").click();
  check("examples dropdown opens with items", q("#dd-list").hidden === false && q("#dd-list").children.length > 5);
  q("#dd-button").click();

  q("#menu-toggle").click();
  check("menu button opens the menu", d.body.classList.contains("menu-open"));
  q("#scrim").click();
  check("scrim closes the menu", !d.body.classList.contains("menu-open"));

  check("no script errors after the clicks", errors.length === 0);
  if (errors.length) console.log(errors.join("\n"));
  process.exit(failed ? 1 : 0);
}, 1500);
