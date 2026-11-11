---
title: The future of programming languages in the age of AI: what makes a language ready for AI-assisted development
seo_title: Programming Languages in the AI Era: Ready for AI-Assisted Coding
description: What makes a programming language work well with AI coding tools: strict compilers, machine-readable errors, one current reference, llms.txt and MCP.
keywords: AI programming, AI-assisted development, AI coding tools, programming languages and AI, LLM code generation, compiler feedback, llms.txt, MCP server, Model Context Protocol, language design for AI
category: future
summary: Code generators are good at languages they have seen millions of times and weak at everything else. A young language cannot change its training data, but it can change what happens after the first draft. What Salam does about it, and what any language designer can learn from it.
date: 2026-11-11
---

A programming language is ready for AI-assisted development when a code generator can find out, quickly and precisely, whether what it wrote is correct, and why not. **Training data favors old, popular languages, but the loop after the first draft is something a language controls: a strict compiler, diagnostics a program can parse, one authoritative reference, and tools that expose all of this to a model.** Salam, a young language with little public code to learn from, has invested in exactly that loop.

This article looks at why AI coding tools struggle with new languages, which properties of a language and its tooling help, and how Salam implements each one. It also says what none of this can solve.

## Why AI coding tools struggle with new languages {#the-problem}

Large language models learn programming from public code. Python, JavaScript, Java, C and Go appear in enormous quantities; a language released a few years ago appears in very small amounts, and its older syntax may outnumber the current one. The result is predictable: asked to write a language it barely knows, a model fills the gaps with the syntax of a language it knows well. The [Salam AI page](/ai/) states the problem directly: Salam's syntax differs from every mainstream language, so a general chatbot tends to make it up.

For Salam the typical borrowings are easy to list, because the compiler recognizes them: a `for` loop instead of `each`, `return` instead of `ret`, `while` and `elif`, Python's `True` and `None`, `self` instead of `this`, braces where Salam closes blocks with `end`. Each one reads naturally to anyone who knows Python or Go, and each one is an error in Salam.

There is also a time problem. Salam is pre-1.0 and still changes; typed local declarations such as `x: int = 5` were removed in 2026. A model trained before that change will keep writing them. Its knowledge is a snapshot, and the language moved on.

A language cannot fix its training data. What it can fix is what happens next.

## Strict compilers are a feedback loop {#strict-compilers}

Code generation works best as a loop: write, check, read the errors, fix, check again. The value of that loop depends on the checker. A permissive language that accepts a mistake and fails at runtime gives the generator nothing to work with; a strict compiler that rejects it immediately, with a specific message, closes the loop in seconds.

Salam is strict on purpose. Variables are immutable unless declared `mut`, and declaring `mut` on a variable that never changes is an error, not a warning. Struct fields are private unless marked `pub`. A `match` on an enum must cover every member. Integer ranges, divisions by zero and indices the compiler can prove wrong are reported before the program runs. Here is a function that forgot a case:

```text
error[E080]: non-exhaustive match on enum 'Status': missing member Review
 --> m.salam:4:9
  |
4 |     ret match s:
  |         ^
```

These rules exist for human readers first. They also turn many plausible but wrong drafts into compile errors with a line number, which is the most useful thing a generator can receive. The [static typing article](/blog/static-typing-in-salam/) covers the type system behind them.

## Diagnostics written for humans and machines {#diagnostics}

A good error message for a model is the same as a good one for a person: what is wrong, where, and what to write instead. Salam's compiler adds a step that matters for generated code. When a line uses syntax from another language, it says so. Given a function that used `for` and `return`, `salam check` printed:

```text
error: Salam has no 'for' loop
 --> guess.salam:3:5
  |
3 |     for x in items:
  |     ^
  |
  = help: write 'each item in items:' (or 'each i, item in items:'), or count with 'repeat n in i:'
  = fix: Write 'each'
error: Salam spells 'return' as 'ret'
 --> guess.salam:6:5
  |
6 |     return sum
  |     ^
  |
  = help: write 'ret value', or a bare 'ret' when the function returns nothing
  = fix: Write 'ret'
```

With `--error-format=json` each diagnostic becomes one JSON object with the file, line, column, help text and a list of fixes, each fix marked as safe or not and expressed as byte-offset edits. A tool does not have to parse English. And `salam check --fix` applies the safe fixes itself: on this file it printed `applied 2 fix(es)` and `no errors`. After the fixes the code reads like idiomatic Salam, and the corrected program, with the missing enum case added as well, runs:

```salam
enum Status: Draft, Review, Published end

func label(s: Status): str:
    ret match s:
        Draft: "draft" end
        Review: "waiting for review" end
        Published: "live" end
    end
end

func total(items: Vector<int>): int:
    mut sum := 0
    each x in items:
        sum += x
    end
    ret sum
end

func main:
    mut words := Vector {} as Vector<int>
    defer words.free()
    words.push(1200)
    words.push(850)
    println label(Status.Review), total(words), "words"
end
```

```output
waiting for review 2050 words
```

Error codes are stable (E012, E080, W147), so a tool can look up a code instead of matching message text, and the same codes appear when the messages are printed in Persian. The [developer tools article](/blog/salam-developer-tools/) shows more of these diagnostics.

## One authoritative reference instead of a pile of tutorials {#one-reference}

A model given outdated documentation writes outdated code with confidence. Salam's answer is a single file, `SKILL.md` in the repository root, that describes itself as the authoritative, up-to-date reference: current syntax in English (Part I) and Persian (Part II), the standard library catalog, the compiler's strict rules and top pitfalls, the page language, and a guide to porting C. It is about 250 KB of text written to be loaded into a model's context, and it says plainly which older material is out of date, such as the tutorial books in the repository.

Next to it, `docs/ai/AGENTS.md` is a shorter orientation for agents, with project facts (where releases live, how versions are named) so that a model points to real files instead of inventing URLs. Both are kept in the repository with the compiler, so a change to the language can update them in the same commit.

## llms.txt: a site map for models {#llms-txt}

[llms.txt](https://llmstxt.org/) is a proposed convention: a Markdown file at the root of a site that tells language models what the site contains and where to look. The Salam site generates one for each language at build time: `llms.txt` with the site description and every page, lesson and standard library package as a link with its summary, plus links to the agent skill and the MCP server, and `llms-full.txt` with the full text of every lesson and library page in Markdown, for loading into a model's context in one piece. The English `llms-full.txt` is about 7 MB.

What makes these files trustworthy is how the site is built. Before generating pages, the build runs every example and compares its output with the expected output stored next to it, so the site never shows code that does not compile or output that is out of date. The magazine you are reading follows the same rule: every Salam program in it with an output block was compiled and run when the site was built. Documentation that is checked by the compiler cannot quietly drift away from the language, and anything derived from it, `llms.txt` included, inherits that property.

## An MCP server: giving the model the real compiler {#mcp}

The [Model Context Protocol](https://modelcontextprotocol.io) (MCP) is an open protocol that lets AI applications call external tools. Salam has an MCP server, written in Salam, that gives a model the three things it otherwise has to guess at:

- **the language**: the full reference, searchable by topic or by error code (`salam_docs`), and the English and Persian keyword table;
- **the standard library**: search by what a function does, every package, each package's exact signatures, and working examples from the test corpus;
- **the compiler**: check, build, run, format, translate between English and Persian, and inspect tokens, syntax trees or LLVM IR.

A failed check returns each diagnostic with its code, position, the source line, the compiler's help, a hint when the code uses another language's syntax, and the minimal failing program for that error code from the test suite. The server is hosted at `https://mcp.salamlang.ir/mcp`, so any MCP client can use it without installing anything:

```json
{
  "mcpServers": {
    "salam": { "type": "http", "url": "https://mcp.salamlang.ir/mcp" }
  }
}
```

The hosted server cannot see your files, so its tools take the program text, and it never runs code: running and building exist only when you start the server on your own machine. You sign in with a Salam AI account, the free plan included. The article on [Salam AI and Salam Agent](/blog/salam-ai-and-salam-agent/) covers the hosted assistant built on the same pieces.

## Language design choices that help {#design-choices}

Some properties make a language easier for generators, and most of them are simply good design:

- **Few ways to say one thing.** Salam has one loop over a collection (`each`), one conditional loop (`until`, which runs while its condition holds), and `repeat` for counting. A smaller surface means fewer plausible wrong variants.
- **Explicit structure.** Blocks end with `end`, mutation needs `mut`, public fields need `pub`. Nothing depends on indentation alone or on an implicit convention.
- **Static types with inference.** The compiler can reject a wrong call before anything runs, but local code stays short: `count := 0` needs no annotation.
- **A standard library with real names.** When the library is large and documented by the same pipeline, the most common hallucination, an invented function, is caught by a search tool or by the compiler's did-you-mean.
- **Tests as the specification.** Compiler errors are tested by small programs that must produce a given code, and those programs double as examples for each error.

Some properties work against generators, and Salam has them too. Its syntax is unfamiliar, it still changes before 1.0, and multi-word names such as `reading time` are legal, which surprises anyone, human or model, who expects identifiers without spaces. The [syntax design article](/blog/salam-syntax-design/) explains why those choices were made anyway.

## What a compiler cannot check {#limits}

A clean compile proves that a program is well formed and well typed. It does not prove that it does the right thing. A generated function can sort in the wrong order, miss an edge case or misread the task and still compile on the first try. The answer is the same for code from any source: tests that state the expected behavior, run on every change. Salam's test runner (`salam test`) makes those cheap to write, but someone has to write them.

Memory is a second limit. Salam has no garbage collector and no borrow checker. The ownership checker reports a definite use after free or double free as an error and warns where only some paths free a value, but it cannot prove a program free of leaks the way Rust's borrow checker proves the absence of certain errors. Generated code that allocates needs the same review as handwritten code.

Third, we make no claim here about how often any model gets Salam right. Those numbers depend on the model, the prompt and the task, change with every release, and would need a published evaluation to mean anything.

## Frequently asked questions {#faq}

### Can AI tools write Salam code?

Yes, with help. A general model often borrows syntax from other languages, but with the compiler's diagnostics, the `SKILL.md` reference or the MCP server in the loop it can check and fix its own drafts. [Salam AI](/ai/) is an assistant built around that loop.

### What is llms.txt?

A proposed convention for a Markdown file at a site's root that tells language models what the site contains. The Salam site generates `llms.txt` and a full-text `llms-full.txt` for each language whenever the site is built.

### What is an MCP server for a programming language?

A server that exposes the language's tools, such as the compiler, the documentation and the standard library index, to AI applications through the Model Context Protocol. Salam's is written in Salam and hosted at `mcp.salamlang.ir`.

### Will AI make new programming languages unnecessary?

Nothing suggests that yet. Generated code still has to be read, reviewed and maintained by people, and a language that makes intent explicit helps with all three. What changes is that a new language now also needs machine-readable tooling, not only good documentation; see [why the world still needs new programming languages](/blog/why-new-programming-languages/).
