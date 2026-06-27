/*
 * Salam Programming Language - VS Code extension entry point.
 */
"use strict";

const vscode = require("vscode");
const cp = require("child_process");
const os = require("os");
const fs = require("fs");
const path = require("path");

const LANG_ID = "salam";
const OUTPUT_NAME = "Salam";

/** @type {vscode.OutputChannel} */
let output;
/** @type {vscode.Terminal | undefined} */
let terminal;

function cfg() {
  return vscode.workspace.getConfiguration("salam");
}

function compilerPath() {
  return cfg().get("compilerPath", "salamc") || "salamc";
}

function shellQuote(arg) {
  if (arg === "") return '""';
  if (/^[A-Za-z0-9_\-./:\\]+$/.test(arg)) return arg;
  return '"' + String(arg).replace(/(["\\$`])/g, "\\$1") + '"';
}

function detectLang(document) {
  const choice = cfg().get("languagePack", "auto");
  if (choice === "en" || choice === "fa") return choice;

  const text = document.getText();
  const head = text.slice(0, 4000);

  const m = head.match(/(?:LANG|lang)\s*:\s*([A-Za-z]{2})/);
  if (m) return m[1].toLowerCase();

  const persian = (text.match(/[؀-ۿݐ-ݿﭐ-﷿ﹰ-﻿]/g) || []).length;
  return persian >= 3 ? "fa" : "en";
}

function getTerminal() {
  if (!terminal || terminal.exitStatus !== undefined) {
    terminal = vscode.window.createTerminal(OUTPUT_NAME);
  }
  return terminal;
}

async function activeSalamDoc() {
  const editor = vscode.window.activeTextEditor;
  if (!editor || editor.document.languageId !== LANG_ID) {
    vscode.window.showWarningMessage("Salam: open a .salam file first.");
    return undefined;
  }
  if (editor.document.isDirty) {
    await editor.document.save();
  }
  return editor.document;
}

function commonArgs(document) {
  const args = ["--lang=" + detectLang(document)];
  const stdlib = cfg().get("stdlibPath", "");
  if (stdlib) args.push("--stdlib-path=" + stdlib);
  return args;
}

function runInTerminal(args, cwd) {
  const term = getTerminal();
  term.show(true);
  const cmd = [compilerPath(), ...args].map(shellQuote).join(" ");
  if (cwd) term.sendText("cd " + shellQuote(cwd));
  term.sendText(cmd);
}

async function cmdRunWith(subcommand) {
  const doc = await activeSalamDoc();
  if (!doc) return;
  const file = doc.uri.fsPath;
  runInTerminal([subcommand, file, ...commonArgs(doc)], path.dirname(file));
}

async function cmdBuild() {
  const doc = await activeSalamDoc();
  if (!doc) return;
  const file = doc.uri.fsPath;
  const out = file.replace(
    /\.salam$/i,
    process.platform === "win32" ? ".exe" : "",
  );
  runInTerminal(
    ["build", file, "--output=" + out, ...commonArgs(doc)],
    path.dirname(file),
  );
}

function execCapture(args, cwd) {
  return new Promise((resolve) => {
    cp.execFile(
      compilerPath(),
      args,
      { cwd, maxBuffer: 32 * 1024 * 1024 },
      (err, stdout, stderr) => {
        resolve({
          code: err && typeof err.code === "number" ? err.code : err ? 1 : 0,
          stdout: stdout || "",
          stderr: stderr || "",
          spawnError: err && err.code === "ENOENT" ? err : null,
        });
      },
    );
  });
}

function reportCompilerMissing() {
  vscode.window
    .showErrorMessage(
      `Salam: could not run '${compilerPath()}'. Set "salam.compilerPath" to the salamc executable.`,
      "Open Settings",
    )
    .then((pick) => {
      if (pick === "Open Settings") {
        vscode.commands.executeCommand(
          "workbench.action.openSettings",
          "salam.compilerPath",
        );
      }
    });
}

async function cmdEmitXml(flag, languageTitle) {
  const doc = await activeSalamDoc();
  if (!doc) return;
  const file = doc.uri.fsPath;
  const res = await execCapture(
    [file, flag, ...commonArgs(doc)],
    path.dirname(file),
  );
  if (res.spawnError) return reportCompilerMissing();
  const body = res.stdout.trim() || res.stderr.trim() || "(no output)";
  const shown = await vscode.workspace.openTextDocument({
    content: body,
    language: "xml",
  });
  await vscode.window.showTextDocument(shown, { preview: true });
  if (res.code !== 0 && res.stderr.trim()) {
    output.appendLine(
      `[${languageTitle}] exited ${res.code}: ${res.stderr.trim()}`,
    );
  }
}

async function formatText(text, lang) {
  const tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), "salam-fmt-"));
  const tmpFile = path.join(tmpDir, "document.salam");
  try {
    fs.writeFileSync(tmpFile, text, "utf8");
    const args = ["fmt", tmpFile, "--lang=" + lang];
    const indent = String(cfg().get("format.indent", "4")).trim();
    if (indent === "tab" || indent === "tabs") args.push("--tabs");
    else if (/^\d+$/.test(indent)) args.push("--indent=" + indent);

    const res = await execCapture(args, tmpDir);
    if (res.spawnError) {
      reportCompilerMissing();
      return null;
    }
    if (res.code !== 0) {
      const msg = (res.stderr || res.stdout).trim();
      output.appendLine("[fmt] " + msg);
      vscode.window.showErrorMessage(
        'Salam: formatting failed (see "Salam" output for details).',
      );
      return null;
    }
    return fs.readFileSync(tmpFile, "utf8");
  } catch (e) {
    output.appendLine("[fmt] " + (e && e.message ? e.message : String(e)));
    return null;
  } finally {
    try {
      fs.rmSync(tmpDir, { recursive: true, force: true });
    } catch (_) {}
  }
}

function wholeDocumentRange(document) {
  const last = document.lineCount - 1;
  return new vscode.Range(0, 0, last, document.lineAt(last).text.length);
}

/** @type {vscode.DocumentFormattingEditProvider} */
const formattingProvider = {
  async provideDocumentFormattingEdits(document) {
    if (!cfg().get("format.enable", true)) return [];
    const formatted = await formatText(
      document.getText(),
      detectLang(document),
    );
    if (formatted == null || formatted === document.getText()) return [];
    return [vscode.TextEdit.replace(wholeDocumentRange(document), formatted)];
  },
};

async function cmdFormat() {
  const editor = vscode.window.activeTextEditor;
  if (!editor || editor.document.languageId !== LANG_ID) {
    vscode.window.showWarningMessage("Salam: open a .salam file first.");
    return;
  }
  await vscode.commands.executeCommand("editor.action.formatDocument");
}

async function cmdNewProject() {
  const name = await vscode.window.showInputBox({
    prompt: "Project name",
    placeHolder: "my-salam-app",
    validateInput: (v) =>
      /^[\w.-]+$/.test(v)
        ? null
        : "Use letters, digits, dots, hyphens or underscores.",
  });
  if (!name) return;

  const langPick = await vscode.window.showQuickPick(
    [
      { label: "English (en)", value: "en" },
      { label: "فارسی (fa)", value: "fa" },
    ],
    { placeHolder: "Keyword language pack for the new project" },
  );
  if (!langPick) return;

  const folders = vscode.workspace.workspaceFolders;
  const cwd = folders && folders.length ? folders[0].uri.fsPath : os.homedir();
  runInTerminal(["new", name, "--lang=" + langPick.value], cwd);
}

function activate(context) {
  output = vscode.window.createOutputChannel(OUTPUT_NAME);
  context.subscriptions.push(output);

  context.subscriptions.push(
    vscode.commands.registerCommand("salam.run", () =>
      cmdRunWith(cfg().get("run.command", "exec")),
    ),
    vscode.commands.registerCommand("salam.exec", () => cmdRunWith("exec")),
    vscode.commands.registerCommand("salam.build", cmdBuild),
    vscode.commands.registerCommand("salam.format", cmdFormat),
    vscode.commands.registerCommand("salam.emitTokens", () =>
      cmdEmitXml("--emit-tokens-xml", "tokens"),
    ),
    vscode.commands.registerCommand("salam.emitAst", () =>
      cmdEmitXml("--emit-ast-xml", "ast"),
    ),
    vscode.commands.registerCommand("salam.newProject", cmdNewProject),
    vscode.languages.registerDocumentFormattingEditProvider(
      { language: LANG_ID, scheme: "file" },
      formattingProvider,
    ),
    vscode.window.onDidCloseTerminal((t) => {
      if (t === terminal) terminal = undefined;
    }),
  );
}

function deactivate() {
  if (terminal) terminal.dispose();
}

module.exports = { activate, deactivate };
