# Editor support for Salam

Salam programs can be written with English or Persian keywords, and a
single file may use either of them. Every integration here highlights both,
treats Arabic-script letters as identifier characters, and accepts
numbers written with ASCII, Persian or Arabic-Indic digits.

| Editor             | Directory                         | Highlighting | Indent  | Snippets | Run / build | Format | Diagnostics                     |
| ------------------ | --------------------------------- | ------------ | ------- | -------- | ----------- | ------ | ------------------------------- |
| Visual Studio Code | [`vscode/`](vscode)               | yes          | yes     | yes      | yes         | yes    | via the compiler                |
| Vim / Neovim       | [`vim/`](vim)                     | yes          | yes     | yes      | yes         | yes    | quickfix, `vim.diagnostic`, ALE |
| Sublime Text       | [`sublime/`](sublime)             | yes          | yes     | yes      | yes         | yes    | build panel                     |
| Emacs              | [`emacs/`](emacs)                 | yes          | yes     | yes      | yes         | yes    | `compile`, flycheck             |
| Kate / KDevelop    | [`kate/`](kate)                   | yes          | generic | no       | no          | no     | no                              |
| gedit / GNOME      | [`gtksourceview/`](gtksourceview) | yes          | generic | no       | no          | no     | no                              |
| JetBrains IDEs     | [`jetbrains/`](jetbrains)         | yes          | generic | no       | no          | no     | no                              |
| Geany              | [`geany/`](geany)                 | yes          | generic | no       | yes         | yes    | build panel                     |
| Notepad++          | [`notepadpp/`](notepadpp)         | yes          | generic | no       | no          | no     | no                              |
| GNU nano           | [`nano/`](nano)                   | yes          | no      | no       | no          | `^T`   | linter view                     |
| micro              | [`micro/`](micro)                 | yes          | no      | no       | no          | no     | no                              |

Each directory has its own readme with installation instructions.

## Where the word lists come from

Nothing here keeps its own copy of the language's keywords. The generator
reads them out of the compiler at generation time:

- `compiler/langpack.salam` gives every keyword's spelling in both
  languages along with the token kind the lexer assigns it, so a keyword is
  grouped by what it _is_ rather than by how it is spelled.
- `compiler/semantic/semantic.salam` gives the primitive type names, the
  intrinsic types (`Vector`, `HashMap`, `MapIter`, `File`, `Variant`) and the
  intrinsic method names.

A new keyword therefore reaches every editor at once, in both languages,
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
build commands, documentation.

## Checking

Two Salam programs run over what the generator produced, and CI runs both:

```sh
salam exec extensions/tools/validate.salam --timeout=300000       # parses every definition, checks its patterns
salam exec extensions/tools/check_engines.salam --timeout=300000  # highlights the probes with real engines
```

`validate.salam` parses each definition, checks every regular expression in
it, checks the cross-references (Kate contexts and item data, GtkSourceView
refs and styles, TextMate repository includes, Sublime context includes and
`{{variable}}` definitions, GtkSourceView `\%{define-regex}` references) and
rejects what Go's RE2 cannot run in the micro rules: lookaround,
backreferences, atomic groups, possessive quantifiers and a repetition count
above 1000.

The pattern checking is a regular-expression syntax checker written for the
two flavours these files actually use, Oniguruma and RE2, rather than a
different engine's parser standing in for them. It reports unbalanced groups
and classes, a quantifier with nothing to repeat, a reversed `{n,m}` or
character range, a backreference to a group that does not exist, a malformed
`\x{...}` or a code point above U+10FFFF, and per flavour the constructs that
engine cannot run.

`check_engines.salam` hands four of the definitions to the engines that
actually read them: the Visual Studio Code grammar to `vscode-textmate` and
`vscode-oniguruma`, the Kate one to `kate-syntax-highlighter`, the Sublime
Text one to syntect (which ships inside `bat`), and the GtkSourceView one to
GtkSourceView through GObject introspection. Each highlights
`tools/fixtures/probe_{en,fa}.salam`. Those two files are the same
program written with the English and Persian keywords, and each one
compiles. The check asserts that a construct comes out styled the same way in
both, so dropping a Persian keyword from one definition fails the build.
Where an engine is not installed the check says so and skips rather than
failing.

Two of those engines can only be reached from their own runtime, so each gets
a driver that does nothing but dump `[token, style]` pairs as JSON:
`tools/check_textmate.mjs` for `vscode-textmate`, which is a JavaScript
library, and `tools/check_gtksource.py` for GtkSourceView, which is reachable
only through GObject introspection. Every decision the check makes lives in
`check_engines.salam`.

`tools/syntaxcheck.salam` holds the checking logic that `validate.salam`
drives, and `tools/syntaxcheck_test.salam` is its test suite: it feeds each
checker a deliberately broken definition and asserts on the diagnostic, then
runs every checker over the real files and expects silence.

```sh
salam exec extensions/tools/syntaxcheck_test.salam --timeout=300000
```

CI additionally loads the syntax in real Vim and checks what it highlights,
runs the indent file over an unindented file and diffs the result, and
byte-compiles the Emacs mode with warnings as errors.

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
| `salam inspect FILE --emit-symbol --xml-out=/dev/null --log-level=error` | Lex, parse and analyze without running the program or writing an artifact: the check every "lint on save" uses. |

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

The programs under `tools/` are the exception: `gen_syntaxes.salam`,
`validate.salam`, `syntaxcheck.salam` and `check_engines.salam` are written in
Salam and import the standard library, so they are covered by the repository's
GPLv3 licence. They are build-time tools and are not part of any plugin. Their
output is data, and that data is MIT along with the rest of this directory.
