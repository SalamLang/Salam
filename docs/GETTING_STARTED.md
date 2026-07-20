<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [🚀 Getting Started with Salam](#-getting-started-with-salam)
  - [What is Salam?](#what-is-salam)
  - [✅ Prerequisites](#-prerequisites)
  - [🔨 Build the compiler](#-build-the-compiler)
    - [Quick build (tcc)](#quick-build-tcc)
    - [CMake build (with tests)](#cmake-build-with-tests)
    - [Makefile build](#makefile-build)
  - [👋 Hello, World!](#-hello-world)
  - [🏗️ Compiling a program with `salam build`](#-compiling-a-program-with-salam-build)
  - [🎨 Formatting with `salam format`](#-formatting-with-salam-format)
  - [🌐 Layout DSL — compile to HTML/CSS/JS](#-layout-dsl--compile-to-htmlcssjs)
  - [🖥️ Interactive REPLs](#-interactive-repls)
  - [📚 Where to go next](#-where-to-go-next)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 🚀 Getting Started with Salam

## What is Salam?

**Salam** (سلام — _peace_) is a statically typed, compiled, general-purpose systems programming language designed to be efficient, readable, and inclusive.

- Write code in **English**, **Persian (فارسی)**, or **Arabic (العربية)**.
- The general language transpiles to C and compiles to a native executable.
- A built-in **layout DSL** compiles `.salam` files directly to HTML, CSS, and JavaScript.
- Runs in the browser via the [web playground](https://salamlang.github.io/Salam/).

## ✅ Prerequisites

| Requirement                        | Notes                                                                                                                 |
| ---------------------------------- | --------------------------------------------------------------------------------------------------------------------- |
| A C compiler                       | [**tcc**](https://bellard.org/tcc/) (bundled, fast), [gcc](https://gcc.gnu.org/), or [clang](https://clang.llvm.org/) |
| [CMake](https://cmake.org/) ≥ 3.15 | Optional — needed for the CMake build and test suite                                                                  |
| [Git](https://git-scm.com/)        | To clone the repository                                                                                               |

No other runtime or library is required. `salam build` inlines a small C runtime into every generated file, so programs link only `-lm`.

## 🔨 Build the compiler

The compiler source lives in [`compiler/`](../compiler/). All build commands are run from that directory.

```sh
cd compiler
```

### Quick build (tcc)

```sh
sh tools/bash/build-compiler.sh   # produces ./salam
```

The `salam` binary is also copied to the repository root.

### CMake build (with tests)

```sh
cmake -B build
cmake --build build
ctest --test-dir build   # run the test suite
```

### Makefile build

```sh
make   # release build -> ../salam at the repo root
```

## 👋 Hello, World!

Save the following to `hello.salam`:

```salam
func main {
    println("Hello, World!")
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

More examples live in [`examples/en/`](../examples/en/).

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
| [Examples](../examples/en/)                                                                                                  | Ready-to-run programs covering basics, types, stdlib, and more  |
| [Editor readme](../editor/README.md)                                                                                         | How the WebAssembly playground is built and served              |
| [Platform & word-size support](PLATFORMS.md)                                                                                 | 32-bit vs 64-bit, C backend vs LLVM backend, release matrix     |
| [Contributing Guide](../CONTRIBUTING.md)                                                                                     | How to fork, branch, build, and open a pull request             |
| [Code of Conduct](../CODE_OF_CONDUCT.md)                                                                                     | Community standards                                             |
| [Visual Studio Code extension](https://marketplace.visualstudio.com/items?itemName=salamlanguage.salam-programming-language) | Syntax highlighting and language support for Visual Studio Code |
| [Discord](https://discord.gg/HfY3QHDPdv)                                                                                     | Real-time community chat                                        |
| [Telegram](https://t.me/SalamProgrammingLanguage)                                                                            | Community on Telegram                                           |
