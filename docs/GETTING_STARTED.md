# 🚀 Getting Started with Salam

## 🌍 What is Salam?

**Salam** (سلام - _peace_) is a statically typed, compiled, general-purpose systems programming language designed to be efficient, readable, and inclusive.

- ✏️ Write code in **English** or **Persian (فارسی)**.
- ⚡ The general language transpiles to C and compiles to a native executable.
- 🌐 A built-in **layout DSL** compiles `.salam` files directly to HTML, CSS, and JavaScript.
- 🔗 Runs in the browser via the [web playground](https://editor.salamlang.ir/).

## ✅ Prerequisites

| Requirement                 | Notes                                                                                                                                                                                     |
| --------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| A Salam release             | The installer below. The Linux, macOS and Windows x86-64 releases have LLVM linked in and compile with it by default; the Windows on Arm release uses the native backend                  |
| A C compiler (sometimes)    | Only for the C backend (`--backend=c`, or a compiler without LLVM): [tcc](https://bellard.org/tcc/) (bundled on Windows), [gcc](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/) |
| [Git](https://git-scm.com/) | To clone the repository                                                                                                                                                                   |

## ⚡ Quick Install

The fastest way to get `salam` on your machine:

**GNU/Linux & macOS:**

```sh
{ curl -fsSL https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.sh || wget -qO- https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.sh; } | sh
```

The installer detects your OS and architecture and adds Salam to your PATH; open a new terminal afterwards.

Options go after `sh -s --`, for example `... | sh -s -- --dir ./bin --version 0.5.0`.
The shell installer takes `--dir DIR`, `--version X.Y.Z`, `--os NAME`, `--arch NAME`,
`--platform NAME`, `--no-modify-path` and `--help`. It detects your system and
processor by itself; `--os` (`linux`, `mac`, `freebsd`, `openbsd`, `netbsd`, `dragonfly`) and
`--arch` (`x86_64`, `i686`, `aarch64`, `armhf`, `riscv64`, `ppc64le`, `s390x`)
pick another build, and `--platform` names the exact release file. Run it with `--help` for
the full list.

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
`--no-modify-path` leaves PATH untouched. The installer finds your processor by itself;
`--arch` (`x86_64`, `i686`, `arm64`) picks another build, `--os windows` is accepted
(Linux, macOS and the BSDs use `install.sh`, and the error prints the command), and
`--platform` names the exact release file (`windows-x86_64`, `windows-i686`,
`windows-arm64`). `SALAM_OS`, `SALAM_ARCH` and `SALAM_PLATFORM` do the same for the
piped form, for example `$env:SALAM_ARCH = "i686"` before `irm ... | iex`.

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

The compiler is written in Salam. Its source lives in
[`compiler/`](../compiler/) and the standard library in [`std/`](../std/).
Building it needs an existing `salam` as the seed, such as a release from
the installer above.

```sh
sh tools/bash/build-selfhost.sh                     # seed: the salam on PATH -> ./salam
sh tools/bash/build-selfhost.sh --seed=/path/to/salam --output=/tmp/salam
```

`build-selfhost.sh` writes `salam` into the current directory and uses this
checkout's `std/`. The result has no LLVM linked in, so it compiles with the
C backend by default; pass `--llvm=DIR` with a `libsalam_llvm.a` built by
`std/llvm/native/build.sh` to link LLVM in.

`compiler/tools/bash/bootstrap.sh <seed>` does a three-stage build with a
fixpoint check, and `tools/bash/run-tests.sh` runs the compiler's test
suite (`SALAM=./salam sh tools/bash/run-tests.sh <section>` for one section).

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

# keep the generated temp files (C source, or LLVM IR) for inspection
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

### 🔎 Checking without building: `salam check`

```sh
salam check app.salam                  # exit 0: no errors, exit 1: errors, exit 2: could not start
salam check app.salam --error-format=json
salam check app.salam --error-lang=fa   # English source, Persian diagnostics
salam check app.salam --backend=native # the rules of that backend, as `salam build` applies them
salam check app.salam --target=x86_64-windows-gnu   # the Windows branches of the program
```

`salam check` runs the part of `salam build` that comes before code generation,
with the same flags, and answers for the build you would get from them. It
exits non-zero whenever it prints an error, in plain and in JSON output alike.

`--error-lang=fa` (or `en`) picks the language of diagnostics and tool messages
independently of the source language: an English file keeps its English
keywords and identifiers and is still compiled as English, while its errors
are printed in Persian. Without the flag, messages follow the source language.

It reports:

- lexer, parser and conditional compilation errors in the entry file and in
  every package it imports, standard library packages too
- type, ownership and range errors in all of those files
- the sibling files of a package entry (`demo/demo.salam` with
  `demo/helpers.salam`) on the C backend; the LLVM and native backends read the
  entry file alone, so `salam check --backend=native` does too
- a package name claimed by two files, on the C backend
- a program with no entry function

These stay out, because they only exist once code is generated or a toolchain
runs, and finding them would cost a full build:

- errors from the LLVM and native code generators (`LLVM backend: ...`) and the
  C compiler failures of the C backend
- link errors: a missing library, an undefined `extern` symbol
- fetching and compiling native libraries (`link` and `[native.x]` in
  `salam.toml`), so the `SALAM_LINK_*` flags they set are not defined either
- options only the build reads: `--export`, `--cpu`, `--subsystem`, `--icon`,
  `--manifest`, `--version-info`
- `SALAM_CC_*` is only defined when you pass `--cc`, since no C compiler is
  chosen otherwise

One difference is deliberate. A library has no entry function, and `salam check`
accepts one: a file whose package is not `main`, a `*_test.salam` file, a page
file, `--lib`, or a project with `kind = "lib"`. `salam build` on such a file
without `--lib` says there is no entry point.

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

`salam serve` also runs several sites from one process when you give it a
config file in Salam's block syntax:

```
server:
    site:
        listen = 80
        names = "example.com www.example.com"
        root = "/srv/example"
        location "/assets/": cache = 30d end
    end
    site:
        names = "blog.example.com"
        root = "/srv/blog"
    end
end
```

```sh
salam serve --config=web.salam --test   # check the file, exit 0 when valid
salam serve --config=web.salam          # run it
```

Sites are chosen by the `Host` header, files are sent with `ETag` and byte
range support, and `net/httpd` (see the std page) lists every setting.

### 🧩 Everything ships with Salam

The whole library is installed with the compiler: the database clients
(`db.sqlite`, `db.postgres`, `db.mysql`, `db.redis`, `db.opensearch`, ...),
machine learning (`ai.tensor`, `ai.nn`, `ai.llm`, ...), audio, the network services
(`grpc`, `ftp`, `ssh`, `net.websocket`, `net.http.swagger`, ...), `gui.webview`,
`ai.opencv` and `excel`. [`std/`](../std/) holds them all and `salam search`
finds them. Import them like any other library; there is nothing to add or
download first:

```salam
import db.redis
```

A package that wraps a C library (`db.sqlite`, `gui.webview`, ...) finds or builds
that library the first time a program that imports it is compiled; see
Native libraries below. Older projects may still list a library package in
`salam.toml`, such as `"db.redis" = "v0.5.9"`. Salam ignores those lines, and
`salam remove db.redis` takes one out.

### 📦 Packages from GitHub

A package outside the standard library, one that somebody else publishes, is
named by its repository and imported with a string:

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
salam update [github.com/owner/repo]      # newest release with the same major (@latest crosses it)
salam deps                                # download what salam.lock lists, verify checksums
salam install github.com/owner/tool       # build a program's main.salam into ~/.salam/bin
salam install                             # the same for the project you are in
salam uninstall [name]                    # remove an installed program, or list them
salam deps clean                          # empty the download cache
salam search websocket                    # standard library and registry/index.toml
```

A local package can stand in for a remote one with
`"example.com/me/lib" = { path = "../lib" }` in `[dependencies]`.
Downloads live in `~/.salam/pkg/<module>@<version>` (`$SALAM_PKG` moves it).
Git is used when it is installed; without it, packages on github.com,
gitlab.com and codeberg.org arrive as a tar.gz that Salam unpacks itself (`SALAM_NO_GIT=1` forces that path).
Dependencies of dependencies come from each package's own `salam.toml`, and
when two ask for different versions the newer one wins (minimal version
selection, as in Go).

The full reference for `salam.toml`, `salam.lock`, the commands and the
environment variables is [docs/PACKAGES.md](PACKAGES.md).

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
install command for this OS. `salam add db.sqlite` prints that library's status up front
(it adds nothing to `salam.toml`, since the package is part of the standard library).
`SALAM_NATIVE=source` or `SALAM_NATIVE=system` forces one way.

### 📚 Building a library

```sh
salam build --lib            # libNAME.so, libNAME.dylib, or NAME.dll + libNAME.dll.a
salam build --lib=static     # libNAME.a
```

Every backend builds libraries. The C backend is the default; `--backend=llvm`
(or a `--target` build with no cross `--cc`) produces the object with LLVM and
links it with `cc -shared` (or archives it for `--lib=static`).
`--backend=native` links a shared library with the built-in linker, with no
C compiler, for Linux (glibc and musl) and macOS targets; see "Shared
libraries" in [docs/NATIVE_LINKER.md](NATIVE_LINKER.md). A Windows DLL
also gets `NAME.def` and, when `llvm-dlltool` is installed, an MSVC import
library `NAME.lib`, next to the mingw `libNAME.dll.a`. With
`--backend=native` and a Windows target the compiler's own linker writes the
DLL and `libNAME.dll.a`, so no mingw `gcc` is needed, only the mingw-w64
runtime files.

The functions in the entry file's `export:` block become the library's C API,
under their own names, and `NAME.h` is written next to the library with their
prototypes. No `main` is needed: the runtime and the package's globals are
set up when the library is loaded. A Windows DLL can be built from Linux with
`--target=x86_64-windows-gnu --cc=x86_64-w64-mingw32-gcc`.

### 🪟 Windows programs: icon, manifest and no console

A program built for Windows with `--backend=native` (or with
`--linker=builtin`) can carry an icon, an application manifest and version
information, and can be a GUI program that opens no console window:

```sh
salam build app.salam --backend=native --target=x86_64-w64-windows-gnu \
    --subsystem=windows --icon=app.ico --manifest=app.manifest \
    --version-info=version=1.2.0 --version-info=company="Example Ltd"
```

`--version-info` takes `version`, `company`, `description`, `product` and
`copyright`. A project can keep the same settings in `salam.toml`; paths are
relative to the project, and `version` defaults to `[package].version`:

```toml
[windows]
subsystem = "windows"
icon = "assets/app.ico"
manifest = "app.manifest"
description = "Salam demo"
company = "Example Ltd"
```

The compiler writes the resources itself, so no `windres` is needed. A GUI
program still starts in `main`; its output only shows up when it is
redirected to a file or a pipe.

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

Run a released compiler in Docker without installing anything else:

```sh
docker run --rm -v "$PWD":/work basemax/salam build app.salam --output=app

# or from a checkout, with the repository mounted at /work
docker compose -f docker/docker-compose.yml run --rm salam build app.salam --output=app
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
| [Platform & word-size support](PLATFORMS.md)                                                                                 | 32-bit vs 64-bit, the three backends, release matrix            |
| [Contributing Guide](../CONTRIBUTING.md)                                                                                     | How to fork, branch, build, and open a pull request             |
| [Code of Conduct](../CODE_OF_CONDUCT.md)                                                                                     | Community standards                                             |
| [Visual Studio Code extension](https://marketplace.visualstudio.com/items?itemName=salamlanguage.salam-programming-language) | Syntax highlighting and language support for Visual Studio Code |
| [Discord](https://discord.gg/HfY3QHDPdv)                                                                                     | Real-time community chat                                        |
| [Telegram](https://t.me/SalamProgrammingLanguage)                                                                            | Community on Telegram                                           |
