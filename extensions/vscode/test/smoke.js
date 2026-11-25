// Loads the compiled extension under a stub `vscode` module and drives every
// command, so a runtime error in the emitted JavaScript (such as a method name
// that JavaScript strings do not have) fails here instead of in the editor.
//
//   node test/smoke.js [--lsp] [path/to/extension.js]
//
// With --lsp the fake compiler answers `lsp --version`, so the language client
// and the debug adapter start too. That pass stubs vscode-languageclient and
// so needs the unbundled `salam build --platform=js` output, not the packaged one.

const assert = require("node:assert");
const fs = require("node:fs");
const os = require("node:os");
const path = require("node:path");
const Module = require("node:module");

const argv = process.argv.slice(2);
const lspMode = argv.includes("--lsp");
const extArg = argv.find((a) => a !== "--lsp");
const extPath = path.resolve(
  extArg || path.join(__dirname, "..", "out", "extension.js"),
);

const work = fs.mkdtempSync(path.join(os.tmpdir(), "salam-vscode-smoke-"));
const fakeCompiler = path.join(work, "fake-salam");
const callsLog = path.join(work, "calls.log");

function calls(sub) {
  if (!fs.existsSync(callsLog)) return [];
  return fs
    .readFileSync(callsLog, "utf8")
    .split("\n")
    .filter((l) => l !== "")
    .map((l) => JSON.parse(l))
    .filter((a) => a[0] === sub);
}
fs.writeFileSync(
  fakeCompiler,
  [
    "#!/usr/bin/env node",
    'const fs = require("fs");',
    "const a = process.argv.slice(2);",
    `fs.appendFileSync(${JSON.stringify(callsLog)}, JSON.stringify(a) + "\\n");`,
    `if (a[0] === "lsp") process.exit(${lspMode ? 0 : 1});`,
    'if (a[0] === "format") {',
    '  fs.writeFileSync(a[1], fs.readFileSync(a[1], "utf8").replace(/  +/g, " "));',
    "  process.exit(0);",
    "}",
    'if (a.includes("--emit-tokens") || a.includes("--emit-ast")) {',
    '  process.stdout.write("<root/>\\n");',
    "}",
    "process.exit(0);",
    "",
  ].join("\n"),
);
fs.chmodSync(fakeCompiler, 0o755);

const failures = [];
process.on("uncaughtException", (e) => failures.push(e));
process.on("unhandledRejection", (e) => failures.push(e));

const messages = [];
const commands = new Map();
const sentText = [];
const outputLines = [];
let formatter = null;
let debugFactory = null;
const clients = [];
let activeEditor;
const opened = [];
const inputBoxValue = "demo-app";

const config = {
  compilerPath: fakeCompiler,
  "run.command": "exec",
  "lsp.enable": true,
  "format.enable": true,
};

function disposable() {
  return { dispose() {} };
}

class Range {
  constructor(sl, sc, el, ec) {
    Object.assign(this, { sl, sc, el, ec });
  }
}

const statusItems = [];
const selectionListeners = [];

const vscodeStub = {
  Range,
  Position: class {
    constructor(line, character) {
      Object.assign(this, { line, character });
    }
  },
  Selection: class {},
  ThemeColor: class {},
  TextEdit: { replace: (range, newText) => ({ range, newText }) },
  DebugAdapterExecutable: class {
    constructor(command, args) {
      this.command = command;
      this.args = args;
    }
  },
  window: {
    createStatusBarItem: () => {
      const it = {
        text: "",
        visible: false,
        show() {
          this.visible = true;
        },
        hide() {
          this.visible = false;
        },
        dispose() {},
      };
      statusItems.push(it);
      return it;
    },
    createTextEditorDecorationType: () => disposable(),
    onDidChangeTextEditorSelection: (cb) => {
      selectionListeners.push(cb);
      return disposable();
    },
    onDidChangeActiveTextEditor: () => disposable(),
    createOutputChannel: () => ({
      appendLine: (t) => outputLines.push(t),
      show() {},
      dispose() {},
    }),
    createTerminal: () => ({
      exitStatus: undefined,
      show() {},
      sendText: (t) => sentText.push(t),
      dispose() {},
    }),
    onDidCloseTerminal: () => disposable(),
    get activeTextEditor() {
      return activeEditor;
    },
    showWarningMessage: (m) => {
      messages.push(`warn: ${m}`);
      return Promise.resolve(undefined);
    },
    showInformationMessage: (m) => {
      messages.push(`info: ${m}`);
      return Promise.resolve(undefined);
    },
    showErrorMessage: (m) => {
      messages.push(`error: ${m}`);
      return Promise.resolve(undefined);
    },
    showInputBox: () => Promise.resolve(inputBoxValue),
    showQuickPick: (items) => Promise.resolve(items[0]),
    showTextDocument: () => Promise.resolve({}),
  },
  workspace: {
    getConfiguration: () => ({
      get: (key, def) => (key in config ? config[key] : def),
    }),
    openTextDocument: (opts) => {
      opened.push(opts);
      return Promise.resolve({ content: opts.content });
    },
    createFileSystemWatcher: () => disposable(),
    onDidChangeTextDocument: () => disposable(),
    workspaceFolders: undefined,
  },
  commands: {
    registerCommand: (id, cb) => {
      commands.set(id, cb);
      return disposable();
    },
    executeCommand: () => Promise.resolve(undefined),
  },
  languages: {
    registerDocumentFormattingEditProvider: (_sel, provider) => {
      formatter = provider;
      return disposable();
    },
  },
  debug: {
    registerDebugAdapterDescriptorFactory: (kind, factory) => {
      debugFactory = { kind, factory };
      return disposable();
    },
  },
};

class LanguageClient {
  constructor(id, name, server, client) {
    Object.assign(this, { id, name, server, client, running: false });
    clients.push(this);
  }
  start() {
    this.running = true;
    return Promise.resolve();
  }
  stop() {
    this.running = false;
    return Promise.resolve();
  }
}

const realLoad = Module._load;
Module._load = function (request, parent, isMain) {
  if (request === "vscode") return vscodeStub;
  if (lspMode && request === "vscode-languageclient/node") {
    return { LanguageClient };
  }
  return realLoad.call(this, request, parent, isMain);
};

function makeDoc(name, text, dirty) {
  const file = path.join(work, name);
  fs.writeFileSync(file, text);
  const lines = text.split("\n");
  return {
    languageId: "salam",
    isDirty: !!dirty,
    uri: { fsPath: file },
    lineCount: lines.length,
    getText: () => text,
    lineAt: (n) => ({ text: lines[n] }),
    save() {
      this.isDirty = false;
      return Promise.resolve(true);
    },
  };
}

function waitFor(pred, what) {
  const deadline = Date.now() + 10000;
  return new Promise((resolve, reject) => {
    const tick = () => {
      if (pred()) return resolve();
      if (Date.now() > deadline) return reject(new Error(`timed out: ${what}`));
      setTimeout(tick, 10);
    };
    tick();
  });
}

function q(s) {
  return /^[A-Za-z0-9_\-./:\\]+$/.test(s) ? s : `"${s}"`;
}

async function runCommand(id, doc, expectLast) {
  activeEditor = doc ? { document: doc } : undefined;
  sentText.length = 0;
  commands.get(id)();
  await waitFor(() => sentText.length >= 2, `${id} sends to the terminal`);
  assert.strictEqual(sentText[0], `cd ${q(work)}`);
  assert.strictEqual(sentText[1], expectLast);
}

const P = (line, character) => ({ line, character });
const R = (a, b, c, d) => ({ start: P(a, b), end: P(c, d) });

async function checkSymbolGuard(c) {
  const mw = c.client.middleware;
  assert.ok(mw && typeof mw.provideDocumentSymbols === "function");
  const doc = { uri: { fsPath: "x.salam" } };
  const good = [{ name: "ok" }];
  assert.strictEqual(
    await mw.provideDocumentSymbols(doc, {}, () => good),
    good,
  );
  const raw = [
    {
      name: "a",
      range: R(9, 15, 13, 10),
      selectionRange: R(13, 8, 13, 12),
      children: [
        {
          name: "kid",
          range: R(20, 0, 21, 0),
          selectionRange: R(30, 0, 30, 3),
        },
        { name: "neg", range: R(-1, 0, 2, 0), selectionRange: R(0, 0, 0, 1) },
        { name: "flip", range: R(5, 4, 5, 2), selectionRange: R(5, 2, 5, 3) },
      ],
    },
    { name: "b", range: R(1, 0, 2, 0), selectionRange: R(1, 0, 1, 1) },
    { range: R(1, 0, 2, 0) },
  ];
  c.sendRequest = async () => raw;
  c.code2ProtocolConverter = { asUri: (u) => u.fsPath };
  c.protocol2CodeConverter = { asDocumentSymbols: (x) => x };
  const out = await mw.provideDocumentSymbols(doc, {}, () =>
    Promise.reject(new Error("selectionRange must be contained in fullRange")),
  );
  const le = (a, b) =>
    a.line < b.line || (a.line === b.line && a.character <= b.character);
  const inside = (o, i) => le(o.start, i.start) && le(i.end, o.end);
  const walk = (list, parent) => {
    for (const s of list) {
      assert.ok(le(s.range.start, s.range.end), s.name);
      assert.ok(inside(s.range, s.selectionRange), s.name);
      if (parent) assert.ok(inside(parent, s.range), s.name);
      walk(s.children, s.range);
    }
  };
  walk(out, null);
  assert.deepStrictEqual(
    out.map((s) => s.name),
    ["a", "b"],
  );
  assert.deepStrictEqual(
    out[0].children.map((s) => s.name),
    ["kid"],
  );
}

async function main() {
  const ext = require(extPath);
  const memento = new Map();
  const context = {
    subscriptions: [],
    workspaceState: {
      get: (k) => memento.get(k),
      update: (k, v) => {
        memento.set(k, v);
        return Promise.resolve();
      },
    },
  };
  ext.activate(context);

  for (const id of [
    "salam.run",
    "salam.exec",
    "salam.build",
    "salam.format",
    "salam.check",
    "salam.emitTokens",
    "salam.emitAst",
    "salam.newProject",
    "salam.restartServer",
    "salam.showBlocks",
    "salam.selectPlatform",
  ]) {
    assert.ok(commands.has(id), `command ${id} is registered`);
  }

  if (lspMode) {
    await waitFor(() => clients.length === 1, "the language client starts");
    const c = clients[0];
    assert.strictEqual(c.id, "salam");
    assert.strictEqual(c.server.command, fakeCompiler);
    assert.deepStrictEqual([...c.server.args], ["lsp"]);
    assert.ok(c.running);
    await checkSymbolGuard(c);
    assert.strictEqual(debugFactory.kind, "salam");
    const exe = debugFactory.factory.createDebugAdapterDescriptor({});
    assert.strictEqual(exe.command, fakeCompiler);
    assert.deepStrictEqual([...exe.args], ["dap"]);
    commands.get("salam.restartServer")();
    await waitFor(() => clients.length === 2, "the language client restarts");
    assert.ok(!c.running && clients[1].running);
  } else {
    await waitFor(() => formatter !== null, "the CLI formatter is registered");
  }

  const en = makeDoc(
    "en.salam",
    "// lang: en\npackage main\n\nfunc main:\n  println 111\nend\n",
  );
  const fa = makeDoc("fa.salam", "// زبان: fa\nتابع اصلی:\n  چاپ ۱۱۱\nپایان\n");
  const plain = makeDoc(
    "plain.salam",
    "package main\n\nfunc main:\n  println 111\nend\n",
  );
  const dirty = makeDoc("dirty.salam", "// LANG: EN\nfunc main:\nend\n", true);
  const fc = q(fakeCompiler);

  const lang = (pack) => (pack ? ` ${q(`--lang=${pack}`)}` : "");
  for (const pack of ["", "en", "fa"]) {
    config.languagePack = pack || "auto";
    await runCommand(
      "salam.run",
      en,
      `${fc} exec ${q(en.uri.fsPath)}${lang(pack)}`,
    );
    await runCommand(
      "salam.run",
      fa,
      `${fc} exec ${q(fa.uri.fsPath)}${lang(pack)}`,
    );
    await runCommand(
      "salam.exec",
      plain,
      `${fc} exec ${q(plain.uri.fsPath)}${lang(pack)}`,
    );
    await runCommand(
      "salam.run",
      dirty,
      `${fc} exec ${q(dirty.uri.fsPath)}${lang(pack)}`,
    );
    config["run.command"] = "run";
    await runCommand(
      "salam.run",
      en,
      `${fc} run ${q(en.uri.fsPath)}${lang(pack)}`,
    );
    config["run.command"] = "exec";

    const exe = process.platform === "win32" ? ".exe" : "";
    await runCommand(
      "salam.build",
      en,
      `${fc} build ${q(en.uri.fsPath)} ${q(`--output=${path.join(work, `en${exe}`)}`)}${lang(pack)}`,
    );
  }
  config.languagePack = "auto";

  activeEditor = { document: en };
  messages.length = 0;
  commands.get("salam.check")();
  await waitFor(() => messages.length > 0, "salam.check reports");
  assert.deepStrictEqual(messages, ["info: Salam: no problems found."]);
  assert.ok(!calls("inspect")[0].some((x) => x.startsWith("--lang")));

  commands.get("salam.emitTokens")();
  commands.get("salam.emitAst")();
  await waitFor(() => opened.length >= 2, "the XML views open");
  assert.deepStrictEqual(opened, [
    { content: "<root/>", language: "xml" },
    { content: "<root/>", language: "xml" },
  ]);

  if (!lspMode) {
    const spaced = makeDoc(
      "spaced.salam",
      "// lang: en\nfunc main:\n  println   1\nend\n",
    );
    const edits = await formatter.provideDocumentFormattingEdits(spaced);
    assert.strictEqual(edits.length, 1);
    assert.strictEqual(
      edits[0].newText,
      "// lang: en\nfunc main:\n println 1\nend\n",
    );
    assert.deepStrictEqual(
      { ...edits[0].range },
      { sl: 0, sc: 0, el: 4, ec: 0 },
    );
    assert.ok(!calls("format")[0].some((x) => x.startsWith("--lang")));
    config.languagePack = "fa";
    await formatter.provideDocumentFormattingEdits(spaced);
    config.languagePack = "auto";
    assert.ok(calls("format")[1].includes("--lang=fa"));
  }

  activeEditor = undefined;
  sentText.length = 0;
  {
    const text = [
      "func main:",
      "    if true:",
      "        println 1",
      "    end",
      "end",
      "",
      "page:",
      "    box:",
      "        button:",
      "            content = \"x\"",
      "        end",
      "    end",
      "end",
    ].join("\n");
    const ed = makeDoc("blocks.salam", text);
    const block = statusItems[0];
    const at = (line) => {
      activeEditor = {
        document: ed,
        selection: { active: { line } },
        setDecorations() {},
      };
      for (const cb of selectionListeners) cb();
    };
    at(2);
    assert.ok(block.text.includes("func main \u203a if true"), block.text);
    at(9);
    assert.ok(block.text.includes("page \u203a box \u203a button"), block.text);
    at(5);
    assert.ok(block.text.includes("top level"), block.text);
  }
  {
    const proj = path.join(work, "proj");
    fs.mkdirSync(proj);
    fs.writeFileSync(
      path.join(proj, "salam.toml"),
      '[package]\nname = "p"\n\n[project]\ntype = "dsl"\nplatforms = ["web", "android"]\nweb = "spa"\n',
    );
    const main = path.join(proj, "main.salam");
    fs.writeFileSync(main, "page:\nend\n");
    activeEditor = {
      document: {
        languageId: "salam",
        isDirty: false,
        uri: { fsPath: main },
        getText: () => "page:\nend\n",
        lineCount: 2,
      },
      selection: { active: { line: 0 } },
      setDecorations() {},
    };
    sentText.length = 0;
    commands.get("salam.run")();
    await waitFor(() => sentText.length >= 2, "project run sends to the terminal");
    assert.strictEqual(sentText[0], `cd ${q(proj)}`);
    assert.strictEqual(sentText[1], `${fc} run ${q("--platform=web")}`);
    await commands.get("salam.selectPlatform")();
    sentText.length = 0;
    commands.get("salam.build")();
    await waitFor(() => sentText.length >= 2, "project build sends to the terminal");
    assert.strictEqual(sentText[1], `${fc} build ${q("--platform=web")}`);
  }

  sentText.length = 0;
  commands.get("salam.newProject")();
  await waitFor(() => sentText.length >= 1, "salam.newProject runs");
  assert.strictEqual(sentText[0], `${fc} init demo-app ${q("--lang=en")}`);

  messages.length = 0;
  activeEditor = undefined;
  commands.get("salam.run")();
  assert.deepStrictEqual(messages, ["warn: Salam: open a .salam file first."]);

  await new Promise((r) => setTimeout(r, 300));
  ext.deactivate();
}

main()
  .catch((e) => failures.push(e))
  .finally(() => {
    fs.rmSync(work, { recursive: true, force: true });
    if (failures.length > 0) {
      for (const f of failures) console.error(f && f.stack ? f.stack : f);
      console.error(`FAIL: ${failures.length} error(s) in ${extPath}`);
      process.exit(1);
    }
    console.log(`ok: ${extPath}`);
  });
