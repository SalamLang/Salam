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
constants and contextual keywords in English, Persian and Arabic; the name in
a `func` or `struct` declaration; `//` and `/* */` comments with alert
highlighting; `//!` directive headers; `"..."`, `"""..."""`, `` `...` `` and
`'c'` literals with escape checking; and integers and floats written with
ASCII, Persian or Arabic-Indic digits.

## Generated

`salam.xml` is generated. Edit the generator and rerun it from the repository
root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=180000
```

## License

MIT. See [LICENSE](LICENSE).
