# Editor support for Salam

Salam programs can be written with English, Persian or Arabic keywords, and a
single file may use any one of them. Every integration here highlights all
three, treats Arabic-script letters as identifier characters, and accepts
numbers written with ASCII, Persian or Arabic-Indic digits.

| Editor          | Directory                         | Highlighting | Indent  | Snippets | Run / build | Format | Diagnostics                     |
| --------------- | --------------------------------- | ------------ | ------- | -------- | ----------- | ------ | ------------------------------- |
| VS Code         | [`vscode/`](vscode)               | yes          | yes     | yes      | yes         | yes    | via the compiler                |
| Vim / Neovim    | [`vim/`](vim)                     | yes          | yes     | yes      | yes         | yes    | quickfix, `vim.diagnostic`, ALE |
| Sublime Text    | [`sublime/`](sublime)             | yes          | yes     | yes      | yes         | yes    | build panel                     |
| Emacs           | [`emacs/`](emacs)                 | yes          | yes     | yes      | yes         | yes    | `compile`, flycheck             |
| Kate / KDevelop | [`kate/`](kate)                   | yes          | generic | no       | no          | no     | no                              |
| gedit / GNOME   | [`gtksourceview/`](gtksourceview) | yes          | generic | no       | no          | no     | no                              |
| JetBrains IDEs  | [`jetbrains/`](jetbrains)         | yes          | generic | no       | no          | no     | no                              |
| Geany           | [`geany/`](geany)                 | yes          | generic | no       | yes         | yes    | build panel                     |
| Notepad++       | [`notepadpp/`](notepadpp)         | yes          | generic | no       | no          | no     | no                              |
| GNU nano        | [`nano/`](nano)                   | yes          | no      | no       | no          | `^T`   | linter view                     |
| micro           | [`micro/`](micro)                 | yes          | no      | no       | no          | no     | no                              |

Each directory has its own README with installation instructions.

## Where the word lists come from

Nothing here keeps its own copy of the language's keywords. The generator
reads them out of the compiler at generation time:

- `compiler/langpack.salam` gives every keyword's spelling in all three
  languages along with the token kind the lexer assigns it, so a keyword is
  grouped by what it _is_ rather than by how it is spelled.
- `compiler/semantic/semantic.salam` gives the primitive type names, the
  intrinsic types (`Vector`, `HashMap`, `MapIter`, `File`, `Variant`) and the
  intrinsic method names.

A new keyword therefore reaches every editor at once, in all three languages,
by rerunning one command.

## Regenerating

From the repository root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=180000
```

Generated files carry a note saying so. Edit `extensions/tools/gen_syntaxes.salam`
instead of editing them. CI regenerates and fails if the result differs from
what is committed.

These files are generated:

```
vscode/syntaxes/salam.tmLanguage.json
sublime/Salam.sublime-syntax
sublime/Salam.sublime-completions
sublime/Indentation Rules.tmPreferences
sublime/Comments.tmPreferences
sublime/Symbol List.tmPreferences
vim/syntax/salam.vim
vim/indent/salam.vim
vim/ftplugin/salam.vim
vim/UltiSnips/salam.snippets
emacs/salam-mode.el
emacs/snippets/salam-mode/*
kate/salam.xml
gtksourceview/salam.lang
jetbrains/Salam.xml
geany/filetypes.Salam.conf
notepadpp/salam.xml
nano/salam.nanorc
micro/salam.yaml
```

Everything else in each directory is hand-written: plugin code, settings,
build systems, documentation.

## Snippets

`vscode/snippets/salam.code-snippets` is the single source for all 24
snippets. The generator converts it to Sublime completions, UltiSnips and
YASnippet, which all use the same `${1:placeholder}` syntax.

## Useful compiler flags

The integrations lean on three compiler options:

| Flag                                                                         | Why                                                                                                             |
| ---------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------- |
| `--error-style=gcc`                                                          | One-line `file:line:col: error: message` diagnostics, which every editor's error parser already understands.    |
| `--error-format=json`                                                        | One JSON object per diagnostic, for editors that build structured diagnostics.                                  |
| `salam inspect FILE --emit-symbol-xml --xml-out=/dev/null --log-level=error` | Lex, parse and analyze without running the program or writing an artifact: the check every "lint on save" uses. |

## Licensing

Everything under `extensions/` is MIT, while the compiler and standard library
at the repository root are GPLv3. The split is deliberate and predates this
directory: a syntax definition is only useful if other people can vendor it,
and plugin registries and projects like
[GitHub Linguist](https://github.com/github-linguist/linguist) take grammar
dependencies only under permissive licences.

The plugin code here talks to the compiler by running it as a separate
program and never links against or imports the standard library, so nothing
in this directory carries a copyleft obligation.

`tools/gen_syntaxes.salam` is the exception: it is written in Salam and
imports the standard library, so it is covered by the repository's GPLv3
licence. It is a build-time tool and is not part of any plugin. Its output is
data, and that data is MIT along with the rest of this directory.
