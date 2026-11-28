// Compiles a page that imports the UI packages with a freshly linked playground
// compiler and saves the std modules it produced, so the bundle can ship them
// and a first build in the browser does not compile them again.
//
// Usage: node tools/js/editor-std-cache.js <bundle.js> <output file>

const fs = require("fs");
const path = require("path");
const vm = require("node:vm");

const bundle = path.resolve(process.argv[2]);
const target = path.resolve(process.argv[3]);
const dir = path.dirname(bundle);

const page = [
  "import gui.ui",
  "",
  "func check():",
  "    println ui.ScopeRuns()",
  "end",
  "",
  "page:",
  "    button:",
  "        click = check",
  '        content = "Check"',
  "    end",
  "end",
  "",
].join("\n");

function fail(message) {
  console.error("std cache: " + message);
  process.exit(1);
}

global.require = require;
global.__dirname = dir;
global.__filename = bundle;
global.Module = {
  thisProgram: "./this.program",
  locateFile: (f) => path.join(dir, f),
  print: () => {},
  printErr: () => {},
  onAbort: (e) => fail("the compiler aborted (" + e + ")"),
};
global.Module.onRuntimeInitialized = () => {
  setTimeout(() => {
    const build = Module.cwrap("salam_web_std_cache", "string", [
      "string",
      "string",
      "string",
    ]);
    const made = "/std-cache.out";
    const diag = build(page, "en", made);
    if (Module._salam_web_last_failed() === 1) {
      fail("the warm up page did not build\n" + diag);
    }
    const out = Buffer.from(Module.FS.readFile(made));
    if (!out.includes("std/gui/ui/")) {
      fail("the cache holds no UI module");
    }
    fs.writeFileSync(target, out);
    console.log("std cache: " + out.length + " bytes");
    process.exit(0);
  }, 0);
};
vm.runInThisContext(fs.readFileSync(bundle, "utf8"), { filename: bundle });
