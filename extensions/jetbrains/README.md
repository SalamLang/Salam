# Salam for JetBrains IDEs

Works in IntelliJ IDEA, PyCharm, WebStorm, CLion, GoLand, Rider, PhpStorm,
RubyMine and Android Studio.

## The Salam plugin (recommended)

`plugin/` is a small plugin that brings everything `salam lsp` does into the
IDE: errors as you type, completion, hover, go to definition, find usages,
rename, call hierarchy, inlay hints and formatting, plus TextMate
highlighting for English and Persian Salam. It runs the server through the
free [LSP4IJ](https://plugins.jetbrains.com/plugin/23257-lsp4ij) plugin, so it
works in every IntelliJ-based IDE from 2024.2 on, the free editions and
Android Studio included.

Build it with a JDK 17 or newer; no Gradle or IDE download is needed:

```sh
extensions/jetbrains/plugin/build.sh
```

Then install LSP4IJ from the Marketplace and
`extensions/jetbrains/plugin/build/salam-jetbrains-<version>.zip` through
**Settings | Plugins | Install Plugin from Disk**. The plugin finds `salam` on
`PATH`, in `~/.salam/bin`, or wherever `SALAM_PATH` points.

To debug, add a **Debug Adapter Protocol** run configuration (LSP4IJ) with
the server command `salam dap` and launch parameters such as
`{"program": "$FILE_PATH$"}`; breakpoints in `.salam` files then work through
gdb or lldb. [`docs/LSP.md`](../../docs/LSP.md) describes `salam dap`.

`build.sh --smoke path/to/salam` also starts that compiler through the
plugin's own classes and checks the reply, without an IDE.

The options below need no plugin of ours.

## Option 1: the custom file type (no plugin)

`Settings > Editor > File Types > Import...` and pick `Salam.xml`.

Or copy it in directly and restart the IDE:

| Platform | Path                                                                         |
| -------- | ---------------------------------------------------------------------------- |
| Linux    | `~/.config/JetBrains/<IDE><version>/filetypes/Salam.xml`                     |
| macOS    | `~/Library/Application Support/JetBrains/<IDE><version>/filetypes/Salam.xml` |
| Windows  | `%APPDATA%\JetBrains\<IDE><version>\filetypes\Salam.xml`                     |

This gives keyword, type, built-in and constant highlighting in all three
languages, `//` and `/* */` comment toggling, brace matching, string and
number literals, and basic completion from words in the file.

## Option 2: the TextMate bundle (richer highlighting)

Install the bundled **TextMate Bundles** plugin, then
`Settings > Editor > TextMate Bundles > +` and select the `extensions/vscode`
directory of this repository. It uses the same TextMate grammar Visual Studio Code does,
so declaration names, `//!` directive headers, layout-DSL elements and escape
sequences are highlighted too.

The two options can be used together: keep the TextMate bundle for colouring
and the file type for comment toggling.

## Option 3: the language server (diagnostics, completion, navigation)

Every IntelliJ-based IDE, the free editions and Android Studio included, can
run `salam lsp` through the [LSP4IJ](https://plugins.jetbrains.com/plugin/23257-lsp4ij)
plugin:

1. Install LSP4IJ from **Settings | Plugins | Marketplace**.
2. In **Settings | Languages & Frameworks | Language Servers**, click **+**.
3. Name it `Salam` and set the command to `salam lsp` (use the full path if
   `salam` is not on the IDE's `PATH`).
4. Under **Mappings | Filename patterns**, add `*.salam` with language id
   `salam`.

Combine it with option 2 for highlighting. The server's features and options
are in [`docs/LSP.md`](../../docs/LSP.md).

## Generated

`Salam.xml` is generated. Edit the generator and rerun it from the repository
root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=180000
```

## License

MIT. See [LICENSE](LICENSE).
