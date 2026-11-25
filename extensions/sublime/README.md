# Salam for Sublime Text

Syntax highlighting, indentation, snippets, build commands and a formatter for
the [Salam programming language](https://github.com/SalamLang/Salam) in
Sublime Text 3 and 4.

Salam keywords are spelled in English or Persian, and one file may use
either. Both spellings are highlighted, and the compiler's
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
- Goto Symbol (`Ctrl+R`) lists functions and types, including Persian names.
- 24 snippets, the same set the Visual Studio Code extension ships.
- Build commands for running, building, checking, formatting and inspecting.
- Project and platform aware run and build, `Salam: New Project` and
  `Salam: Select Platform`.
- A status bar entry with the blocks around the cursor.
- `Salam: Format File` and `Salam: Check File` in the command palette.

## Build commands

Pick one with `Tools > Build System > Salam`, then `Ctrl+B`. `Ctrl+Shift+B`
chooses a variant:

| Variant                         | Command                                      |
| ------------------------------- | -------------------------------------------- |
| _(default)_ Run                 | `salam run` in a project, else `salam exec`  |
| Run (interpreter)               | `salam exec`                                 |
| Build and Run                   | `salam run FILE`                             |
| Build (chosen platform)         | `salam build [FILE] --platform=NAME`         |
| Build Executable                | `salam build FILE`                           |
| Build Project                   | `salam build` in the project folder          |
| Build Project for All Platforms | `salam build --platform=all`                 |
| Build Web Page                  | `salam build FILE --platform=web`            |
| Compile to JavaScript           | `salam build FILE --platform=js`             |
| Check (no run)                  | `salam inspect --emit-symbol`                |
| Format                          | `salam format`                               |
| Check Formatting                | `salam format --check`                       |
| Show Tokens / Show AST          | `salam inspect --emit-tokens` / `--emit-ast` |

Errors are reported in the gcc one-line style, so `F4` walks them.

## Projects and platforms

A project made by `salam init` has a `salam.toml` whose `[project]` section
lists `platforms`. The default build, `Salam: Run` and `Salam: Build` find that
file above the current one, run from the project folder, and add
`--platform=NAME` for the platform you chose. A web project served by
`salam run` reloads the browser on every save.

Command palette:

- `Salam: Select Platform` lists the project's platforms (all of them outside a
  project), plus `auto` and `all`. The choice is remembered per project.
- `Salam: New Project` asks for a folder, the type, the platforms, and for a
  `dsl` project the design system and web mode, runs `salam init` with those
  answers and adds the folder to the window.
- `Salam: Run` and `Salam: Build` do what the default build and
  `Build (chosen platform)` do without a menu choice.

`all` and `js` apply to builds only; a run with either falls back to the
default run.

## Status bar

The status bar shows the blocks around the cursor, found by indentation, and
the chosen platform:

```
Salam: page > box > button  |  platform: web
```

`Salam: Go to Enclosing Block` (`Alt+Shift+Up`) jumps to the header of the block
around the cursor, and `Salam: Go to Block in Chain` lists the whole chain.
`status_block_chain` and `status_platform` turn the two parts off.

## Language server

With the [LSP](https://packagecontrol.io/packages/LSP) package installed, add
this to **Preferences | Package Settings | LSP | Settings** for diagnostics as
you type, completion, hover, go to definition, references and rename:

```json
{
  "clients": {
    "salam": {
      "enabled": true,
      "command": ["salam", "lsp"],
      "selector": "source.salam"
    }
  }
}
```

See [`docs/LSP.md`](../../docs/LSP.md) for the server's options.

## Settings

`Preferences > Package Settings > Salam > Settings`, or
`Preferences: Salam Settings` in the command palette.

| Setting              | Default   | Meaning                                                                                                    |
| -------------------- | --------- | ---------------------------------------------------------------------------------------------------------- |
| `compiler_path`      | `"salam"` | Compiler executable; an absolute path when it is not on `PATH`.                                            |
| `run_command`        | `"exec"`  | What a single file outside a project runs with: `exec` or `run`.                                           |
| `status_block_chain` | `true`    | Show the block chain in the status bar.                                                                    |
| `status_platform`    | `true`    | Show the chosen platform in the status bar.                                                                |
| `language_pack`      | `"auto"`  | `--lang` passed to the compiler. `auto` passes nothing and the compiler detects the language of each file. |
| `format_indent`      | `"4"`     | `--indent` for the formatter: `tab`, or 1-16 spaces.                                                       |
| `format_on_save`     | `false`   | Reformat on every save.                                                                                    |
| `stdlib_path`        | `""`      | `--stdlib-path` for `std/...` imports.                                                                     |
| `timeout_seconds`    | `60`      | Seconds a compiler run may take before it is killed.                                                       |

`Salam: Run`, `Salam: Build` and the default build read `compiler_path`. The
other variants call `salam` directly; if the compiler is not on your `PATH`,
copy `Salam.sublime-build` into `Packages/User/` and put the absolute path in it.

## Snippets

`main`, `func`, `funcv`, `if`, `ifelse`, `until`, `each`, `eachi`, `eachkv`,
`repeat`, `repeati`, `repeatrange`, `var`, `mut`, `const`, `struct`, `enum`,
`interface`, `impl`, `import`, `extern`, `export`, `page`, `component`, `println`.

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
