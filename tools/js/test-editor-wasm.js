const fs = require("fs");
const path = require("path");
const vm = require("node:vm");

const dir = path.resolve(process.argv[2] || "editor");
const js = fs.readdirSync(dir).find((f) => /^salam-wa-.*\.js$/.test(f));
if (!js) {
  console.error(
    "no salam-wa-*.js in " + dir + "; build it with tools/bash/build-wasm.sh",
  );
  process.exit(2);
}

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

const cases = [
  {
    name: "a console program runs",
    kind: "app",
    lang: "en",
    src: 'func main:\n    name := "Salam"\n    println "Hello,", name\nend\n',
    want: (out) => out.indexOf("Hello, Salam") >= 0,
  },
  {
    name: "a loop runs",
    kind: "app",
    lang: "en",
    src: "func main:\n    mut s := 0\n    repeat 1000 in i:\n        s += i\n    end\n    println s\nend\n",
    want: (out) => out.trim() === "499500",
  },
  {
    name: "a page with colour names builds",
    kind: "layout",
    lang: "en",
    src: 'page:\n    title = "Colours"\n    box:\n        background = "blue"\n        color = "white"\n        paragraph: content = "hi" end\n    end\nend\n',
    want: (out) => /#0000ff/i.test(out) && /#ffffff/i.test(out),
  },
  {
    name: "a Persian page with colour names builds",
    kind: "layout",
    lang: "fa",
    src: 'صفحه:\n    عنوان = "رنگ"\n    جعبه:\n        پس‌زمینه = "آبی"\n        پاراگراف: محتوا = "سلام" پایان\n    پایان\nپایان\n',
    want: (out) => /#0000ff/i.test(out),
  },
  {
    name: "a component with state builds a page that updates",
    kind: "layout",
    lang: "en",
    src: 'component App:\n    count := 0\n    button:\n        content = "Go"\n        click = (): count++ end\n    end\n    paragraph: content = "Clicked " + count + " times" end\nend\npage:\n    App: end\nend\n',
    want: (out) =>
      out.indexOf("<script>") >= 0 && out.indexOf("Clicked ${") >= 0,
  },
  {
    name: "a page importing the UI package builds from the shipped std cache",
    kind: "layout",
    lang: "en",
    src: 'import gui.ui\n\nfunc check():\n    println ui.ScopeRuns()\nend\n\npage:\n    button:\n        click = check\n        content = "Check"\n    end\nend\n',
    want: (out) =>
      out.indexOf("function ui_ScopeRuns") >= 0 && _sbrk(0) >>> 0 < 536870912,
    limit: 10000,
  },
  {
    name: "a page run as a program points to layout mode",
    kind: "app",
    lang: "en",
    src: 'page:\n    paragraph: content = "hi" end\nend\n',
    fail: true,
    want: (out) => out.indexOf("switch to Layout mode") >= 0,
  },
  {
    name: "a parse error is reported",
    kind: "app",
    lang: "en",
    src: "func main:\n    println (\nend\n",
    fail: true,
    want: (out) => /error/.test(out),
  },
];

for (const ex of [
  "01-hello.salam",
  "12-layout.salam",
  "26-bank_account.salam",
]) {
  const src = fs.readFileSync(
    path.join(__dirname, "../../tests/en/editor-selected", ex),
    "utf8",
  );
  const layout = /^page:/m.test(src);
  cases.push({
    name: "example " + ex + " runs",
    kind: layout ? "layout" : "app",
    lang: "en",
    src,
    want: () => true,
  });
}

global.require = require;
global.__dirname = dir;
global.__filename = path.join(dir, js);
global.Module = {
  thisProgram: "./this.program",
  locateFile: (f) => path.join(dir, f),
  print: () => {},
  printErr: () => {},
  onAbort: (e) => {
    console.log("FAIL the module aborted (" + e + ")");
    process.exit(1);
  },
};
global.Module.onRuntimeInitialized = () => {
  setTimeout(() => {
    const run = Module.cwrap("salam_web_run_app", "string", [
      "string",
      "string",
    ]);
    const lay = Module.cwrap("salam_web_build_layout", "string", [
      "string",
      "string",
    ]);
    const times = [];
    for (const c of cases) {
      const t0 = Date.now();
      const out = c.kind === "layout" ? lay(c.src, c.lang) : run(c.src, c.lang);
      const ms = Date.now() - t0;
      times.push(ms);
      const failed = Module._salam_web_last_failed() === 1;
      check(
        c.name + " in " + ms + " ms",
        failed === !!c.fail && c.want(out) && !(c.limit && ms > c.limit),
        out,
      );
    }
    const ui = cases.find((c) => c.name.indexOf("shipped std cache") >= 0).src;
    let early = 0;
    let rebuilt = true;
    for (let i = 0; i < 40; i++) {
      const line = "    println ui.ScopeRuns()\n";
      lay(ui.replace(line, line.repeat(i + 1)), "en");
      rebuilt = rebuilt && Module._salam_web_last_failed() === 0;
      if (i === 9) early = _sbrk(0) >>> 0;
    }
    const grown = (_sbrk(0) >>> 0) - early;
    check(
      "rebuilding a UI page does not keep growing the heap",
      rebuilt && Math.abs(grown) < 8388608,
      grown + " bytes over 30 builds",
    );
    const cards =
      'page:\n    title = "Cards"\n    design = "shadcn"\n    card:\n        card header:\n            card title: content = "Title" end\n        end\n    end\nend\n';
    const first = lay(cards, "en");
    let same = Module._salam_web_last_failed() === 0;
    for (let i = 0; same && i < 80; i++) {
      same = lay(cards, "en") === first;
    }
    check("many compiles stay within the memory limit", same, first);
    times.sort((a, b) => a - b);
    console.log(
      "median call " +
        times[Math.floor(times.length / 2)] +
        " ms, slowest " +
        times[times.length - 1] +
        " ms",
    );
    console.log(
      failures === 0 ? "editor wasm ok" : failures + " check(s) failed",
    );
    process.exit(failures === 0 ? 0 : 1);
  }, 0);
};
vm.runInThisContext(fs.readFileSync(path.join(dir, js), "utf8"), {
  filename: js,
});
