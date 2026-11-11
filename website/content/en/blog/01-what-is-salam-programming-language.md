---
title: What is the Salam programming language? A tour of a modern general-purpose and systems language
seo_title: What Is the Salam Programming Language? A Complete Introduction
description: Salam is an open source, statically typed systems language you can write in English or Persian. What it is, how it compiles, what it can build and where it falls short.
keywords: Salam programming language, what is Salam, SalamLang, Persian programming language, bilingual programming language, systems programming language, compiled programming language, new programming language
category: language
summary: A complete introduction to Salam: the language, the self-hosting compiler, the standard library, the tools, the measured performance and the honest limits of a young language.
date: 2026-11-11
---

Salam is an open source, general-purpose and systems programming language. It is statically typed, it compiles to native machine code, and its compiler is written in Salam itself. What makes it unusual is that every keyword, type, standard library package and compiler error message exists in two spellings, English and Persian, and both spellings compile to exactly the same program.

This article is the starting point of the Salam Magazine. It explains what kind of language Salam is, shows real code, walks through the compiler and the toolchain, gives the measured performance with its caveats, and is plain about what a young language does not have yet.

## Salam in one paragraph {#summary}

**Salam is a compiled, statically typed language for building everything from command-line tools and web servers to desktop apps and bare-metal kernels.** You write `func main:` and a block that ends with `end`; the compiler checks the types, generates code through C, LLVM or its own native backend, and leaves you with a single executable for Linux, macOS, Windows or the web. There is no virtual machine and no garbage collector. A standard library of more than 170 packages covers networking, databases, cryptography, compression, images, machine learning and more, and a built-in page language turns Salam code into HTML, CSS and JavaScript. The name means "peace" and "hello" in Persian.

## A first program {#first-program}

Here is the smallest complete Salam program. Blocks open with a colon and close with `end`, and `println` prints its arguments followed by a new line.

```salam
func main:
    println "Hello, Salam!"
end
```

```output
Hello, Salam!
```

The same program written in Persian. The compiler detects the language from the keywords, so there is no switch to flip and no separate toolchain: `تابع` is `func`, `ریشه` is the entry function `main`, `سرچاپ` is `println` and `پایان` is `end`.

```salam
تابع ریشه:
    سرچاپ "سلام دنیا!"
پایان
```

```output
سلام دنیا!
```

Both files go through the same frontend, the same type checker and the same code generator. Their executables are interchangeable. The article [One language, two scripts](/blog/one-language-two-scripts/) explains how that works in detail.

## What kind of language is Salam? {#kind}

If you have used Go, Rust, Swift or Python, most of Salam will look familiar. The table puts the main design choices in one place.

| Question         | Salam's answer                                                                                      |
| ---------------- | --------------------------------------------------------------------------------------------------- |
| Typing           | Static, with type inference: `count := 0` is an `int`, `x := 3.14` is an `f64`                      |
| Compilation      | Ahead of time to native code, through C, LLVM or Salam's own native backend                         |
| Memory           | No garbage collector; values live on the stack, heap containers are freed with `.free()` or `defer` |
| Data types       | Structs with methods, enums that carry data, tuples, generics, interfaces, `T?` optionals           |
| Errors           | `Result` values with `try`, plus `throws`, `throw` and `try`/`catch`                                |
| Concurrency      | Threads, a bounded task pool, the `go` keyword, channels, futures, atomics                          |
| Targets          | Linux, macOS, Windows, WebAssembly, JavaScript, Android, iOS and freestanding bare metal            |
| Source languages | English and Persian, with one shared standard library                                               |
| License          | GPL-3.0, developed in the open on GitHub                                                            |

Variables are immutable unless you ask otherwise: `name := "Sara"` cannot be reassigned, `mut count := 0` can. Reassigning an immutable variable is a compile error, not a warning. That small rule removes a whole category of accidental bugs and makes code easier to read, because a name without `mut` never changes.

## A taste of real code {#taste}

The next program uses a struct, an array literal, a function with a multi-word name, an `each` loop and compound assignment. It is short, but it shows how Salam code tends to read.

```salam
struct Book:
    pub title: str
    pub pages: int
end

func reading time(b: Book): int:
    ret b.pages / 40
end

func main:
    books := [
        Book { title = "The Little Prince", pages = 96 },
        Book { title = "Dune", pages = 412 },
        Book { title = "SICP", pages = 657 },
    ]
    mut total := 0
    each b in books:
        hours := reading time(b)
        println b.title, "takes about", hours, "hours"
        total += b.pages
    end
    println "Pages in total:", total
end
```

```output
The Little Prince takes about 2 hours
Dune takes about 10 hours
SICP takes about 16 hours
Pages in total: 1165
```

Two details stand out. First, `reading time` is a single function name made of two words. Salam allows spaces inside names so that a call can read like a sentence, which matters even more in Persian, where many concepts are naturally written as two words. Second, the fields are marked `pub`: struct fields are private to their package unless you say otherwise, so the public surface of a type is always explicit.

## One language, two spellings {#bilingual}

Most "programming in your native language" projects stop at translated keywords. Salam goes further. The keywords, the primitive types, the built-in methods, the entry function, every standard library package and every function in it, and every compiler diagnostic have a Persian name next to the English one. A Persian programmer can read an error message such as "unknown identifier" in Persian, with the error code written in Persian digits, while an English programmer sees the same message in English.

Because both spellings map to one program, the two communities share one compiler, one standard library and one set of tests. The `salam translate` command rewrites a file from one spelling to the other, which is how many of the examples on this site exist in both languages. Persian source also accepts Persian digits in number literals and the Persian comma as a separator, and the online editor and the editor plugins lay Persian code out right to left.

## The compiler {#compiler}

The Salam compiler is self-hosting: it is written in Salam and compiles itself. The original C implementation was retired in August 2026, and since then every release has been built by the previous release of the compiler. Writing the compiler in the language it compiles is the strongest test a language can face, because a compiler needs nearly every feature: structs, enums, hash maps, strings, files, recursion and a lot of careful memory handling.

A build goes through the usual stages: lexing, parsing, semantic analysis with type inference and many static checks, then code generation. There are three backends:

- **LLVM**, the default in the release builds (which embed it), and also the engine behind the WebAssembly target;
- **C**, selected with `--backend=c` and used whenever LLVM is not built-in, which hands generated C to gcc, clang or the bundled tcc;
- **a native backend**, selected with `--backend=native`, which writes x86-64, 32-bit x86, AArch64, 32-bit ARM or RISC-V machine code itself and links it with a built-in linker, so it needs no external toolchain at all.

The checks run before any code is generated. `salam check` stops there, which makes it the fast way to find mistakes, and its diagnostics carry stable codes (E001, W147 and so on) that you can look up. The [compiler article](/blog/how-the-salam-compiler-works/) follows a program through every stage.

## The standard library {#std}

Salam ships with a large standard library, documented package by package on the [library reference](/std/) with an example for every public function. It includes:

- text, regular expressions, Unicode, Persian and Arabic text helpers, and templates;
- files, processes, the terminal, time and calendars, including the Jalali (Persian) calendar;
- HTTP clients and servers with a router and middleware, WebSocket, HTTP/2, gRPC, GraphQL, SSH and TLS;
- drivers for SQLite, PostgreSQL, MySQL, Redis and ClickHouse, clients for Elasticsearch, OpenSearch, Meilisearch and Typesense, and a checker for SQL strings;
- cryptography, JWT, compression and archives (ZIP, tar, gzip, xz, zstd, 7z, and reading RAR);
- images, audio, matrices, tensors, automatic differentiation and neural network layers.

Packages written by other people are added from GitHub with `salam add`, pinned in `salam.lock` and verified by checksum. The [package manager page](/packages/) explains the workflow.

## The toolchain {#tools}

Everything is one executable called `salam`. The commands you will use most are:

```bash
salam run hello.salam        # build and run, keep nothing
salam build hello.salam      # produce an executable
salam check src/             # type-check without generating code
salam test                   # run every test_ function in *_test.salam files
salam format .               # format the source in place
salam translate fa app.salam # rewrite English source as Persian
salam page build site.salam  # turn a page program into HTML, CSS and JS
```

There is also an interpreter (`salam exec`) that needs no C toolchain, a REPL, a documentation generator, a debugger launcher, a memory checker, a language server for editors and plugins for Visual Studio Code and ten other editors. If you would rather not install anything, the [online editor](https://editor.salamlang.ir) compiles and runs Salam in the browser.

## How fast is it? {#performance}

Salam compiles to native code without a garbage collector, so it belongs with C, Rust and Go rather than with Python. The project's own benchmark compares 39 programs that use the same algorithm in every language. Measured on 27 October 2026, Salam averaged 10.6 ms, C 11.7 ms and Rust 11.3 ms. Against the fastest of C, C++, Rust and Go on each program, Salam was ahead on 6, level on 27 and behind on 6. Python took about 56 times as long.

Read those numbers as "the same range as C and Rust", not as a ranking. Most of the programs finish in a few milliseconds, so process start-up is part of every measurement, and gaps under about 5% change from run to run. The [speed page](/speed/) publishes every number and the raw data, and the article [Is Salam faster than C and Rust?](/blog/is-salam-faster-than-c-and-rust/) reads them carefully.

## Who is Salam for? {#who}

- **Persian speakers learning to program**, who can start in their own language and move to English keywords later without changing languages.
- **Systems and backend developers** who want native performance with a smaller, more readable syntax than C or C++.
- **Web developers** who want one language for the server and for the pages it serves.
- **Teachers and students** of compilers, because the compiler is written in the language it compiles and its source is open.

## What Salam is not, yet {#limits}

An honest introduction has to say where a language falls short. Salam is young, and it shows in a few places:

- **No borrow checker.** Salam does not prove memory safety the way Rust does. It has bounds checks, ownership warnings and a memory checker, but freeing heap memory is still your job.
- **A small ecosystem.** The standard library is large, but the community and the number of third-party packages are tiny compared with Python, Go or Rust.
- **Pre-1.0.** The language still changes between releases. Typed local declarations, for example, were removed in 2026. Expect to update code when you upgrade.
- **Few production deployments** so far, which means fewer battle-tested answers to unusual problems.

None of these is permanent, but they matter when you choose a language for a project today.

## How to start {#start}

Install the compiler with one command on Linux or macOS:

```bash
curl -fsSL https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.sh | sh
```

Windows users run the PowerShell command on the [install page](/install/). Then follow the [free course](/learn/), which goes from the first program to web servers in 47 short lessons, every one with complete programs and their real output.

## Frequently asked questions {#faq}

### Is Salam a Persian programming language?

It is a bilingual one. You can write a whole program in Persian or in English, and the two are the same language underneath. Most of the code on GitHub is in English, and the Persian spelling is fully supported, down to the error messages.

### Is Salam compiled or interpreted?

Compiled. `salam build` produces a native executable. An interpreter, `salam exec`, also exists for quick experiments on machines without a C compiler.

### Is Salam free?

Yes. The compiler, the standard library and the tools are open source under the GPL-3.0 license and developed publicly on [GitHub](https://github.com/SalamLang/Salam).

### What can I build with Salam today?

Command-line tools, HTTP and gRPC servers, database applications, static and interactive web pages, desktop apps with a web interface, games, and freestanding programs that boot without an operating system. The [Building with Salam](/blog/#build) category covers each of these.
