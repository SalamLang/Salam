<div align="center">

<p>
    <a href="https://salamlang.ir/">
        <img width="150" src="https://raw.githubusercontent.com/SalamLang/Salam/main/design/logo/logo-box.svg" alt="The Salam Programming Language - زبان برنامه نویسی سلام">
    </a>
</p>

# Salam Programming Language

Salam is a general-purpose and systems programming language designed for efficient software development, featuring a built-in domain-specific language (DSL)

</div>

[![Basic Checks Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/test-git-clone.yml/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/test-git-clone.yml)
[![Dependabot Updates Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/dependabot/dependabot-updates/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/dependabot/dependabot-updates)
[![GitHub Pages Build Deployment Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/pages/pages-build-deployment/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/pages/pages-build-deployment)
[![Pull Request Labeler Workflow Status](https://github.com/SalamLang/Salam/actions/workflows/pr-labeler.yml/badge.svg)](https://github.com/SalamLang/Salam/actions/workflows/pr-labeler.yml)

---

## ✨ Introducing Salam Language

**Salam Language**, inspired by the word _salam_ (سلام), meaning _peace_, is the first **Persian/Arabic-based programming language**.
It is designed for simplicity, accessibility, and inclusivity-bridging the gap between developers and technology in Persian, Arabic, and related linguistic communities.

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
# or just: make            # (release build via the Makefile)
```

There is no separate runtime library: `salam build` emits the small C runtime
(print/strcat/pow/alloc and optional bounds checks) inline into the generated C, so
compiled programs are self-contained and link only `-lm` (`-lmsvcrt` with tcc).

### Usage

```sh
# general language -> native executable
salam build app.salam --output=app.exe        # then ./app.exe
salam cli build app.salam --keep-c            # optional 'cli' prefix; keep generated C
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

## 🤝 Contributing

We welcome contributions from the community!

- 📖 Read our [Contributing Guide](CONTRIBUTING.md).
- 🌟 Follow our [Code of Conduct](CODE_OF_CONDUCT.md).

Together, let’s make coding accessible to all.
For inquiries or feedback, visit: [salamlang.ir](https://salamlang.ir)

© 2024-2026 Salam Language Team
