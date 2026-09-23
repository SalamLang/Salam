# Salam for Geany

A custom file type definition, with build, run, check and format wired into
the Build menu.

## Install

```sh
mkdir -p ~/.config/geany/filedefs
cp filetypes.Salam.conf ~/.config/geany/filedefs/
```

Restart Geany. `.salam` files open as Salam; `Document > Set Filetype > Salam`
forces it.

## What it covers

Keyword and type highlighting in English and Persian on top of Geany's
C lexer, which also handles `//` and `/* */` comments, strings and numbers.
Comment toggling, four-space indentation and a symbol list come with the
file type.

The Build menu gets:

| Entry   | Command                       |
| ------- | ----------------------------- |
| Compile | `salam build`                 |
| Check   | `salam inspect --emit-symbol` |
| Format  | `salam format`                |
| Execute | `salam exec`                  |

Errors are parsed out of the gcc-style diagnostics, so clicking one jumps to
the line.

## Generated

`filetypes.Salam.conf` is generated. Edit the generator and rerun it from the
repository root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=180000
```

## License

MIT. See [LICENSE](LICENSE).
