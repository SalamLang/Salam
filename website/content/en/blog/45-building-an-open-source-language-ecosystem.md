---
title: How a new programming language builds an open source ecosystem: packages, registry, documentation and contributors in Salam
seo_title: Building an Open Source Language Ecosystem: Salam's Package Manager
description: How a young language grows an open source ecosystem: Salam's package manager, lock file, checksums, registry and docs, and what is still missing.
keywords: open source ecosystem, package manager, programming language ecosystem, salam add, Salam packages, package registry, lock file checksums, minimal version selection, open source contributors, language tooling
category: future
summary: A language is only as useful as the code around it. This is how Salam's package manager, standard library, documentation and contributor process are built, and an honest account of how small the ecosystem still is.
date: 2026-11-11
---

An open source ecosystem is everything around a language that lets people reuse each other's work: a package manager, a place to find packages, a way to trust what you download, documentation, editor support, and a community that keeps all of it alive. For a new programming language, building that ecosystem is harder than building the compiler. This article looks at how Salam approaches it: a large standard library first, then a Git-based package manager with lock files and checksums, a registry, and tooling that makes packages usable in both English and Persian. It is also plain about the current state, which is early.

## What a language ecosystem is made of {#definition}

**A programming language ecosystem is the set of shared code, tools, infrastructure and people that turn a compiler into something you can build products with.** Rust has crates.io and Cargo, Python has PyPI and pip, JavaScript has npm, and Go has module paths, a module proxy and a checksum database. Each of those took years and a large community to grow.

A new language faces a chicken-and-egg problem. Developers wait for libraries before they adopt a language, and libraries only appear once developers have adopted it. There are only a few ways out:

- ship a standard library big enough that early users do not need third-party code for common work;
- make it trivial to publish and consume packages, so the first few authors face no friction;
- make interop with an existing ecosystem cheap, so missing libraries can be borrowed;
- invest in documentation and tooling, which are part of the ecosystem even when nobody calls them that.

Salam does all four, and the balance between them says a lot about where the project is.

## The standard library as the first ecosystem {#standard-library}

Salam's main bet so far is a large standard library that ships with the compiler. The [library reference](/std/) documents more than 170 packages: text and Unicode, files and processes, HTTP servers and clients with a router, WebSocket, gRPC, SSH and TLS, drivers for SQLite, PostgreSQL, MySQL, Redis and ClickHouse, compression and archives, cryptography, images, audio, matrices, tensors and neural networks. None of it needs `salam add`; an `import` is enough.

This is the "batteries included" strategy Python made famous, and it has a clear cost: every package in `std/` is maintained by the core project, so the core project carries all of it. It also has a clear benefit for a young language. Someone evaluating Salam can write a web service with a database, a JSON API and TLS without depending on a single package written by a stranger.

The project made this choice explicit in October 2026, when a separate `packages/` tree of libraries was folded into `std/`. The package manager page states the rule: everything in the standard library ships with the compiler, and the package commands are for code somebody publishes in their own repository.

For C libraries, the [FFI](/blog/salam-c-interop-ffi/) is the third escape hatch: an `extern` block can call any C function, and a package manifest can describe how to find or build the C library it wraps.

## The package manager: salam.toml, salam add and the lock file {#package-manager}

Third-party code lives in ordinary Git repositories. A project is a folder with `main.salam` and a `salam.toml` manifest, and `salam new` or `salam init` creates both. Adding a dependency is one command:

```bash
salam init
salam add github.com/owner/repo          # newest release
salam add github.com/owner/repo@^1.2     # a version range
salam deps                               # after a fresh clone: download and verify
salam update                             # newest release with the same major version
salam install github.com/owner/tool      # build a program into ~/.salam/bin
```

The manifest records what you asked for, and `salam.lock` records exactly what you got:

```toml
[package]
name = "myapp"
version = "0.1.0"
salam = ">=0.5.9"

[dependencies]
"github.com/owner/repo" = "v1.2.0"
"example.com/me/lib" = { path = "../lib" }
```

The design borrows openly from Go modules. A package is named by its repository path, so there is no central name to register. Versions are Git tags (`v1.2.0`), with pseudo-versions for repositories that have no tags. When two dependencies ask for different versions of the same module, the newer one wins, which the [PACKAGES.md](https://github.com/SalamLang/Salam/blob/main/docs/PACKAGES.md) document calls minimal version selection, "as in Go". A new major version is a separate module path (`.../v2`), so v1 and v2 can live in one program.

Two details make this practical for a small community. Packages can be fetched from GitHub, GitLab and Codeberg even without Git installed, through each host's archive API. And `[package].salam` declares the oldest compiler a package builds with, so `salam add` stops with "needs Salam X or newer" instead of failing with a confusing compile error. That matters for a pre-1.0 language whose syntax still changes between releases.

## Trust: checksums and immutable versions {#trust}

A package manager is a supply-chain tool, and the dangerous failure is silently installing different code than the author published. Salam answers this in three layers.

1. **Lock file sums.** Every entry in `salam.lock` has a SHA-256 sum over all files of the package (except `.git`), so a Git checkout and a tarball download produce the same sum. `salam deps` re-checks it.
2. **A machine-local record.** The first time a machine downloads a version, its sum is kept in `~/.salam/pkg/sums.txt`. If a later download of the same version differs, for example because someone moved a tag after release, the command stops.
3. **A published list.** Sums are also checked against `registry/sums.txt` in the Salam repository, a reviewed list of `module@version sum` lines. A download that disagrees with a published line is refused. `salam deps sums` prints the lines for a pull request.

That third layer is a small, reviewed cousin of Go's checksum database. It is honest to add that it is new: the file exists and the tooling uses it, but it holds no entries yet.

## Discovery: the registry and salam search {#registry}

Finding packages is the other half of an ecosystem. `salam search` looks in two places: the standard library, and `registry/index.toml`, a file in the main repository where authors add an entry for their package with a pull request.

```bash
salam search gguf
```

```text
ai.gguf  (standard library, import it directly)
```

The registry format is deliberately simple: a name, a one-line description and tags. Publishing is equally simple: push a repository with a `salam.toml` and an entry file named after the repository, tag a release, and open a pull request to the index. The [package manager page](/packages/) walks through it with a working example.

Here is the uncomfortable fact: as of November 2026, the registry index lists no third-party packages. The machinery is in place, and the standard library covers most early needs, but the community of package authors has not formed yet. Any honest description of the Salam ecosystem has to start from that number.

## Writing a package for two language communities {#bilingual-packages}

One part of Salam's ecosystem design has no direct equivalent in mainstream languages. Salam source can be written in English or Persian, and a public function can carry a name in each. The standard library does this for every public name; it has more than 21,000 `@fa` annotations. A third-party package can do the same:

```salam
import str

@en "Slug"
@fa "نامک"
pub func Slug(title: str): str:
    mut out := ""
    mut gap := false
    each ch in str.Lower(str.Trim(title)):
        if (ch >= "a" and ch <= "z") or (ch >= "0" and ch <= "9"):
            if gap and out != "":
                out += "-"
            end
            out += ch
            gap = false
        else:
            gap = true
        end
    end
    ret out
end

func main:
    println Slug("Hello, World!")
    println Slug("  Salam 0.6: what's new?  ")
end
```

```output
hello-world
salam-0-6-what-s-new
```

An English program calls `Slug`, a Persian program calls `نامک`, and both reach the same function in the same compiled package. For a library author that means one codebase serves two communities, without a wrapper library and without a fork. Whether that turns into a real advantage depends on Persian-speaking developers publishing and using packages, which is part of the open question. The article [One language, two scripts](/blog/one-language-two-scripts/) covers the bilingual model itself.

The same toolchain helps with evolution. A function marked `deprecated` keeps working, and every caller gets warning W013 at compile time, which gives authors a standard way to rename an API without breaking users overnight.

## Documentation, tests and editors are ecosystem too {#tooling}

Libraries are only half of what newcomers look for. The other half is "can I learn this, and will my editor understand it?" The Salam repository puts a lot of weight here:

- **Documentation built from tested code.** The library reference has a page per package, and its examples are compiled when the website builds. This magazine works the same way: every Salam program in these articles is compiled, and the ones with output are run and compared.
- **A free course** of 47 lessons in English and Persian, from the first program to web servers, in the [learn section](/learn/).
- **`salam doc`** generates an API page for your own project, and **`salam test`** runs the `test_` functions in `*_test.salam` files, so package authors get documentation and testing without extra tools.
- **Editors.** The repository has plugins or syntax files for eleven editors, among them VS Code, Vim, Emacs, Sublime Text, Kate and the JetBrains IDEs, a tree-sitter grammar, and a language server built into the compiler (`salam lsp`). The [developer tools article](/blog/salam-developer-tools/) covers them.

One licensing decision belongs here. The repository is GPL-3.0, but the VS Code extension directory, including the TextMate grammar, is MIT. The README explains why: highlighting is only useful if other projects can embed the grammar, and some, GitHub Linguist among them, accept only permissive licenses for grammars. It is a small example of thinking about the ecosystem rather than only the compiler.

## Contributors and how work gets in {#contributors}

Open source ecosystems are made of people, so the numbers matter. In 2026 the main branch received about 2,750 commits from roughly fifty different author names (some of them the same person under different spellings). About two thirds of those commits came from the language's designer, Seyyed Ali Mohammadiyeh, and one other contributor accounts for several hundred more. That is a typical shape for a young language, and it is also a risk: a project carried mostly by one person depends on that person.

The process for outside contributions is conventional and documented. [CONTRIBUTING.md](https://github.com/SalamLang/Salam/blob/main/CONTRIBUTING.md) explains the fork-and-branch workflow, commit message style and how to build the compiler. There are issue templates for bugs and feature requests, a code of conduct, and CI that runs tests, linters and CodeQL scanning on pull requests. Discussion happens in a Telegram group, on Discord, and on two Iranian messengers, Eitaa and Bale, which reflects where the early community is.

The community page also lists contributions that do not touch the compiler: writing tutorials and examples in either language, reporting bugs with small reproductions, and introducing Salam to students and teachers. For a young ecosystem, those are not lesser contributions.

## What has to happen next {#next}

This section is analysis, not a published roadmap. Looking at what exists and what is missing, these are the steps that would move Salam from "a language with a big standard library" to "a language with an ecosystem":

1. **The first real third-party packages.** The registry needs its first entries, ideally libraries that the standard library deliberately does not cover: domain-specific clients, frameworks with opinions, bindings to popular C libraries.
2. **A filled checksum list**, so the trust model is exercised by real packages, not only by tests.
3. **More maintainers.** Spreading review and ownership across more people is the single biggest step toward long-term health; the [production-readiness article](/blog/what-makes-a-language-production-ready/) explains why buyers look at this first.
4. **Stability.** Package authors need confidence that their code still compiles next year. A 1.0 release with a compatibility promise matters more for the ecosystem than any single feature.
5. **Showcase projects** with source code, so newcomers can read idiomatic Salam at scale.

None of this is guaranteed, and every language that has a healthy ecosystem today went through the same empty-registry phase. What Salam has done is remove most of the technical excuses: publishing is a Git tag, consuming is one command, and the trust model is designed in from the start. The rest depends on people. The [future of Salam](/blog/future-of-salam/) article looks at that longer arc.

## Frequently asked questions {#faq}

### How do I install a Salam package?

Inside a project, run `salam add github.com/owner/repo`. It downloads the newest release, records it in `salam.toml` and `salam.lock`, and you import it with `import "github.com/owner/repo"`. Standard library packages need no installation at all.

### Is there a central package registry for Salam?

There is a registry index in the main repository that `salam search` reads, but packages are hosted in their own Git repositories, as with Go modules. As of November 2026 the index has no third-party entries yet.

### How does Salam protect against tampered packages?

Every locked package has a SHA-256 sum that `salam deps` verifies. A machine also remembers the sum of each version it downloaded and stops if a later copy differs, and downloads are checked against a reviewed list of published sums in the Salam repository.

### How can I contribute to Salam?

Read CONTRIBUTING.md on GitHub, pick a small issue, and open a pull request from a fresh branch. Documentation, examples, translations between English and Persian, bug reports and packages of your own are all useful contributions.
