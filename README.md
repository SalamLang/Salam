<div align="center">

<p>
    <img width="150" src="https://raw.githubusercontent.com/SalamLang/Salam/main/design/logo/logo-box.svg" alt="The Salam Programming Language - زبان برنامه نویسی سلام">
</p>

# Salam Programming Language

Salam is a general-purpose and systems programming language designed for efficient software development, featuring a built-in [domain-specific language (DSL)](https://en.wikipedia.org/wiki/Domain-specific_language)

[![Discord](https://dcbadge.limes.pink/api/server/https://discord.gg/HfY3QHDPdv)](https://discord.gg/HfY3QHDPdv)
[![GitHub Release](https://img.shields.io/github/v/release/salamlang/salam?style=for-the-badge&color=007ACC)](https://github.com/SalamLang/Salam/releases)
[![GitHub repository size](https://img.shields.io/github/repo-size/salamlang/salam?style=for-the-badge&color=444444)](https://github.com/SalamLang/Salam)

[![Salam Programming Language Playground](https://img.shields.io/badge/Salam-Playground-1A1A1A?style=for-the-badge&labelColor=333333&color=00FFCC)](https://salamlang.github.io/Salam/)

---

</div>

[![Basic Checks Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/test-git-clone.yml/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/test-git-clone.yml)
[![Dependabot Updates Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/dependabot/dependabot-updates/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/dependabot/dependabot-updates)
[![GitHub Pages Build Deployment Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/pages/pages-build-deployment/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/pages/pages-build-deployment)
[![Pull Request Labeler Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/pr-labeler.yml/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/pr-labeler.yml)
[![Prek Standard Hooks Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/prek.yml/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/prek.yml)
[![Prek Manual Hooks Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/prek-manual.yml/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/prek-manual.yml)
[![Prek Audit Hooks Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/prek-audit.yml/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/prek-audit.yml)
[![Docker Build & Test Alpine + LLVM 22 Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/make-run.yml/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/make-run.yml)
[![Build and Release Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/build-release.yml/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/build-release.yml)
[![Super-Linter Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/super-linter.yml/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/super-linter.yml)
[![WebAssembly Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/web-assembly-build-deploy.yml/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/web-assembly-build-deploy.yml)
[![VS Code Marketplace Version](https://vsmarketplacebadges.dev/version/salamlanguage.salam-programming-language.svg)](https://marketplace.visualstudio.com/items?itemName=salamlanguage.salam-programming-language)
[![VS Code Marketplace Installs](https://vsmarketplacebadges.dev/installs/salamlanguage.salam-programming-language.svg)](https://marketplace.visualstudio.com/items?itemName=salamlanguage.salam-programming-language)

---

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [✨ Introducing Salam](#-introducing-salam)
  - [Supported Languages](#supported-languages)
  - [Why Choose Salam?](#why-choose-salam)
  - [Key Features](#key-features)
- [🧩 Editor Support](#-editor-support)
  - [Visual Studio Code](#visual-studio-code)
- [🛠️ The Compiler (`salam`)](#-the-compiler-salam)
  - [Build](#build)
  - [Usage](#usage)
- [🐳 Docker & Docker Compose](#-docker--docker-compose)
  - [Development (live reload)](#development-live-reload)
  - [Production (copy & build)](#production-copy--build)
  - [Plain Docker (without Compose)](#plain-docker-without-compose)
  - [Books (XeLaTeX)](#books-xelatex)
- [🚀 Bun Workspaces: Multi-App Development & Static Site Guide](#-bun-workspaces-multi-app-development--static-site-guide)
  - [🚀 1. Quickstart Execution Guide](#-1-quickstart-execution-guide)
  - [📁 2. Monorepo Architecture & Core Setup](#-2-monorepo-architecture--core-setup)
    - [Root Configuration Files](#root-configuration-files)
      - [`package.json` (Workspace Root)](#packagejson-workspace-root)
      - [`bunfig.toml` (Workspace Root)](#bunfigtoml-workspace-root)
  - [🛠️ 3. Static Site Package Implementation](#-3-static-site-package-implementation)
    - [Static Site Configuration](#static-site-configuration)
  - [⚡ 4. The Serve Methods Evaluated](#-4-the-serve-methods-evaluated)
    - [Method A: Bun Native Dev Engine (`bun run --watch index.html`)](#method-a-bun-native-dev-engine-bun-run---watch-indexhtml)
    - [Method B: The Local Isolation Method (`bunx vite`)](#method-b-the-local-isolation-method-bunx-vite)
  - [🔄 5. Global Monorepo Package Updates](#-5-global-monorepo-package-updates)
    - [Explaining the Flags Behind the Script (`bun update -i -r`)](#explaining-the-flags-behind-the-script-bun-update--i--r)
  - [🔒 6. Security Breakdown & Best Practices](#-6-security-breakdown--best-practices)
    - [The Port Selection Architecture](#the-port-selection-architecture)
    - [Network Address Binding (`0.0.0.0` vs `127.0.0.1`)](#network-address-binding-0000-vs-127001)
    - [Safe Command Formula](#safe-command-formula)
  - [📚 7. MyST Documentation](#-7-myst-documentation)
    - [Configuration Files](#configuration-files)
    - [Running the Documentation Locally](#running-the-documentation-locally)
- [🤝 Contributing](#-contributing)
- [🔍 Joining Code Reviews](#-joining-code-reviews)
  - [How a GitHub PR Review Works](#how-a-github-pr-review-works)
    - [Leaving an inline comment](#leaving-an-inline-comment)
    - [Suggesting a code change](#suggesting-a-code-change)
    - [Submitting the review](#submitting-the-review)
    - [Tips for great reviews](#tips-for-great-reviews)
  - [Growing with the Community](#growing-with-the-community)
  - [💬 Real-Time Community](#-real-time-community)
- [📖 Glossary](#-glossary)

<!-- END doctoc -->

<div align="center">

## ✨ Introducing Salam

</div>

**Salam Language**, inspired by the word _salam_ (سلام), meaning _peace_, is
engineered for exceptional efficiency, reliability, and modern software
development. Designed from the
ground up to balance low-level control with developer productivity, Salam
eliminates the unnecessary syntactic friction often found in systems
engineering. By prioritizing a clean, scannable, and intuitive architecture,
it lowers the cognitive barrier to entry, fostering an accessible and
inclusive environment for building high-performance software.

### Supported Languages

- **English**: Fully supported and ready for use.
- **Persian (فارسی)**: Fully supported and ready for use.
- **Arabic (العربية)**: Currently under development. We need contributions from native Arabic speakers to complete support.

### Why Choose Salam?

- 🌍 **Localized for Persian and Arabic Speakers**: Write and read code in your native language.
- 🧑‍🎓 **Beginner-Friendly**: Salam makes programming approachable for everyone, including students and new developers.
- 💻 **Powerful Tools**: Easy to use, but with robust capabilities for all your coding needs.

### Key Features

- ✏️ Intuitive scripting and code compilation.
- 🚀 Docker support for seamless development.
- 🔧 Linting and code checks for best practices.
- 🔄 Built-in commands for versioning and updates.

## 🧩 Editor Support

### [Visual Studio Code](https://code.visualstudio.com/)

Install the official Salam Language extension from the [Visual Studio Code Marketplace](https://marketplace.visualstudio.com/) for syntax highlighting and language support.

[![Install on VS Code](https://vsmarketplacebadges.dev/version/salamlanguage.salam-programming-language.svg)](https://marketplace.visualstudio.com/items?itemName=salamlanguage.salam-programming-language)

## 🛠️ The Compiler (`salam`)

Salam is statically typed and compiled. The **general** language transpiles to [C](https://en.wikipedia.org/wiki/C_(programming_language)) and is
built to a native executable; embedded **`layout:`** blocks compile to HTML/CSS/JS.

### Build

Requires a C compiler. [**tcc**](https://bellard.org/tcc/) is the default backend (bundled math, fast); [gcc](https://gcc.gnu.org/)/[clang](https://clang.llvm.org/) also work.

The compiler lives in [`compiler/`](compiler/) (a self-contained subproject:
`src/`, `std/`, `tests/`, `tools/`, `Makefile`, `CMakeLists.txt`). Build it from
there:

```sh
cd compiler
sh tools/bash/build-compiler.sh   # quick build with tcc  ->  ./salam
# or, with CMake (out-of-tree build, then run the test suite via ctest):
cmake -B build && cmake --build build && ctest --test-dir build
# or just: make            # (release build via the Makefile -> ../salam at the repo root)
```

There is no separate runtime library: `salam build` emits the small C runtime
(print/strcat/pow/alloc and optional bounds checks) inline into the generated C, so
compiled programs are self-contained and link only `-lm` (`-lmsvcrt` with tcc).

### Usage

```sh
# general language -> native executable
salam build app.salam --output=app.exe         # then ./app.exe
salam cli build app.salam --keep-c             # optional 'cli' prefix; keep generated C
salam obj app.salam                            # compile to .o only

# layout DSL -> website
salam layout build page.salam                  # page.html + page.css + page.js
salam layout build page.salam --inline         # one self-contained page.html
salam layout build a.salam b.salam             # per-page html + merged style.css/script.js

# inspect any stage (general or layout)
salam app.salam --emit-tokens-xml | --emit-ast-xml | --emit-symbol-xml
salam app.salam --log-level=trace
salam build app.salam -DDEBUG                  # preprocessor define

# format source in place (auto-detects nothing - pass --lang=fa for Persian files)
salam fmt app.salam                            # reformat one file
salam fmt                                      # reformat every .salam under the cwd, recursively
salam fmt compiler/src/ examples/              # reformat given files and/or directories
salam fmt --check                              # report files that need formatting (exit 1 if any)
salam fmt app.salam --tabs                     # indent with tabs (convert spaces to tabs)
salam fmt compiler/src/ --indent=2             # indent with 2 spaces per level
salam fmt page.salam --lang=fa                 # Persian source

# REPLs
salam cli       # general
salam layout    # layout
```

Hello, World:

```salam
func main {
    println("Hello, World!")
}
```

## 🐳 [Docker](https://www.docker.com/) & [Docker Compose](https://docs.docker.com/compose/)

The compiler ships a multi-stage [`Dockerfile`](compiler/docker/Dockerfile) and a
[`docker-compose.yml`](compiler/docker/docker-compose.yml) built on [**Alpine Linux**](https://www.alpinelinux.org/)
([musl](https://musl.libc.org/), the lightest practical base). The build context is the compiler root
(`compiler/`). The image includes everything the compiler needs: a C compiler
(`gcc` + `tcc`), the [**LLVM 22**](https://llvm.org/) backend (`clang-22`, `llc-22`, `opt-22`,
`lli-22`), and [`make`](https://www.gnu.org/software/make/), no [CMake](https://cmake.org/) required.

The compose file lives in `compiler/docker/`; pass it with `-f` so you can run
from the repository root. There are two modes:

### Development (live reload)

The compiler tree is bind-mounted and `./salam` is **recompiled automatically on
every change** to `src/` (powered by [`entr`](https://eradman.com/entrproject/), see
[`tools/bash/docker-dev.sh`](compiler/tools/bash/docker-dev.sh)):

```sh
docker compose -f compiler/docker/docker-compose.yml up dev   # build, then watch & rebuild
```

Edit any file under `compiler/src/` on your host and the container rebuilds
`./salam` incrementally. To get a shell inside the same environment:

```sh
docker compose -f compiler/docker/docker-compose.yml run --rm dev sh
```

### Production (copy & build)

The production stage runs `COPY .` then `make && make install`, baking a fully
built, self-contained compiler into the image. `salam` is the entrypoint, and the
repository root is mounted at `/work` so shared `examples/` are reachable:

```sh
docker compose -f compiler/docker/docker-compose.yml build prod
docker compose -f compiler/docker/docker-compose.yml run --rm prod build examples/en/basics/hello.salam --output=hello
docker compose -f compiler/docker/docker-compose.yml run --rm prod layout build page.salam --inline
docker compose -f compiler/docker/docker-compose.yml run --rm prod --help
```

### Plain Docker (without Compose)

Build with `compiler/` as the context (so the Dockerfile's `COPY .` picks up the
compiler tree):

```sh
docker build --target prod -f compiler/docker/Dockerfile -t salam:prod compiler   # production image
docker run --rm -v "$PWD":/work salam:prod build app.salam --output=app

docker build --target dev  -f compiler/docker/Dockerfile -t salam:dev  compiler   # dev image
docker run --rm -it -v "$PWD/compiler":/app salam:dev                             # live-rebuild loop
```

> The LLVM and Alpine versions are configurable build args
> (`--build-arg LLVM_VERSION=22`, `--build-arg ALPINE_VERSION=edge`). LLVM 22
> currently lives in Alpine's `edge` repositories, which is why `edge` is the
> default base.

### Books (XeLaTeX)

The tutorial books have their own image under
[`books/docker/`](books/docker/) so you can typeset them without installing a
local TeX distribution. One **common** [XeLaTeX](https://tug.org/xetex/) image
builds **both** languages: it is based on the full
[TeX Live](https://tug.org/texlive/) scheme, which already bundles
[`xepersian`](https://ctan.org/pkg/xepersian) (for the heavier right-to-left
Persian book), `polyglossia` + `tcolorbox` (for the lighter English book) and
Latin Modern. The `books/` tree is bind-mounted, so the generated PDFs are
written back to the host at `books/<lang>/intro-programming/book.pdf`:

```sh
docker compose -f books/docker/docker-compose.yml run --rm books        # both
docker compose -f books/docker/docker-compose.yml run --rm books en     # English only
docker compose -f books/docker/docker-compose.yml run --rm books fa     # Persian only
```

Or with plain Docker (build once, then run against the mounted `books/` tree):

```sh
docker build -t salam-books books/docker
docker run --rm -v "$PWD/books":/books salam-books all
```

## 🚀 [Bun](https://bun.sh/) Workspaces: Multi-App Development & Static Site Guide

### 🚀 1. Quickstart Execution Guide

To kick off your entire localized monorepo environment simultaneously, run the following steps from your root directory.

- Install workspace links and cached instances:

```bash
bun install
```

- Boot up all servers concurrently on high ports:

```bash
bun run dev:all
```

Your terminal window will display interleaved logs, cleanly prefixed by their respective package origins:

```text
Salam % bun run dev:all
$ bun run --filter='*' --parallel dev
@workspace/pages:dev         |
@workspace/pages:dev         |   VITE v8.1.3  ready in 61 ms
@workspace/pages:dev         |
@workspace/pages:dev         |   ➜  Local:   http://127.0.0.1:55002/
@workspace/editor:dev        |
@workspace/editor:dev        |   VITE v8.1.3  ready in 65 ms
@workspace/editor:dev        |
@workspace/editor:dev        |   ➜  Local:   http://127.0.0.1:55001/
@workspace/vercel-editor:dev |
@workspace/vercel-editor:dev |   VITE v8.1.3  ready in 91 ms
@workspace/vercel-editor:dev |
@workspace/vercel-editor:dev |   ➜  Local:   http://localhost:5173/
@workspace/vercel-editor:dev |   ➜  Network: use --host to expose
@workspace/runner:dev        |
@workspace/runner:dev        |  ⛅️ wrangler 4.107.0
@workspace/runner:dev        | ────────────────────
@workspace/runner:dev        | ⎔ Starting local server...
@workspace/runner:dev        | [wrangler:info] Ready on http://localhost:8787
@workspace/runner:dev        | [wrangler:info] GET / 200 OK (7ms)
```

---

### 📁 2. Monorepo Architecture & Core Setup

To configure the workspace, structure your repository directory tree alphabetically as follows:

```text
salam-monorepo/
├── bunfig.toml
├── package.json
├── editor/
├── extensions/
│   └── vscode/
├── pages/
├── runner/
└── vercel-editor/
```

#### Root Configuration Files

##### `package.json` (Workspace Root)

This file defines the workspaces in strict alphabetical order and
leverages Bun's parallel filtering mechanics. It also centralizes shared
dependency versions via Bun's workspace `catalog` and exposes root
scripts for local development, builds, Wrangler type generation, and
dependency updates.

```json
{
  "name": "salam-monorepo",
  "private": true,
  "workspaces": {
    "packages": [
      "editor",
      "extensions/vscode",
      "pages",
      "runner",
      "vercel-editor"
    ],
    "catalog": {
      "react": "^19.2.7",
      "react-dom": "^19.2.7",
      "tailwindcss": "^4.3.2",
      "@tailwindcss/vite": "^4.3.2",
      "@vitejs/plugin-react": "^6.0.3",
      "vite": "^8.1.3",
      "typescript": "^6.0.3",
      "wrangler": "^4.107.0",
      "@types/bun": "latest",
      "@types/node": "^26.1.0",
      "@types/react": "^19.2.17",
      "@types/react-dom": "^19.2.3"
    }
  },
  "scripts": {
    "dev:all": "bun run --filter='*' --parallel --if-present dev",
    "dev:editor": "bun run --filter='@workspace/editor' dev",
    "docs:myst": "bun run myst start",
    "dev:pages": "bun run --filter='@workspace/pages' dev",
    "dev:runner": "bun run --filter='@workspace/runner' dev",
    "dev:vercel": "bun run --filter='@workspace/vercel-editor' dev",
    "build:all": "bun run --filter='*' --if-present build",
    "build:myst": "bun run myst build --html",
    "build:vercel": "bun run --filter='@workspace/vercel-editor' build",
    "update:deps": "bun update -i -r",
    "clean": "rm -rf node_modules **/node_modules .bun-cache",
    "generate": "bun run --filter='@workspace/runner' generate",
    "typecheck": "bun run --filter='@workspace/runner' typecheck"
  },
  "devDependencies": {
    "@tailwindcss/vite": "catalog:",
    "@vitejs/plugin-react": "catalog:",
    "mystmd": "^1.10.1",
    "vite": "catalog:"
  }
}
```

Useful root commands:

- `bun run dev:all`: start every workspace that exposes a `dev` script
- `bun run dev:editor`, `bun run dev:pages`, `bun run dev:runner`, and
  `bun run dev:vercel`: start one workspace at a time
- `bun run build:all`: run every workspace `build` script that exists
- `bun run build:vercel`: build the React/Vite app in `vercel-editor/`
- `bun run docs:myst`: start the [MyST](https://mystmd.org/) documentation dev server
- `bun run build:myst`: build the MyST documentation to static HTML (`_build/html/`)
- `bun run generate`: refresh Cloudflare Wrangler types for `runner/`
- `bun run typecheck`: regenerate Wrangler types and run the `runner/`
  TypeScript check
- `bun run update:deps`: interactively update dependencies across the workspace

##### `bunfig.toml` (Workspace Root)

Forces Bun to persistently treat execution environments with development-first behaviors.

```toml
[development]
development = true
```

---

### 🛠️ 3. Static Site Package Implementation

The independent workspace utilizes your specific package scope (`@workspace/pages`). It is set to `"private": true` to protect against accidental package publishing to the public [npm](https://www.npmjs.com/) registry.

#### Static Site Configuration

`pages/package.json`

```json
{
  "name": "@workspace/pages",
  "version": "1.0.0",
  "private": true,
  "scripts": {
    "dev": "bunx vite --port 55002 --host 127.0.0.1"
  }
}
```

---

### ⚡ 4. The Serve Methods Evaluated

Depending on your asset compilation pipelines, you can run and serve your static file assets via two distinct command-line approaches.

#### Method A: Bun Native Dev Engine (`bun run --watch index.html`)

Bun acts as a full-stack compiler. It looks for runtime entry points, hooks a watcher loop, and injects code straight into the runtime.

```bash
PORT=55002 bun run --watch index.html
```

- 🟥 **The Critical Crash Bug**: If an HTML file links a `<script src="./app.js">` tag and `app.js` is missing, deleted, or generated out-of-order by another builder script, Bun's dependency engine throws a fatal compilation or segmentation panic and crashes your entire root dev terminal.

#### Method B: The Local Isolation Method (`bunx vite`)

Bypasses complex multi-environment integrations entirely by mounting a dedicated Vite development context over the directory.

```bash
bunx vite --port 55002 --host 127.0.0.1
```

- 🟩 **The Fix**: Missing scripts or components safely emit standard frontend browser `404 Not Found` messages instead of breaking your backend engine processes.

---

### 🔄 5. Global Monorepo Package Updates

To manage and upgrade your external dependencies across all application subdirectories together without navigating into individual package folders, trigger your custom root shortcut:

```bash
bun run update:deps
```

#### Explaining the Flags Behind the Script (`bun update -i -r`)

- **`-i` (`--interactive`)**: Spawns an interactive Terminal User Interface (TUI) mapping out all out-of-date assets. Use your Arrow keys and spacebar to selectively pick packages to upgrade, or toggle `l` to force a package beyond its defined [SemVer](https://semver.org/) range to its absolute `--latest` release.
- **`-r` (`--recursive`)**: Forces Bun's resolution engine to sweep across all items listed under your `workspaces` field (`editor`, `pages`, `runner`, etc.) instead of executing purely inside the main directory.

When initialized, Bun appends a **Workspace** column to your terminal output grid so you can verify exactly where every target module upgrade is bound before writing the changes to disk.

---

### 🔒 6. Security Breakdown & Best Practices

When operating servers on your host machine, observe these boundaries:

#### The Port Selection Architecture

Network ports scale from `1` to `65535`.

- **`1` – `1023`**: System-privileged root ports. Avoid using these.
- **`1024` – `49151`**: Registered user application ports (e.g., `8080`, `3000`). High risk of overlapping with other applications.
- **`49152` – `65535`**: **Dynamic, Private, and High Ports**. Ideal for setting up workspace dev contexts cleanly.

#### Network Address Binding (`0.0.0.0` vs `127.0.0.1`)

- **`0.0.0.0` (All Interfaces)**: Default configuration for most servers. If you are on an open public Wi-Fi network (like a cafe), anyone on that network can read your unreleased static code by querying your local IP address.
- **`127.0.0.1` (Loopback Localhost)**: Hardware-restricted isolation. Only your exact physical computer can access the ports. Highly recommended for untrusted networks.

#### Safe Command Formula

To enforce instant browser updates, skip disk caches, isolate network eavesdroppers, and bypass compiler engine crashes, use this script formula:

```bash
bunx vite --port 55002 --host 127.0.0.1
```

---

### 📚 7. MyST Documentation

The repository ships a [MyST (Markedly Structured Text)](https://mystmd.org/) docs site powered by [`mystmd`](https://mystmd.org/guide). Source content comes from `books/` (TeX book files) and `.md` files across the repository.

#### Configuration Files

- **[`myst.yml`](myst.yml)** — MyST project config (site template, repository link, and custom style file).
- **[`.readthedocs.yaml`](.readthedocs.yaml)** — [Read the Docs](https://readthedocs.org/) build config used to host the generated documentation.
- **`_build/`** — generated static output directory from `myst build --html` (including `_build/html/`), ignored by git in [`.gitignore`](.gitignore).

#### Running the Documentation Locally

```bash
# Install workspace dependencies (includes mystmd)
bun install

# Start the live-reload docs server
bun run docs:myst

# Build the static HTML output to _build/html/
bun run build:myst
```

The dev server (`myst start`) watches `books/` and `.md` files across the repository and rebuilds on changes. Read the Docs uses `.readthedocs.yaml` to autobuild and deploy the site.

## 🤝 Contributing

We welcome contributions from the community!

- 📖 Read our [Contributing Guide](CONTRIBUTING.md).
- 🌟 Follow our [Code of Conduct](CODE_OF_CONDUCT.md).

Together, let’s make coding accessible to all.

---

## 🔍 Joining Code Reviews

**Everyone is welcome — and encouraged — to participate in code reviews.** You do not need to be a core maintainer to review a pull request. In fact, reviewing PRs is one of the most impactful ways to contribute to Salam, and we want to make it as approachable as possible.

> 💡 **Code review is harder than writing code.** Reading someone else's work, understanding their intent, spotting edge cases, and communicating feedback kindly and clearly takes real skill. If you are doing it, thank you — it matters enormously.

### How a GitHub PR Review Works

Open any pull request, then click the **Files changed** tab to see a diff of every file that was modified.

#### Leaving an inline comment

1. Hover over any line number in the diff — a blue **`+`** button appears.
2. Click it to open a comment box for that specific line or block.
3. Write your thought, then click **Start a review** (not "Add single comment") so all your notes are batched together.

#### Suggesting a code change

Inside an inline comment you can propose an exact replacement using a fenced suggestion block:

````markdown
```suggestion
your replacement code here
```
````

The author can accept your suggestion with a single click — no manual editing required.

#### Submitting the review

When you have finished reading all the files:

1. Click the green **Review changes** button (top right of the diff).
2. Write an overall summary comment.
3. Choose one of three outcomes:
   - 💬 **Comment** — share thoughts without formally approving or blocking. Great for questions, first-timer feedback, or discussion starters.
   - ✅ **Approve** — you are satisfied with the changes. This is the **green tick** that moves a PR toward merging. Only approve if you have read the changes carefully.
   - 🚫 **Request changes** — something needs to be fixed before the PR can merge. Be specific and constructive.
4. Click **Submit review**.

#### Tips for great reviews

- **A simple comment is perfectly fine.** If you are new to reviewing, just leaving a thoughtful question or noting something you learned is genuinely valuable. You don't have to approve or block.
- **Be kind and specific.** Point to the exact line, explain _why_ something may be a concern, and suggest an alternative when you can.
- **Assume good intent.** The author put effort into this. Phrase feedback as questions ("Could this be…?") rather than commands ("Change this to…").
- **First-timer PRs deserve extra warmth.** If a contributor's bio or commit history shows they are new, a short encouraging note goes a long way.
- **Approve when you mean it.** The green checkmark signals trust. Reserve it for changes you have genuinely read and feel confident about.

### Growing with the Community

Consistent reviewers and contributors are noticed. Active participation in reviews, issues, and discussions is how community members become trusted collaborators. Over time, standout contributors may be invited to join the **Salam core team** and take on additional responsibilities:

- 🏷️ **Triage role** — help label, categorise, and prioritise incoming issues and pull requests.
- 🛡️ **Discord / Telegram moderator** — help keep real-time channels welcoming and on-topic.
- 🔑 **Maintainer access** — merge PRs, manage releases, and help shape the roadmap.

### 💬 Real-Time Community

The Salam core team lives in our real-time channels. Join us to ask questions, share ideas, discuss reviews in progress, or just say hello:

| Platform | Link |
|---|---|
| **Discord** | [discord.gg/HfY3QHDPdv](https://discord.gg/HfY3QHDPdv) |
| **Telegram** | [t.me/SalamProgrammingLanguage](https://t.me/SalamProgrammingLanguage) |

These spaces are open to everyone — from curious newcomers to experienced systems programmers. The more voices the better.

---

## 📖 Glossary

Terms used across this readme, the [Contributing Guide](CONTRIBUTING.md), and the [editor readme](editor/README.md).

| Term | Definition |
|---|---|
| **[Alpine Linux](https://alpinelinux.org/)** | Lightweight Linux distribution built on musl libc. Used as the base image for Salam's Docker containers. |
| **[AST (Abstract Syntax Tree)](https://en.wikipedia.org/wiki/Abstract_syntax_tree)** | Tree representation of parsed source code. Each node represents a construct (expression, statement, declaration). The Salam compiler builds an AST before semantic analysis and code generation. |
| **[Biome](https://biomejs.dev/)** | Fast JavaScript/TypeScript linter and formatter. Run as a prek hook (`biome-check`). |
| **[Bun](https://bun.sh/)** | JavaScript runtime, package manager, and bundler. Used in the Salam monorepo to manage workspaces and run dev servers. |
| **[C ABI](https://en.wikipedia.org/wiki/Application_binary_interface)** | C Application Binary Interface — the low-level contract for how functions are called and data is laid out in memory. Salam's FFI and `extern "C"` declarations rely on the C ABI. |
| **[CI (Continuous Integration)](https://en.wikipedia.org/wiki/Continuous_integration)** | Automated pipeline that builds, tests, and lints every pull request. Salam uses GitHub Actions for CI. |
| **[Clang](https://clang.llvm.org/)** | LLVM-based C compiler. One of the supported backends for building the Salam compiler. |
| **[Cloudflare Workers](https://workers.cloudflare.com/)** | Serverless compute platform that runs JavaScript/TypeScript at the edge. The `runner/` workspace deploys to Cloudflare Workers via Wrangler. |
| **[CMake](https://cmake.org/)** | Cross-platform build tool generator. The Salam compiler can be built with `cmake -B build && cmake --build build`. |
| **[Codegen](https://en.wikipedia.org/wiki/Code_generation_(compiler))** | Code-generation backend. Transforms the compiler's AST and type information into target output (C source, LLVM IR, or a WebAssembly module). |
| **[Codespell](https://github.com/codespell-project/codespell)** | Spell checker for source code and documentation. Run as a prek hook to catch typos. |
| **[CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html)** | CMake's built-in test runner. Invoked with `ctest --test-dir build` after a CMake build of the compiler. |
| **defer** | Salam keyword that schedules a statement or block to run at the end of the enclosing scope, regardless of how the scope is exited. |
| **[Docker](https://www.docker.com/)** | Container platform used to build and run the Salam compiler in an isolated, reproducible environment. |
| **[Docker Compose](https://docs.docker.com/compose/)** | Tool for defining and running multi-container Docker applications. Salam provides a `docker-compose.yml` in `compiler/docker/` with `dev` and `prod` service targets. |
| **[doctoc](https://github.com/thlorenz/doctoc)** | Tool that auto-generates a Table of Contents inside Markdown files. The TOC blocks in this readme and the Contributing Guide are maintained by doctoc. |
| **[DSL (Domain-Specific Language)](https://en.wikipedia.org/wiki/Domain-specific_language)** | A language tailored to a specific problem domain. Salam includes a built-in layout DSL that compiles `.salam` files to HTML, CSS, and JavaScript. |
| **[Emscripten](https://emscripten.org/)** | LLVM-based toolchain that compiles C/C++ to WebAssembly. Used to build `salam-wa.wasm` and `salam-wa.js` for the web playground. |
| **[entr](https://eradman.com/entrproject/)** | Utility that re-runs a command whenever watched files change. Used in Salam's Docker dev mode to rebuild the compiler on source edits. |
| **[FFI (Foreign Function Interface)](https://en.wikipedia.org/wiki/Foreign_function_interface)** | Mechanism for calling functions written in another language (typically C). Salam supports FFI via `extern "C"` declarations. |
| **[Fork](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/working-with-forks/fork-a-repo)** | A personal copy of a repository on GitHub. Contributors fork `SalamLang/Salam`, work in their fork, then open a pull request back to the original. |
| **[GCC](https://gcc.gnu.org/)** | GNU Compiler Collection. A C compiler supported by the Salam build tool. |
| **[Gitleaks](https://github.com/gitleaks/gitleaks)** | Secret-detection tool run as a prek audit hook to prevent accidental commits of API keys or credentials. |
| **[GitHub Actions](https://docs.github.com/en/actions)** | Salam's CI/CD platform. Workflows are defined in `.github/workflows/`. |
| **Layout DSL** | Salam's built-in sublanguage for describing UIs. `layout:` blocks, or `.salam` files processed with `salam layout build`, compile to HTML, CSS, and JavaScript. |
| **[Lexer](https://en.wikipedia.org/wiki/Lexical_analysis)** | First stage of the compiler pipeline: reads source text and produces a stream of tokens. |
| **[LLVM](https://llvm.org/)** | Low Level Virtual Machine — a compiler infrastructure project. Used as an optional backend (`clang-22`, `llc-22`, `opt-22`, `lli-22` are included in the Docker image). |
| **[Makefile](https://www.gnu.org/software/make/)** | GNU Make build script (`compiler/Makefile`). Running `make` from `compiler/` produces a release build of the `salam` binary at the repository root. |
| **[Markdownlint](https://github.com/igorshubovych/markdownlint-cli)** | Linter for Markdown files. Enforces consistent style in `README.md`, `CONTRIBUTING.md`, and other docs. |
| **[musl](https://musl.libc.org/)** | A C standard library used by Alpine Linux. Salam's Docker images are based on Alpine + musl for a minimal footprint. |
| **[MyST (Markedly Structured Text)](https://mystmd.org/)** | Documentation toolchain that extends Markdown with roles, directives, and cross-references for technical writing. Configured via `myst.yml`; driven by the `mystmd` package (scripts `docs:myst` and `build:myst`). |
| **[npm](https://www.npmjs.com/)** | Node Package Manager. Used alongside Bun for package management in the Salam monorepo. |
| **[Parser](https://en.wikipedia.org/wiki/Parsing)** | Second stage of the compiler pipeline: consumes the token stream from the lexer and builds an AST. |
| **[PR (Pull Request)](https://docs.github.com/en/pull-requests)** | A GitHub mechanism for proposing changes: a contributor opens a PR from a feature branch, reviewers comment, and a maintainer merges it. |
| **[prek](https://prek.j178.dev/)** | Git hook manager used by Salam. Hooks are defined in `prek.toml` (standard and manual stages) and `prek-audit.toml` (security-focused audit checks). |
| **prek-audit.toml** | prek configuration file for security-focused audit hooks. Run separately with `prek run --all-files --config prek-audit.toml`. |
| **[Prettier](https://prettier.io/)** | Opinionated code formatter for JavaScript, TypeScript, CSS, and JSON. Run as a prek hook. |
| **[React](https://react.dev/)** | JavaScript library for building user interfaces with a component model. Used in the `vercel-editor/` workspace. |
| **[Read the Docs](https://readthedocs.org/)** | Free documentation hosting platform. Salam's MyST docs are automatically built and published there via `.readthedocs.yaml`. |
| **[REPL](https://en.wikipedia.org/wiki/Read%E2%80%93eval%E2%80%93print_loop)** | Read-Eval-Print Loop — an interactive session where you type expressions and see results immediately. `salam cli` starts a general REPL; `salam layout` starts a layout REPL. |
| **[RTL (Right-to-Left)](https://en.wikipedia.org/wiki/Right-to-left_script)** | Text direction used by Arabic and Persian scripts. The Salam web playground supports RTL and switches direction when the Persian language is selected. |
| **[Semantic Analyzer](https://en.wikipedia.org/wiki/Semantic_analysis_(compilers))** | Third stage of the compiler pipeline: resolves names, checks types, and validates the AST before code generation. |
| **[SemVer (Semantic Versioning)](https://semver.org/)** | Version numbering scheme (`MAJOR.MINOR.PATCH`). Used by Bun's interactive update tooling and GitHub releases. |
| **[shfmt](https://github.com/mvdan/sh)** | Shell script formatter. Run as a prek manual-stage hook to normalise indentation and style in `.sh` files. |
| **[Super-Linter](https://github.com/super-linter/super-linter)** | GitHub Actions workflow that runs a broad set of language-specific linters across the repository in CI. |
| **[Tailwind CSS](https://tailwindcss.com/)** | Utility-first CSS framework. Used in the Salam monorepo workspaces; integrated via the `@tailwindcss/vite` plugin. |
| **[TCC (Tiny C Compiler)](https://bellard.org/tcc/)** | Lightweight, fast C compiler. The default backend used by Salam's quick-build script. |
| **[Tree-walking interpreter](https://en.wikipedia.org/wiki/Interpreter_(computing))** | An interpreter that evaluates the AST directly without first compiling to native code. Used by the web playground and via `salam exec` / `salam run --interp` for pure-compute programs. |
| **[TUI (Terminal User Interface)](https://en.wikipedia.org/wiki/Text-based_user_interface)** | Interactive, keyboard-driven interface rendered in the terminal. Bun's `bun update -i` flag opens a TUI for selecting which packages to upgrade. |
| **[TypeScript](https://www.typescriptlang.org/)** | Typed superset of JavaScript that compiles to plain JavaScript. Used in the `runner/` and `vercel-editor/` workspaces. |
| **[Upstream](https://en.wikipedia.org/wiki/Upstream_(software_development))** | The original `SalamLang/Salam` repository. Contributors add it as a Git remote (`git remote add upstream …`) to keep their fork in sync. |
| **[Virtual filesystem](https://en.wikipedia.org/wiki/Virtual_file_system)** | Emscripten's in-browser filesystem layer. `build-wasm.sh` preloads the `std/` directory into it so import resolution and the layout schema work when running Salam in the browser. |
| **[Vite](https://vite.dev/)** | Frontend build tool and dev server. Used in the Salam monorepo to serve the editor and pages workspaces. |
| **[WebAssembly (Wasm)](https://webassembly.org/)** | Portable binary instruction format that runs at near-native speed in modern browsers. Salam's web playground is powered by a Wasm build of the compiler produced by Emscripten. |
| **[Wrangler](https://developers.cloudflare.com/workers/wrangler/)** | Cloudflare's CLI for building and deploying Workers. The `runner/` workspace uses Wrangler for local development (`wrangler dev`) and CI type generation. |
| **[yamllint](https://github.com/adrienverge/yamllint)** | YAML linter. Run as a prek hook to validate `.yml` workflow and configuration files. |

---

© 2024-2026 Salam Language Team
