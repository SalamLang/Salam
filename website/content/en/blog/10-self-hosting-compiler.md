---
title: When a language builds itself: inside Salam's self-hosting compiler
seo_title: Self-Hosting Compiler: How Salam Bootstraps Itself
description: What a self-hosting compiler is, how bootstrapping a compiler works, and how Salam replaced its C implementation in 2026 and now builds itself.
keywords: self-hosting compiler, bootstrapping a compiler, compiler bootstrapping, self-hosted compiler, Salam compiler, trusting trust, compiler written in its own language, seed compiler, three-stage bootstrap
category: compiler
summary: The Salam compiler is written in Salam. This is the story of how it got there, how a release is bootstrapped from the previous one, what self-hosting buys a young language and what it costs.
date: 2026-11-11
---

A **self-hosting compiler** is a compiler written in the language it compiles. The Salam compiler is one: its source is about 340 files of Salam in the `compiler/` directory, and every release since August 2026 was compiled by an earlier `salam` binary. Getting there required **bootstrapping a compiler**: an older implementation, written in C, compiled the first Salam version of the compiler, and that C implementation was deleted from the repository on 20 August 2026. This article explains how bootstrapping works in general, how Salam did it, how a release is built today, and why a young language would take on the trouble.

## What "self-hosting" means {#definition}

**A language is self-hosting when its own compiler (or interpreter) is written in that language and can compile itself.** C compilers such as GCC and Clang are written in C and C++. The Go compiler was rewritten from C to Go for Go 1.5 in 2015. Rust's first compiler was written in OCaml and was replaced by a compiler in Rust in 2011. Zig spent several years replacing its C++ compiler with one written in Zig. Being self-hosting is a milestone, not a requirement: plenty of good languages, Python's reference implementation among them, are implemented in another language.

For a compiled systems language, though, it is a strong statement. A compiler uses nearly every feature a language has: structs and enums for the syntax tree, hash maps for symbol tables, strings and Unicode for source text, files and processes for the build, careful memory handling everywhere, and a lot of code. If the language cannot carry its own compiler, it is probably not ready for other large programs either.

## The chicken-and-egg problem of bootstrapping {#bootstrapping}

There is an obvious puzzle. To compile a compiler written in Salam, you need a Salam compiler. Where does the first one come from?

The general answer, called **bootstrapping**, has three steps:

1. Write a first compiler in an existing language. The one that bootstrapped today's Salam compiler was written in C.
2. Write a second compiler, for the same language, in the new language itself. Compile it with the first one. You now have a working compiler that was produced from Salam source.
3. Use that compiler to compile its own source again. From here on, the old implementation is no longer needed, because each new version can be compiled by the previous one.

The compiler that starts a build is called the **seed**. After step 3, the seed for every build is simply an earlier release. The cost is that you can no longer build the compiler from source alone: you always need some working binary of the language first. Salam's build script says this plainly when there is no seed on the machine:

```text
error: no seed compiler on PATH.
  The compiler is written in Salam, so building it needs a Salam.
  Install a released one:  sh install.sh
```

## Salam's path from C to Salam {#history}

The repository's history goes back to June 2018 and includes several earlier implementations. Until the summer of 2026 the compiler was a C program, in its last form under `c/src/`: lexer, parser, semantic analysis, a C code generator, an LLVM backend, an interpreter, a JavaScript backend and the tooling around them. The port to Salam happened over the summer of 2026, and `git log` dates the milestones:

| Date            | Milestone                                                                                            |
| --------------- | ---------------------------------------------------------------------------------------------------- |
| 27 July 2026    | The compiler written in Salam is added to the repository (PR #1461)                                  |
| 1 August 2026   | A release cut with both the C and the Salam compilers in the tree ("First self-host with C version") |
| 20 August 2026  | The C implementation is removed: 220 C files, about 62,900 lines (PR #1546)                          |
| 22 October 2026 | Compile-time use-after-free and double-free checks land in the self-hosted compiler (PR #2282)       |
| 25 October 2026 | The native backend and built-in linker, written in Salam, are merged (PR #2363)                      |
| 3 November 2026 | Compile-time range, overflow and division checks (PR #2604)                                          |

At the moment of the switch, the Salam version of the compiler was 164 files and about 64,900 lines, roughly the same size as the C program it replaced. Since then it has tripled, to close to 200,000 lines, and the largest additions (the native code generator, the linker, the ownership and range analyses) were written in Salam from the start. They never existed in C.

The Salam compiler mirrors the structure of the C one: the same pipeline of source, lexer, tokens, parser, AST, semantic analysis and code generation. The porting guide in the repository describes the method: port module by module in dependency order, keep the C build runnable, and check each stage against it. The deleted C sources are still in Git history, where they remain useful as a design reference.

## How the port was checked {#verification}

A rewrite of a compiler is only useful if the new one compiles programs the same way as the old one. The checks used here are worth knowing, because they apply to any port:

- **Compare every stage, not only the final output.** Both compilers can print their tokens, syntax tree and symbol table as XML (`salam inspect --emit-tokens`, `--emit-ast`, `--emit-symbol`), and the porting guide in the repository (`SKILL.md`) recommends validating each ported stage against that output from the reference compiler.
- **Sweep the whole test suite with both.** `docs/LLVM_DEFAULT_BACKEND.md` records one such sweep from that period: the same 802 test files run through the LLVM backend of the C compiler and of the self-hosted one gave 675 successes, 0 code generation failures and 127 expected-error tests for both, with zero differing files.
- **Run real programs and compare their output byte for byte.** The 73 programs in `tests/en/llvm/` are built, run and compared with expected output files, and both compilers passed all 73.
- **Keep a parity gate.** `compiler/tools/bash/selfhost-parity.sh` runs the same test suites with a reference compiler and with the self-hosted one, and fails when the self-hosted one fails more tests than the reference, beyond a small per-suite budget written down in the repository.

Today the `tests/` directory holds about 4,300 English and 2,700 Persian Salam files, and changes to the compiler run against them.

## How a Salam release is built today {#stages}

Two scripts in the repository do the work.

`tools/bash/build-selfhost.sh` is the one-stage build: take a seed (the `salam` on your `PATH`, or the newest published release in CI) and compile `compiler/main.salam` with it. When the result should carry LLVM, lld and the cross-compilation sysroots inside it, the script first builds a plain compiler with the seed and then uses that compiler to build the final one, so the final binary is compiled by a compiler made from the current source.

`compiler/tools/bash/bootstrap.sh` is the classic **three-stage bootstrap**:

```bash
sh compiler/tools/bash/bootstrap.sh "$(command -v salam)" build/selfhost
```

1. **Stage 1:** the seed compiles the current source. The result is new code compiled by an old compiler.
2. **Stage 2:** stage 1 compiles the same source. Now the new code was compiled by the new compiler.
3. **Stage 3:** stage 2 compiles the source once more.

Then comes the **fixpoint check**: stage 2 and stage 3 were built by the same compiler logic from the same source, so they should be byte-identical. When they are, the script prints `identical - bootstrap has reached a fixpoint.` When they are not, it prints a warning rather than failing, because link order and link-time optimization do not have to be reproducible, but a persistent difference on one machine is a signal worth investigating.

The native backend adds its own self-test of the same kind: emit the compiler's LLVM IR with `salam llvm compiler/main.salam`, build that IR with the native backend, and have the resulting compiler emit the IR again. The two files must be identical apart from the build timestamp.

Here, for flavor, is the kind of code the compiler is made of: a tiny tokenizer that cuts an arithmetic expression into tokens, the same job as the first stage of the real lexer, written in ordinary Salam.

```salam
enum Kind: Number, Operator, Space, Unknown end

func kind of(ch: str): Kind:
    if "0123456789".Includes(ch):
        ret Kind.Number
    end
    if "+-*/".Includes(ch):
        ret Kind.Operator
    end
    if ch == " ":
        ret Kind.Space
    end
    ret Kind.Unknown
end

func main:
    source := "12 + 3 * 45"
    mut digits := ""
    mut tokens := Vector {} as Vector<str>
    defer tokens.Free()
    each ch in source + " ":
        k := kind of(ch)
        if k == Kind.Number:
            digits += ch
            continue
        end
        if digits != "":
            tokens.Push("NUMBER " + digits)
            digits = ""
        end
        if k == Kind.Operator:
            tokens.Push("OPERATOR " + ch)
        end
    end
    each t in tokens:
        println t
    end
end
```

```output
NUMBER 12
OPERATOR +
NUMBER 3
OPERATOR *
NUMBER 45
```

The real lexer is the same idea with many more cases: Persian digits, multi-word names, raw strings, text blocks and the keywords of two languages. The [compiler pipeline article](/blog/how-the-salam-compiler-works/) shows what happens to the tokens next.

## Why self-hosting matters for a young language {#why}

The arguments for self-hosting are practical, not symbolic.

**The compiler is the biggest test program.** At close to 200,000 lines, the compiler is by far the largest Salam program in existence, and it exercises the [memory management](/blog/memory-management-in-salam/) and [static checks](/blog/static-typing-in-salam/) of the language harder than anything else. Every slow path, memory leak and missing feature in the language shows up there first, and the people who can fix it are the ones who hit it. Several features are shaped by that pressure: bitwise operators, which compiler code needs everywhere; text blocks, designed for emitting generated C and LLVM IR; and the ownership checker, whose stricter warnings only became the default once they reported nothing on `compiler/`, `std/` and the test programs.

**Optimizations pay twice.** When the code generator gets better, the compiler that ships with it gets faster too. When the ownership work started freeing reassigned strings, the peak memory of compiling the compiler itself to C dropped from 1,037 MB to 854 MB, as recorded in `docs/design/OWNERSHIP.md`.

**Contributors need one language.** A Salam programmer can read and change the Salam compiler without knowing C. For a language that also wants to be a teaching tool, an open compiler written in the language being taught is a strong asset: a student who finishes the [course](/learn/) can read the compiler that runs their programs.

**Backends test each other.** The compiler can be built through the C backend, the LLVM backend and the native backend. A bug in one code generator usually shows up as a compiler built with it that behaves differently from one built with another. The article [Three ways to generate code](/blog/salam-code-generation-c-llvm-native/) describes the three.

## Trusting trust: can you trust a compiler that compiled itself? {#trusting-trust}

In his 1984 Turing Award lecture, "Reflections on Trusting Trust", Ken Thompson described a compiler that inserts a back door into the programs it compiles and also reinserts that behavior into any new version of itself, so that the malicious code disappears from the source but survives in every binary. It is the classic argument that reading a self-hosting compiler's source does not, by itself, prove anything about the binary.

What Salam offers on this front, stated honestly:

- **The whole chain is public.** The compiler, the build scripts and the CI workflows that produce releases are in the [GitHub repository](https://github.com/SalamLang/Salam), and the C implementation the chain started from is in its history.
- **Fixpoint checks.** A three-stage bootstrap that reaches a fixpoint shows that the compiler compiles itself consistently. It does not show that the seed was clean.
- **Independent code generators.** Building the compiler through C (with gcc or clang as the C compiler), through LLVM and through the native backend gives binaries produced by unrelated machine-code generators. That makes a hidden payload in one of them harder to keep consistent, but they all share the same Salam frontend and the same seed.

What has not been done, as far as the repository shows, is a formal **diverse double-compiling** check, the technique David A. Wheeler proposed for answering Thompson's attack, which rebuilds a compiler with an independent trusted compiler and compares the results bit for bit. Salam's multiple backends make that kind of experiment possible; it has not been published.

## The costs of self-hosting {#costs}

Self-hosting is not free, and the costs are visible in Salam's repository:

- **New syntax waits a release.** The compiler and the standard library are compiled by the previous release in CI, so they can only use syntax that release already understands. A feature lands in the language first and in the compiler's own source later. The project's design notes record what happens otherwise: the first bootstrap stage fails with a misleading semantic error.
- **Old seeds stop working.** `bootstrap.sh` carries hints for seeds that are too old, such as releases before 0.2.8 that cannot collapse `..` in import paths. `build-selfhost.sh` requires 0.3.1 or newer.
- **The compiler needs real memory to build.** A compiler from the current tree needs about 1 GB to analyze its own source. Seeds from before an incremental analysis pass needed about 6 GB, more than a 32-bit process can address, so on 32-bit hosts the seed stage now runs with the ownership checks turned off.
- **Bugs can hide in the seed.** A compiler bug that also affects how the compiler compiles itself can be hard to see, because the broken compiler built the binary you are testing with. Comparing stages and backends is the defense.

For Salam these costs are worth paying. The language is pre-1.0 and still changes between releases, so having its most demanding program written in it keeps the design honest.

## Frequently asked questions {#faq}

### What is a self-hosting compiler?

It is a compiler written in the same language it compiles, which can compile its own source code. GCC, Clang, the Go compiler, rustc and the Salam compiler are examples.

### How do you bootstrap a compiler?

Write a first compiler in an existing language, use it to compile a second compiler written in the new language, then let that compiler compile itself. After that, each release is built by an earlier one, the seed. The compiler that bootstrapped today's Salam compiler was written in C.

### Can I build Salam from source without a Salam binary?

Not from the current tree: you need a seed. Install a released compiler from the [install page](/install/) and run `sh tools/bash/build-selfhost.sh`. In principle you could start from the C implementation in Git history and climb through the releases, but that is a long road and the repository does not document it.

### Is the Salam compiler written in Persian or English Salam?

English. The compiler handles both spellings, and the standard library and the compiler's messages exist in both, but the compiler's own source code uses the English keywords. The article [One language, two scripts](/blog/one-language-two-scripts/) explains how the two relate.
