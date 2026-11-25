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
constants and contextual keywords in both languages; the name in a
`func` or `struct` declaration; `//` and `/* */` comments with spell checking
disabled in code; `//!` directive headers; every string form; annotations;
and numbers in ASCII, Persian and Arabic-Indic digits.

## Comments, strings, spell checking and brackets

The language file sets the `//` and `/* */` comment markers, so gedit's and
Builder's **Comment Code** command and `Ctrl+M` work. Comments and strings
carry the `comment` and `string` context classes (character literals, raw
strings, `"""` strings and text blocks included), which does two things:

- spell checking runs inside comments and strings only, never in code; turn it
  on in gedit with **Tools | Check Spelling** or in Builder with
  **Preferences | Editor | Spell Check**
- bracket matching ignores `(`, `)`, `[`, `]`, `{` and `}` inside those
  contexts, so `'('` or `"("` does not unbalance the highlighted pair

GtkSourceView draws matching brackets for `()`, `[]` and `{}` only. It cannot
pair a block's `:` with its `end`, and a language file cannot carry indent
rules, so indentation comes from the editor's own settings. `salam format`
writes four spaces per level (`--tabs` for tabs), so use these:

- gedit and Pluma: **Preferences | Editor**, tab width 4, **Insert spaces
  instead of tabs**, **Enable automatic indentation**
- GNOME Builder and GNOME Text Editor: **Preferences | Editor**, the same
  settings, or put an `.editorconfig` in the project:

```ini
[*.salam]
indent_style = space
indent_size = 4
end_of_line = lf
insert_final_newline = true
```

## Running and building

### gedit and Pluma: External Tools

Enable the **External Tools** plugin (Preferences | Plugins), open **Tools |
Manage External Tools** and add a tool per command. Each one is a small shell
script; set Input to **Nothing**, Output to **Display in bottom pane** and Save
to **Current document**. In gedit:

```sh
salam run "$GEDIT_CURRENT_DOCUMENT_PATH"
salam build "$GEDIT_CURRENT_DOCUMENT_PATH"
salam check "$GEDIT_CURRENT_DOCUMENT_PATH"
salam build "$GEDIT_CURRENT_DOCUMENT_PATH" --platform=web
```

A Format tool is `salam format "$GEDIT_CURRENT_DOCUMENT_PATH"`, with Output set to
**Nothing**; gedit reloads the file. In Pluma the variables start with
`PLUMA_` instead (`$PLUMA_CURRENT_DOCUMENT_PATH`). Use the full path to
`salam` (for example `$HOME/.salam/bin/salam`) when the editor was not started
from a shell with it on `PATH`. A project made by `salam init` lists its
platforms in `salam.toml`; swap `--platform=web` for `js`, `android`, `ios`,
`linux`, `macos`, `windows` or `all`.

### GNOME Builder

Builder reads a `.buildconfig` in the project folder, which is also what
**Build Preferences** writes. Put this next to the project's `salam.toml`:

```ini
[default]
name=Salam
runtime=host
build-commands=salam build
run-commands=salam run
```

`runtime=host` builds with the `salam` on your own system. With `salam init`
projects, `salam build` and `salam run` use the first platform in
`salam.toml`; to pin one, write `salam build --platform=linux` and `salam run
--platform=web`. A second configuration section (`[web]`, with `name=Web` and
`build-commands=salam build --platform=web`) gives a second entry in the
build configuration menu. Builder shows the compiler's errors in its build output. If your Builder version
ignores these keys, enter the same commands under **Build Preferences |
Commands**.

## Generated

`salam.lang` is generated. Edit the generator and rerun it from the repository
root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=180000
```

## License

MIT. See [LICENSE](LICENSE).
