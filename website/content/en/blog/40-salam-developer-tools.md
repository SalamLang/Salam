---
title: The Salam developer toolchain: language server, editor plugins, debugger, formatter, REPL and test runner
seo_title: Salam Developer Tools: LSP, Editors, Debugger, Tests and REPL
description: Salam's developer tools: the built-in language server, plugins for 11 editors, quick fixes, formatter, test runner, REPL, debugger and memory checks.
keywords: developer tools, language server, LSP, code formatter, test runner, REPL, debugger, VS Code extension, editor plugins, Salam toolchain, quick fixes
category: build
summary: A language is only as pleasant as its tools. Salam puts the compiler, language server, formatter, test runner, REPL, debugger launcher and memory checker in one executable, with plugins for eleven editors. What each tool does, with real output, and what is still missing.
date: 2026-11-11
---

The Salam toolchain is a single executable, `salam`, that contains the compiler and almost every tool around it. **The language server, formatter, test runner, REPL, documentation generator, debugger launcher and memory checker are all subcommands of the same program**, so they always agree with the compiler about the language, and editors get the same diagnostics you see on the command line. Plugins for eleven editors, a tree-sitter grammar and an online editor complete the set.

This article goes through the tools a working developer uses every day, shows what they print, and ends with the gaps a young toolchain still has.

## One executable, one source of truth {#one-executable}

Most language ecosystems grow their tools separately: a compiler from one team, a formatter from another, a language server that reimplements parts of the type checker. Salam's tools share code with the compiler instead. `salam lsp` runs the compiler's own parser and semantic analysis, `salam format` is the formatter the language server calls, and the editor plugins take their keyword lists from the compiler's tables. The consequence is practical: when a release adds a keyword or a diagnostic, every tool knows about it at once, in English and in Persian.

| Task | Command |
|---|---|
| Find errors without building | `salam check app.salam` |
| Apply the safe fixes | `salam check app.salam --fix` |
| Format code | `salam format .` |
| Run tests | `salam test` |
| Interactive prompt | `salam cli` (general) or `salam page` (page language) |
| Debug | `salam debug app.salam` |
| Memory errors | `salam memcheck app.salam` |
| API documentation | `salam doc` |
| Language server | `salam lsp` |

## Compiler diagnostics and quick fixes {#diagnostics}

The first tool is the compiler itself. `salam check` runs everything a build does before code generation, imported packages included, and exits with 1 when it prints an error, which makes it the fast inner loop. Diagnostics carry stable codes and, where the compiler can tell what you meant, a fix. Here is a test file that guessed a function name:

```text
error[E012]: package 'str' has no exported function 'ToLower'; did you mean 'IsLower' or 'Lower'?
 --> slug_test.salam:5:21
  |
5 |     ret str.Replace(str.ToLower(str.Trim(title)), " ", "-")
  |                     ^
  |
  = fix: Change to 'IsLower'
  = fix: Change to 'Lower'
```

Some fixes are safe enough to apply automatically: missing or unused imports, a `mut` that is never needed, and keywords borrowed from other languages such as `return` or `for`. `salam check --fix` applies those in place and checks again. On a file that declared `mut total := 10` and never changed it, the compiler first reported `error[E069]: variable 'total' is declared 'mut' but never mutated` with `= fix: Remove 'mut'`, and `--fix` printed `applied 1 fix(es)` and `no errors`. Two more options matter for tooling: `--error-format=json` emits each diagnostic with its fixes as byte-offset edits, and `--error-lang=fa` prints the messages in Persian for an English program, or the other way round. The article on [how the compiler works](/blog/how-the-salam-compiler-works/) explains where these checks run.

## The language server: salam lsp {#language-server}

`salam lsp` is a [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) server built into the compiler since version 0.5.8. Any editor with an LSP client gets the same features from it:

- errors and warnings as you type, with the compiler's codes and help text;
- hover with the declaration, type and doc comment, standard library included;
- go to definition, declaration and type definition, into `std/` as well;
- find references, highlight and rename across the workspace;
- call hierarchy, implementations of interfaces and type hierarchy;
- completion for names in scope, struct and package members, keywords and block snippets, plus signature help;
- inlay hints showing the inferred type after `name := value`;
- semantic highlighting that understands multi-word names, folding, outline, workspace symbols and smart selection;
- formatting through the compiler's formatter, and the compiler's quick fixes as code actions.

Messages follow the editor's language, so a Persian locale gets Persian diagnostics and hover text while the error codes stay the same. Options such as `stdlibPath`, `defines` for conditional compilation and `switchLint` go in `initializationOptions`. Setting it up is a few lines in most editors. In Helix, for example:

```toml
[language-server.salam]
command = "salam"
args = ["lsp"]

[[language]]
name = "salam"
scope = "source.salam"
file-types = ["salam"]
language-servers = ["salam"]
```

and in Sublime Text with the LSP package:

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

The repository's `docs/LSP.md` has the configuration for VS Code, Neovim, Emacs (eglot and lsp-mode), the JetBrains IDEs through LSP4IJ, Kate and others.

## Editor plugins for eleven editors {#editor-plugins}

The `extensions/` directory holds plugins for VS Code, Vim and Neovim, Sublime Text, Emacs, the JetBrains IDEs, Kate and KDevelop, Geany, gedit and GNOME (GtkSourceView), Notepad++, GNU nano and micro, plus a tree-sitter grammar. All of them highlight both the English and the Persian keyword sets, treat Arabic-script letters as identifier characters and accept ASCII, Persian and Arabic-Indic digits.

None of them keeps its own keyword list. A generator written in Salam reads the spellings from `compiler/langpack.salam` and the type and method names from the semantic analyzer, writes every syntax file, and CI fails if the committed files differ from what the generator produces. The VS Code extension, published on the [marketplace](https://marketplace.visualstudio.com/items?itemName=salamlanguage.salam-programming-language), adds run and build commands, snippets, the language server and debugging. The plugins are listed on the [tools page](/tools/).

## Formatting: salam format {#formatter}

`salam format` rewrites Salam source into the canonical style: four-space indentation by default, spaces after commas and around type annotations, and top-level declarations reordered to satisfy the compiler's ordering rules (imports, constants, globals, types, functions) when that is a pure reordering. Given this file:

```text
func area(w:int,h:int):int:
  ret w*h
end
func main:
        println area(3,4)
end
```

it printed `formatted messy.salam` and wrote:

```text
func area(w: int, h: int): int:
    ret w*h
end
func main:
    println area(3, 4)
end
```

`--check` reports files that need formatting without touching them and exits with 1, which is what you put in CI. `--tabs`, `--indent=N`, `--exclude=PATTERN` and a `.salamfmtignore` file cover the usual project needs, and `--minify` strips comments and indentation while keeping the token stream unchanged.

## Testing: salam test {#testing}

`salam test` needs no configuration. It walks the directory, finds every `*_test.salam` and `*.test.salam` file, compiles each in its own temporary directory and runs the files in parallel, one per CPU core by default. Output is printed in sorted file order, so two runs produce the same text. There are three kinds of test file.

**Unit tests** are `test_` functions that use the [`testing`](/std/testing/) package. The runner writes the `main` for you:

```salam fragment
import testing
import str

func slug(title: str): str:
    ret str.Replace(str.Lower(str.Trim(title)), " ", "-")
end

func test_slug_lowercases():
    testing.AssertEqStr(slug("Hello World"), "hello-world")
end

func test_slug_trims():
    testing.AssertEqStr(slug("  Salam  "), "salam")
end
```

Running `salam test` in that directory printed:

```text
=== RUN   test_slug_lowercases
--- PASS: test_slug_lowercases
=== RUN   test_slug_trims
--- PASS: test_slug_trims
ok   slug_test.salam: 2 tests passed
PASS: 1 test files
```

**Whole-program tests** have a `main`, and comment directives at the top check what it prints and how it exits. This file is a normal program you can run, and `salam test` also checks it:

```salam
// EXPECT-LINE: 15 FizzBuzz
// EXPECT-NOT: error
// EXPECT-EXIT: 0

func label(n: int): str:
    if n % 15 == 0:
        ret "FizzBuzz"
    else n % 3 == 0:
        ret "Fizz"
    else n % 5 == 0:
        ret "Buzz"
    end
    ret "plain"
end

func main:
    each n in [3, 5, 7, 15]:
        println n, label(n)
    end
end
```

```output
3 Fizz
5 Buzz
7 plain
15 FizzBuzz
```

**Compile-check tests** use `// EXPECT-CODE: E059` to assert that the compiler reports a specific diagnostic, which is how the compiler's own error messages are tested. Other directives restrict a test to an OS or architecture, set a timeout or compare output with a file, and every directive has a Persian spelling. `--run=NAME` filters tests, `--format=teamcity` and `--report=FILE` feed CI systems, and `--interp` runs the tests on the interpreter instead of compiling them.

## The REPL: salam cli {#repl}

`salam cli` starts an interactive prompt that keeps definitions between lines, which is handy for checking how an expression behaves. A short session, with input piped in:

```text
Salam general REPL (lang=en). ':help' for commands.
salam> x := 21
salam> println x * 2
42
salam> func sq(n: int): int:
  ...     ret n * n
  ... end
defined sq.
salam> println sq(9)
81
```

Redefining a function by name replaces it. `salam page` starts a second prompt for the page language. For quick experiments without installing anything, the [online editor](https://editor.salamlang.ir) runs the compiler as WebAssembly in the browser, and `salam exec` runs a file on the interpreter with no C toolchain at all, as described in [salam run vs salam build](/blog/salam-run-vs-salam-build/).

## Debugging and memory checking {#debugging}

`salam build -g` embeds DWARF debug information (version 5 by default, `-gdwarf-4` for older debuggers) and emits line directives, so a debugger shows Salam source lines. `salam debug app.salam` builds that way and launches GDB or LLDB. In VS Code, pressing F5 in a Salam file goes through `salam dap`, a Debug Adapter Protocol bridge that drives gdb or lldb-dap with breakpoints, stepping and locals in the Salam source. It is not available on Windows yet; there, build with `-g` and use a debugger directly.

For memory errors, `salam memcheck app.salam` builds with debug symbols and runs the program under Valgrind when it is installed, or builds with AddressSanitizer through gcc or clang otherwise. On a small program it reported the heap summary and finished with `salam memcheck: no memory errors detected.` Before any of that runs, the compiler's ownership checker warns about use after free, double free and pointers kept across a call that may grow a vector, and the range checks flag indexes, overflows and divisions by zero it can prove. Both are on by default and cost nothing at run time; [memory management in Salam](/blog/memory-management-in-salam/) explains them.

## Profiling, documentation and the gaps {#profiling-docs}

`--time-report` prints where the compiler itself spent its time, phase by phase, with the slowest input files, and `--time-trace` writes a Chrome trace you can open in Perfetto. These are tools for compile times. **A profiler for your own programs is not part of the toolchain yet**: the design document describes a `salam prof` command and a `salam bench` harness, and marks both as designed but not implemented. Until then, use the system profilers (`perf`, Instruments, Valgrind's callgrind) on a binary built with `-g`.

`salam doc` turns a project into a single self-contained HTML page of its public API. Doc comments are runs of `//` or `///` lines, or a `/** */` block, directly above a declaration, and `@en` and `@fa` tags let one comment carry both languages, with a language switcher in the generated page.

### What is missing {#gaps}

A fair list for a pre-1.0 toolchain:

- **No program profiler** of its own, as above.
- **Debugging in VS Code is not available on Windows** through `salam dap`.
- **Smaller editors get less.** Kate, gedit, Notepad++, nano and micro have highlighting; the language server features depend on the editor having an LSP client.
- **Fewer third-party integrations.** There is no ecosystem of linters, code-coverage services or IDE plugins written by others, as there is for Go or Rust. Coverage reporting in particular is absent.

The tools that exist are unusually consistent, because they are the compiler. For the wider picture of how packages, documentation and contributors fit together, see [building an open source language ecosystem](/blog/building-an-open-source-language-ecosystem/), and for habits that keep the compiler quiet, the [best practices lesson](/learn/best-practices/).

## Frequently asked questions {#faq}

### Does Salam have a language server?

Yes. `salam lsp` is built into the compiler, so there is nothing extra to install. It provides diagnostics, completion, hover, navigation, rename, formatting and quick fixes to any editor with an LSP client, in English and Persian.

### Which editors support Salam?

VS Code, Vim and Neovim, Sublime Text, Emacs, the JetBrains IDEs, Kate and KDevelop, Geany, gedit, Notepad++, nano and micro have plugins in the repository, and there is a tree-sitter grammar. Editors such as Helix work through the language server alone.

### How do I write unit tests in Salam?

Put `test_` functions in a file whose name ends in `_test.salam`, use the assertions of the `testing` package, and run `salam test`. Files with a `main` can be tested by their output with `// EXPECT` comments.

### Can I debug Salam code step by step?

Yes. Build with `-g` and use GDB or LLDB, or run `salam debug`, which does both. VS Code users press F5, which uses the `salam dap` adapter on Linux and macOS.
