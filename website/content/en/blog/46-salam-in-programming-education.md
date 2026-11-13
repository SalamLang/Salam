---
title: Salam in programming education: teaching beginners, computer science and compilers with one bilingual language
seo_title: Salam in Programming and Computer Science Education
description: How Salam can be used to teach programming, computer science and compilers: Persian-first lessons, strict compiler feedback, a readable self-hosting compiler.
keywords: programming education, teaching programming, CS education, teaching compilers, learn programming in Persian, programming language for beginners, compiler course, software engineering education, bilingual programming language, Salam course
category: future
summary: Salam lets a student start in Persian, move to English keywords without switching languages, and later read a production compiler written in the same language. What exists for teachers today, and what classrooms would still need.
date: 2026-11-11
image: /assets/img/blog/salam-in-programming-education-en.png
image_alt: Cover image for "Salam in programming education: teaching beginners, computer science and compilers with one bilingual language"
---

Salam can serve three stages of programming education with one language: a first course taught in the student's own language, a computer science curriculum that needs a real compiled language, and a compilers course that studies a compiler written in the very language the students already know. **Salam is a statically typed, compiled language whose keywords, standard library and error messages exist in both Persian and English, which makes it usable as a first language for Persian speakers and as a bridge to the English-keyword languages used in industry.** This article looks at what the project offers educators today, where it could fit in a curriculum, and what is still missing before schools and universities could rely on it.

A note on scope: this is the "future" category. Nothing below claims that a school or university has adopted Salam; the repository documents no such adoption. It describes what exists and argues where it could help.

## Why the language of instruction matters {#why}

Most programming languages assume the learner reads English. Keywords such as `while`, `return` and `struct` are English words, and so are almost all compiler error messages. For a native English speaker that vocabulary is free. For a 14-year-old in Tehran or Kabul it is a second subject stacked on top of the first: the student has to learn a foreign vocabulary and the idea of a loop at the same time.

Whether teaching in the first language improves outcomes for programming specifically is a research question, and the evidence should be read carefully; the article [Can programming in your native language change computer science education?](/blog/programming-in-your-native-language/) goes through it. The narrower claim made here is practical: a language that lets a teacher remove the vocabulary barrier, without giving up a real compiler, real types and a real standard library, gives teachers an option they did not have.

## A first course in the student's own language {#first-course}

The basic exercise every introductory course reaches in its first weeks is a loop with a condition. Here is Euclid's algorithm for the greatest common divisor, written in Persian. `تابع` is `func`, `ناپایا` is `mut`, `تا` loops while its condition holds, `برگشت` is `ret`, and the entry function is `ریشه`. Persian digits and the Persian comma are accepted as written.

```salam
تابع مقسوم علیه مشترک(الف: صحیح، ب: صحیح): صحیح:
    ناپایا ایکس := الف
    ناپایا ایگرگ := ب
    تا ایگرگ != ۰:
        باقی := ایکس % ایگرگ
        ایکس = ایگرگ
        ایگرگ = باقی
    پایان
    برگشت ایکس
پایان

تابع ریشه:
    سرچاپ «ب.م.م ۸۴ و ۳۶ برابر است با»، مقسوم علیه مشترک(۸۴، ۳۶)
    سرچاپ «ب.م.م ۱۷ و ۵ برابر است با»، مقسوم علیه مشترک(۱۷، ۵)
پایان
```

```output
ب.م.م ۸۴ و ۳۶ برابر است با 12
ب.م.م ۱۷ و ۵ برابر است با 1
```

Two details matter for a classroom. The function name is three words with spaces, `مقسوم علیه مشترک`, because Persian concepts are often several words and forcing them into one token or an underscore makes code harder to read. And the program is not a toy dialect: it goes through the same type checker and code generator as English Salam and produces a native executable.

The teaching material around it is already substantial:

- **A free course of 47 lessons** in both [English](/learn/) and Persian, from the [first program](/learn/start/) through [structs](/learn/structs/), [generics](/learn/generics/), [memory](/learn/memory/) and threads to building web pages. Every example in every lesson is compiled when the site builds, and the output shown is the real output.
- **Nothing to install.** The online editor at [editor.salamlang.ir](https://editor.salamlang.ir) runs the whole compiler in the browser through WebAssembly, which matters in computer labs where students cannot install software.
- **An Android app** with the compiler built-in, so programs run offline on a phone or tablet running Android 5.0 or newer. For students whose main computer is a phone, that is not a detail.
- **An interpreter** (`salam exec`) and a REPL (`salam cli`) for quick experiments without a C toolchain.

## A strict compiler as a patient teacher {#strict-compiler}

Beginners learn as much from error messages as from lectures, so the quality of diagnostics is a teaching feature. Salam's compiler is strict by design: variables are immutable unless declared `mut`, unused variables are errors rather than warnings, and many common mistakes come with a suggested fix. Take a student's first attempt at summing 1 to 5:

```salam fragment
func main:
    total := 0
    repeat 1 to 5 in i:
        total += i
    end
    println total
end
```

`salam check sum.salam` stops before generating any code and points at the exact spot:

```text
error[E013]: cannot reassign immutable variable 'total'; declare it 'mut' to reassign it, or assign to its elements or fields instead
 --> sum.salam:4:9
  |
4 |         total += i
  |         ^
```

The same error appears in Persian when the program is written in Persian, or when a teacher passes `--error-lang=fa` for an English program:

```text
خطا[خطا۰۱۳]: نمی‌توان متغیر تغییرناپذیر 'total' را دوباره مقداردهی کرد؛ برای این کار آن را 'ناپایا' تعریف کنید، یا به‌جای آن به عناصر یا فیلدهایش مقدار دهید
```

The compiler keeps more than 1,300 diagnostic and tool messages in one catalog, each with a Persian counterpart. The error codes are stable (`E013` here), so a teacher can build a page of "common errors and what they mean" that does not go stale with every release. Strictness has a cost for beginners, who can find a compiler that refuses unused variables annoying, but it trains the habit of reading and fixing diagnostics early.

## A bridge from Persian to English, inside one language {#bridge}

The obvious objection to teaching in Persian is that professional software is written with English keywords. Salam's answer is that the student never has to switch languages to switch vocabulary. The same program in English keywords looks like this:

```salam
func gcd(a: int, b: int): int:
    mut x := a
    mut y := b
    until y != 0:
        rest := x % y
        x = y
        y = rest
    end
    ret x
end

func main:
    println "gcd(84, 36) =", gcd(84, 36)
    println "gcd(17, 5) =", gcd(17, 5)
end
```

```output
gcd(84, 36) = 12
gcd(17, 5) = 1
```

The `salam translate` command rewrites keywords, the entry function and built-in type names from one spelling to the other and leaves the student's own identifiers and comments alone. A course can therefore start in Persian, introduce the English keywords halfway through as a vocabulary exercise, and finish in English, with every concept carrying over unchanged. The article [From Persian code to English and back](/blog/salam-translate-persian-english-code/) describes the tool in detail.

From English Salam, the step to Python, Go or C is a syntax change, not a conceptual one: static types, functions, structs, enums, generics, `defer` and explicit memory management all have close relatives in mainstream languages.

## Teaching compilers with a self-hosting compiler {#teaching-compilers}

Compiler courses usually study a toy compiler, because production compilers are written in a different language from the one they compile and are too large to read. Salam is unusual on both counts. Its compiler is written in Salam, and its structure follows the textbook pipeline: `compiler/lexer`, `compiler/parser`, `compiler/semantic`, then separate code generators in `compiler/codegen` (C), `compiler/llvmgen` (LLVM IR), `compiler/nativegen` (x86-64 machine code) and `compiler/nativelink` (a built-in linker), plus an interpreter in `compiler/interp` and a JavaScript generator in `compiler/jsgen`.

Students can watch each stage from the command line. `salam inspect` dumps the token stream, the decorated syntax tree or the symbol table as XML:

```bash
salam inspect tiny.salam --emit-tokens
salam inspect tiny.salam --emit-ast
salam build tiny.salam --emit-c
salam llvm tiny.salam
salam build tiny.salam --backend=native --emit-asm
```

The first token of `func main:` comes out like this:

```text
<token>
  <base>token_t</base>
  <type>KEYWORD_FUNC</type>
  <source>func</source>
  <location>
    <begin_line>1</begin_line>
    <begin_column>1</begin_column>
    <end_line>1</end_line>
    <end_column>5</end_column>
  </location>
</token>
```

The last three commands show the same program as C, as LLVM IR and as x86-64 assembly, which makes the trade-offs between compiling through C, through LLVM and directly to machine code concrete rather than theoretical. A compilers course could assign "add a warning" or "add a constant-folding rule" as a project in a real, self-hosting compiler and have students test it with the project's own test suite. The [compiler pipeline article](/blog/how-the-salam-compiler-works/) and the [self-hosting article](/blog/self-hosting-compiler/) are good starting points for such a course. The bilingual frontend is a teaching topic in its own right: the keyword tables for both languages live in one file, `compiler/langpack.salam`.

The scale is a fair warning. The compiler and its tools are more than 300 source files, which is too large to read end-to-end in one semester. It is better used as a codebase to explore and modify than as a textbook.

## Software engineering practice from the first project {#software-engineering}

Software engineering courses need more than a compiler: projects, packages, tests, formatting, documentation and version control. Salam puts all of these behind the one `salam` command, which keeps the tool list a student must learn short. A test file is any `*_test.salam` file with `test_` functions:

```salam fragment
func gcd(a: int, b: int): int:
    mut x := a
    mut y := b
    until y != 0:
        rest := x % y
        x = y
        y = rest
    end
    ret x
end

func test_gcd_common():
    testing.AssertEqInt(gcd(84, 36), 12)
end

func test_gcd_coprime():
    testing.AssertEqInt(gcd(17, 5), 1)
end
```

```text
$ salam test
=== RUN   test_gcd_common
--- PASS: test_gcd_common
=== RUN   test_gcd_coprime
--- PASS: test_gcd_coprime
ok   gcd_test.salam: 2 tests passed
PASS: 1 test files
```

`salam new` creates a project with a manifest, `salam format` settles style arguments before they start, `salam doc` generates an API page, and `salam add` brings in packages from Git repositories with a lockfile and checksums. A team project in Salam therefore exercises the same workflow students will meet in industry, in a language where they can also read the error messages. The [developer tools article](/blog/salam-developer-tools/) covers these commands.

## Younger learners: a block-based prototype {#younger-learners}

The repository also contains an experiment aimed at children, in the `kids/` folder: a drag-and-drop block editor written in Salam itself. Children build a program from blocks such as "Say", "Ask", "Repeat" and "Move" in two modes, "Talk & Print" and a turtle-graphics "Draw & Move", and a "Grown-up Code" panel shows the Salam program the blocks correspond to. That last idea is the interesting one: the bridge from blocks to text code is built-in.

It is a prototype in the source tree, not a published product, and its labels are in English. It shows a direction rather than a finished tool.

## What classrooms would still need {#gaps}

Being honest about the gaps is what makes the opportunity credible.

- **Stability.** Salam is pre-1.0 and its syntax still changes. Typed local declarations were removed in 2026, and self-assignments such as `x = x + 1` became a compile error in version 0.6.3. Course material written for one release can break on the next, which is a real cost for teachers who reuse notes for years.
- **Teacher materials.** The course is written for learners. There are no published lesson plans, exercise banks with solutions, assessment rubrics or teacher guides. The repository also holds LaTeX books in Persian and English, but the project's own language reference warns that they are out of date.
- **Evidence.** There is no published study of students learning with Salam. Anyone adopting it for a course would be running an experiment, and should measure it.
- **Community size.** Students who get stuck will find far fewer forum answers than for Python or Java. The community channels (Telegram, Discord, Eitaa, Bale) are small, and the [Salam AI](/ai/) assistant covers part of that gap.
- **Outreach so far is small.** The repository holds slides for a Persian talk prepared for the DSTO event in Kashan in October 2026, which is the kind of grassroots work an educational language starts with, not evidence of adoption.

The opportunity is just as concrete: a free, open source, bilingual language with a real compiler, a 47-lesson course in two languages, a browser editor and an Android app is a reasonable foundation for a pilot course. A teacher who runs one and publishes the results would be contributing something the project does not have yet.

## Frequently asked questions {#faq}

### Is Salam good for beginners?

It can be, especially for Persian speakers, because keywords, the standard library and error messages exist in Persian. Its strictness (immutable variables by default, unused variables as errors) helps form good habits but can frustrate at first, and its community is much smaller than Python's.

### Can I teach a compilers course with Salam?

Yes, the self-hosting compiler is open source, follows the classic lexer, parser, semantic analysis and code generation structure, and `salam inspect` exposes tokens, syntax trees and symbols. It is large, so it works best as a codebase for exploration and small projects rather than as a complete reading assignment.

### Do students need to install anything?

No. The online editor runs the compiler in the browser, and there is an Android app with the compiler built-in. Installing the compiler itself is one command on Linux, macOS or Windows.

### Will students who learn in Persian be able to move to English languages?

That is the purpose of the bilingual design. The same program can be rewritten with English keywords using `salam translate`, and the concepts (types, functions, structs, loops, memory) carry over to Python, Go, C and others.
