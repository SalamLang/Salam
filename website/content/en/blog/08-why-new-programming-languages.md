---
title: Why does the world still need new programming languages? Salam's design philosophy and goals
seo_title: Why New Programming Languages? Salam's Design Philosophy
description: Why new programming languages keep appearing, what Go, Rust, Swift, Kotlin and Zig each fixed, and the specific gap Salam's design philosophy and goals aim at.
keywords: new programming languages, programming language design philosophy, why create a new programming language, Salam design goals, Go Rust Swift Kotlin Zig history, systems programming language, bilingual programming language, language design trade-offs, modern programming language
category: language
summary: Every successful language of the last twenty years was built to fix a specific pain. This article looks at what drove Go, Rust, Swift, Kotlin and Zig, then at the gap Salam targets and the design decisions that follow from it.
date: 2026-11-11
---

New programming languages keep appearing because the problems programmers face keep changing: hardware gains more cores, software moves to the network and the browser, security failures become expensive, and the people who program are no longer mostly English-speaking engineers in a few countries. **A new language is worth creating when it removes a cost that existing languages cannot remove without breaking their own users, and every widely adopted language of the last two decades started that way: as a fix for one specific pain.** Salam is a new language built around two such costs: the friction of using several languages for one product, and the requirement to work in English before you can program at all.

This article looks at what drove the best-known recent languages, what each fixed and what each gave up, and then describes Salam's design philosophy and the gap it aims at, with an honest account of what it costs.

## Why existing languages cannot simply absorb new ideas {#why}

A mature language has millions of lines of code depending on its current behavior. It can add features, but it can rarely take any away or change a default. C cannot become memory-safe by default without breaking C programs. Java cannot drop its virtual machine. Python cannot make its variables immutable or its types mandatory. English keywords cannot be made optional in a language whose entire ecosystem, documentation and community assume them.

That is why the big shifts have come through new languages rather than through new versions of old ones. A new language gets one chance to choose its defaults, and the defaults are most of what a language is.

## What recent languages were created to fix {#recent-history}

Five languages from the last twenty years illustrate the pattern, each with a clear origin:

| Language   | Origin                                                                | The pain it targeted                                                       | What it gave up                                                          |
| ---------- | --------------------------------------------------------------------- | -------------------------------------------------------------------------- | ------------------------------------------------------------------------ |
| **Go**     | Google, begun 2007, public in 2009, 1.0 in 2012                       | Slow C++ builds, complex large codebases, concurrency on multicore servers | Expressiveness: no generics until 1.18 (2022), a garbage collector       |
| **Rust**   | Started by Graydon Hoare, sponsored by Mozilla, 1.0 in 2015           | Memory-safety bugs in C and C++ systems code                               | Simplicity: the ownership and borrowing model takes real effort to learn |
| **Swift**  | Apple, 2014                                                           | Objective-C's age and unsafety for Apple platforms                         | Portability, at first: its ecosystem is still centered on Apple          |
| **Kotlin** | JetBrains, 1.0 in 2016, Google's preferred Android language from 2019 | Java's verbosity and null-pointer errors, without leaving the JVM          | Independence from the JVM's model for most uses                          |
| **Zig**    | Andrew Kelley, 2016                                                   | C's hidden control flow, preprocessor and cross-compilation pain           | A stable language: it has changed significantly between releases         |

The common thread is not novelty for its own sake. Each language picked a cost its users paid every day and designed around removing it, accepting a new cost in return. Go traded expressiveness for simplicity and fast builds. Rust traded ease of learning for safety without a garbage collector. Kotlin traded a clean start for full compatibility with Java code.

## The costs Salam targets {#costs-targeted}

Salam's design reflects two costs that the languages above do not address.

**The first is fragmentation.** A typical web product today is written in several languages: a systems language or Go for the backend, TypeScript for the browser, HTML and CSS for the page, SQL for the database, a shell or YAML for the build. Each comes with its own toolchain, type system and package manager. Salam's answer is one language that reaches from bare-metal code to the web page: it compiles to native code for servers and command-line tools, it has a built-in page language that generates HTML, CSS and JavaScript, it can compile to WebAssembly and JavaScript, and its standard library includes HTTP, database drivers and a checker for SQL strings.

**The second is the English prerequisite.** Every mainstream language assumes its users read English: keywords, library names, error messages, documentation. Salam makes English optional. Every keyword, type, standard library package and compiler message has a Persian spelling, and both spellings are the same language, so a learner can start in Persian and move to English without changing languages. The article [one language, two scripts](/blog/one-language-two-scripts/) explains the mechanism.

The project's readme states the broader aim: to balance low-level control with developer productivity and to lower the cognitive barrier to entry. The rest of the design follows from these goals.

## Design principles that follow from the goals {#principles}

**Native code, no virtual machine, no garbage collector.** If one language is to cover systems programming, servers and tools, it has to produce small native executables with predictable performance. Salam compiles ahead of time through C, LLVM or its own x86-64 backend. On the project's benchmark of 39 programs that use the same algorithm in every language, Salam averaged 10.6 ms, C 11.7 ms and Rust 11.3 ms, which puts it in the same range as C and Rust. Those numbers come with caveats: the programs are short, so start-up time counts; differences under about 5% are noise; the runs are on a CI virtual machine; and Salam was behind the fastest compiled language on 6 of the 39. The [speed page](/speed/) has every number.

**A strict compiler as the safety net.** Without a garbage collector or a borrow checker, the compiler has to catch what it can. Salam rejects unused variables and functions, reassignment of immutable values, conditions that are not booleans, non-exhaustive `match` expressions and a range of out-of-bounds indices it can prove at compile time, with error codes and fixes that `salam check --fix` can apply.

**Batteries included.** A language that wants to replace several tools needs a large standard library from day one. Salam's includes networking, TLS, databases, compression, cryptography, images and numerical computing, and every package has a Persian name.

**Readable, one-way syntax.** Blocks open with `:` and close with `end`; variables are immutable unless declared `mut`; logic uses `and`, `or` and `not`. Where two spellings would do the same thing, Salam usually allows one. The [syntax design article](/blog/salam-syntax-design/) goes through each choice.

**Self-hosting.** The Salam compiler is written in Salam. It is one of the largest Salam programs and the strongest test of the language, since a compiler exercises nearly every feature.

## What that looks like in code {#in-code}

A short program shows several of these principles together: static types inferred from values, explicit memory release with `defer` instead of a garbage collector, and a standard library call.

```salam
import str

func main:
    text := "the quick fox and the lazy dog and the cat"
    words := str.Split(text, " ")
    defer words.free()
    mut counts := HashMap {} as HashMap<str, int>
    defer counts.free()
    each w in words:
        before := counts.has(w) ? counts.get(w) : 0
        counts.put(w, before + 1)
    end
    each w in ["the", "and", "fox", "owl"]:
        println w, counts.has(w) ? counts.get(w) : 0
    end
end
```

```output
the 3
and 2
fox 1
owl 0
```

And the fragmentation goal: a web server whose HTML page is written in the same language, in the same file, type-checked by the same compiler. This program is checked here but not run, because it listens on a port:

```salam
import net.http
import net.router

page Status(service: str, healthy: bool):
    title = service + " status"
    heading: size = 1 content = service end
    paragraph: content = healthy ? "All systems running." : "Degraded." end
end

func status(ctx: i64):
    http.Html(ctx, Status("Billing", true))
end

func main:
    mut r := router.NewRouter()
    router.Get(r, "/status", status)
    mut s := http.NewServer(8080)
    http.Use(s, r)
    http.Run(s)
end
```

The `page` block is Salam's layout language: elements and styles written as Salam code, turned into HTML by the compiler. The [layout lesson](/learn/layout/) introduces it, and the [web server lesson](/learn/serve/) covers deployment.

## The gap, stated plainly {#gap}

Put together, Salam aims at this combination:

- a **compiled, statically typed systems language** in the performance range of C and Rust;
- that is **simpler to read and write** than C, C++ or Rust;
- that covers **the web page as well as the server** without a second language;
- and that can be **written in Persian as well as English**, with error messages in both.

Each item exists somewhere else. Go is simpler than C++ but has a garbage collector and no page language. Rust is safer than Salam but harder to learn. TypeScript covers the browser and the server but runs on a JavaScript engine. Educational languages offer native-language keywords but are not systems languages. The combination is what is new.

## What a new language costs {#costs}

Honesty matters here, because the costs of a new language are real and fall on its early users:

- **No borrow checker.** Salam does not prove memory safety the way Rust does. Heap collections are freed by hand, with `defer` and an ownership checker to help, and mistakes are possible.
- **A small ecosystem.** The standard library is large, but third-party packages, Stack Overflow answers, books and experienced developers are few compared with established languages.
- **Pre-1.0 change.** The language still changes between releases; syntax that worked last year may not compile today.
- **Few production deployments.** There are fewer battle-tested answers for unusual problems.
- **Two spellings to maintain.** Every new feature needs a Persian spelling, a Persian error message and tests in both languages.

None of these is unusual for a young language. Go, Rust and Kotlin all went through the same stage. Whether a language gets past it depends on whether enough people find its particular trade-off worth the cost.

## How to judge a new language {#how-to-judge}

If you are deciding whether a new language deserves your time, a few questions cut through marketing, including ours:

1. **What specific cost does it remove?** If the answer is "it is nicer", be skeptical.
2. **What did it give up?** Every design has a price; a language that claims none is hiding it.
3. **Can you verify its claims?** Benchmarks should publish their code and raw data; features should have runnable examples. Salam's [course](/learn/) and [library reference](/std/) run every example when the site is built.
4. **Can it interoperate?** A new language that cannot call C or speak common protocols forces a rewrite. Salam calls C directly through `extern` blocks.
5. **Is it developed in the open?** Salam is GPL-3.0 and developed on [GitHub](https://github.com/SalamLang/Salam).

For a full introduction, start with [what is the Salam programming language](/blog/what-is-salam-programming-language/), then [install it](/install/) or try it in the [online editor](https://editor.salamlang.ir).

## Frequently asked questions {#faq}

### Why are new programming languages still being created?

Because mature languages cannot change their defaults without breaking existing code, and new problems (multicore hardware, memory-safety costs, the web, a global and multilingual population of programmers) call for different defaults. A new language is the only place to choose them freely.

### What problem does Salam solve?

Two main ones: using several languages and toolchains for one product, and the need to know English before you can program. Salam is a native, statically typed language with a built-in page language for the web, and it can be written in Persian or English.

### Is Salam trying to replace C, Rust or Go?

No language replaces those in the short term. Salam targets the same native performance range with simpler syntax and a broader scope, but it does not have Rust's memory-safety guarantees or Go's ecosystem and production record. It is an alternative for projects where its trade-off fits.

### How long does it take for a new language to become mainstream?

Usually years. Go took about three years from its announcement to 1.0, Rust about nine from its start to 1.0, and Kotlin about five from its unveiling to 1.0 and three more before Google made it the preferred language for Android. Salam is still before 1.0.
