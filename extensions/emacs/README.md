# Salam for Emacs

`salam-mode`: font locking, indentation, imenu, snippets, formatting and type
checking for the [Salam programming language](https://github.com/SalamLang/Salam).

Salam keywords are spelled in English, Persian or Arabic. All three are
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

| Key       | Command               | What it does               |
| --------- | --------------------- | -------------------------- |
| `C-c C-r` | `salam-run`           | Run the file               |
| `C-c C-b` | `salam-build`         | Compile to an executable   |
| `C-c C-c` | `salam-check`         | Type-check without running |
| `C-c C-f` | `salam-format-buffer` | Reformat, keeping point    |

`salam-run`, `salam-build` and `salam-check` go through `compile`, so `C-x \``
walks the errors.

## Options

```elisp
(setq salam-compiler "salam"
      salam-lang "auto"          ; auto | en | fa | ar
      salam-indent-offset 4
      salam-format-indent "4"    ; tab, or 1-16
      salam-format-on-save nil
      salam-run-command "exec"   ; exec | run
      salam-stdlib-path "")
```

`salam-lang` of `"auto"` reads a `//! lang:` directive, then falls back to
detecting the script the buffer is written in.

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
`interface`, `impl`, `import`, `extern`, `layout`, `component`, `println`.

## What is generated

`salam-mode.el` and everything under `snippets/` are generated. Edit the
generator, not these files, and rerun it from the repository root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=120000
```

The keyword lists come from `compiler/langpack.salam` and
`compiler/semantic/semantic.salam`, so highlighting cannot drift away from
what the compiler accepts. `flycheck-salam.el` is hand-written.

## License

MIT. See [LICENSE](LICENSE).
