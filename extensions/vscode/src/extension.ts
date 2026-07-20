/*
 * Salam Programming Language - VS Code extension entry point.
 */

import * as cp from "node:child_process";
import * as fs from "node:fs";
import * as os from "node:os";
import * as path from "node:path";
import * as vscode from "vscode";

const LANG_ID = "salam";
const OUTPUT_NAME = "Salam";

interface CaptureResult {
  code: number;
  stdout: string;
  stderr: string;
  spawnError: ExecError | null;
}

type ExecError = Error & { code?: number | string };

let output!: vscode.OutputChannel;
let terminal: vscode.Terminal | undefined;

function cfg(): vscode.WorkspaceConfiguration {
  return vscode.workspace.getConfiguration("salam");
}

function compilerPath(): string {
  return cfg().get<string>("compilerPath", "salam") || "salam";
}

function shellQuote(arg: string): string {
  if (arg === "") return '""';
  if (/^[A-Za-z0-9_\-./:\\]+$/.test(arg)) return arg;
  return `"${String(arg).replace(/(["\\$`])/g, "\\$1")}"`;
}

function detectLang(document: vscode.TextDocument): string {
  const choice = cfg().get<string>("languagePack", "auto");
  if (choice === "en" || choice === "fa" || choice === "ar") return choice;

  const text = document.getText();
  const head = text.slice(0, 4000);

  const m = head.match(/(?:LANG|lang|زبان)\s*:\s*([A-Za-z]{2})/);
  if (m) return m[1].toLowerCase();

  if (/[پچژگکی۰-۹]/.test(head)) return "fa";
  const arabic = (head.match(/[؀-ۿݐ-ݿﭐ-﷿ﹰ-﻿]/g) || []).length;
  return arabic >= 3 ? "ar" : "en";
}

function getTerminal(): vscode.Terminal {
  if (!terminal || terminal.exitStatus !== undefined) {
    terminal = vscode.window.createTerminal(OUTPUT_NAME);
  }
  return terminal;
}

async function activeSalamDoc(): Promise<vscode.TextDocument | undefined> {
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

function commonArgs(document: vscode.TextDocument): string[] {
  const args = [`--lang=${detectLang(document)}`];
  const stdlib = cfg().get<string>("stdlibPath", "");
  if (stdlib) args.push(`--stdlib-path=${stdlib}`);
  return args;
}

function runInTerminal(args: string[], cwd?: string): void {
  const term = getTerminal();
  term.show(true);
  const cmd = [compilerPath(), ...args].map(shellQuote).join(" ");
  if (cwd) term.sendText(`cd ${shellQuote(cwd)}`);
  term.sendText(cmd);
}

async function cmdRunWith(subcommand: string): Promise<void> {
  const doc = await activeSalamDoc();
  if (!doc) return;
  const file = doc.uri.fsPath;
  runInTerminal([subcommand, file, ...commonArgs(doc)], path.dirname(file));
}

async function cmdBuild(): Promise<void> {
  const doc = await activeSalamDoc();
  if (!doc) return;
  const file = doc.uri.fsPath;
  const target = String(cfg().get("build.target", "")).trim();
  const wantsExe =
    process.platform === "win32" || /windows|mingw|win32/.test(target);
  const out = file.replace(/\.salam$/i, wantsExe ? ".exe" : "");
  const args = ["build", file, `--output=${out}`];
  if (target) args.push(`--target=${target}`);
  runInTerminal([...args, ...commonArgs(doc)], path.dirname(file));
}

function execCapture(args: string[], cwd?: string): Promise<CaptureResult> {
  return new Promise((resolve) => {
    cp.execFile(
      compilerPath(),
      args,
      { cwd, maxBuffer: 32 * 1024 * 1024 },
      (err, stdout, stderr) => {
        const e = err as ExecError | null;
        resolve({
          code: e && typeof e.code === "number" ? e.code : e ? 1 : 0,
          stdout: stdout || "",
          stderr: stderr || "",
          spawnError: e && e.code === "ENOENT" ? e : null,
        });
      },
    );
  });
}

function reportCompilerMissing(): void {
  vscode.window
    .showErrorMessage(
      `Salam: could not run '${compilerPath()}'. Set "salam.compilerPath" to the salam executable.`,
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

async function cmdEmitXml(flag: string, languageTitle: string): Promise<void> {
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

async function formatText(text: string, lang: string): Promise<string | null> {
  const tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), "salam-fmt-"));
  const tmpFile = path.join(tmpDir, "document.salam");
  try {
    fs.writeFileSync(tmpFile, text, "utf8");
    const args = ["format", tmpFile, `--lang=${lang}`];
    const indent = String(cfg().get("format.indent", "4")).trim();
    if (indent === "tab" || indent === "tabs") args.push("--tabs");
    else if (/^\d+$/.test(indent)) args.push(`--indent=${indent}`);

    const res = await execCapture(args, tmpDir);
    if (res.spawnError) {
      reportCompilerMissing();
      return null;
    }
    if (res.code !== 0) {
      const msg = (res.stderr || res.stdout).trim();
      output.appendLine(`[fmt] ${msg}`);
      vscode.window.showErrorMessage(
        'Salam: formatting failed (see "Salam" output for details).',
      );
      return null;
    }
    return fs.readFileSync(tmpFile, "utf8");
  } catch (e) {
    const err = e as Error;
    output.appendLine(`[fmt] ${err?.message ? err.message : String(e)}`);
    return null;
  } finally {
    try {
      fs.rmSync(tmpDir, { recursive: true, force: true });
    } catch {
      /* best-effort cleanup */
    }
  }
}

function wholeDocumentRange(document: vscode.TextDocument): vscode.Range {
  const last = document.lineCount - 1;
  return new vscode.Range(0, 0, last, document.lineAt(last).text.length);
}

const formattingProvider: vscode.DocumentFormattingEditProvider = {
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

async function cmdFormat(): Promise<void> {
  const editor = vscode.window.activeTextEditor;
  if (!editor || editor.document.languageId !== LANG_ID) {
    vscode.window.showWarningMessage("Salam: open a .salam file first.");
    return;
  }
  await vscode.commands.executeCommand("editor.action.formatDocument");
}

interface LangPickItem extends vscode.QuickPickItem {
  value: string;
}

async function cmdNewProject(): Promise<void> {
  const name = await vscode.window.showInputBox({
    prompt: "Project name",
    placeHolder: "my-salam-app",
    validateInput: (v) =>
      /^[\w.-]+$/.test(v)
        ? null
        : "Use letters, digits, dots, hyphens or underscores.",
  });
  if (!name) return;

  const langPick = await vscode.window.showQuickPick<LangPickItem>(
    [
      { label: "English (en)", value: "en" },
      { label: "فارسی (fa)", value: "fa" },
      { label: "العربية (ar)", value: "ar" },
    ],
    { placeHolder: "Keyword language pack for the new project" },
  );
  if (!langPick) return;

  const folders = vscode.workspace.workspaceFolders;
  const cwd = folders?.length ? folders[0].uri.fsPath : os.homedir();
  runInTerminal(["new", name, `--lang=${langPick.value}`], cwd);
}

export function activate(context: vscode.ExtensionContext): void {
  output = vscode.window.createOutputChannel(OUTPUT_NAME);
  context.subscriptions.push(output);

  context.subscriptions.push(
    vscode.commands.registerCommand("salam.run", () =>
      cmdRunWith(cfg().get<string>("run.command", "exec")),
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

export function deactivate(): void {
  if (terminal) terminal.dispose();
}
