---
title: Can a new programming language compete with decades-old ones? Salam's place in the global ecosystem
seo_title: Can a New Programming Language Compete? The Case of Salam
description: How new programming languages win adoption, what an ecosystem really consists of, and an honest look at where Salam stands against decades-old languages.
keywords: new programming language adoption, programming language ecosystem, how programming languages become popular, Salam programming language, language adoption history, open source language community, Persian programming language, Salam vs established languages
category: compare
summary: Every popular language was once new, and most new languages never become popular. This article looks at what carried Go, Rust, Kotlin and others to adoption, and measures Salam against the same forces without pretending the gap is small.
date: 2026-11-11
---

Can a new programming language compete with languages that have thirty or fifty years of libraries, books and developers behind them? **Yes, but almost never head-on. New languages that succeed do so by being clearly better at something specific, by riding a new platform, or by fitting into an ecosystem that already exists, and then they grow outward from that niche over many years.** Salam, a young compiled language you can write in English or Persian, faces exactly this question, and the honest answer for it is "not yet, and only in some places".

This article looks at how the last generation of languages broke through, what an ecosystem actually consists of, where Salam stands today item by item, and what it would still have to earn. It is written for developers and technical decision makers weighing a young language, and it tries to be as clear about the gaps as about the strengths.

## How the last generation of languages broke through {#history}

The languages that dominate today did not win on syntax. Each had a force behind it that made adoption easier than staying with the old tools.

| Language | First released | What carried it |
|---|---|---|
| C | 1972 | Unix, written in it and spread with it |
| Python | 1991 | Readability, then decades later the scientific and machine learning stack |
| Java | 1995 | The JVM's "write once, run anywhere", then enterprise servers |
| JavaScript | 1995 | The only language every browser runs |
| C# | 2000 | Microsoft's .NET platform and Windows development |
| TypeScript | 2012 | Types layered over JavaScript, adoptable one file at a time |
| Go | 2009 (1.0 in 2012) | Google, simple concurrency, then Docker and Kubernetes |
| Swift | 2014 | Apple's platforms |
| Rust | 2015 (1.0) | Memory safety without a garbage collector |
| Kotlin | 2016 (1.0) | Full Java interop, then Google's support for Android |
| Zig | 2016 | A simpler C with first-class cross-compilation |

Three patterns stand out. A **platform owner** can make a language the default (Swift on Apple devices, C# on .NET, Kotlin on Android). **Interoperability** removes the cost of leaving the old world behind (TypeScript runs all JavaScript, Kotlin calls all Java, Zig compiles C). And a **distinct technical advantage** gives early adopters a reason to accept the risk (Rust's safety guarantees, Go's simple deployment and concurrency). Python is the reminder that it can also take a long time: it was more than fifteen years old before the scientific stack made it a default choice.

What none of them did was replace their predecessors. C, Java and Python are all still here. Success for a new language means owning some territory, not winning everything.

## What an ecosystem actually is {#ecosystem}

When people say a language "has no ecosystem", they mean several different things, and it helps to separate them:

- **Libraries.** Can I parse this format, talk to that database, call this cloud API, without writing it myself?
- **Tools.** Editors, debuggers, profilers, formatters, linters, build and package management, CI integration.
- **Knowledge.** Documentation, books, courses, answers to the exact error message you are looking at.
- **People.** Developers who can be hired, maintainers who answer issues, a community that reviews code.
- **Trust.** Stability promises, security processes, a track record in production, an organization that will still be there in ten years.

A new language can build the first three surprisingly quickly with focused work. The last two only come with time and use, and no amount of engineering can shortcut them.

## Where Salam stands today {#salam-today}

Measured against that list, here is Salam's position at the end of 2026, using only what can be checked in the [repository](https://github.com/SalamLang/Salam).

**The compiler.** Salam's compiler is written in Salam and builds itself; the original C implementation was retired in August 2026. It generates code through C, through LLVM or through its own native backend, and targets Linux, macOS, Windows, WebAssembly, JavaScript and freestanding bare-metal programs. Self-hosting is a meaningful milestone, because a compiler exercises nearly every feature of a language (see [Inside Salam's self-hosting compiler](/blog/self-hosting-compiler/)).

**Libraries.** This is the strongest part. More than 170 standard library packages ship with the compiler, each with a reference page and checked examples on the library reference: HTTP servers with routing and middleware, WebSocket and gRPC, TLS written in Salam, drivers for PostgreSQL, MySQL, SQLite, Redis and ClickHouse, JSON, YAML, TOML, XML and CSV, compression and archives, cryptography, images and audio, matrices, tensors and neural network layers. A small program can do real work without a single dependency:

```salam
import crypto
import encoding.json

struct Artifact:
    pub file: str
    pub size: int
    pub sha256: str
end

func main:
    body := "hello from a release file"
    a := Artifact { file = "notes.txt", size = body.len(), sha256 = crypto.Sha256Hex(body) }
    println json.Marshal(a)
end
```

```output
{"file":"notes.txt","size":25,"sha256":"41d7154cf936747e57ae6453db9b284af95250e0b32758dd6316ed294a98e487"}
```

The [standard library tour](/blog/salam-standard-library-tour/) goes through the main areas.

**Third-party packages.** This is the weakest part. `salam add` fetches packages from GitHub, pins them in a lock file and verifies checksums, and `salam search` reads a public registry. The registry exists and is open to pull requests, but it is still essentially empty. Every package a Salam program uses today comes from the standard library, from the project itself or from C.

**Tools.** One executable builds, runs, tests, formats, type-checks, translates between English and Persian, builds web pages, launches a debugger and checks memory. A language server gives editors errors and completion, and there are plugins for VS Code and ten other editors, plus an [online editor](https://editor.salamlang.ir) that runs the compiler in the browser.

**Knowledge.** A free [47-lesson course](/learn/) in English and Persian with complete programs and real output, the library reference, and this magazine. The site also publishes an `llms.txt` file and a detailed language reference for coding assistants, which matters more each year (see [programming languages in the AI era](/blog/programming-languages-in-the-ai-era/)).

**Performance.** On the project's benchmark of 39 same-algorithm programs, Salam averaged 10.6 ms against 11.7 ms for C and 11.3 ms for Rust, which the [speed page](/speed/) reads as the same range, not a ranking: the programs are short, start-up is part of each time and gaps under about 5% are noise. Salam was ahead on 6 programs, level on 27 and behind on 6.

**People and trust.** Here Salam is at the beginning. The community is small, the language is pre-1.0 and still changes between releases, there are few production deployments, and no platform owner requires it. The license is GPL-3.0 and development is public on GitHub. The article [What makes a programming language production ready?](/blog/what-makes-a-language-production-ready/) works through these points one by one.

## A niche no other language fills {#niche}

The historical pattern says a new language needs a distinct advantage. Salam's most distinctive one is not speed, which C and Rust already offer, but that it is a complete, compiled systems language in two scripts. Every keyword, type, standard library package and compiler error message has a Persian name as well as an English one, and both compile to the same program. Here is a small grade-average program written entirely in Persian:

```salam
تابع میانگین(نمره‌ها: اعشار۶۴[]): اعشار۶۴:
    ناپایا جمع := ۰.۰
    هر نمره در نمره‌ها:
        جمع += نمره
    پایان
    برگشت جمع / (طول(نمره‌ها) برگردان اعشار۶۴)
پایان

تابع ریشه:
    نمره‌ها := [۱۷.۵، ۱۹.۰، ۱۴.۲۵، ۱۸.۰]
    معدل := میانگین(نمره‌ها)
    اگر معدل بزرگتربرابر ۱۷.۰:
        سرچاپ "عالی:"، معدل
    وگرنه:
        سرچاپ "معمولی:"، معدل
    پایان
پایان
```

```output
عالی: 17.1875
```

Persian digits, the Persian comma and multi-word names all work, and `salam translate` converts the keywords of a file between the two spellings. Translating keywords is the easy part. What Salam adds is everything around them: a self-hosting native compiler, a large standard library, compiler messages and tools that all work in a right-to-left, non-Latin script. That gives Salam a clear audience: Persian-speaking students, teachers and developers who want to start in their own language and move to English without switching languages. The articles [One language, two scripts](/blog/one-language-two-scripts/) and [Programming in your native language](/blog/programming-in-your-native-language/) explore this.

Beyond that niche, Salam's case is a combination rather than a single feature: native performance in the same range as C and Rust, a smaller and more readable language than C++ or Rust, a large application-level standard library, and one toolchain. That combination is attractive for web services, command-line tools and system components. It is also a crowded space where Go, Rust and Zig are already established.

## Borrowing decades of C {#interop}

Every successful new language has found a way to use the old world's code. For Salam that bridge is C. An `extern:` block declares C functions and Salam calls them directly, structs pass by value with the platform's C ABI, and `link dynamic "name"` links a system library. The standard library uses this itself: the SQLite driver links the system's `sqlite3` library rather than reimplementing it. In the other direction, `export:` and `salam build --lib` publish Salam functions as a C library with a header, so Salam code can be called from any language that can call C. The details are in [Calling C from Salam](/blog/salam-c-interop-ffi/).

This does not give Salam the npm or PyPI ecosystems, but it means a missing library is a wrapper away rather than a rewrite away, which is how Zig, Nim and others have bridged the same gap.

## What Salam still has to earn {#hard-parts}

Being honest about the remaining work is part of being taken seriously:

1. **Stability.** A 1.0 release with a compatibility promise. Until then, upgrading may mean changing code, as when typed local declarations were removed in 2026.
2. **Third-party packages.** The package manager works; it needs people publishing libraries for the APIs, formats and services the standard library does not cover.
3. **Production stories.** Public, verifiable accounts of Salam services running for months under real load, with what went wrong and how it was fixed.
4. **Contributors.** A compiler and a standard library of this size need more maintainers and reviewers than a small team can provide.
5. **Memory safety tooling.** Salam frees heap memory explicitly and has no borrow checker. Its compile-time checks and `salam memcheck` help, but teams coming from garbage-collected languages need more assurance over time.
6. **Visibility.** Most developers have never heard of Salam, and coding assistants know far less about it than about Python. Documentation in English, this magazine and machine-readable references are a start.

None of these is a technical impossibility. All of them take years, which is consistent with every language in the table above.

## What success could realistically look like {#success}

Success for Salam does not look like replacing Python or Java. A realistic picture is narrower: the default way Persian speakers learn to program and then move into systems and web work; a credible choice for small, fast native services and tools where a single binary and predictable memory matter; a teaching compiler that students can read because it is written in the language it compiles. From there, as with Python's slow climb or Rust's decade before mainstream use, wider adoption depends on the people who build things with it. The article [Salam in five to ten years](/blog/future-of-salam/) sketches that path.

## Frequently asked questions {#faq}

### Why do most new programming languages fail?

Usually not because of technical flaws, but because switching costs more than staying: there are no libraries, no tools, no developers to hire and no proof that it will be maintained. Languages that succeed lower those costs through interoperability, a platform that requires them or an advantage large enough to justify the risk.

### Is it worth learning a new language like Salam?

As a second language, yes, especially if you want to understand compilers, systems programming or programming in Persian. As the only language for a career, no: learn an established language too, because the job market for Salam is tiny today.

### How long does it take a programming language to become mainstream?

Usually a decade or more. Rust reached 1.0 in 2015 and spent years more before broad industry use, and Python was over fifteen years old before it became the default for science and machine learning.

### Can Salam use existing C libraries?

Yes. Salam calls C functions declared in an `extern:` block and links libraries with `link dynamic`, and its own SQLite driver works this way. That makes the large body of existing C code available without rewriting it.
