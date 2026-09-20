// Tokenize the probe files with vscode-textmate and vscode-oniguruma, the
// engine VS Code itself runs, and report the scope each token is given.
//
// Usage: node extensions/tools/check_textmate.mjs <grammar.json> <file...>

import { readFile } from "node:fs/promises";
import { createRequire } from "node:module";
import { join } from "node:path";
import { pathToFileURL } from "node:url";

// The two packages are not repository dependencies; CI installs them into a
// scratch directory and points SALAM_TEXTMATE_MODULES at its parent.
const installed = process.env.SALAM_TEXTMATE_MODULES;
const require = createRequire(
  installed ? pathToFileURL(join(installed, "resolve.js")).href : import.meta.url,
);
const { Registry, parseRawGrammar, INITIAL } = require("vscode-textmate");
const oniguruma = require("vscode-oniguruma");

const [grammarPath, ...sources] = process.argv.slice(2);

const wasm = await readFile(require.resolve("vscode-oniguruma/release/onig.wasm"));
await oniguruma.loadWASM(wasm.buffer);

const registry = new Registry({
  onigLib: Promise.resolve({
    createOnigScanner: (patterns) => new oniguruma.OnigScanner(patterns),
    createOnigString: (text) => new oniguruma.OnigString(text),
  }),
  loadGrammar: async (scope) => {
    if (scope !== "source.salam") return null;
    const text = await readFile(grammarPath, "utf8");
    return parseRawGrammar(text, grammarPath);
  },
});

const grammar = await registry.loadGrammar("source.salam");
if (!grammar) {
  console.error("FAIL: the engine could not load source.salam");
  process.exit(1);
}

const output = {};
for (const source of sources) {
  const text = await readFile(source, "utf8");
  let stack = INITIAL;
  const tokens = [];
  for (const line of text.split("\n")) {
    const result = grammar.tokenizeLine(line, stack);
    for (const token of result.tokens) {
      const piece = line.slice(token.startIndex, token.endIndex);
      if (piece.trim()) {
        tokens.push([piece.trim(), token.scopes[token.scopes.length - 1]]);
      }
    }
    stack = result.ruleStack;
  }
  output[source] = tokens;
}

console.log(JSON.stringify(output));
