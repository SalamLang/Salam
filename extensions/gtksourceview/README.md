# Salam for GtkSourceView

Covers gedit, GNOME Text Editor, GNOME Builder, Pluma, Xed, meld and anything
else built on GtkSourceView.

## Install

GtkSourceView 4 (gedit 3.x, Pluma, Xed):

```sh
mkdir -p ~/.local/share/gtksourceview-4/language-specs
cp salam.lang ~/.local/share/gtksourceview-4/language-specs/
```

GtkSourceView 5 (GNOME Text Editor, GNOME Builder 42 and later):

```sh
mkdir -p ~/.local/share/gtksourceview-5/language-specs
cp salam.lang ~/.local/share/gtksourceview-5/language-specs/
```

GtkSourceView 3 (older gedit):

```sh
mkdir -p ~/.local/share/gtksourceview-3.0/language-specs
cp salam.lang ~/.local/share/gtksourceview-3.0/language-specs/
```

Restart the editor. `.salam` files are detected by glob.

## What it covers

Keywords, primitive and intrinsic types, built-in output statements,
constants and contextual keywords in all three languages; the name in a
`func` or `struct` declaration; `//` and `/* */` comments with spell checking
disabled in code; `//!` directive headers; every string form; annotations;
and numbers in ASCII, Persian and Arabic-Indic digits.

## Generated

`salam.lang` is generated. Edit the generator and rerun it from the repository
root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=180000
```

## License

MIT. See [LICENSE](LICENSE).
