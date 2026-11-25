# Salam for micro

A syntax file and a plugin. The syntax highlights; the plugin adds commands to
run, build, check and format, a linter, and a status line item that shows the
block the cursor is in.

## Install

```sh
mkdir -p ~/.config/micro/syntax ~/.config/micro/plug
cp salam.yaml ~/.config/micro/syntax/
cp -r salam ~/.config/micro/plug/
```

On Windows the folder is `%APPDATA%\micro`. Restart micro. `.salam` files are
detected by extension, and `salam` must be on the `PATH` micro sees. The plugin
needs micro 2.0 or later and the `linter` plugin that ships with it.

## Commands

Press `Ctrl-e` and type a command.

| Command                    | What it does                                                  |
| -------------------------- | ------------------------------------------------------------- |
| `salamrun`                 | Save and run: `salam run FILE`, or `salam run` in the project |
| `salambuild`               | Save and build: `salam build FILE`, or the project            |
| `salamcheck`               | Save, run `salam check`, jump to the first error              |
| `salamformat`              | Save, run `salam format`, reload the buffer                   |
| `salamnew [dir]`           | Start the `salam init` wizard for a new project               |
| `salamplatform [P [ARCH]]` | Pick `--platform` (web, js, android, ios, linux, macos, windows, all) and `--arch` for run and build; `default` clears it |

A file counts as part of a project when a `salam.toml` sits in its folder or
above. Then run and build use `salam run` and `salam build` from the project
root, so a project made by `salam init` serves its web page with live reload or
builds its first platform, unless `salamplatform` has chosen another one. The
choice is stored as the `salam.platform` and `salam.arch` options.

Run and build use micro's shell, so the output stays on screen until you press a
key. In `help salam` inside micro the same list is available.

Suggested keys, in `~/.config/micro/bindings.json`:

```json
{
    "F5": "command:salamrun",
    "F6": "command:salambuild",
    "F7": "command:salamcheck",
    "F8": "command:salamformat"
}
```

## Linter

The plugin registers `salam check FILE --error-style=gcc` with micro's linter
plugin and the format `%f:%l:%c: %m`, so errors show in the gutter and the info
bar after a save. Turn it off with `set linter false`.

## Status line

`salam.block` is a status line item with the headers around the cursor, found by
walking up the indentation to lines that end in `:`. Lines closing a block are
`end` or `پایان`. Inside a method it reads like
`pub struct Box > pub func Size(): int > if x > 0`. Add it to the status line:

```
set statusformatl "$(filename) $(modified)($(line),$(col)) $(opt:filetype) $(salam.block)"
```

Put the same `statusformatl` line in `settings.json` to keep it. At most the
four innermost headers are shown and long ones are shortened.

## What the syntax file covers

Keywords, types, intrinsic types, built-in output statements, constants and
contextual keywords in English and Persian; numbers in ASCII, Persian
and Arabic-Indic digits; string, raw-string and character literals with escape
highlighting; `//` and `/* */` comments with `TODO` markers; `//!` directive
headers; annotations; and operators.

## A limitation

micro uses Go's RE2 engine, which has no lookaround and whose `\b` only knows
ASCII. English keywords are matched with a real word boundary; Persian
keywords are matched by an alternation ordered longest first. A Persian
keyword that happens to sit inside a longer Persian identifier can therefore
be coloured. Every other editor in `extensions/` uses an engine that can
express the boundary properly.

## Generated

`salam.yaml` is generated. Edit the generator and rerun it from the repository
root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=180000
```

The `salam/` plugin folder is written by hand.

## License

MIT. See [LICENSE](LICENSE).
