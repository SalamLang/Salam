# Design: `salam lsp`, the Salam language server

Status: **phase 1 and most of phase 2 implemented** on the `feat/lsp`
branch (2026-10-22). `salam lsp` runs today in Visual Studio Code, Neovim,
Emacs, Sublime Text, Kate, Helix and JetBrains IDEs (through LSP4IJ); the
user guide is [`docs/LSP.md`](../LSP.md) and the end-to-end tests are
`tools/lsp/test_lsp.py`. What is built and what is left is listed under
[Progress](#progress) at the end.

The bar is GoLand: diagnostics as you type, completion that knows types,
navigation across packages and `std/`, safe rename, quick fixes for every
common mistake, and refactorings. Salam has one thing no other language
server has to handle, which is a second keyword language written right to
left, and this design treats that as a feature to get right rather than an
edge case.

## Where we are today

Every editor integration runs the `salam` binary once per action and reads
its output. Nothing keeps state between keystrokes.

| What exists | Where | Useful for |
| --- | --- | --- |
| Syntax highlighting for 11 editors, generated from the compiler's word lists | `extensions/`, `extensions/tools/gen_syntaxes.salam` | stays as the fallback when no server runs |
| JSON diagnostics, one object per line: `level, code, message, file, line, column, length, help` | `compiler/diag_render.salam` `RenderJson`, `--error-format=json` | the shape `publishDiagnostics` maps onto |
| Neovim diagnostics via `salam inspect --error-format=json` | `extensions/vim/lua/salam/init.lua` | the closest thing to an LSP client today |
| Whole-file formatter, token based | `compiler/sal_fmt.salam` `Source`, `ReorderToplevel` | `textDocument/formatting` |
| In-memory pipeline: lex, parse, condcomp prune, layout expand, sema | `compiler/sal_web.salam` `front_end` | the template for analysing an unsaved buffer |
| Doc comment extraction with `@en` / `@fa` sections | `compiler/docgen/` | hover and completion docs |
| Did-you-mean suggestions | `compiler/semantic/sema_suggest.salam` | quick fixes |
| `Content-Length` framing and JSON-RPC 2.0 codecs | `std/rpc/rpc_framing.salam`, `std/rpc/jsonrpc.salam` | the transport, as is |
| A Salam JSON-RPC server with tools, docs and a std index | `tools/mcp/` | patterns to copy; the std scanner in `mcp_api.salam` |
| Keyword spellings per language, Persian identifier equality | `compiler/langpack.salam` `Spelling`, `IdentEqual` | completion, rename |

What is missing in the compiler, and blocks a real server:

1. **Identifiers are not linked to symbols.** `sema_check_expr_body` resolves
   an `AST_IDENTIFIER` through `scope_lookup` and throws the result away. Only
   overloaded calls keep `ovl_decl`. Go to definition and find references need
   this link.
2. **No map from a position to a scope.** Scopes live in an arena with parent
   links, but nothing records which scope covers which byte range, so
   completion cannot ask "what is visible here".
3. **Lexer and parser errors are log text.** They go through `lg.LogAt` with
   code 0 and only a start position, bypassing `dg.Engine`. `Diag` itself
   drops the help text and has no room for fixes or related locations.
4. **Packages are read from disk.** `sema_packages.salam` always calls
   `src.Load`, so an unsaved edit in one file is invisible to the files that
   import it.
5. **Global state is never reset.** `g_sm_*`, `g_lay_*`, `g_lx_intern`,
   `g_src_loaded`, `g_i18n_lang` and the logger capture buffer assume one
   compilation per process.
6. **A run leaks.** The self-host leak budget is still tens of thousands of
   allocations per compile; a resident process that recompiles on every
   keystroke would grow without bound.
7. **Some end spans are missing.** `p_fin` is not called on every rule, so
   some nodes have a start and no end.

Measured with 0.5.7 on a loaded machine (CPU time, which is the honest number
here): `check` of hello world costs about 190 ms, most of it opening ~1,100
files under `std/`; a std-heavy test (`tests/en/stdlib/opensearch/knn.salam`)
costs about 800 ms; the compiler itself (`compiler/main.salam`) takes seconds.
A server that reruns `salam check` per keystroke would feel broken, so the
design below keeps the expensive parts warm.

## Decisions

### One binary: `salam lsp`

The server is a subcommand of the compiler, written in Salam, living in
`compiler/lsp/`. Every editor configuration is the same one line: run
`salam lsp` over stdio. This has three consequences that matter:

- The server can never disagree with the compiler, because it *is* the
  compiler. Version skew between an extension and the toolchain disappears.
- Installing Salam (`install.sh`, `install.ps1`, Homebrew, the Android and
  desktop packages) installs the server. Editor plugins stay thin.
- It is the strongest dogfooding the language can get: a long-running,
  concurrent, JSON-heavy Salam program that people use every day.

Transport is stdio by default, plus `--socket=PORT` and `--pipe=NAME` for
clients that want them (JetBrains and Visual Studio Code both accept stdio).

### A supervisor and a worker

Because of points 5 and 6 above, analysis does not run in the process that
talks to the editor.

```
editor  <--stdio-->  salam lsp            (supervisor)
                       | protocol, document store, std index,
                       | request queue, cancellation, debounce
                       |
                       +--pipe-->  salam lsp --worker   (analysis)
                                     lex, parse, sema, index
                                     warm std package cache
```

- The **supervisor** owns everything that must survive: open documents and
  their versions, client capabilities, configuration, the workspace and std
  symbol indices, and the latest *snapshot* of each file. It never runs sema.
  Its allocations are bounded and it can be held to a zero-leak budget.
- The **worker** runs the compiler frontend. It keeps parsed std packages
  warm between requests, which removes most of the fixed cost. It is recycled
  when its RSS passes a ceiling (default 1 GiB) or after N analyses, and
  restarted if it crashes. A compiler crash on half-typed code costs one
  analysis, not the editor session.
- Supervisor and worker talk over the same `Content-Length` framing from
  `std/rpc`, with a small private message set (`analyze`, `query`, `cancel`).

Once the leak work in `docs/design/OWNERSHIP.md` and the global-state reset
land, the worker can run as a thread instead of a process. The protocol
between them does not change.

### Snapshots, not live state

Every analysis produces an immutable **snapshot** for one document version:
the AST, the decorated types, the symbol links, the scope map, the token
list with original byte spans, and the diagnostics. Queries (hover,
definition, completion) are answered from the latest snapshot, never by
waiting for a new analysis.

When the document has moved on since the snapshot, positions are mapped
through the edits in between. If the requested position falls inside an
edited range, the server answers with what it can (keywords, local words,
the last good scope) rather than blocking. Diagnostics are always published
with the document `version` they were computed for, so a client never paints
stale squiggles on new text.

Edits are debounced (default 150 ms) before an analysis starts. A newer edit
cancels the running analysis: the supervisor tells the worker to stop, and
kills and restarts it if the analysis does not stop within 200 ms.

### Positions: bytes inside, negotiated outside

The compiler counts in UTF-8 bytes (`Token.begin_index`, one column per
byte). LSP clients default to UTF-16 code units. The server:

- offers `positionEncoding: "utf-8"` when the client lists it in
  `general.positionEncodings` (LSP 3.17; Neovim, Helix and Zed do), and
  converts nothing;
- otherwise uses UTF-16, converting through a per-document line index built
  once per version (line start offsets plus a lazily built UTF-16 column
  table for lines that contain non-ASCII text).

Ranges always come from `begin_index` / `end_index`, never from
`lexeme.len()`. The lexer normalises Persian identifiers (ZWNJ to space,
Arabic yeh and kaf to Persian, harakat removed, built-in words
canonicalised), so the lexeme is often not the text on screen.

### Configuration

All settings live under `salam.*`, pulled with `workspace/configuration` and
refreshed on `workspace/didChangeConfiguration`:

| Setting | Default | Meaning |
| --- | --- | --- |
| `salam.path` | `salam` on PATH | compiler to run as the server (read by the client) |
| `salam.stdlibPath` | compiler's own resolution | same as `--stdlib-path` |
| `salam.lang` | `auto` | `en`, `fa` or `auto` (marker comment, then keyword count, as `lx.Detect` does) |
| `salam.messages` | client `locale` | language of diagnostic messages and hover text: `en` or `fa` |
| `salam.target` | host triple | which `if SALAM_OS_X:` branches are live |
| `salam.defines` | `[]` | same as `-D`, for conditional compilation |
| `salam.ownership` | `warn` | ownership checker mode: `off`, `warn`, `on` |
| `salam.format.indent` | `4` | `tab` or 1-16, same as `--indent` |
| `salam.format.fixOrder` | `true` | let formatting reorder top-level declarations |
| `salam.inlayHints.*` | see below | per hint kind |
| `salam.worker.memoryLimitMb` | `1024` | worker recycle threshold |

## Feature set

Everything in LSP 3.17 that makes sense for Salam, and what "done" means for
each. The phase column refers to the roadmap at the end.

### Lifecycle and synchronisation (phase 1)

- `initialize` / `initialized` / `shutdown` / `exit`, with honest capability
  advertising: a feature is advertised only once it works.
- `textDocument/didOpen`, `didChange` with **incremental** sync, `didClose`,
  `didSave`, `willSaveWaitUntil` (format on save without a round trip).
- `workspace/didChangeWorkspaceFolders`: several roots, each a Salam project
  (a folder with `salam.salam`, or any folder of `.salam` files).
- `workspace/didChangeWatchedFiles`: the client watches `**/*.salam` and
  `salam.cfg` for us; `std/` has no file watcher of its own, and needs none.
- `$/cancelRequest`, `$/setTrace`, `window/logMessage`, and
  `window/workDoneProgress` for the first std index build and workspace
  scans.

### Diagnostics (phase 1, pull in phase 4)

- `textDocument/publishDiagnostics` after every analysis, with the document
  version.
- `textDocument/diagnostic` and `workspace/diagnostic` (pull model) once the
  workspace index exists, so files that are not open still show their errors
  in the problems panel. JetBrains and Visual Studio Code both prefer pull.
- Each diagnostic carries:
  - `code`: always the ASCII form (`E085`, `W124`), even in Persian mode,
    where the human message is Persian. Today `code_str` localises the code
    itself; the server asks for the English code and the Persian message
    separately.
  - `codeDescription.href`: a page per code on the site,
    `https://salamlang.github.io/Salam/errors/E085/`, generated from the same
    text `salam_docs` serves for `E085`.
  - `source: "salam"`, `severity` from the level.
  - `tags`: `Unnecessary` for unused imports, functions and parameters (they
    are errors in Salam, but the fade still helps), `Deprecated` where std
    marks a symbol.
  - `relatedInformation`: the first declaration for "already declared", the
    import for "unused import", the previous definition for overload clashes.
  - `data`: the fix IDs for the code actions below, so `codeAction` needs no
    second analysis.
- The ownership checker's findings (`sema_own.salam`) are reported at the
  configured `salam.ownership` level.
- Code inside an inactive `if SALAM_OS_X:` branch gets no diagnostics,
  matching what the compiler does for the current `salam.target`.

### Semantic tokens (phase 1)

`textDocument/semanticTokens/full`, `/full/delta` and `/range`. This matters
more for Salam than for most languages, because TextMate grammars cannot
highlight it correctly:

- Persian names can contain spaces and ZWNJ (`اندازه مثبت`), and keywords
  can be made of several words; a regular expression grammar cannot tell a two-word name
  from a name followed by a keyword.
- `page` / `صفحه` is a soft keyword, and layout elements and attributes only
  exist inside layout blocks.
- Inactive conditional-compilation branches should look inactive.

Token types: `keyword`, `type`, `struct`, `interface`, `enum`, `enumMember`,
`function`, `method`, `parameter`, `variable`, `property`, `namespace` (a
package), `string`, `number`, `comment`, `operator`, plus custom
`layoutElement` and `layoutAttribute`. Modifiers: `declaration`, `readonly`
(an immutable `:=` binding), `mutable` (custom, for `mut`), `defaultLibrary`
(from `std/`), `deprecated`, `outParameter` (custom, for `&:`), and
`inactive` (custom, for a pruned branch).

### Navigation (phase 1 local, phase 2 everywhere)

- `textDocument/definition`, `declaration`, `typeDefinition` (the struct or
  enum behind a value), `implementation` (the `impl` blocks and structs that
  satisfy an interface, and the interface methods a struct implements).
- `textDocument/references` and `documentHighlight` (read and write kinds;
  an assignment or `&:` argument is a write).
- Navigation into `std/` opens the real std source, read-only.
- `include util "util.salam"` and `import encoding.json` are
  `textDocument/documentLink`s to the file or package folder. URLs in
  comments and strings are links too.
- `textDocument/documentSymbol`, hierarchical: package, imports, constants,
  globals, structs with fields and methods, interfaces, enums with members,
  functions, layout pages and components. The names shown are the ones the
  file is written in; the `@en` / `@fa` alias goes in `detail`.
- `workspace/symbol` across the workspace and `std/`, matching either
  language's spelling, with `workspaceSymbol/resolve` filling in the
  location lazily.
- `callHierarchy` (incoming and outgoing calls) and `typeHierarchy`
  (interfaces and the structs that implement them).

### Hover (phase 1 types, phase 2 docs)

`textDocument/hover` shows, in Markdown:

- the declaration as Salam would print it: `func Add(a: int, b: int): int`,
  `mut count: int`, `struct Point`, with `&:` out parameters marked;
- the inferred type for any expression (from `Node.sema_type`), so hovering
  `x` in `x := json.Decode(s)` shows what `x` is;
- the doc comment, using the `@en` or `@fa` section matching
  `salam.messages`, and falling back to the other one;
- the symbol's other-language name when it has one (its `@en` / `@fa`
  alias), and for a keyword its spelling in the other language (`println` /
  `سرچاپ`);
- for std symbols, a link to the site page for that package;
- for an error code under the cursor (in a `// E085` style comment, or in the
  problems panel), the explanation.

### Completion (phase 2)

`textDocument/completion` with `completionItem/resolve` for documentation.
The context decides what is offered:

| Context | Items |
| --- | --- |
| statement start | keywords of the file's language (`each`, `repeat`, `until` or their Persian spellings), locals, globals, functions, snippets |
| after `value.` | fields and methods of the value's type, including `Vector` / `HashMap` / `str` intrinsics, `pub` only outside the struct |
| after `pkg.` | the package's `pub` symbols |
| after `:` in a declaration, after `as`, inside `<...>` | types |
| `import` line | std package paths (`encoding.json`), from the std index |
| inside a `page` or `component` block | layout elements and attributes from the layout schema (`sm.LayoutSchemaInit`), with their Persian aliases |
| `match` arm | the remaining enum members |
| struct literal `P { ... }` | the fields not set yet |

Details that make it feel like GoLand rather than a word list:

- **Auto-import.** Choosing `json.Decode` when `encoding.json` is not
  imported adds the `import` with `additionalTextEdits`, placed where the
  top-level order rules allow it.
- **Snippets for blocks**, because every Salam block ends with `end`:
  `if ${1:cond}:\n\t$0\nend`, `each ${1:x} in ${2:v}:`, `func ${1:name}(...)`,
  `switch` with `break` already in each arm (the fallthrough rule is the
  single most surprising thing in the language).
- **Both spellings match.** `filterText` carries the English and Persian
  names, and a Persian name without diacritics and with either yeh, so a std function can be found by either of its names in a file
  of either language, and the item inserts the spelling that file uses.
- Reserved words are never offered as names (`by`, `pure`, `in`, `type` and
  the rest), so completion never suggests something that will not compile.
- Commit characters: `.`, `(`, `:`.

### Signature help (phase 2)

`textDocument/signatureHelp`, triggered on `(` and `,`, with every overload
of the callee, the active parameter, `&:` out parameters marked, and the doc
comment of each overload.

### Inlay hints (phase 4)

`textDocument/inlayHint` with `inlayHint/resolve`, each kind behind its own
setting:

- the inferred type after `x := ...` (`salam.inlayHints.types`);
- parameter names at call sites when the argument is a literal
  (`salam.inlayHints.parameterNames`);
- `&` at call sites for out parameters, so a mutation is visible
  (`salam.inlayHints.outParameters`);
- the element type of `each` variables (`salam.inlayHints.eachTypes`);
- `ret 0` at the end of a `func main` that omits it, off by default.

### Code actions (phase 3)

`textDocument/codeAction` with `codeAction/resolve`. Every quick fix comes
from a diagnostic's `data`, so it costs no second analysis.

Quick fixes:

- Did you mean `X`? From `sema_suggest.salam`, for unknown names, fields,
  methods and packages.
- Add the missing `import` for an unknown package prefix.
- Remove an unused variable, import, parameter or function, or prefix it with
  `_` where that is allowed.
- Drop a `mut` that is never reassigned.
- Add `pub` to a field or method used outside its struct.
- Reorder top-level declarations to satisfy E087 and the file-order rules
  (`ReorderToplevel`).
- Add the `break` an arm is missing in a `switch` that falls through.
- Rewrite foreign syntax the parser already recognises: `for` / `while` to
  `each` / `repeat` / `until`, `return` to `ret`, `elif` to `else`,
  `let` / `var` to `:=` / `mut`, `=>` to `:`, `def` / `class` to `func` /
  `struct`. `p_foreign_error` already knows each case; it gains a fix.
- Add `defer x.free()` where the ownership checker reports a leak, and remove
  a second `free` it reports as a double free.

Refactorings (`refactor.*`):

- Extract a local variable, extract a function.
- Inline a variable.
- Convert an `if` / `else` chain over one value into a `switch` with comma
  labels, and back.
- Convert `switch` to `match` for enums.
- Translate the file or the selection between English and Persian Salam
  (`salam translate`), with a preview.

Source actions:

- `source.organizeImports`: sort, drop unused.
- `source.fixAll.salam`: every fix that is safe to apply without asking.
- `source.reorder.salam`: the formatter's declaration reordering on its own.

### Rename (phase 3)

`textDocument/prepareRename` and `textDocument/rename` across the workspace:

- refuses std symbols, keywords and reserved words, and names that would
  collide in the target scope (checked by re-running sema on the result
  before answering);
- finds references by symbol, not by text, so the Persian spellings that the
  lexer treats as one name (`lp.IdentEqual`: ZWNJ or space, Arabic or Persian
  yeh) are all renamed, each written in the form the user typed;
- renames the `@en` / `@fa` alias when the user renames through it, and
  offers to rename the other spelling too;
- renames a file when an `include` path is renamed, through
  `workspace/willRenameFiles` / `didRenameFiles`, and updates the `include`
  lines when a file is renamed in the editor.

### Formatting (phase 1, range in phase 3)

- `textDocument/formatting` through `sal_fmt.Source` on the in-memory
  buffer, returned as a minimal set of edits (a line diff, not one
  whole-file replacement, so the cursor and undo history survive).
- `textDocument/rangeFormatting`: format the top-level declarations that
  overlap the range, then return only the edits inside it.
- `textDocument/onTypeFormatting` on newline and on typing `end`: indent
  after a line ending in `:`, dedent a line that is just `end` (or `پایان`).
- A file with lexical errors is not formatted, and the server says why
  instead of silently doing nothing.

### Structure (phase 1)

- `textDocument/foldingRange`: every block from `:` to its `end`, `extern:`
  blocks, layout blocks, runs of `//` comments, the import section.
- `textDocument/selectionRange`: expand selection along the AST (identifier,
  call, statement, block, function).
- `textDocument/linkedEditingRange` for layout element names where an element
  has a closing name.

### Code lens (phase 4)

`textDocument/codeLens` with `codeLens/resolve`:

- `Run` and `Debug` above `func main` / `ریشه`;
- `Run test` above each test in a `tests/` style file, with the output
  compared against its `.out` file;
- reference counts above `pub` declarations, resolved lazily;
- `Preview` above a `page` block, opening the rendered HTML.

### Colours (phase 4)

`textDocument/documentColor` and `colorPresentation` for colours in layout
style attributes, so the editor shows a swatch and a picker.

### Commands (phase 2 onward)

`workspace/executeCommand`, for clients to put in menus:

- `salam.run`, `salam.exec`, `salam.build`, `salam.test`;
- `salam.translate` (`en` / `fa`, file or selection);
- `salam.showTokens`, `salam.showAst`, `salam.showC`, `salam.showLlvmIr`,
  `salam.showJs`, opened as virtual documents;
- `salam.explainError` for a code;
- `salam.restartWorker`.

### Salam extensions to the protocol

Kept small and optional; a client that ignores them loses nothing else.

- `salam/inspect`: tokens, AST or symbols for a document, as JSON, for the
  "show AST" views and for tooling.
- `salam/explain`: the long explanation of an error code, as Markdown.
- `salam/layoutPreview`: the HTML a `page` block renders to, for a live
  preview panel.
- `salam/translate`: a translated copy of a document, without writing it.

## Persian Salam

Every feature above works the same in a Persian file. Specifically:

- **Language per file.** Detection follows `lx.Detect`: a `// lang: fa` /
  `// زبان: فارسی` marker in the first 8 lines wins, then keyword counts.
  Imported packages are detected on their own, as the compiler does.
- **Messages in the user's language.** `initialize.locale` (or
  `salam.messages`) picks `en` or `fa` for diagnostic messages, hover text
  and completion details, through `i18n.Tr`. Codes stay ASCII.
- **Identifiers are matched the way the compiler matches them**, through
  `lp.IdentEqual`, so search, references and rename agree with sema.
- **Edits keep the user's spelling.** A quick fix or completion inserts the
  spelling of the file's language, and a rename writes what the user typed.
- **No bidi control characters in output.** Hover text, snippets and edits
  never insert LRO/RLO or similar marks; direction is the editor's job.
- **Persian digits** (`۱۲۳`) are numbers to semantic tokens and to hover.

## Compiler changes this needs

These are the phase 0 tasks. Each one is useful on its own (the CLI,
`salam check` and the MCP server get better too).

1. **Link identifiers to symbols.** Add `Node.sym` and set it at every
   `scope_lookup` site in sema (identifiers, member access, calls, types,
   struct literals, enum members, layout component references). Record
   references per symbol in the snapshot index.
2. **Scope ranges.** Record each scope's byte range when it is pushed, so a
   position resolves to the innermost scope and its visible symbols.
3. **Structured diagnostics everywhere.** Route lexer and parser errors
   through `dg.Engine` with full spans and real codes. Give `Diag` `help`,
   `related: Vector<Related>` and `fixes: Vector<Fix>` (a fix is a set of
   byte-range text edits plus a title). `RenderJson` gains the new fields;
   the human renderer shows fixes as "help: try ...".
4. **Source overlay.** `src.Load` checks an overlay map (path to text and
   version) before the disk, so unsaved buffers are what imports see.
5. **Global state reset.** One `ResetForTool()` per module that has `g_*`
   state, separating caches that may be kept (stdlib root, layout schema,
   intern table) from per-compile state that must be cleared.
6. **Complete end spans.** Call `p_fin` on every rule; a test walks the AST
   of every file in `tests/` and fails on a node with no end.
7. **Persistent package cache.** Keep parsed and checked std packages in the
   worker between runs, keyed by path and mtime. This is the change that
   turns 190 ms into tens of milliseconds.
8. **Keep pruned branches' tokens.** `cc.PruneProgram` drops inactive
   branches from the AST; the snapshot keeps their byte ranges so semantic
   tokens can mark them inactive.
9. **Error-tolerant sema.** Sema already runs on the partial AST after a
   parse error; make sure a `<error>` node never cascades into unrelated
   diagnostics, so the problems panel shows the one real mistake.

## Editor clients

The server is the product; each client is configuration plus a little UI.

| Editor | Client | Work |
| --- | --- | --- |
| Visual Studio Code | `vscode-languageclient` from `extensions/vscode` (the extension is Salam compiled with `salam js`, so it needs bindings for the client library) | replaces the hand-written format provider; adds settings UI |
| JetBrains (IntelliJ IDEA, GoLand, CLion, PyCharm, WebStorm, Rider) | the platform's built-in LSP API, plus LSP4IJ for the free editions | a small plugin: file type, the existing `Salam.xml` highlighting as fallback, the server launcher, run configurations |
| Neovim | built-in client, an `nvim-lspconfig` entry | replace the `inspect` polling in `lua/salam/init.lua` |
| Emacs | `eglot` (built-in since 29) and `lsp-mode` | one entry in `salam-mode.el` |
| Sublime Text | LSP package | a `LSP-salam` settings file |
| Helix | `languages.toml` | one entry, upstreamed |
| Zed | an extension | needs the tree-sitter grammar below |
| Kate / KDevelop | built-in LSP client | one JSON entry |

Two companions that an IDE vendor will ask about, outside the LSP itself:

- **A tree-sitter grammar** (`tree-sitter-salam`), for Zed, Helix, Neovim
  highlighting, and GitHub's syntax highlighting. It must handle both
  keyword languages and the layout DSL.
- **A Debug Adapter Protocol server** (`salam dap`) over the existing
  `salam debug` GDB / LLDB path, so breakpoints and stepping work in every
  editor that has an LSP client. The `#line` directives from `-g` already map
  generated code back to Salam source.

## Performance targets

Measured on the open-source test files with a warm worker:

| Operation | Target (p95) |
| --- | --- |
| diagnostics after the last keystroke, typical file | 300 ms, including the 150 ms debounce |
| hover, definition, highlight | 30 ms (answered from the snapshot) |
| completion | 80 ms |
| semantic tokens, full | 50 ms |
| rename across the workspace | 1 s for a symbol with 1,000 references |
| first std index on a cold start | under 3 s, with progress shown |
| supervisor memory | flat over an 8 hour session |

## Testing

- **Transcript tests** in `tests/lsp/`: a `.jsonl` file of client messages
  and the expected server messages, replayed by `salam lsp --replay`. Paths
  and versions are normalised. English and Persian fixtures for every
  feature.
- **Position tests** with Persian text, emoji and mixed lines, in both UTF-8
  and UTF-16 modes.
- **Incremental sync fuzzing:** apply random edits incrementally and as full
  replacements; the snapshots must match.
- **Robustness sweep:** open every `.salam` file under `tests/` and `std/`,
  ask for semantic tokens, symbols, folding, and hover and definition at every
  identifier. No crash, no hang, no worker restart.
- **Memory:** a 10,000-edit soak test with an RSS ceiling for the supervisor,
  and the worker recycled at most as often as configured.
- **Client smoke tests** in CI with Neovim headless and the Visual Studio Code extension
  test runner.

## Roadmap

| Phase | Contents | Ships with |
| --- | --- | --- |
| 0 | compiler changes 1 to 6 above | each one as its own PR, useful to `salam check` immediately |
| 1 | `salam lsp`, supervisor and worker, sync, diagnostics, semantic tokens, document symbols, folding, selection range, formatting, hover with types, definition within a file | Visual Studio Code, Neovim and JetBrains clients; enough to demo |
| 2 | package cache (change 7), workspace index, completion, signature help, definition, references and highlight everywhere, workspace symbols, document links, commands | the release after phase 1 |
| 3 | code actions and quick fixes, rename, range and on-type formatting, organise imports | |
| 4 | pull diagnostics, inlay hints, code lens, call and type hierarchy, colours, Persian polish | |
| 5 | tree-sitter grammar, `salam dap`, a JetBrains plugin with run configurations and a test runner | the version to put in front of JetBrains |

Each phase is advertised in `initialize` only when its features pass the
transcript tests, so a client always sees an honest capability list.

## Progress

Built, with tests in `tools/lsp/test_lsp.py`:

- `salam lsp` and the `--analyze` worker, one short-lived worker process per
  analysis instead of the long-lived one described above. It sidesteps the
  global state and the leaks for now; queries never wait on it, and a new edit
  kills a stale run.
- Compiler changes 1, 3 (structured capture of every lexer, parser and sema
  diagnostic with its span; fixes are still help text), 4 (source overlays),
  and package-aware analysis: a file of a folder package is checked with its
  siblings through a virtual entry.
- A background workspace index (light worker runs over every workspace
  file), so references, rename, call hierarchy, code lens and workspace
  symbols reach files that are not open.
- Diagnostics with codes and help, hover with types, docs and the other
  language's name, definition, declaration and type definition into `std/`,
  references, highlight and rename across open files, outline, workspace
  symbols, folding, semantic tokens, completion (scope, members, packages,
  keywords and snippets in the file's language), signature help, inlay hints
  for inferred types, formatting with a minimal edit, on-type indentation,
  selection ranges, call hierarchy, implementations and type hierarchy for
  structural interfaces, code lens, document links for
  `include`/`import`, completion scoped block by block, and quick fixes for
  did-you-mean (English and Persian), unused `mut`, unused variables and the
  foreign keywords the parser recognises.
- Persian: messages follow `messages` or the client locale, multi-word names
  are one token and one range, positions are UTF-16 throughout.
- Clients: the Visual Studio Code extension (with `vscode-languageclient` bundled),
  Neovim (`require('salam').setup({ lsp = true })`), Emacs (eglot and
  lsp-mode), a JetBrains plugin on LSP4IJ (`extensions/jetbrains/plugin`,
  built with javac alone and smoke-tested in CI without an IDE), and
  documented setups for Sublime Text, Kate and Helix.
- `tools/lsp/sweep.py` runs the worker on all 6,300 files under `tests/` and
  `std/` and checks that every reference spells the symbol it points at;
  it is clean.

Also built since:

- `salam dap`, a Debug Adapter Protocol server that builds the program with
  debug information and hands the session to gdb (14+) or lldb-dap;
  breakpoints, stepping and locals work in `.salam` files. The VS Code
  extension registers it; `tools/lsp/test_dap.py` drives a real gdb session in
  CI.
- Pull diagnostics (`textDocument/diagnostic`, `workspace/diagnostic` from the
  index) and colour swatches for hex colours.
- `extensions/tree-sitter-salam`, a tree-sitter grammar generated from the
  compiler's keyword tables, with a scanner that reads multi-word names the
  way the compiler does. It parses 97.8% of the 4,230 valid Salam files in the
  repository with no error node; Neovim gets its queries from the Vim plugin.

Left:

1. A warm worker. Measured, a small file costs about 0.2 s of CPU per analysis
   and every query is answered from the snapshot without waiting, so a
   long-lived worker would save about 0.15 s per analysis in exchange for
   carrying the compiler's leaks and global state across runs. It is not worth
   it until the compiler resets its state (change 5) and frees its
   temporaries (docs/design/OWNERSHIP.md).
2. Run configurations and a test runner in the JetBrains plugin. They need the
   IntelliJ execution and settings APIs, so they are built and tested against
   the full IntelliJ SDK, not the handful of jars the plugin compiles against
   today. Until then, LSP4IJ's Debug Adapter Protocol run configuration with
   `salam dap` runs and debugs Salam files.
3. A published grammar repository (`tree-sitter-salam` with the generated
   `src/parser.c`), which Zed and Helix need to fetch the grammar by URL.
4. Structured fixes in `Diag` (change 3), so every quick fix comes from the
   compiler rather than from matching words, and `Node.sym` as a field.
