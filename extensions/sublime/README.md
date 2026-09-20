# Salam for Sublime Text

Syntax highlighting, indentation, snippets, build commands and a formatter for
the [Salam programming language](https://github.com/SalamLang/Salam) in
Sublime Text 3 and 4.

Salam keywords are spelled in English, Persian or Arabic, and one file may use
any of the three. All three spellings are highlighted, and the compiler's
keyword pack is detected per file, so a Persian program needs no configuration.

## Install

Copy or symlink this directory into your Sublime `Packages` folder under the
name `Salam`:

| Platform | Packages folder                                       |
| -------- | ----------------------------------------------------- |
| Linux    | `~/.config/sublime-text/Packages`                     |
| macOS    | `~/Library/Application Support/Sublime Text/Packages` |
| Windows  | `%APPDATA%\Sublime Text\Packages`                     |

```sh
git clone https://github.com/SalamLang/Salam.git
ln -s "$PWD/Salam/extensions/sublime" ~/.config/sublime-text/Packages/Salam
```

Restart Sublime Text. Any `.salam` file now opens as Salam; the syntax is also
picked for a file whose first line is a `//!` directive header.

## Features

- Highlighting for keywords, primitive types, intrinsic types and methods,
  string forms (`"..."`, `"""..."""`, `` `...` ``, `'c'`), every numeric base,
  annotations, `//!` directive headers and layout-DSL elements.
- Indentation: a trailing `:` opens a block, `end` closes it, `else` dedents.
- Goto Symbol (`Ctrl+R`) lists functions and types, including Persian and
  Arabic names.
- 24 snippets, the same set the Visual Studio Code extension ships.
- Build commands for running, building, checking, formatting and inspecting.
- `Salam: Format File` and `Salam: Check File` in the command palette.

## Build commands

Pick one with `Tools > Build System > Salam`, then `Ctrl+B`. `Ctrl+Shift+B`
chooses a variant:

| Variant                       | Command                             |
| ----------------------------- | ----------------------------------- |
| _(default)_ Run (interpreter) | `salam exec`                        |
| Build and Run                 | `salam run`                         |
| Build Executable              | `salam build`                       |
| Build Project                 | `salam build` in the project folder |
| Check (no run)                | `salam inspect --emit-symbol-xml`   |
| Format                        | `salam format`                      |
| Check Formatting              | `salam format --check`              |
| Show Tokens / Show AST        | `salam inspect --emit-*-xml`        |
| Build Web Page                | `salam web`                         |
| Compile to JavaScript         | `salam js`                          |

Errors are reported in the gcc one-line style, so `F4` walks them.

## Settings

`Preferences > Package Settings > Salam > Settings`, or
`Preferences: Salam Settings` in the command palette.

| Setting           | Default   | Meaning                                                                                                     |
| ----------------- | --------- | ----------------------------------------------------------------------------------------------------------- |
| `compiler_path`   | `"salam"` | Compiler executable; an absolute path when it is not on `PATH`.                                             |
| `language_pack`   | `"auto"`  | `--lang` passed to the compiler. `auto` reads a `//! lang:` directive, then falls back to script detection. |
| `format_indent`   | `"4"`     | `--indent` for the formatter: `tab`, or 1-16 spaces.                                                        |
| `format_on_save`  | `false`   | Reformat on every save.                                                                                     |
| `stdlib_path`     | `""`      | `--stdlib-path` for `std/...` imports.                                                                      |
| `timeout_seconds` | `60`      | Seconds a compiler run may take before it is killed.                                                        |

The build commands call `salam` directly and do not read `compiler_path`. If the
compiler is not on your `PATH`, copy `Salam.sublime-build` into
`Packages/User/` and put the absolute path in it.

## Snippets

`main`, `func`, `funcv`, `if`, `ifelse`, `until`, `each`, `eachi`, `eachkv`,
`repeat`, `repeati`, `repeatrange`, `var`, `mut`, `const`, `struct`, `enum`,
`interface`, `impl`, `import`, `extern`, `layout`, `component`, `println`.

## Maintenance

`Salam.sublime-syntax`, `Salam.sublime-completions` and the `.tmPreferences`
files are generated. Do not edit them; edit the generator and rerun it from
the repository root:

```sh
salam exec extensions/tools/gen_syntaxes.salam
```

The keyword lists come from `compiler/langpack.salam` and
`compiler/semantic/semantic.salam`, so the highlighting cannot drift away from
what the compiler accepts.

## License

MIT. See [LICENSE](LICENSE).
