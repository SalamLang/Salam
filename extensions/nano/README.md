# Salam for GNU nano

Highlighting, a linter, a formatter, comment and indent settings, and optional
run and build keys. Needs nano 5.0 or later for the key macros; the rest works
on nano 4.0 and later.

## Install

```sh
mkdir -p ~/.nano
cp salam.nanorc ~/.nano/
echo 'include "~/.nano/salam.nanorc"' >> ~/.nanorc
```

System-wide instead:

```sh
sudo cp salam.nanorc /usr/share/nano/
```

nano loads everything in `/usr/share/nano/` by default.

## Settings worth adding

`salam.nanorc` sets the comment marker (`//`, toggled with `M-3`), the tab
text (four spaces), the linter and the formatter. Options that belong to the
whole editor go in `~/.nanorc`:

```
set tabsize 4
set tabstospaces
set autoindent
```

`autoindent` keeps the indentation of the previous line, which is what you want
after a `func main:` line. Nothing indents the next line for you after the
colon, so press Tab once.

## Linter and formatter

With a `.salam` buffer open:

| Action        | Keys                          | Runs                          |
| ------------- | ----------------------------- | ----------------------------- |
| Linter        | `^T` then `^Y`                | `salam check --error-style=gcc` |
| Formatter     | `^T` then `^O`                | `salam format`                |

The linter lists each error with its line; `^Y` and `^V` step between them.
The formatter rewrites the buffer in place; save with `^O` afterwards.

## Run and build with ^T

`^T` (Execute) is the general way to run anything from nano. Press `^T`, then
`M-F` for a new buffer so the output does not land in your file, and type a
command such as:

```
salam run main.salam
salam build main.salam --platform=web
salam run main.salam --platform=web
```

The last one serves the page with live reload; stop it with `^C`. In a project
made by `salam init`, plain `salam run` and `salam build` use `salam.toml`.

## Key bindings

`salam-keys.nanorc` binds the same commands to function keys. nano ignores
`bind` lines in included files, so append them to `~/.nanorc` after the
`include` line instead of including them:

```sh
cat salam-keys.nanorc >> ~/.nanorc
```

| Key  | Action                                       |
| ---- | -------------------------------------------- |
| `F5` | `salam run` in the current directory         |
| `F6` | `salam build`                                |
| `F7` | `salam build --platform=web`                 |
| `F8` | linter                                       |
| `F9` | formatter                                    |

nano runs these in the directory it was started in, so start it from the
project folder. Output opens in a new buffer; close it with `^X`.

## What it covers

Keywords, types, built-in output statements, constants and contextual
keywords in both languages, numbers in ASCII, Persian and Arabic-Indic
digits, string, raw-string and character literals, `//` and `/* */` comments,
`TODO` markers, and trailing whitespace.

The digit rules use Arabic-Indic digit ranges inside bracket expressions. The
C library of a few locales rejects those ranges (seen with `C.UTF-8` on glibc)
and nano then reports mistakes in the file at start; the other rules still load
in a locale that accepts them.

## Generated

`salam.nanorc` is generated. Edit the generator and rerun it from the repository
root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=180000
```

`salam-keys.nanorc` is written by hand.

## License

MIT. See [LICENSE](LICENSE).
