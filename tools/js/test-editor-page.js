"use strict";
const fs = require("fs");
const { JSDOM, VirtualConsole } = require("jsdom");

const file = process.argv[2];
if (!file) {
  console.error("usage: node test-editor-page.js editor/index.html");
  process.exit(2);
}

let failures = 0;
function check(name, ok, detail) {
  if (ok) {
    console.log("PASS " + name);
  } else {
    failures++;
    console.log("FAIL " + name + (detail ? " (" + detail + ")" : ""));
  }
}

const errors = [];
const vc = new VirtualConsole();
vc.on("jsdomError", (e) => errors.push(String(e.message).slice(0, 200)));

const html = fs.readFileSync(file, "utf8");
const dom = new JSDOM(html, {
  runScripts: "dangerously",
  pretendToBeVisual: true,
  virtualConsole: vc,
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

const controls = [
  "#menu-toggle", "#mode-seg", "#lang-seg", "#dd-button", "#dd-list", "#theme",
  "#autorun", "#run", "#copy-code", "#copy-out", "#view-seg", "#code", "#hl",
  "#ln", "#output", "#preview", "#gutter", "#ac", "#anim-bar", "#scrim",
];
for (const s of controls) check("control " + s, q(s) !== null);
check("mode buttons carry data-value", q('#mode-seg [data-value="layout"]') !== null);
check("language buttons carry data-value", q('#lang-seg [data-value="fa"]') !== null);

setTimeout(() => {
  check("no script errors on load", errors.length === 0, errors.join(" | "));

  q('#lang-seg [data-value="fa"]').click();
  check("fa switches dir to rtl", d.documentElement.dir === "rtl");
  check("fa button becomes active", q('#lang-seg [data-value="fa"]').classList.contains("active"));
  q('#lang-seg [data-value="en"]').click();
  check("en switches dir to ltr", d.documentElement.dir === "ltr");

  const before = q("#theme .theme-lbl").textContent;
  q("#theme").click();
  check("theme button changes the label", q("#theme .theme-lbl").textContent !== before);

  const on = q("#autorun").classList.contains("on");
  q("#autorun").click();
  check("auto-run button toggles", q("#autorun").classList.contains("on") !== on);

  q("#dd-button").click();
  check("examples list opens", q("#dd-list").hidden === false && q("#dd-list").children.length > 0);

  q("#menu-toggle").click();
  check("menu button opens the menu", d.body.classList.contains("menu-open"));
  q("#scrim").click();
  check("scrim closes the menu", !d.body.classList.contains("menu-open"));

  q('#mode-seg [data-value="layout"]').click();
  check("layout mode becomes active", q('#mode-seg [data-value="layout"]').classList.contains("active"));

  check("no script errors after clicks", errors.length === 0, errors.join(" | "));
  console.log(failures === 0 ? "editor page ok" : failures + " check(s) failed");
  process.exit(failures === 0 ? 0 : 1);
}, 1500);
