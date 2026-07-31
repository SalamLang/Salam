# 🚀 Getting Started with Salam

## What is Salam?

**Salam** (سلام — _peace_) is a statically typed, compiled, general-purpose systems programming language designed to be efficient, readable, and inclusive.

- Write code in **English**, **Persian (فارسی)**, or **Arabic (العربية)**.
- The general language transpiles to C and compiles to a native executable.
- A built-in **layout DSL** compiles `.salam` files directly to HTML, CSS, and JavaScript.
- Runs in the browser via the [web playground](https://salamlang.github.io/Salam/).

## ✅ Prerequisites

| Requirement                 | Notes                                                                                                                                                                                 |
| --------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| A C compiler                | [**tcc**](https://bellard.org/tcc/) (bundled, fast), [gcc](https://gcc.gnu.org/), or [clang](https://clang.llvm.org/) — used at runtime by `salam build`, not to build `salam` itself |
| [Git](https://git-scm.com/) | To clone the repository                                                                                                                                                               |

No other runtime or library is required. `salam build` inlines a small C runtime into every generated file, so programs link only `-lm`.

## 🔨 The compiler

Salam is **self-hosted** — the compiler ([`compiler/`](../compiler/)) is
written entirely in Salam, and `compiler/salam` is the tracked, prebuilt
bootstrap binary (Linux). There is no C source and nothing to build on first
clone. To rebuild it from current source (self-hosting: the current
`compiler/salam` builds a fresh one from `compiler/main.salam`):

```sh
sh compiler/tools/bash/build-compiler.sh   # rebuilds compiler/salam
```

Run the end-to-end test suite the same way `compiler-build-test.yml` (CI) does:

```sh
sh compiler/tools/bash/run-tests.sh
```

## 👋 Hello, World!

Save the following to `hello.salam`:

```salam
func main {
    println "Hello, World!"
}
```

Compile and run:

```sh
salam build hello.salam --output=hello
./hello
```

Expected output:

```
Hello, World!
```

## 🏗️ Compiling a program with `salam build`

```sh
# compile to a native executable
salam build app.salam --output=app

# keep the generated C source for inspection
salam build app.salam --keep-c

# compile to an object file only
salam obj app.salam

# pass a compile-time flag (for conditional compilation)
salam build app.salam -DDEBUG
```

More examples live in [`tests/en/`](../tests/en/).

## 🎨 Formatting with `salam format`

`salam format` rewrites source files in place to canonical style.

```sh
# format a single file
salam format app.salam

# format every .salam file under the current directory, recursively
salam format

# check formatting without modifying files (exits 1 if any file needs changes)
salam format --check

# format with 2-space indentation
salam format app.salam --indent=2

# format with tabs
salam format app.salam --tabs

# format a Persian-language source file
salam format app.salam --lang=fa
```

## 🌐 Layout DSL — compile to HTML/CSS/JS

Salam includes a built-in DSL for describing web layouts.

```sh
# compile a layout file to page.html + page.css + page.js
salam layout build page.salam

# produce a single self-contained HTML file
salam layout build page.salam --inline

# compile multiple pages (merged style.css / script.js)
salam layout build a.salam b.salam
```

## 🖥️ Interactive REPLs

```sh
salam cli      # general-language REPL
salam layout   # layout DSL REPL
```

## 📚 Where to go next

| Resource                                                                                                                     | Description                                                     |
| ---------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------- |
| [Web Playground](https://salamlang.github.io/Salam/)                                                                         | Try Salam in your browser — no install needed                   |
| [Examples](../tests/en/)                                                                                                     | Ready-to-run programs covering basics, types, stdlib, and more  |
| [Editor readme](../editor/README.md)                                                                                         | How the WebAssembly playground is built and served              |
| [Platform & word-size support](PLATFORMS.md)                                                                                 | 32-bit vs 64-bit, C backend vs LLVM backend, release matrix     |
| [Contributing Guide](../CONTRIBUTING.md)                                                                                     | How to fork, branch, build, and open a pull request             |
| [Code of Conduct](../CODE_OF_CONDUCT.md)                                                                                     | Community standards                                             |
| [Visual Studio Code extension](https://marketplace.visualstudio.com/items?itemName=salamlanguage.salam-programming-language) | Syntax highlighting and language support for Visual Studio Code |
| [Discord](https://discord.gg/HfY3QHDPdv)                                                                                     | Real-time community chat                                        |
| [Telegram](https://t.me/SalamProgrammingLanguage)                                                                            | Community on Telegram                                           |
