# Salam for Kate

A KSyntaxHighlighting definition, so it also covers KWrite, KDevelop, Kile and
anything else built on KTextEditor.

## Install

```sh
mkdir -p ~/.local/share/org.kde.syntax-highlighting/syntax
cp salam.xml ~/.local/share/org.kde.syntax-highlighting/syntax/
```

Restart Kate. `.salam` files are detected by extension; pick
`Tools > Highlighting > Sources > Salam` to force it.

## What it covers

Keywords, primitive and intrinsic type names, built-in output statements,
constants and contextual keywords in English and Persian; the name in
a `func` or `struct` declaration; `//` and `/* */` comments with alert
highlighting; `//!` directive headers; `"..."`, `"""..."""`, `` `...` `` and
`'c'` literals with escape checking; and integers and floats written with
ASCII, Persian or Arabic-Indic digits.

## Language server

Kate's LSP client can run `salam lsp`. In **Settings | Configure Kate | LSP
Client | User Server Settings**:

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

See [`docs/LSP.md`](../../docs/LSP.md) for what the server does.

## External tools: run, build, check, format

`external-tools.json` defines Kate External Tools that call the compiler:

| Tool                 | Command                                            | Output           |
| -------------------- | -------------------------------------------------- | ---------------- |
| Salam: Run           | `salam run FILE`                                   | tool output pane |
| Salam: Build         | `salam build FILE`                                 | tool output pane |
| Salam: Check         | `salam check FILE`                                 | tool output pane |
| Salam: Format        | `salam format FILE`, then the file is reloaded     | none             |
| Salam: Build for web | `salam build FILE --platform=web`                  | tool output pane |
| Salam: Run project   | `salam run . --platform=web` in the project folder | tool output pane |
| Salam: New project   | `konsole -e salam init` in the file's folder       | a terminal       |

Every tool saves the current document first and runs in the file's folder; the
project tool runs in the root of the open Kate project. Platforms other than
`web` (`js`, `android`, `ios`, `linux`, `macos`, `windows`, `all`) work the same
way: copy a tool and change `--platform=`. A project made by `salam init` lists
its platforms in `salam.toml`.

Install:

1. Put `salam` on `PATH` (Kate starts tools with Kate's own `PATH`, so a
   program in `~/.salam/bin` needs `~/.salam/bin` in your login profile, or
   the full path in each tool's Executable field).
2. In **Settings | Configure Kate | External Tools**, open the **Add** menu
   and choose **Import Tools...** (older versions: the **Import** button),
   then pick `external-tools.json`.
3. The tools appear under **Tools | External Tools | Salam**. Give them
   shortcuts in **Settings | Configure Shortcuts**.

New project needs a terminal program: it uses `konsole`; change the Executable
and Arguments to your own (`gnome-terminal -- salam init`, `xterm -e salam
init`) if you do not have it. The wizard asks for the name, type, platforms,
design and web mode, so it must run in a terminal. Kate's built-in Terminal
panel works too: open it, `cd` into a folder and run `salam init`.

The fields are the ones Kate stores for a tool. If your Kate version rejects
the import, add the tools by hand with the same Name, Executable, Arguments
and Working directory values from the file.

## Projects, KDevelop

`project/kateproject` and `project/kateproject.build` are examples for the Kate
Projects and Build plugins. Copy them into a project folder as `.kateproject`
and `.kateproject.build`; the first lists `*.salam`, `salam.toml` and
Markdown files, the second adds build targets for `salam build`, `salam check`,
`salam build . --platform=web` and `salam run`, which Kate's Build plugin
shows in its target list.

KDevelop has no Salam project template. Open the folder from `salam init` with
**Project | Open / Import Project**, choose the custom build tool, and in
**Project | Open Configuration**, under the custom build tool, set the build action to
`salam build` and the install action to `salam install`, with the folder as
the build directory. Add a launch configuration of type **Compiled Binary** or
**Custom Script** running `salam run` in the project folder. The same
`salam.toml` platforms apply: use `salam build --platform=NAME` to pick one.
The highlighting above is shared with KDevelop, and `salam lsp` works through
its language-server support.

## Generated

`salam.xml` is generated. Edit the generator and rerun it from the repository
root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=180000
```

## License

MIT. See [LICENSE](LICENSE).
