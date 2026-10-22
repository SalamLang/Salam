# 🚀 Getting Started with Salam

## 🌍 What is Salam?

**Salam** (سلام - _peace_) is a statically typed, compiled, general-purpose systems programming language designed to be efficient, readable, and inclusive.

- ✏️ Write code in **English** or **Persian (فارسی)**.
- ⚡ The general language transpiles to C and compiles to a native executable.
- 🌐 A built-in **layout DSL** compiles `.salam` files directly to HTML, CSS, and JavaScript.
- 🔗 Runs in the browser via the [web playground](https://editor.salamlang.ir/).

## ✅ Prerequisites

| Requirement                        | Notes                                                                                                                 |
| ---------------------------------- | --------------------------------------------------------------------------------------------------------------------- |
| A C compiler                       | [**tcc**](https://bellard.org/tcc/) (bundled, fast), [gcc](https://gcc.gnu.org/), or [clang](https://clang.llvm.org/) |
| [CMake](https://cmake.org/) ≥ 3.15 | Optional - needed for the CMake build and test suite                                                                  |
| [Git](https://git-scm.com/)        | To clone the repository                                                                                               |

No other runtime or library is required. `salam build` inlines a small C runtime into every generated file, so programs link only `-lm`.

## ⚡ Quick Install

The fastest way to get `salam` on your machine:

**GNU/Linux & macOS:**

```sh
{ curl -fsSL https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.sh || wget -qO- https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.sh; } | sh
```

The installer detects your OS and architecture and adds Salam to your PATH; open a new terminal afterwards.

Options go after `sh -s --`, for example `... | sh -s -- --dir ./bin --version 0.5.0`.
The shell installer takes `--dir DIR`, `--version X.Y.Z`, `--platform NAME`,
`--no-modify-path` and `--help`; run it with `--help` for the full list.

**Windows** (PowerShell):

```powershell
irm https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.ps1 -OutFile "$env:TEMP\salam-install.ps1"; powershell -NoProfile -ExecutionPolicy Bypass -File "$env:TEMP\salam-install.ps1"
```

Getting `Could not create SSL/TLS secure channel` from that `irm` call? That
happens when PowerShell's default protocol list doesn't include TLS 1.2,
which is common on older Windows Server builds even though the OS itself
supports it. Enable it for the session first, then retry:

```powershell
[Net.ServicePointManager]::SecurityProtocol = 3072 -bor [Net.ServicePointManager]::SecurityProtocol
irm https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.ps1 -OutFile "$env:TEMP\salam-install.ps1"; powershell -NoProfile -ExecutionPolicy Bypass -File "$env:TEMP\salam-install.ps1"
```

If that still fails, use the `cmd.exe` installer below instead - `curl.exe`
and `certutil` negotiate TLS through Windows itself, not .NET, so they are
unaffected by this.

Or from `cmd.exe`:

```bat
curl -fsSLo "%TEMP%\salam-install.bat" https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.bat && "%TEMP%\salam-install.bat"
```

Both land in `%USERPROFILE%\.salam\bin` and put that directory on your user
PATH; open a new terminal afterwards so the change is visible. `--dir DIR`
installs somewhere else, `--version X.Y.Z` picks an exact release, and
`--no-modify-path` leaves PATH untouched.

On Windows 7 and 8, where `curl.exe` and `irm` do not exist yet, bootstrap
with `certutil` instead:

```bat
certutil -urlcache -split -f https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.bat "%TEMP%\salam-install.bat" && "%TEMP%\salam-install.bat"
```

Or download a pre-built binary from [GitHub Releases](https://github.com/SalamLang/Salam/releases).

### Updating

Once Salam is installed, it updates itself:

```sh
salam upgrade           # newest release: compiler, std/ and bundled tools together
salam upgrade --check   # only say whether a newer release is out
salam upgrade 0.5.4     # install that exact release, older ones included
```

`salam upgrade` asks GitHub for the latest release and runs the same
installer as above against the directory `salam` runs from, so the whole
bundle is replaced at once and an old `std/` never lingers. It leaves PATH
alone. A copy that Homebrew, Scoop, Chocolatey, winget, npm, Nix, snap or a
Linux distribution package installed is left to that package manager, and
`salam upgrade` prints the command to run instead (`--force` overrides that).
A compiler built from a source checkout is updated with `git pull` and a
rebuild.

## 🔨 Build the Compiler from Source

The compiler source lives in [`compiler/`](../compiler/). All build commands are run from that directory.

```sh
cd compiler
```

### ⚡ Quick build (tcc)

```sh
sh tools/bash/build-selfhost.sh   # produces ./salam
```

The `salam` binary is also copied to the repository root.

### 🏗️ CMake build (with tests)

```sh
cmake -B build
cmake --build build
ctest --test-dir build   # run the test suite
```

### 🔧 Makefile build

```sh
make   # release build -> ../salam at the repo root
```

## 👋 Hello, World!

Save the following to `hello.salam`:

```salam
func main:
    println "Hello, World!"
end
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

## 🏗️ Compiling a Program with `salam build`

```sh
# compile to a native executable
salam build app.salam --output=app

# keep the generated C source for inspection
salam build app.salam --keep-temp

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

### 📁 Project entry file: `main.salam`

Passing a single file always compiles exactly that file. When you pass a
**directory** instead, or nothing at all, the compiler builds "the project
in that directory", and its entry file is always `main.salam`:

```sh
salam build                 # build the project in the current directory
salam build .               # same
salam build ../myproject/   # build the project in ../myproject/
salam run                   # build + run the project here
salam exec                  # interpret the project here
```

- `<dir>/main.salam` must exist and define `main` (`ریشه` in Persian).
  No other filename is accepted and no scanning or guessing happens; a
  directory without `main.salam` is an error that names the file to rename.
- The default executable name for a project is its **directory name**
  (`myproject`, or `myproject.exe` on Windows), not `main`.
- `salam new <name>` creates `<name>/` with `main.salam`, `salam.toml` and a
  `.gitignore`. `salam init [dir]` does the same in an existing directory
  and never overwrites a file that is already there.

Frontend (layout DSL) projects have no `main` function; there `main.salam`
is the project page:

```sh
salam web                   # build the page project here: main.salam if
                            # present, else the single .salam page
salam page build            # main.salam if present, else EVERY layout
                            # page in the directory (multi-page site)
```

A `main.salam` layout page compiles to `index.html` (the web server entry
name), so `salam serve` picks it up directly.

### 🧩 Official packages

Many packages are maintained with Salam but do not ship with it, so a fresh
install stays small: the database clients (`db.sqlite`, `db.postgres`,
`db.redis`, `db.opensearch`, ...), machine learning (`tensor`, `nn`, `llm`, ...), audio,
the network services (`grpc`, `ftp`, `ssh`, `net.websocket`,
`net.http.swagger`, ...), `webview`, `opencv` and `excel`.
[`packages/`](../packages/) holds them all and `salam search` finds them.
Import them as usual (`import db.redis`) after adding them to the project:

```sh
salam add db.redis
```

`salam add` downloads the official packages that match your compiler's
version (`salam-packages-<version>.tar.gz` from its release) into
`~/.salam/pkg/salam-packages@v<version>/` and records the package in
`salam.toml` and `salam.lock`. `salam update` moves them to the compiler you
are running now. In a source checkout, `packages/` next to `std/` is used
directly, and `$SALAM_PACKAGES` points at another copy.

### 📦 Packages from GitHub

A package outside the standard library is named by its repository and
imported with a string:

```salam
import "github.com/owner/repo"        // binds repo
import t "github.com/owner/repo/sub"  // the package in the repo's sub/ folder
```

These imports work inside a project only. `salam.toml` lists what the project
needs and `salam.lock` records the exact version, commit and checksum of each
download:

```sh
salam add github.com/owner/repo           # newest release, or @v1.2.0
salam remove github.com/owner/repo
salam update [github.com/owner/repo]      # move to the newest release
salam deps                                # download what salam.lock lists, verify checksums
salam install github.com/owner/tool       # build a program's main.salam into ~/.salam/bin
salam install                             # the same for the project you are in
salam search websocket                    # standard library and registry/index.toml
```

A local package can stand in for a remote one with
`"example.com/me/lib" = { path = "../lib" }` in `[dependencies]`.
Downloads live in `~/.salam/pkg/<module>@<version>` (`$SALAM_PKG` moves it).
Git is used when it is installed; without it, GitHub packages arrive as a
tar.gz that Salam unpacks itself (`SALAM_NO_GIT=1` forces that path).
Dependencies of dependencies come from each package's own `salam.toml`, and
when two ask for different versions the newer one wins (minimal version
selection, as in Go).

### 🔗 Native libraries

A package that wraps a C library says so with `link dynamic "sqlite3"` and
can describe that library in a `salam.toml` beside its source:

```toml
[native.sqlite3]
pkg-config = "sqlite3"
source-url = "https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip"
source-sha256 = "77823cb1..."
sources = ["sqlite-amalgamation-3460100/sqlite3.c"]
cflags = ["-O2", "-DSQLITE_THREADSAFE=1"]
apt = "libsqlite3-dev"
brew = "sqlite"
```

When a build meets that `link` line it uses the system library if
pkg-config or the C compiler can find it. Otherwise it downloads the listed
sources, checks the sha256, compiles them once per compiler and caches the
objects under `~/.salam/pkg/native/`. With no sources it stops and prints the
install command for this OS. `salam add db.sqlite` does the same work up front.
`SALAM_NATIVE=source` or `SALAM_NATIVE=system` forces one way.

### 📚 Building a library

```sh
salam build --lib            # libNAME.so, libNAME.dylib, or NAME.dll + libNAME.dll.a
salam build --lib=static     # libNAME.a
```

The functions in the entry file's `export:` block become the library's C API,
under their own names, and `NAME.h` is written next to the library with their
prototypes. No `main` is needed: the runtime and the package's globals are
set up when the library is loaded. A Windows DLL can be built from Linux with
`--target=x86_64-windows-gnu --cc=x86_64-w64-mingw32-gcc`. Library builds go
through a C compiler, whichever backend is the default.

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

Top-level code is split into sections: `package`, imports, includes, each
`link` kind, `extern`, pub consts, consts, vars, types and funcs (pub and
private apart). The formatter puts exactly one empty line between two
sections. Inside a section your own empty lines are kept, at most one in a row.

## 🌐 Layout DSL - compile to HTML/CSS/JS

Salam includes a built-in DSL for describing web layouts.

```sh
# compile a layout file to page.html + page.css + page.js
salam page build page.salam

# produce a single self-contained HTML file
salam page build page.salam --inline

# compile multiple pages (merged style.css / script.js)
salam page build a.salam b.salam
```

A page is a tree of elements with `name = value` attributes:

```salam
page:
    title = "Hello"
    header:
        heading: size = 1 content = "Welcome" end
    end
    list:
        item: content = "Fast" end
        item: content = "Typed" end
    end
    link: url = "https://salamlang.ir" content = "Learn more" end
end
```

Elements and attributes are checked when the page is built: a misspelled name,
a value of the wrong type (a URL, a number, a color), a missing required
attribute or an element in the wrong place (an `item` outside a `list`) is a
compile error, with a suggestion for the closest valid name. See
[LAYOUT_ELEMENTS.md](LAYOUT_ELEMENTS.md) for every element and
[LAYOUT_SCHEMA.md](LAYOUT_SCHEMA.md) for how they are defined.

## 🖥️ Interactive REPLs

```sh
salam cli      # general-language REPL
salam page   # layout DSL REPL
```

## 🐳 Docker

Run the compiler in Docker without a local toolchain:

```sh
# production image - compile any .salam file
docker compose -f compiler/docker/docker-compose.yml run --rm prod build app.salam --output=app

# development mode - live-rebuild on source changes
docker compose -f compiler/docker/docker-compose.yml up dev
```

See the [Docker section in the main README](../README.md#-docker--docker-compose) for full details.

## 🧩 Editor Support

| Editor                 | Extension                                                                                                                                        |
| ---------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------ |
| **Visual Studio Code** | [salam-programming-language](https://marketplace.visualstudio.com/items?itemName=salamlanguage.salam-programming-language) - syntax highlighting |
| **Web Playground**     | [editor.salamlang.ir](https://editor.salamlang.ir/) - no install needed                                                                          |

## 📚 Where to Go Next

| Resource                                                                                                                     | Description                                                     |
| ---------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------- |
| [Web Playground](https://editor.salamlang.ir/)                                                                               | Try Salam in your browser - no install needed                   |
| [GitHub Releases](https://github.com/SalamLang/Salam/releases)                                                               | Pre-built binaries for Linux, macOS, and Windows                |
| [Examples](../tests/en/)                                                                                                     | Ready-to-run programs covering basics, types, stdlib, and more  |
| [Editor readme](../editor/README.md)                                                                                         | How the WebAssembly playground is built and served              |
| [Platform & word-size support](PLATFORMS.md)                                                                                 | 32-bit vs 64-bit, C backend vs LLVM backend, release matrix     |
| [Contributing Guide](../CONTRIBUTING.md)                                                                                     | How to fork, branch, build, and open a pull request             |
| [Code of Conduct](../CODE_OF_CONDUCT.md)                                                                                     | Community standards                                             |
| [Visual Studio Code extension](https://marketplace.visualstudio.com/items?itemName=salamlanguage.salam-programming-language) | Syntax highlighting and language support for Visual Studio Code |
| [Discord](https://discord.gg/HfY3QHDPdv)                                                                                     | Real-time community chat                                        |
| [Telegram](https://t.me/SalamProgrammingLanguage)                                                                            | Community on Telegram                                           |
