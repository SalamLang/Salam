<div align="center">

<p>
    <a href="https://salamlang.ir/">
        <img width="150" src="https://raw.githubusercontent.com/SalamLang/Salam/main/assets/logo-box.svg" alt="The Salam Programming Language - زبان برنامه نویسی سلام">
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

## 🛠️ The Compiler (`salamc`)

Salam is statically typed and compiled. The **general** language transpiles to C and is
built to a native executable; embedded **`layout:`** blocks compile to HTML/CSS/JS.

### Build

Requires a C compiler. **tcc** is the default backend (bundled math, fast); gcc/clang also work.

```sh
sh tools/build-compiler.sh        # quick build with tcc  ->  ./salamc
# or, with CMake (out-of-tree build, then run the test suite via ctest):
cmake -B build && cmake --build build && ctest --test-dir build
```

`runtime/` builds into `libsalam.a` (linked into compiled programs). `salamc build` also
embeds a self-contained runtime copy, so generated programs compile with no external deps.

### Usage

```sh
# general language -> native executable
salamc build app.salam --output=app.exe        # then ./app.exe
salamc cli build app.salam --keep-c            # optional 'cli' prefix; keep generated C
salamc obj app.salam                            # compile to .o only

# layout DSL -> website
salamc layout build page.salam                  # page.html + page.css + page.js
salamc layout build page.salam --inline         # one self-contained page.html
salamc layout build a.salam b.salam             # per-page html + merged style.css/script.js

# inspect any stage (general or layout)
salamc app.salam --emit-tokens-xml | --emit-ast-xml | --emit-symbol-xml
salamc app.salam --log-level=trace
salamc build app.salam -DDEBUG                  # preprocessor define

# format source in place (auto-detects nothing - pass --lang=fa for Persian files)
salamc fmt app.salam                            # reformat one file
salamc fmt                                      # reformat every .salam under the cwd, recursively
salamc fmt src/ examples/                       # reformat given files and/or directories
salamc fmt --check                              # report files that need formatting (exit 1 if any)
salamc fmt page.salam --lang=fa                 # Persian source

# REPLs
salamc cli       # general
salamc layout    # layout
```

Hello, World:

```salam
func main {
    println("Hello, World!")
}
```

### Documentation

- [doc/user-guide.md](doc/user-guide.md) - language tour + CLI guide.
- [doc/architecture.md](doc/architecture.md) - compiler internals for contributors.
- [doc/hashmap.md](doc/hashmap.md) - arrays and the HashMap type.
- [doc/interface.md](doc/interface.md) - interfaces as generic bounds (`<T: Shape>`); struct vs interface.
- [doc/extern.md](doc/extern.md) - calling C directly via `extern "C"` (FFI) + `@link`.
- [doc/format.md](doc/format.md) - the `salamc fmt` source formatter (rules, comments, recursion).
- [doc/abi.md](doc/abi.md) - the C ABI: type mapping, struct layout, linkage.
- Specs: [SALAM-SPEC.md](SALAM-SPEC.md) · [SALAM-TYPES.md](SALAM-TYPES.md) · [SALAM-GRAMMAR.md](SALAM-GRAMMAR.md).
- Tests: `sh tools/run-tests.sh`.

## 🤝 Contributing

We welcome contributions from the community!

- 📖 Read our [Contributing Guide](CONTRIBUTING.md).
- 🌟 Follow our [Code of Conduct](CODE_OF_CONDUCT.md).

Together, let’s make coding accessible to all.
For inquiries or feedback, visit: [salamlang.ir](https://salamlang.ir)

© 2024-2026 Salam Language Team
