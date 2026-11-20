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
vc.on("jsdomError", (e) => {
  const m = String(e.message);
  if (m.indexOf("Could not parse CSS stylesheet") === 0) return;
  errors.push(m.slice(0, 200));
});

const html = fs.readFileSync(file, "utf8");
const dom = new JSDOM(html, {
  runScripts: "dangerously",
  pretendToBeVisual: true,
  virtualConsole: vc,
  url: "http://localhost/",
  beforeParse(w) {
    w.TextEncoder = TextEncoder;
    w.TextDecoder = TextDecoder;
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
  "#menu-toggle",
  "#mode-seg",
  "#lang-seg",
  "#dd-button",
  "#dd-list",
  "#theme",
  "#autorun",
  "#run",
  "#copy-code",
  "#copy-out",
  "#view-seg",
  "#code",
  "#hl",
  "#ln",
  "#output",
  "#preview",
  "#gutter",
  "#ac",
  "#anim-bar",
  "#scrim",
];
for (const s of controls) check("control " + s, q(s) !== null);
check(
  "mode buttons carry data-value",
  q('#mode-seg [data-value="layout"]') !== null,
);
check(
  "the code area is a direct child of the editor wrap",
  q("#code").parentElement === q("#edwrap"),
);
check(
  "language buttons carry data-value",
  q('#lang-seg [data-value="fa"]') !== null,
);

setTimeout(() => {
  check("no script errors on load", errors.length === 0, errors.join(" | "));

  const code = q("#code");
  let text = "func main:\n";
  for (let i = 0; i < 1500; i++) text += "    println " + i + "\n";
  code.value = text + "end\n";
  code.dispatchEvent(
    new dom.window.InputEvent("input", { inputType: "insertFromPaste" }),
  );
  check(
    "one line number per line",
    d.querySelectorAll("#ln-inner .ln-row").length ===
      code.value.split("\n").length,
  );
  check(
    "the line number column widens for four digits",
    /4ch/.test(q("#ln").style.width),
    q("#ln").style.width,
  );
  code.value = "func main:\nend\n";
  code.dispatchEvent(
    new dom.window.InputEvent("input", { inputType: "insertFromPaste" }),
  );
  check(
    "line numbers shrink with the code",
    d.querySelectorAll("#ln-inner .ln-row").length === 3,
  );

  q('#lang-seg [data-value="fa"]').click();
  check("fa switches dir to rtl", d.documentElement.dir === "rtl");
  check(
    "fa button becomes active",
    q('#lang-seg [data-value="fa"]').classList.contains("active"),
  );
  q('#lang-seg [data-value="en"]').click();
  check("en switches dir to ltr", d.documentElement.dir === "ltr");

  const before = q("#theme .theme-lbl").textContent;
  q("#theme").click();
  check(
    "theme button changes the label",
    q("#theme .theme-lbl").textContent !== before,
  );

  const on = q("#autorun").classList.contains("on");
  q("#autorun").click();
  check(
    "auto-run button toggles",
    q("#autorun").classList.contains("on") !== on,
  );

  q("#dd-button").click();
  check(
    "examples list opens",
    q("#dd-list").hidden === false && q("#dd-list").children.length > 0,
  );

  q("#menu-toggle").click();
  check("menu button opens the menu", d.body.classList.contains("menu-open"));
  q("#scrim").click();
  check("scrim closes the menu", !d.body.classList.contains("menu-open"));

  q('#mode-seg [data-value="layout"]').click();
  check(
    "layout mode becomes active",
    q('#mode-seg [data-value="layout"]').classList.contains("active"),
  );

  const gutter = q("#gutter");
  const split = q("#main-split");
  const PE = dom.window.PointerEvent || dom.window.MouseEvent;
  const ptr = (target, type, x, buttons, pointerId = 1) =>
    target.dispatchEvent(
      new PE(type, {
        bubbles: true,
        cancelable: true,
        pointerId,
        clientX: x,
        clientY: 200,
        buttons,
      }),
    );
  let captured = false;
  gutter.setPointerCapture = () => {
    captured = true;
  };
  const cols = () => split.style.gridTemplateColumns;
  const cleared = () =>
    !gutter.classList.contains("dragging") &&
    !split.classList.contains("resizing");
  const drag = (endWith) => {
    split.style.gridTemplateColumns = "";
    ptr(gutter, "pointerdown", 500, 1);
    ptr(gutter, "pointermove", 420, 1);
    const during = cols();
    endWith();
    const ended = cleared();
    const after = cols();
    ptr(d.body, "pointermove", 300, 1);
    return { during, ended, after, later: cols() };
  };
  const ends = [
    ["pointerup", () => ptr(gutter, "pointerup", 420, 0)],
    ["pointercancel", () => ptr(gutter, "pointercancel", 420, 0)],
    ["lostpointercapture", () => ptr(gutter, "lostpointercapture", 420, 0)],
    ["a window pointerup", () => ptr(d.body, "pointerup", 420, 0)],
    [
      "a window blur",
      () => dom.window.dispatchEvent(new dom.window.Event("blur")),
    ],
    ["a move with no button held", () => ptr(d.body, "pointermove", 360, 0)],
  ];
  for (const [what, end] of ends) {
    const r = drag(end);
    check(
      "the gutter resizes while dragged (" + what + ")",
      r.during === "420px 6px 1fr",
      r.during,
    );
    check("the drag classes clear on " + what, r.ended);
    check(
      "the split stays put after " + what,
      r.later === r.after,
      r.after + " then " + r.later,
    );
  }

  split.style.gridTemplateColumns = "";
  ptr(gutter, "pointerdown", 500, 1);
  ptr(gutter, "pointermove", 420, 1);
  ptr(d.body, "pointermove", 250, 1, 2);
  check(
    "a second pointer does not move the split",
    cols() === "420px 6px 1fr",
    cols(),
  );
  ptr(d.body, "pointerup", 250, 0, 2);
  ptr(d.body, "pointercancel", 250, 0, 2);
  check("a second pointer does not end the drag", !cleared());
  ptr(gutter, "pointermove", 400, 1);
  check("the first pointer still drags", cols() === "400px 6px 1fr", cols());
  ptr(gutter, "pointerup", 400, 0);
  check("the first pointer ends the drag", cleared());
  check("the gutter captures the pointer", captured);

  check(
    "no script errors after clicks",
    errors.length === 0,
    errors.join(" | "),
  );
  console.log(
    failures === 0 ? "editor page ok" : failures + " check(s) failed",
  );
  process.exit(failures === 0 ? 0 : 1);
}, 1500);
