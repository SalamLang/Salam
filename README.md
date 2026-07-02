<div align="center">

<p>
    <img width="150" src="https://raw.githubusercontent.com/SalamLang/Salam/main/design/logo/logo-box.svg" alt="The Salam Programming Language - زبان برنامه نویسی سلام">
</p>

# Salam Programming Language

Salam is a general-purpose and systems programming language designed for efficient software development, featuring a built-in domain-specific language (DSL)

[![Discord](https://dcbadge.limes.pink/api/server/https://discord.gg/HfY3QHDPdv)](https://discord.gg/HfY3QHDPdv)
[![GitHub Release](https://img.shields.io/github/v/release/salamlang/salam?style=for-the-badge&color=007ACC)](https://github.com/SalamLang/Salam/releases)
[![GitHub repo size](https://img.shields.io/github/repo-size/salamlang/salam?style=for-the-badge&color=444444)](https://github.com/SalamLang/Salam)

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
[![WebAssembly Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/web-assembly.yml/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/web-assembly.yml)
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
    - [Method B: The Local Isolation Method (`bunx http-server`)](#method-b-the-local-isolation-method-bunx-http-server)
  - [🔄 5. Global Monorepo Package Updates](#-5-global-monorepo-package-updates)
    - [Explaining the Flags Behind the Script (`bun update -i -r`)](#explaining-the-flags-behind-the-script-bun-update--i--r)
  - [🔒 6. Security Breakdown & Best Practices](#-6-security-breakdown--best-practices)
    - [The Port Selection Architecture](#the-port-selection-architecture)
    - [Network Address Binding (`0.0.0.0` vs `127.0.0.1`)](#network-address-binding-0000-vs-127001)
    - [Safe Command Formula](#safe-command-formula)
- [🤝 Contributing](#-contributing)
- [🔍 Joining Code Reviews](#-joining-code-reviews)
  - [How a GitHub PR Review Works](#how-a-github-pr-review-works)
    - [Leaving an inline comment](#leaving-an-inline-comment)
    - [Suggesting a code change](#suggesting-a-code-change)
    - [Submitting the review](#submitting-the-review)
    - [Tips for great reviews](#tips-for-great-reviews)
  - [Growing with the Community](#growing-with-the-community)
  - [💬 Real-Time Community](#-real-time-community)

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

### Visual Studio Code

Install the official Salam Language extension from the Visual Studio Code Marketplace for syntax highlighting and language support.

[![Install on VS Code](https://vsmarketplacebadges.dev/version/salamlanguage.salam-programming-language.svg)](https://marketplace.visualstudio.com/items?itemName=salamlanguage.salam-programming-language)

## 🛠️ The Compiler (`salam`)

Salam is statically typed and compiled. The **general** language transpiles to C and is
built to a native executable; embedded **`layout:`** blocks compile to HTML/CSS/JS.

### Build

Requires a C compiler. **tcc** is the default backend (bundled math, fast); gcc/clang also work.

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

## 🐳 Docker & Docker Compose

The compiler ships a multi-stage [`Dockerfile`](compiler/docker/Dockerfile) and a
[`docker-compose.yml`](compiler/docker/docker-compose.yml) built on **Alpine Linux**
(musl, the lightest practical base). The build context is the compiler root
(`compiler/`). The image includes everything the compiler needs: a C compiler
(`gcc` + `tcc`), the **LLVM 22** backend (`clang-22`, `llc-22`, `opt-22`,
`lli-22`), and `make`, no CMake required.

The compose file lives in `compiler/docker/`; pass it with `-f` so you can run
from the repository root. There are two modes:

### Development (live reload)

The compiler tree is bind-mounted and `./salam` is **recompiled automatically on
every change** to `src/` (powered by `entr`, see
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

## 🚀 Bun Workspaces: Multi-App Development & Static Site Guide

### 🚀 1. Quickstart Execution Guide

To kick off your entire localized monorepo environment simultaneously, run the following steps from your root directory:

```bash
# 1. Install workspace links and cached instances
bun install

# 2. Boot up all servers concurrently on high ports
bun run dev:all
```

Your terminal window will display interleaved logs, cleanly prefixed by their respective package origins:

```text
Salam % bun run dev:all  
$ bun run --filter='*' --parallel dev
@workspace/editor:dev | Starting up http-server, serving .
@workspace/pages:dev  | Starting up http-server, serving .
@workspace/editor:dev |
@workspace/editor:dev | http-server version: 14.1.1
@workspace/pages:dev  |
@workspace/pages:dev  | http-server version: 14.1.1
@workspace/pages:dev  |
@workspace/pages:dev  | http-server settings:
@workspace/pages:dev  | CORS: disabled
@workspace/pages:dev  | Cache: -1 seconds
@workspace/pages:dev  | Connection Timeout: 120 seconds
@workspace/pages:dev  | Directory Listings: visible
@workspace/pages:dev  | AutoIndex: visible
@workspace/pages:dev  | Serve GZIP Files: false
@workspace/pages:dev  | Serve Brotli Files: false
@workspace/pages:dev  | Default File Extension: none
@workspace/pages:dev  |
@workspace/pages:dev  | Available on:
@workspace/pages:dev  |   http://127.0.0.1:55002
@workspace/pages:dev  | Hit CTRL-C to stop the server
@workspace/editor:dev |
@workspace/editor:dev | http-server settings:
@workspace/editor:dev | CORS: disabled
@workspace/editor:dev | Cache: -1 seconds
@workspace/editor:dev | Connection Timeout: 120 seconds
@workspace/editor:dev | Directory Listings: visible
@workspace/editor:dev | AutoIndex: visible
@workspace/editor:dev | Serve GZIP Files: false
@workspace/editor:dev | Serve Brotli Files: false
@workspace/editor:dev | Default File Extension: none
@workspace/editor:dev |
@workspace/editor:dev | Available on:
@workspace/editor:dev |   http://127.0.0.1:55001
@workspace/editor:dev | Hit CTRL-C to stop the server
@workspace/editor:dev |
@workspace/pages:dev  |
vercel-editor:dev     |
vercel-editor:dev     |   VITE v8.1.3  ready in 175 ms
vercel-editor:dev     |
vercel-editor:dev     |   ➜  Local:   http://localhost:5173/
vercel-editor:dev     |   ➜  Network: use --host to expose
runner:dev            |
runner:dev            |  ⛅️ wrangler 4.107.0
runner:dev            | ────────────────────
runner:dev            | ⎔ Starting local server...
runner:dev            | [wrangler:info] Ready on http://localhost:8787
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

This file defines the workspaces in strict alphabetical order and leverages Bun's parallel filtering mechanics. It contains your global batch scripts, your automated interactive update target, and custom individual granular app triggers.

```json
{
  "name": "salam-monorepo",
  "private": true,
  "workspaces": [
    "editor",
    "extensions/vscode",
    "pages",
    "runner",
    "vercel-editor"
  ],
  "scripts": {
    "dev:all": "bun run --filter='*' --parallel dev",
    "dev:editor": "bun run --filter='editor' dev",
    "dev:pages": "bun run --filter='@workspace/pages' dev",
    "dev:runner": "bun run --filter='runner' dev",
    "dev:vercel": "bun run --filter='vercel-editor' dev",
    "build:all": "bun run --filter='*' build",
    "update:deps": "bun update -i -r",
    "clean": "rm -rf node_modules **/node_modules .bun-cache"
  }
}
```

##### `bunfig.toml` (Workspace Root)

Forces Bun to persistently treat execution environments with development-first behaviors.

```toml
[development]
development = true
```

---

### 🛠️ 3. Static Site Package Implementation

The independent workspace utilizes your specific package scope (`@workspace/pages`). It is set to `"private": true` to protect against accidental package publishing to the public npm registry.

#### Static Site Configuration

`pages/package.json`

```json
{
  "name": "@workspace/pages",
  "version": "1.0.0",
  "private": true,
  "scripts": {
    "dev": "bunx http-server . -p 55002 -c-1 -a 127.0.0.1"
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

#### Method B: The Local Isolation Method (`bunx http-server`)

Bypasses Bun's strict internal compilation logic entirely. It mounts a Node-compatible, file-agnostic server context over the directory.

```bash
bunx http-server . -p 55002 -c-1 -a 127.0.0.1
```

- 🟩 **The Fix**: Missing scripts or components safely emit standard frontend browser `404 Not Found` messages instead of breaking your backend engine processes.

---

### 🔄 5. Global Monorepo Package Updates

To manage and upgrade your external dependencies across all application subdirectories together without navigating into individual package folders, trigger your custom root shortcut:

```bash
bun run update:deps
```

#### Explaining the Flags Behind the Script (`bun update -i -r`)

- **`-i` (`--interactive`)**: Spawns an interactive Terminal User Interface (TUI) mapping out all out-of-date assets. Use your Arrow keys and spacebar to selectively pick packages to upgrade, or toggle `l` to force a package beyond its defined SemVer range to its absolute `--latest` release.
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
bunx http-server . -p 55002 -c-1 -a 127.0.0.1
```

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

© 2024-2026 Salam Language Team
