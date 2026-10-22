# The Salam language server

`salam lsp` is a [Language Server Protocol](https://microsoft.github.io/language-server-protocol/)
server built into the compiler. Any editor with an LSP client gets the same
features from it, for English and Persian Salam alike. There is nothing extra
to install: if `salam version` works, `salam lsp` does too (from 0.5.8).

```sh
salam lsp --version
```

## What it does

| Feature | LSP request |
| --- | --- |
| Errors and warnings as you type, with the compiler's codes and help text | `publishDiagnostics` |
| Hover: declaration, type and doc comment, also for the standard library | `hover` |
| Go to definition, declaration and type definition, into `std/` as well | `definition`, `declaration`, `typeDefinition` |
| Find references and highlight them, across the whole workspace | `references`, `documentHighlight` |
| Rename a symbol in every file that uses it | `prepareRename`, `rename` |
| Who calls this function, and what it calls | `callHierarchy` |
| Which structs satisfy an interface, which interfaces a struct satisfies, and the methods that implement an interface method | `implementation`, `typeHierarchy` |
| Reference counts above declarations, and `Run` above `main` in Visual Studio Code | `codeLens` |
| `include` and `import` paths open the file they name | `documentLink` |
| Inferred types after `name := value` | `inlayHint` |
| Completion: names in scope (block by block), struct members, package members, keywords, block snippets | `completion` |
| Parameter hints while typing a call | `signatureHelp` |
| Outline, breadcrumbs and workspace symbols | `documentSymbol`, `workspace/symbol` |
| Semantic highlighting, including multi-word Persian names | `semanticTokens/full` |
| Folding for blocks, comments and imports | `foldingRange` |
| Formatting through the compiler's formatter, and indent after `:` | `formatting`, `rangeFormatting`, `onTypeFormatting` |
| Quick fixes: did-you-mean, remove a `mut` that is never reassigned, prefix an unused name with `_`, and rewrite `while`, `return`, `elif`, `def`, `class`, `let` and `var` into Salam | `codeAction` |
| Smart selection | `selectionRange` |

Messages follow the editor's language: a Persian (`fa`) locale gets Persian
diagnostics and hover text. The error codes (`E012`) stay the same in both.

## Options

Send them as `initializationOptions`, or later in
`workspace/didChangeConfiguration` under a `salam` key:

| Option | Default | Meaning |
| --- | --- | --- |
| `stdlibPath` | the compiler's own lookup | same as `--stdlib-path` |
| `messages` | from the client's `locale` | `en` or `fa` |
| `defines` | `[]` | conditional compilation flags, as `-D` takes them |
| `formatFixOrder` | `false` | let formatting reorder top-level declarations, like `salam format` |
| `index` | `true` | analyze every Salam file in the workspace in the background, for references, rename, call hierarchy and code lens |
| `indexMaxFiles` | `400` | skip the background index when a workspace has more Salam files than this |

The same can be set on the command line: `salam lsp --stdlib-path=DIR --messages=fa`.

## Editor setup

### Visual Studio Code

Install the Salam extension (`extensions/vscode`). It starts the server by
itself; `Salam: Restart Language Server` restarts it after a compiler update.

### Neovim

The plugin in `extensions/vim` starts the server for `salam` buffers:

```lua
require('salam').setup({ lsp = true })
```

Without the plugin, Neovim 0.11 needs only:

```lua
vim.filetype.add({ extension = { salam = 'salam' } })
vim.lsp.config('salam', { cmd = { 'salam', 'lsp' }, filetypes = { 'salam' }, root_markers = { 'salam.salam', '.git' } })
vim.lsp.enable('salam')
```

### Emacs

`salam-mode` registers the server with both clients, so `M-x eglot` or
`M-x lsp` in a Salam buffer is enough.

### JetBrains IDEs (IntelliJ IDEA, GoLand, PyCharm, WebStorm, CLion, Rider, Android Studio)

Install the free [LSP4IJ](https://plugins.jetbrains.com/plugin/23257-lsp4ij)
plugin, then the Salam plugin built by `extensions/jetbrains/plugin/build.sh`
(**Settings | Plugins | Install Plugin from Disk**). It starts `salam lsp` for
`.salam` files and adds TextMate highlighting; see
[`extensions/jetbrains/README.md`](../extensions/jetbrains/README.md).

Without the Salam plugin, LSP4IJ alone works too. In **Settings | Languages &
Frameworks | Language Servers** add a server:

- **Name:** `Salam`
- **Command:** `salam lsp`
- **Mappings | Filename patterns:** `*.salam`, language id `salam`

For highlighting before the server answers, also import the TextMate bundle
described in `extensions/jetbrains/README.md`.

### Sublime Text

With the [LSP](https://packagecontrol.io/packages/LSP) package, add to
**Preferences | Package Settings | LSP | Settings**:

```json
{
  "clients": {
    "salam": {
      "enabled": true,
      "command": ["salam", "lsp"],
      "selector": "source.salam"
    }
  }
}
```

### Helix

In `languages.toml`:

```toml
[language-server.salam]
command = "salam"
args = ["lsp"]

[[language]]
name = "salam"
scope = "source.salam"
file-types = ["salam"]
roots = ["salam.salam"]
comment-token = "//"
indent = { tab-width = 4, unit = "    " }
language-servers = ["salam"]
```

### Kate

In **Settings | Configure Kate | LSP Client | User Server Settings**:

```json
{
  "servers": {
    "salam": {
      "command": ["salam", "lsp"],
      "highlightingModeRegex": "^Salam$"
    }
  }
}
```

### Any other client

Run `salam lsp` over stdio. The server speaks LSP 3.17 with UTF-16 positions
and incremental document sync.

## Debugging: `salam dap`

`salam dap` is a [Debug Adapter Protocol](https://microsoft.github.io/debug-adapter-protocol/)
server for editors. On a `launch` request whose `program` is a `.salam` file
(or a project folder), it builds the program with debug information, then
hands the session to gdb 14 or newer (`gdb -i=dap`) or `lldb-dap`. The
debug information points back at the Salam source, so breakpoints, the call
stack, stepping and local variables all work in `.salam` files.

```json
{ "type": "salam", "request": "launch", "name": "Debug Salam file", "program": "${file}" }
```

- **Visual Studio Code:** the extension registers the `salam` debugger; press
  F5 in a Salam file.
- **JetBrains IDEs:** with LSP4IJ, add a **Debug Adapter Protocol** run
  configuration whose server command is `salam dap`, and set `program` in its
  launch parameters.
- **Neovim (nvim-dap), Emacs (dape), Helix, Zed:** register an executable
  adapter `salam dap` with the launch configuration above.

`SALAM_DAP_DEBUGGER` picks the debugger explicitly, for example
`SALAM_DAP_DEBUGGER=lldb-dap`. `salam dap` runs on Linux and macOS; on Windows,
point the editor at `lldb-dap` and a program built with `salam build -g`.

## How it works

The server keeps every open document in memory. After an edit settles
(200 ms), it starts `salam lsp --analyze` on the document, a short-lived
worker that runs the compiler frontend: lexer, parser, conditional
compilation, layout, and the full semantic pass, with the other open
documents passed in as unsaved overlays. The worker writes one snapshot:
diagnostics, outline, folding, semantic tokens, every resolved reference with
the declaration it points at, and the scopes that completion needs.

Queries are answered from the latest snapshot without waiting, so hover stays
instant while a large file is being analyzed. A new edit cancels an analysis
that is still running. Requests whose answer depends on exact positions
(outline, folding, semantic tokens) wait for the analysis of the current text.

When nothing else is pending, the server also walks the workspace folders
(skipping hidden directories, `node_modules`, `build`, `out`, `dist`, `target`
and a bundled `std/`) and analyzes each Salam file in the background with a
lighter worker. Those snapshots answer references, rename, call hierarchy,
code lens and workspace symbols for files that are not open. Saving a file
queues the index again.

A file inside a folder package (`pkg/pkg.salam` plus siblings) is analyzed
together with its whole package, the way the compiler sees it on import.

The design and the roadmap are in [`docs/design/LSP.md`](design/LSP.md).

## Troubleshooting

- **Nothing happens:** run `salam lsp --version`. An older compiler prints
  `unknown command`; update Salam.
- **Imports of `std/` are not found:** set `stdlibPath`, or check that
  `salam check yourfile.salam` works from a terminal.
- **The client shows "analysis failed":** the worker crashed on that file.
  `salam check` on the same file usually shows why; please report it with the
  file attached.
