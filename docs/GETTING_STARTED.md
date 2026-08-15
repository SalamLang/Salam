# 🚀 Getting Started with Salam

## What is Salam?

**Salam** (سلام - _peace_) is a statically typed, compiled, general-purpose systems programming language designed to be efficient, readable, and inclusive.

- Write code in **English**, **Persian (فارسی)**, or **Arabic (العربية)**.
- The general language transpiles to C and compiles to a native executable.
- A built-in **layout DSL** compiles `.salam` files directly to HTML, CSS, and JavaScript.
- Runs in the browser via the [web playground](https://salamlang.github.io/Salam/).

## ✅ Prerequisites

| Requirement                        | Notes                                                                                                                 |
| ---------------------------------- | --------------------------------------------------------------------------------------------------------------------- |
| A C compiler                       | [**tcc**](https://bellard.org/tcc/) (bundled, fast), [gcc](https://gcc.gnu.org/), or [clang](https://clang.llvm.org/) |
| [CMake](https://cmake.org/) ≥ 3.15 | Optional - needed for the CMake build and test suite                                                                  |
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

# define a compile-time constant the program reads by name
salam build app.salam -dMAX_USERS=500 -dBUILD_TAG=nightly
```

`-D` and `-d` are different tools. `-DNAME` sets a conditional-compilation
flag, which is only legal in an `if` condition. `-dNAME=VALUE` defines a
constant, as if the source had written `const NAME := VALUE` at top level:

```salam
func main:
    println MAX_USERS       // 500, an integer
    println BUILD_TAG       // "nightly", a string
end
```

The name must be upper case (`A-Z`, `0-9` and `_`; `-` is not allowed), so a
build-injected constant reads as one wherever it appears. The value is taken
as an integer, a float, `true`/`false`, or otherwise a string; wrap it in
quotes to force the string reading (`-dVERSION="1.20"`). A bare `-dNAME`
means `true`. A constant declared in the source wins over one defined here.

The compiler's own build info is available under the same mechanism, with no
flag needed: `SALAM_VERSION`, `SALAM_VERSION_CODE`, `SALAM_GIT_COMMIT`,
`SALAM_GIT_DATE`, `SALAM_GIT_DIRTY`, and `SALAM_BUILD_DATE` - the last being
the moment your program was compiled, so a binary can print its own build
stamp.

### Project entry file: `salam.salam`

Passing a single file always compiles exactly that file. When you pass a
**directory** instead - or nothing at all - the compiler builds "the project
in that directory", and the fixed filename `salam.salam` is the project's
entry file:

```sh
salam build                 # build the project in the current directory
salam build .               # same
salam build ../myproject/   # build the project in ../myproject/
salam run                   # build + run the project here
salam exec                  # interpret the project here
```

- If `<dir>/salam.salam` exists, it must define `main` and is used as the
  entry file; no scanning or guessing happens.
- If it does not exist, the directory's top-level `.salam` files are scanned
  for exactly one file defining `main` (more than one is an error that
  suggests naming the entry file `salam.salam`).
- The default executable name for a project is its **directory name**
  (`myproject.exe`), not `salam.exe`.
- `salam new <name>` scaffolds the project with a `salam.salam` entry file.

Frontend (layout DSL) projects have no `main` function; there the same rule
applies to the `layout` block instead:

```sh
salam web                   # build the page project here: salam.salam if
                            # present, else the single .salam layout page
salam layout build          # salam.salam if present, else EVERY layout
                            # page in the directory (multi-page site)
```

A `salam.salam` layout page compiles to `index.html` (the web server entry
name), so `salam serve` picks it up directly.

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

## 🌐 Layout DSL - compile to HTML/CSS/JS

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
| [Web Playground](https://salamlang.github.io/Salam/)                                                                         | Try Salam in your browser - no install needed                   |
| [Examples](../tests/en/)                                                                                                     | Ready-to-run programs covering basics, types, stdlib, and more  |
| [Editor readme](../editor/README.md)                                                                                         | How the WebAssembly playground is built and served              |
| [Platform & word-size support](PLATFORMS.md)                                                                                 | 32-bit vs 64-bit, C backend vs LLVM backend, release matrix     |
| [Contributing Guide](../CONTRIBUTING.md)                                                                                     | How to fork, branch, build, and open a pull request             |
| [Code of Conduct](../CODE_OF_CONDUCT.md)                                                                                     | Community standards                                             |
| [Visual Studio Code extension](https://marketplace.visualstudio.com/items?itemName=salamlanguage.salam-programming-language) | Syntax highlighting and language support for Visual Studio Code |
| [Discord](https://discord.gg/HfY3QHDPdv)                                                                                     | Real-time community chat                                        |
| [Telegram](https://t.me/SalamProgrammingLanguage)                                                                            | Community on Telegram                                           |
