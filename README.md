<div align="center">

<p>
    <a href="https://salamlang.ir/">
        <img width="150" src="https://raw.githubusercontent.com/SalamLang/Salam/main/design/logo/logo-box.svg" alt="The Salam Programming Language - زبان برنامه نویسی سلام">
    </a>
</p>

# Salam Language

A new programming language empowering developers in Persian and Arabic-speaking communities.
Unlock the joy of coding-accessible, intuitive, and culturally resonant.

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

```sh
sh tools/build-compiler.sh        # quick build with tcc  ->  ./salam
# or, with CMake (out-of-tree build, then run the test suite via ctest):
cmake -B build && cmake --build build && ctest --test-dir build
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
salam fmt src/ examples/                       # reformat given files and/or directories
salam fmt --check                              # report files that need formatting (exit 1 if any)
salam fmt app.salam --tabs                     # indent with tabs (convert spaces to tabs)
salam fmt src/ --indent=2                      # indent with 2 spaces per level
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

The repository ships a multi-stage [`Dockerfile`](Dockerfile) and a
[`docker-compose.yml`](docker-compose.yml) built on **Alpine Linux** (musl, the
lightest practical base). The image includes everything the compiler needs:
a C compiler (`gcc` + `tcc`), the **LLVM 22** backend (`clang-22`, `llc-22`,
`opt-22`, `lli-22`), and `make`, no CMake required.

There are two modes:

### Development (live reload)

The source tree is bind-mounted and `./salam` is **recompiled automatically on
every change** to `src/` (powered by `entr`, see [`tools/docker-dev.sh`](tools/docker-dev.sh)):

```sh
docker compose up dev          # first build, then watch & rebuild on save
```

Edit any file under `src/` on your host and the container rebuilds `./salam`
incrementally. To get a shell inside the same environment:

```sh
docker compose run --rm dev sh
```

### Production (copy & build)

The production stage runs `COPY .` then `make && make install`, baking a fully
built, self-contained compiler into the image. `salam` is the entrypoint:

```sh
docker compose build prod                                  # build the image
docker compose run --rm prod build examples/en/hello.salam --output=hello
docker compose run --rm prod layout build page.salam --inline
docker compose run --rm prod --help
```

Your current directory is mounted at `/work`, so the compiler can read your
`.salam` sources and write its output back to the host.

### Plain Docker (without Compose)

```sh
docker build --target prod -t salam:prod .                 # production image
docker run --rm -v "$PWD":/work salam:prod build app.salam --output=app

docker build --target dev  -t salam:dev  .                 # dev image
docker run --rm -it -v "$PWD":/app salam:dev               # live-rebuild loop
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
