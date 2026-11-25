# Salam for Geany

A custom file type definition with the Build menu wired to the `salam`
command, clickable compiler errors and a symbol list.

## Install

```sh
mkdir -p ~/.config/geany/filedefs ~/.config/geany/ctags
cp filetypes.Salam.conf ~/.config/geany/filedefs/
cp ctags/Salam.ctags ~/.config/geany/ctags/
```

Restart Geany. `.salam` files open as Salam; `Document > Set Filetype > Salam`
forces it. On Windows the folders are under `%APPDATA%\geany`.

The `ctags` folder needs Geany 1.37 or later. Without it the file type still
works and highlights, builds and runs; only the symbol list stays empty. To
use the built-in C parser for the symbol list instead, change `tag_parser=Salam`
to `tag_parser=C` in the copied `.conf`.

## Build menu

`salam` must be on the `PATH` Geany sees.

| Entry          | Command                                   | Shortcut |
| -------------- | ----------------------------------------- | -------- |
| Compile        | `salam build FILE`                        | F8       |
| Build          | `salam build` in the project folder       | F9       |
| Check          | `salam check FILE`                        |          |
| Format         | `salam format FILE`                       |          |
| Build for web  | `salam build FILE --platform=web`         |          |
| Run            | `salam run FILE`                          | F5       |
| Serve          | `salam run FILE --platform=web`           |          |
| Interpret      | `salam exec FILE`                         |          |

The shortcuts are Geany's own for Compile, Build and Run. Format rewrites the
file on disk and Geany offers to reload it.

Build runs in the Geany project base path (`Project > New`), which is where a
`salam.toml` made by `salam init` lives. In a project, plain `salam run` serves
the web page with live reload and `salam build` builds the first platform of
`salam.toml`; to use them, edit a command in `Build > Set Build Commands`. Use
`salam init` in a terminal to start a project.

Errors are parsed out of the gcc-style diagnostics (`--error-style=gcc`), so
clicking an error in the Compiler tab jumps to its line.

## Settings

The file type uses four spaces, no tabs, and `//` for line comments and
`/* */` for blocks, so `Edit > Commentation > Toggle Line Commentation`
(Ctrl+E) works. Highlighting for both English and Persian keywords sits on top
of Geany's C lexer, which also handles comments, strings and numbers.

## Symbol list

`ctags/Salam.ctags` lists `func`, `struct`, `enum`, `interface`, `component`,
`page` and `type` declarations, with their Persian spellings, in the Symbols
tab.

## Generated

`filetypes.Salam.conf` is generated. Edit the generator and rerun it from the
repository root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=180000
```

`ctags/Salam.ctags` is written by hand.

## License

MIT. See [LICENSE](LICENSE).
