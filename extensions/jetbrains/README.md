# Salam for JetBrains IDEs

Works in IntelliJ IDEA, PyCharm, WebStorm, CLion, GoLand, Rider, PhpStorm,
RubyMine and Android Studio.

## The Salam plugin (recommended)

`plugin/` is a small plugin that brings everything `salam lsp` does into the
IDE: errors as you type, completion, hover, go to definition, find usages,
rename, call hierarchy, inlay hints and formatting, plus TextMate
highlighting for English and Persian Salam, and **Salam** run configurations
with a run button beside every `func main` (`تابع ریشه`). It runs the server through the
free [LSP4IJ](https://plugins.jetbrains.com/plugin/23257-lsp4ij) plugin, so it
works in every IntelliJ-based IDE from 2024.2 on, the free editions and
Android Studio included.

Build it with a JDK 17 or newer; no Gradle is needed. The first build
downloads IntelliJ IDEA Community 2024.2 (about 1 GB, kept in
`~/.cache/salam-jetbrains`) to compile against:

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

A Salam run configuration runs a file or a project directory with one of
`salam run`, `build`, `check`, `exec` or `test`, with program arguments and a
working directory. For **run** and **build** it also takes a **Platform**
(`web`, `js`, `android`, `ios`, `linux`, `macos`, `windows` or `all`), which
becomes `--platform=...`; leave it empty for the compiler's default. Right-click
a `.salam` file and choose **Run**, or click the green arrow beside `func main`.

The **Tools | Salam** menu works on the project around the current file, found
by walking up to its `salam.toml`:

| Action                        | Runs                                                       |
| ----------------------------- | ---------------------------------------------------------- |
| Run Project                   | `salam run DIR --platform=<first platform in [project]>`   |
| Run Project for Platform...   | the same, with the platform picked from the list           |
| Build Project                 | `salam build DIR --platform=<first platform in [project]>` |
| Build Project for Platform... | the same, with the platform picked from the list           |
| Check Project                 | `salam check DIR`                                          |
| New Project...                | asks for a folder, then `salam init` in a terminal tab     |

Each of these creates (and reuses) a run configuration, so results appear in the
Run window and can be rerun from the toolbar. Outside a project with a
`[project]` table they run without `--platform`. New Project needs the bundled
Terminal plugin; the wizard asks its questions in that terminal. If the terminal
cannot be opened the plugin shows the command to run by hand.

The status bar shows the blocks around the caret in a Salam file, such as
`page > box > button` or `func main > if`. The chain is read from the text (a
line ending in a colon opens a block, `end` closes one), so it works while the
file has errors. It is a status bar item, not breadcrumbs, because TextMate
files have no syntax tree for the breadcrumbs bar to use. Right-click the status
bar to hide it.

Tests run the same way. In a `_test.salam` file there is a green arrow beside
every `test_` function, and right-clicking the file or a directory runs all of
its tests with `salam test`; the results fill the IDE's test tree, failures
show the assertion that failed, and clicking a test jumps to its line. The
**Test** command of a Salam run configuration takes a file or directory and an
optional test name filter.

`build.sh --smoke path/to/salam` also starts that compiler through the
plugin's own classes and checks the reply, and checks the block chain and
`salam.toml` readers, without an IDE. `build.sh --verify`
runs JetBrains' Plugin Verifier against the IDE build.

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
