---
title: What makes a programming language ready for production? Salam against an enterprise adoption checklist
seo_title: Is Salam Production Ready? An Enterprise Adoption Checklist
description: A fair audit of Salam against a production-readiness checklist: stability, versioning, security, tooling, support and GPL-3.0 licensing, gaps included.
keywords: production ready, enterprise adoption, language maturity, programming language evaluation checklist, is Salam production ready, language stability policy, software supply chain, GPL-3.0 compiler license, security disclosure policy, release process
category: future
summary: Production readiness is not one feature but a dozen boring guarantees. This article lists them, then scores Salam honestly on each one, with the evidence from the repository and the gaps that remain.
date: 2026-11-11
---

A programming language is ready for production when a business can depend on it for years without nasty surprises: code keeps compiling across upgrades, releases are trustworthy, security problems have a reporting path, tools and platforms are covered, people can be hired or trained, and the license fits the product. **Production readiness is a property of the whole project around a language, not of its compiler alone.** This article sets out a checklist that enterprise teams commonly use, in one form or another, and audits Salam against each item using what the repository actually contains. The verdict is mixed: Salam is strong on engineering and weak on the institutional guarantees that come with age.

## The checklist {#checklist}

There is no single official standard for "production ready", but evaluation checklists used by platform teams converge on the same questions. Here they are with Salam's status as of November 2026. "Met" means the evidence is there; "partly" means some of it is; "not yet" means it is missing.

| Item                                        | Salam's status  | Evidence in short                                              |
| ------------------------------------------- | --------------- | -------------------------------------------------------------- |
| Language stability and compatibility policy | Not yet         | Version 0.6, syntax still changes, no written promise          |
| Versioning and release process              | Partly          | Frequent tagged releases, checksums, release gate; no LTS      |
| Platform support                            | Partly          | Many targets; 32-bit and cross builds are best-effort          |
| Developer tooling                           | Mostly met      | LSP, formatter, tests, debugger, memcheck; no program profiler |
| Quality assurance of the compiler           | Met for its age | Self-hosting, thousands of tests, differential fuzzing         |
| Security process                            | Not yet         | No `SECURITY.md`, no audit, no advisory history                |
| Memory safety                               | Partly          | Bounds checks and warnings; no borrow checker                  |
| Performance evidence                        | Met             | Published benchmark with raw data and caveats                  |
| Ecosystem and libraries                     | Partly          | Large standard library; third-party registry empty             |
| Documentation                               | Mostly met      | Course, library reference, design docs; books out of date      |
| Governance and continuity                   | Not yet         | No governance document; work concentrated in one person        |
| Support                                     | Not yet         | Community chat only; no commercial support                     |
| Licensing                                   | Condition       | GPL-3.0 throughout, no runtime exception                       |

The sections below give the evidence for each row.

## Stability and compatibility {#stability}

This is the item that matters most and the one Salam fails most clearly today. The current release line is 0.6, and the language still changes between releases in ways that break code. Typed local declarations (`x: int = 5`) were removed in 2026 in favor of `x := 5`. The `pattern => expr` form of match arms was removed in 0.5.0, along with the `=>` token. In 0.6.3, self-assignments such as `x = x + 1` became compile error E162 (write `x++` to add one and `x += n` for other amounts). Each change has a reason and a clear error message, but a team with 100,000 lines of Salam would have to touch code on upgrade.

There is no written compatibility policy, no long-term-support branch and no announced 1.0 date. What exists are good building blocks for one. A function can be marked `deprecated`, and every caller gets warning W013 while the old name keeps working:

```salam
pub func Total(prices: int[]): int:
    mut sum := 0
    each p in prices:
        sum += p
    end
    ret sum
end

pub deprecated func SumPrices(prices: int[]): int:
    ret Total(prices)
end

func main:
    println Total([120, 80, 45])
    println SumPrices([10, 20])
end
```

```output
245
30
```

The compiler prints the warning while the program still builds and runs:

```text
warning[W013]: call to deprecated function 'SumPrices'
 --> dep.salam:15:13
   |
15 |     println SumPrices([10, 20])
   |             ^
```

Packages can also declare the oldest compiler they support in `salam.toml` (`salam = ">=0.6.1"`), and the package commands stop with a clear message on an older one. A policy that combined these tools ("deprecate for at least N releases before removal") would be the single most valuable step toward production use.

## Versioning and the release process {#releases}

The release machinery is more mature than the version number suggests. From v0.3.0 on 15 August 2026 to v0.6.3 on 11 November 2026 the project tagged 33 releases, plus nightly builds triggered by a daily schedule. The release workflow:

- refuses to reuse a version number that already has a published release, because release tags are treated as immutable;
- can run the full test suite as a release gate, with live MySQL, PostgreSQL, Redis, Elasticsearch and OpenSearch instances for the database packages;
- publishes a `.sha256` file beside every asset and a `SHA256SUMS` file covering all of them, and verifies the list before upload;
- creates a signed, annotated Git tag when a signing key is configured;
- supports reproducible builds of the compiler through `SOURCE_DATE_EPOCH`, as documented in the packaging guide.

Release notes are generated from merged pull requests; there is no hand-written `CHANGELOG` file at the repository root. The weakness here is cadence, not rigor: 33 releases in under three months is fast iteration, and enterprises usually want a slower, supported line beside it. There is no long-term-support release, and there is no software bill of materials (SBOM) published with releases.

## Platforms, tooling and quality assurance {#platforms-tooling}

**Platforms.** Release builds cover Linux on x86-64, ARM64, ARMv7 and 32-bit x86, macOS on Apple silicon, Windows on x64, x86 and ARM64, plus WebAssembly, JavaScript and Android. The installers also accept the BSDs and other architectures. The honest qualifier is in [docs/PLATFORMS.md](https://github.com/SalamLang/Salam/blob/main/docs/PLATFORMS.md) and the release workflow: the 32-bit and cross-compiled jobs are marked `continue-on-error`, so a release can ship without them. There is no formal support-tier policy saying which targets are guaranteed. The [cross-compilation article](/blog/cross-compilation-salam/) has the details.

**Tooling.** The single `salam` command includes a language server, a formatter, a test runner, a documentation generator, a debugger launcher for GDB and LLDB, an AddressSanitizer-based memory checker, a REPL and plugins for eleven editors. One gap: [docs/PROFILING.md](https://github.com/SalamLang/Salam/blob/main/docs/PROFILING.md) describes a program profiler (`salam prof`) as designed but not implemented, so profiling a Salam program means using general native tools. The [developer tools article](/blog/salam-developer-tools/) covers the toolchain.

**Quality assurance.** Here Salam does well for a young language. The compiler is self-hosting, so every release compiles the compiler, which exercises most of the language. The `tests/` tree holds about 7,000 Salam files, test programs and their helpers, in English and Persian. A differential fuzzer generates random programs and compares the native backend's output with the C or LLVM backend's, and a CI job builds the compiler with AddressSanitizer and checks it against a leak budget that is meant only to go down. CodeQL scanning runs on pull requests.

## Security and memory safety {#security}

**Process.** This is a clear gap. The repository has no `SECURITY.md` or other published process for reporting vulnerabilities privately, no history of security advisories, and no independent audit of the compiler or of security-sensitive packages. The project has done some review work in the open: a written security review of its VPN and proxy packages, which states itself that it is "one reviewer reading the code and fuzzing the parsers; it is not an audit". The TLS 1.3 and 1.2 implementation in the standard library needs no OpenSSL, which removes a dependency but also means its correctness rests on this project's own review.

**Memory safety.** Salam has no garbage collector and no borrow checker. Array and vector accesses are bounds-checked in normal builds, the compiler warns about ownership mistakes, and `salam memcheck` runs a program under AddressSanitizer. Two caveats belong in any risk assessment: freeing heap memory is the programmer's job, and a `--release` build strips bounds checks for speed. For code that parses hostile input, Rust's compile-time guarantees are stronger; the [Salam vs Rust article](/blog/salam-vs-rust/) compares the two fairly.

**Error handling** is explicit, which helps auditing. Functions that can fail are marked `throws`, and callers must handle or propagate the error:

```salam
import conv

func parse port(text: str): int throws:
    port := try conv.Atoi(text)
    if port < 1 or port > 65535:
        throw Error { message = "port out of range: " + text }
    end
    ret port
end

func main:
    each text in ["8080", "http", "70000"]:
        try:
            println "listening on", parse port(text)
        catch e:
            println "config error:", e.to_str()
        end
    end
end
```

```output
listening on 8080
config error: conv: invalid character 'h' (line 1, column 1)
config error: port out of range: 70000
```

## Performance evidence {#performance}

Salam publishes the kind of performance evidence an evaluation team should demand: the programs, the method, every number and the raw timing of every run. On 39 programs that use the same algorithm in every language, measured on 27 October 2026, Salam averaged 10.6 ms, C 11.7 ms and Rust 11.3 ms; against the fastest of C, C++, Rust and Go on each program, Salam was ahead on 6, level on 27 and behind on 6. The [speed page](/speed/) is explicit about the limits: short programs where start-up time counts, a shared CI virtual machine, and gaps under about 5% that flip between runs. The fair summary is "the same range as C and Rust" for this kind of code. The benchmark does not cover large applications, memory-heavy workloads or build times, so a team should still measure its own service.

## Ecosystem, documentation, people and support {#ecosystem-people}

**Ecosystem.** The standard library is large, more than 170 documented packages from HTTP and gRPC to database drivers, compression and cryptography, which covers most needs without third-party code. The package manager is well designed (Git-based modules, lockfile, checksums, a reviewed sum list). But the registry index lists no third-party packages yet, and the checksum list is empty. The [ecosystem article](/blog/building-an-open-source-language-ecosystem/) discusses this.

**Documentation.** A 47-lesson course in English and Persian, a reference page per standard library package with compiled examples, and design documents in `docs/`. Some documents lag the code: the LaTeX books are marked out of date by the project itself, and some design notes still refer to the C implementation that was retired in August 2026.

**Governance and continuity.** There is a code of conduct and a CODEOWNERS file assigning review to a maintainers team, but no governance document describing how decisions are made or how maintainership passes on. In 2026 about two thirds of the commits on the main branch came from the language's designer. For a buyer, that concentration is the most important non-technical risk.

**Support.** Help comes from community channels (GitHub issues, Telegram, Discord, Eitaa and Bale) and the [Salam AI](/ai/) assistant. There is no commercial support, no service-level agreement and no vendor behind the project.

## Licensing: GPL-3.0 without a runtime exception {#licensing}

The compiler, the standard library and the tools are licensed under the GNU GPL version 3. The Visual Studio Code extension directory is MIT, so its grammar can be reused. The important detail for businesses is that there is no runtime-library exception, of the kind the GCC Runtime Library Exception provides for code linked against GCC's own libraries. The readme describes Salam code that imports the standard library as GPL-derived when built.

What that means in practice depends on distribution. Internal tools and software you only run as a network service are treated differently by GPL-3.0, which, unlike the AGPL, does not count network use as distribution. Distributing binaries to customers brings the GPL's source-availability obligations. That is a legitimate choice for a free software project and a deciding factor for many proprietary products. Ask a lawyer before relying on any reading of it, including this one.

## The verdict, and what would change it {#verdict}

Measured against this checklist, Salam today is suitable for experiments, internal tools, education and contained services where the team accepts upgrade work, and not yet suitable as the foundation of a long-lived proprietary product. The engineering items (QA, performance evidence, tooling, release integrity) are ahead of what its age would predict. The institutional items (compatibility promise, security process, governance, support, licensing clarity for proprietary use) are where the gaps are.

The good news for anyone following the project is that most of those gaps are policies, not code: a `SECURITY.md` with a disclosure address, a written deprecation and compatibility policy, a governance document, a supported release line, and a clear statement about linking exceptions. The [industry article](/blog/salam-in-industry/) describes how to pilot Salam safely in the meantime, and [Salam in five to ten years](/blog/future-of-salam/) looks at the longer path to 1.0.

## Frequently asked questions {#faq}

### Is Salam production ready?

Not for most enterprise uses yet. Its compiler quality, tooling and performance evidence are strong for a young language, but it is pre-1.0 with changing syntax, has no published security process, no long-term support and is GPL-3.0 without a runtime exception.

### What does production ready mean for a programming language?

It means a business can rely on the language for years: stable syntax and libraries across upgrades, trustworthy releases, a security response process, good tools, platform coverage, available developers and support, and a license that fits the product.

### Does Salam have a stable release?

It has tagged, checksummed releases, the latest in the 0.6 line, and nightly builds. None of them comes with a compatibility guarantee or long-term support, and breaking changes still happen between minor versions.

### Can I ship closed-source software written in Salam?

The standard library is GPL-3.0 with no linking exception, and the project treats programs that use it as GPL-derived. Distributing closed-source binaries would conflict with that, so take the question to legal counsel before you start.
