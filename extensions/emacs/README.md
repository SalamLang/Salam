# Salam for Emacs

`salam-mode`: font locking, indentation, imenu, snippets, formatting and type
checking for the [Salam programming language](https://github.com/SalamLang/Salam).

Salam keywords are spelled in English or Persian. Both are
recognised, Arabic-script letters and ZWNJ count as word constituents, and
source lines stay left to right even when the code in them is Persian.

## Install

`use-package` with a local checkout:

```elisp
(use-package salam-mode
  :load-path "~/src/Salam/extensions/emacs"
  :mode "\\.salam\\'")
```

`straight.el`:

```elisp
(straight-use-package
 '(salam-mode :type git :host github :repo "SalamLang/Salam"
              :files ("extensions/emacs/*.el" "extensions/emacs/snippets")))
```

Manually:

```elisp
(add-to-list 'load-path "~/src/Salam/extensions/emacs")
(require 'salam-mode)
```

The `salam` compiler must be on `exec-path`, or `salam-compiler` must give its
full path.

## Commands

| Key       | Command                      | What it does                                   |
| --------- | ---------------------------- | ---------------------------------------------- |
| `C-c C-r` | `salam-run`                  | Run the project (`salam run`) or the file      |
| `C-c C-b` | `salam-build`                | `salam build` for the project or the file      |
| `C-c C-c` | `salam-check`                | Type-check without running                     |
| `C-c C-f` | `salam-format-buffer`        | Reformat, keeping point                        |
| `C-c C-p` | `salam-select-platform`      | Choose the platform for run and build          |
| `C-c C-n` | `salam-new-project`          | Run the `salam init` wizard in a terminal      |
| `C-c C-u` | `salam-goto-enclosing-block` | Jump to the enclosing block (prefix: further)  |
| `C-c C-j` | `salam-pick-block`           | Choose one of the enclosing blocks to jump to  |

`salam-run`, `salam-build` and `salam-check` go through `compile`, so `C-x \``
walks the errors.

## Projects and platforms

In a project made by `salam init` (a `salam.toml` above the file),
`salam-run` runs `salam run` from the project root, which serves a web
project with live reload, and `salam-build` runs `salam build`.

`salam-select-platform` offers the `platforms` listed under `[project]` in
`salam.toml` (every platform outside a project), plus `auto` and `all`. The
choice is remembered per project in `salam-platform-file` and added as
`--platform=NAME`. Outside a project `salam-run` keeps using
`salam-run-command` until a platform is chosen. `all` and `js` apply to
builds only.

`salam-new-project` asks for a directory and runs the `salam init` wizard in a
terminal buffer.

## Block chain

`salam-block-chain-mode` is on in every Salam buffer. It shows the headers
that enclose point, found by indentation, for example `[page > box > button]`
in the mode line. To use the header line instead:

```elisp
(setq salam-block-chain-display 'header-line)   ; or 'mode-line, or nil
```

The innermost block is also what `which-function-mode` reports.
`salam-goto-enclosing-block` moves to the header of the block around point,
`C-u 3 C-c C-u` three blocks outward, and `C-x C-x` comes back.
`(salam-block-chain)` returns the chain as a string for your own mode line.

## Options

```elisp
(setq salam-compiler "salam"
      salam-lang "auto"          ; auto | en | fa | ar
      salam-indent-offset 4
      salam-format-indent "4"    ; tab, or 1-16
      salam-format-on-save nil
      salam-run-command "exec"   ; exec | run
      salam-stdlib-path ""
      salam-block-chain-display 'mode-line  ; mode-line | header-line | nil
      salam-block-chain-separator " > ")
```

`salam-lang` of `"auto"` reads a `// زبان: فارسی` or `// lang: en` directive, then falls back to
detecting the script the buffer is written in.

## Language server

`salam-mode` registers `salam lsp` with both
[eglot](https://www.gnu.org/software/emacs/manual/html_node/eglot/) (built into
Emacs 29) and [lsp-mode](https://emacs-lsp.github.io/lsp-mode/). Run
`M-x eglot` or `M-x lsp` in a Salam buffer, or start it for every buffer:

```elisp
(add-hook 'salam-mode-hook #'eglot-ensure)
```

`salam-lsp-messages` picks the language of the server's messages (`"en"` or
`"fa"`); `salam-stdlib-path` is passed on too. See
[`docs/LSP.md`](../../docs/LSP.md) for what the server does.

## Flycheck

Optional, and separate so `salam-mode` does not depend on it:

```elisp
(require 'flycheck-salam)
```

The checker runs the compiler's own analyzer, so it reports exactly what a
build would.

## Snippets

`snippets/salam-mode/` holds 24 YASnippet definitions and registers itself
with `yas-snippet-dirs` when YASnippet is loaded. Triggers: `main`, `func`,
`funcv`, `if`, `ifelse`, `until`, `each`, `eachi`, `eachkv`, `repeat`,
`repeati`, `repeatrange`, `var`, `mut`, `const`, `struct`, `enum`,
`interface`, `impl`, `import`, `extern`, `export`, `page`, `component`, `println`.

## What is generated

`salam-mode.el` and everything under `snippets/` are generated. Edit the
generator, not these files, and rerun it from the repository root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=120000
```

The keyword lists come from `compiler/langpack.salam` and
`compiler/semantic/semantic.salam`, so highlighting cannot drift away from
what the compiler accepts. `flycheck-salam.el` and `salam-project.el` are
hand-written.

## License

MIT. See [LICENSE](LICENSE).
