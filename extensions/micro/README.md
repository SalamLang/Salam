# Salam for micro

## Install

```sh
mkdir -p ~/.config/micro/syntax
cp salam.yaml ~/.config/micro/syntax/
```

Restart micro. `.salam` files are detected by extension.

## What it covers

Keywords, types, intrinsic types, built-in output statements, constants and
contextual keywords in English, Persian and Arabic; numbers in ASCII, Persian
and Arabic-Indic digits; string, raw-string and character literals with escape
highlighting; `//` and `/* */` comments with `TODO` markers; `//!` directive
headers; annotations; and operators.

## A limitation

micro uses Go's RE2 engine, which has no lookaround and whose `\b` only knows
ASCII. English keywords are matched with a real word boundary; Persian and
Arabic keywords are matched by an alternation ordered longest first. A Persian
keyword that happens to sit inside a longer Persian identifier can therefore
be coloured. Every other editor in `extensions/` uses an engine that can
express the boundary properly.

## Generated

`salam.yaml` is generated. Edit the generator and rerun it from the repository
root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=180000
```

## License

MIT. See [LICENSE](LICENSE).
