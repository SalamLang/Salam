---
title: Salam in five to ten years: from an open source project to a global ecosystem, and what has to happen first
seo_title: The Future of Salam: A Grounded Programming Language Roadmap
description: Where could the Salam programming language be in five to ten years? A grounded look at today's state, how other languages grew, and the milestones that matter.
keywords: future of Salam, programming language roadmap, Salam 1.0, new programming language future, language adoption history, Persian programming language future, open source language growth, compatibility promise, Salam ecosystem, programming language sustainability
category: future
summary: A vision piece with its feet on the ground: where Salam stands in November 2026, what the history of Go, Rust, Python and Kotlin says about how languages grow, and the five milestones that would turn Salam into a durable ecosystem.
date: 2026-11-11
---

The future of Salam depends less on new features than on a few hard, slow milestones: a 1.0 release with a compatibility promise, a real third-party package ecosystem, a secure and shared governance model, and a home community large enough to sustain it. **Salam today is a pre-1.0, self-hosting, bilingual systems language with an unusually large standard library and a very small community.** This article is the last in the magazine's "future" category and, unlike most of the others, is openly speculative. It separates three things carefully: what exists now (checked against the repository), what the project's own design documents describe as planned, and what is this article's vision. Nothing here is an official roadmap.

## Where Salam stands in November 2026 {#today}

Any forecast has to start from an honest baseline. These are the facts the rest of the article builds on.

| Area             | State today                                                                            |
| ---------------- | -------------------------------------------------------------------------------------- |
| Version          | 0.6.3, tagged 11 November 2026; 33 tagged releases from v0.3.0 (August 2026) to v0.6.3 |
| History          | Git history from 2018; first tagged release v0.1 in November 2024                      |
| Compiler         | Self-hosting since the C implementation was retired in August 2026                     |
| Backends         | C, LLVM, and a native backend for x86-64, 32-bit x86, AArch64, 32-bit ARM and RISC-V   |
| Targets          | Linux, macOS, Windows, WebAssembly, JavaScript, Android, iOS, freestanding             |
| Standard library | More than 170 documented packages, every public name in English and Persian            |
| Learning         | A 47-lesson course in English and Persian, an online editor, an Android app            |
| Tools            | LSP, formatter, test runner, debugger launcher, memory checker, 11 editor plugins      |
| Performance      | Same range as C and Rust on the project's 39-program benchmark                         |
| Packages         | Git-based package manager with lockfile and checksums; registry index still empty      |
| People           | Most commits from the designer; a few dozen contributors in 2026                       |
| License          | GPL-3.0                                                                                |

Two things stand out. The technical surface is far larger than the age of the project would suggest: three code generators, a built-in linker, a page language for the web, desktop and native UI work, and machine learning packages. And the social surface is far smaller: one dominant maintainer, small chat communities, and no third-party packages in the registry. The next decade is mostly about closing that second gap.

## How languages have actually grown {#history}

The history of programming languages gives a few lessons that apply directly. These are well-documented cases, not analogies chosen to flatter.

- **Python** was released in 1991 and took well over a decade to become a default choice. Its growth came from a readable syntax, a "batteries included" standard library, and, much later, scientific computing and data science as a killer domain.
- **Ruby** existed for about a decade before Ruby on Rails, released in 2004, gave it one killer application that brought a wave of web developers.
- **Go** was announced in 2009 and released Go 1 in 2012 with an explicit compatibility promise: programs written for Go 1 would keep compiling. That promise, plus Google's backing and a strong fit for cloud tooling, made it safe for companies.
- **Rust** spent years in public development before 1.0 in 2015, and its adoption accelerated as large companies used it for security-sensitive systems code. It built a foundation and a formal governance structure along the way.
- **Kotlin** had its turning point in 2017, when Google announced official support for it on Android: a platform owner chose it.

The pattern is consistent: a stability promise, at least one domain where the language is the obvious choice, and institutions (a company, a foundation, a platform) that make it safe to depend on. Salam currently has none of the three in full. It does have something most of those languages lacked at the same age: a clearly underserved community, the tens of millions of people who read and write Persian, and no other full systems language that speaks their language.

## Milestone 1: version 1.0 and a compatibility promise {#one-point-zero}

The single most important step is a 1.0 release with a written compatibility rule. Today, minor and even patch releases can break code. The small program below applies the usual reading of semantic versioning, where anything can change before 1.0 when the minor number moves, and only a major version may break code after 1.0:

```salam
import str
import conv

struct Version:
    pub major: int
    pub minor: int
    pub patch: int
end

func parse version(text: str): Version:
    parts := str.Split(str.TrimPrefix(text, "v"), ".")
    defer parts.Free()
    ret Version {
        major = conv.AtoiOr(parts.get(0), 0),
        minor = conv.AtoiOr(parts.get(1), 0),
        patch = conv.AtoiOr(parts.get(2), 0),
    }
end

func breaking upgrade(from: Version, next: Version): bool:
    if from.major == 0:
        ret next.major > 0 or next.minor != from.minor
    end
    ret next.major != from.major
end

func main:
    olds := ["v0.6.2", "v0.5.9", "v1.4.0", "v1.9.2"]
    news := ["v0.6.3", "v0.6.0", "v1.9.2", "v2.0.0"]
    repeat olds.len() in i:
        a := parse version(olds[i])
        b := parse version(news[i])
        verdict := breaking upgrade(a, b) ? "may break code" : "should be compatible"
        println olds[i], "->", news[i], verdict
    end
end
```

```output
v0.6.2 -> v0.6.3 should be compatible
v0.5.9 -> v0.6.0 may break code
v1.4.0 -> v1.9.2 should be compatible
v1.9.2 -> v2.0.0 may break code
```

By that rule, moving from 0.6.2 to 0.6.3 should be safe. In reality 0.6.3 made self-assignments such as `x = x + 1` a compile error. That is a reasonable language change, and exactly the kind of surprise a 1.0 policy exists to rule out.

The repository already shows what would have to settle before such a promise. [docs/design/naming-standard.md](https://github.com/SalamLang/Salam/blob/main/docs/design/naming-standard.md) is a proposal to make the standard library's public names consistent, and it measures how many declarations would be renamed. [docs/design/OWNERSHIP.md](https://github.com/SalamLang/Salam/blob/main/docs/design/OWNERSHIP.md) describes compile-time memory checks without lifetime annotations: its first five phases are implemented, and its sixth would, after a release, turn ownership warnings into errors by default. Both are the kind of breaking change a language should finish before 1.0, not after. The tools for a gentle transition exist (`deprecated` functions with warning W013, the `salam = ">=X"` field in package manifests); the policy does not yet.

## Milestone 2: a real package ecosystem {#ecosystem}

Salam's standard library is its strongest asset for early adopters, and also a ceiling. A core team cannot maintain every database driver, cloud SDK and framework forever. The package manager is ready for the next stage: Git-based modules, a lockfile, checksums, a reviewed sum list and a registry index that `salam search` reads. What it lacks is packages. The [ecosystem article](/blog/building-an-open-source-language-ecosystem/) describes the mechanics.

A realistic five-year goal, in this article's view, is a registry with a few hundred maintained packages, several of them maintained by people outside the core team, and a handful of frameworks with their own users. The bilingual naming system gives Salam packages something unusual to offer: a library written once can be called by its English names from an English program and by its Persian names from a Persian one.

## Milestone 3: the Persian-speaking world as a home base {#home-base}

Every successful language has a home: Go had cloud infrastructure, Ruby had web startups, Kotlin had Android. Salam's most natural home is the Persian-speaking world, in two roles.

**Education.** A complete systems language with Persian keywords, Persian error messages and a free Persian course can be taught from the first lesson in the students' own language, and then bridged to English keywords with `salam translate`. The repository already holds slides for a Persian talk prepared for the DSTO event in Kashan in October 2026, which is how such adoption starts: one event, one teacher, one course at a time. The [education article](/blog/salam-in-programming-education/) looks at what classrooms would still need.

**Local software.** Businesses that work in Persian need the Jalali calendar, Persian digits, right-to-left text and Persian-language tooling. The standard library already treats those as first-class (`calendar`, `bidi`, `arabic`, `i18n`). A generation of developers who learned in Salam and build local software in it would give the language a durable base, much as regional communities sustained other languages before they spread.

That home base should not become a boundary. English Salam is the same language, the English site and course exist, and the compiler's design keeps both communities on one ecosystem. The [multilingual programming article](/blog/multilingual-programming-future/) explores whether the model could extend further.

## Milestone 4: a domain where Salam is the obvious choice {#killer-apps}

Breadth is not a strategy for adoption. A language spreads when, for one kind of project, it is clearly the best tool. Looking at what Salam already does well, three candidates stand out:

1. **One language for a web product.** The page language compiles to HTML, CSS and JavaScript, and `net.http` serves it from the same program, with an HTTP stack that measured in the same class as fast Go frameworks on the project's router benchmark. A small team could build an entire web product, server and pages, in one compiled language. See the [full-stack article](/blog/full-stack-development-with-salam/).
2. **Native apps from one description.** The same page language can already run in a native desktop window with `salam run page.salam --ui=native`, and the design document for it plans later phases. If that matures and reaches mobile, "write the UI once in the page language, ship web, desktop and mobile" would be a distinctive offer.
3. **Tools for the AI era.** Salam's strict compiler, stable error codes and machine-readable diagnostics make it a good target for code-writing assistants, and the project already runs its own assistant, agent and MCP server, with a backend written in Salam. The articles on [AI-assisted development](/blog/programming-languages-in-the-ai-era/) and [Salam AI](/blog/salam-ai-and-salam-agent/) cover this.

Choosing one of these and making it excellent would do more for adoption than adding a fourth backend.

## Milestone 5: governance, security and sustainability {#governance}

The least glamorous milestone decides whether organizations can bet on Salam at all. Today there is no `SECURITY.md` or private vulnerability reporting process, no governance document, no long-term-support release and no institution behind the project. The [production-readiness audit](/blog/what-makes-a-language-production-ready/) scores each of these.

What would change that, in rough order of cost:

- a security policy with a reporting address and a commitment to publish advisories;
- a written decision process and more people with merge and release rights, so the project survives any one person's absence;
- a supported release line beside the fast-moving one;
- a clear answer on licensing for proprietary use, since the GPL-3.0 standard library without a linking exception rules out many commercial products today;
- eventually, a legal home such as a foundation or fiscal sponsor that can hold the name, accept funding and pay for audits.

None of these is announced. They are what every language that reached enterprise use eventually needed.

## What success would look like, and the risks {#success}

As a closing exercise, here is what this article would count as success, stated as goals rather than predictions.

**In five years (2031):** a 1.0 release that has kept its compatibility promise for at least a couple of years; a registry with hundreds of packages and outside maintainers; Salam taught in real courses in Persian-speaking schools or universities, with published experience reports; a security policy and a governance document in place.

**In ten years (2036):** Salam as the natural first systems language for Persian speakers; a recognized niche internationally, most likely full-stack web products or teaching compilers; several companies willing to describe their use publicly; and a project that no longer depends on its founder.

The risks are just as concrete. The project could stay a one-person effort and stall. Breaking changes could exhaust early users before 1.0 arrives. The GPL licensing choice could keep commercial adopters away. Breadth could keep winning over depth, so that many features stay at 80 percent. And the community could stay too small for a package ecosystem to form. Every young language faces some of these; the ones that survive address them deliberately.

Salam has already done the hardest technical thing a language can do, compiling itself, and it has a clear reason to exist that no other systems language shares. Whether it becomes a global ecosystem in ten years is not a question its compiler can answer. It depends on the next few hundred contributors, teachers and package authors. The [first article of this magazine](/blog/what-is-salam-programming-language/) is the place to start, and the [free course](/learn/) is the place to begin writing code.

## Frequently asked questions {#faq}

### When will Salam 1.0 be released?

No date has been announced. The current line is 0.6, and the project's design documents still describe breaking changes (such as stricter ownership rules and standard library naming) that would normally land before a 1.0 compatibility promise.

### Is Salam backed by a company or foundation?

No. Salam is an open source project on GitHub, designed by Seyyed Ali Mohammadiyeh and developed with outside contributors, under the GPL-3.0. There is no company, foundation or commercial support behind it today.

### Will Salam stay focused on Persian?

The language is fully bilingual: English and Persian are equal spellings of the same language, with one compiler and one standard library. Persian speakers are its most natural home community, but the English site, course and tools are complete, and most code in the repository is written in English.

### How can I help Salam grow?

Write and publish a package, report bugs with small reproductions, translate or write tutorials, teach a class with it, or contribute to the compiler. The contributing guide on GitHub explains the workflow, and every one of these addresses a milestone in this article.
