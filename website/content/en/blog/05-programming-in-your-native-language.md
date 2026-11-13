---
title: Can programming in your native language change computer science education?
seo_title: Programming in Your Native Language: Can It Change CS Education?
description: Can programming in your native language help students? What the evidence says, and how Salam's Persian course lets learners start in Persian and move on.
keywords: programming in native language, learn programming in Persian, teaching programming, native language programming education, Persian programming course, non-native English speakers programming, computer science education, programming for beginners, mother tongue education, Salam course
category: language
summary: English is the hidden prerequisite of most programming courses. This article looks at what removing it can achieve, what the research supports, and how a bilingual language lets a student start in Persian and finish in English.
date: 2026-11-11
image: /assets/img/blog/programming-in-your-native-language-en.png
image_alt: Cover image for "Can programming in your native language change computer science education?"
---

Programming in your native language means learning and writing code whose keywords, library names and error messages are in the language you think in, rather than in English. **It can lower one specific barrier to computer science education, the need to read technical English while also learning to think like a programmer, but it is not a shortcut around the hard part, which is learning to reason precisely about programs.** Whether that barrier matters depends on the learner: for a ten-year-old in Tehran or a first-year student who reads English slowly, it can be the difference between following a lesson and copying it.

This article looks at the question carefully: what English costs a beginner, what research does and does not tell us, what a native-language language has to provide to be useful in a classroom, and how Salam's design lets a learner start in Persian and move to English later without changing languages.

## The hidden prerequisite in programming courses {#cost-of-english}

Almost every popular programming language uses English keywords, English library names, English documentation and English error messages. For a native English speaker these are mnemonic: `while`, `return` and `print` mean roughly what they say. For a learner who does not read English well, the same words are opaque symbols that must be memorized before they can be understood.

The cost shows up in several places at once:

- **Keywords and names** must be memorized as symbols instead of read as words.
- **Error messages**, the main feedback a beginner gets, arrive in English at exactly the moment the learner is confused.
- **Documentation and search results** are mostly in English.
- **Naming things**, one of the main skills of programming, happens in a second language, and beginners often fall back to single letters or transliterations.

None of this is impossible to overcome; millions of programmers worldwide learned in English as a second or third language. The question is how much of a beginner's limited attention it consumes, and who gives up because of it.

## What the evidence does and does not say {#evidence}

It is worth being careful here, because this topic attracts strong opinions.

On the general question, there is long-standing support for teaching in a language the learner understands. UNESCO's 1953 report _The Use of Vernacular Languages in Education_ argued for teaching children in their mother tongue, and the organization has promoted mother-tongue based multilingual education ever since. That research is about schooling in general, not programming.

On programming specifically, the evidence is thinner but points the same way. Philip Guo's 2018 CHI paper _Non-Native English Speakers Learning Computer Programming: Barriers, Desires, and Design Opportunities_ surveyed learners around the world and documented the extra load English places on them, from reading instructional material and technical vocabulary to understanding code examples. Felienne Hermans's educational language Hedy was built so that its keywords can be used in many natural languages, on the argument that English keywords are an unnecessary obstacle for children.

What we are not aware of is a large controlled study showing that native-language keywords alone produce better programmers. Keywords are a small part of a programming language, and the effect of a native-language environment is hard to separate from the effect of better teaching, better materials and better error messages. A fair summary: the language barrier is real for many learners, removing it matters a great deal for some and not at all for others, and translating only the keywords leaves most of the barrier in place.

## What a native-language programming language must provide {#requirements}

If the goal is education, a language with translated keywords is not enough. A teacher needs:

1. **A complete language**, not a toy subset, so that students do not hit a wall and have to restart in another language.
2. **Error messages in the student's language**, because that is where most of the learning happens.
3. **A course and reference material** in that language, with examples that actually run.
4. **Tooling that works**: an editor that displays the script correctly, ideally in a browser so nothing needs installing.
5. **A path to the global ecosystem**, so that a student who starts in their own language is not stranded there.

The last point is the one most projects miss. A language that only exists in Persian traps its students; a language that is the same in Persian and English gives them a bridge.

## How Salam approaches it {#salam-approach}

Salam is a general-purpose compiled language with two complete spellings, English and Persian, described in detail in [one language, two scripts](/blog/one-language-two-scripts/). For education, the relevant facts are:

- Everything the language defines has a Persian name: keywords, types, built-in functions and methods, every standard library package and its functions, and all of the compiler's messages.
- The compiler reports errors in Persian for Persian files, with Persian digits in the error code and fixes that name the Persian spelling. An English file can get Persian diagnostics too, with `--error-lang=fa`.
- The [online editor](https://editor.salamlang.ir) runs in the browser, switches to a full right-to-left layout for Persian, and needs no installation.
- It is not a teaching subset. The same Persian spelling is used for web servers, databases and the rest of the standard library.

Here is the kind of program a student writes in the first week, in Persian:

```salam
تابع میانگین(نمره‌ها: صحیح[]): اعشار۶۴:
    ناپایا جمع := ۰
    هر ن در نمره‌ها:
        جمع += ن
    پایان
    برگشت جمع برگردان اعشار۶۴ / طول(نمره‌ها)
پایان

تابع ریشه:
    نمره‌ها := [۱۸، ۱۵، ۲۰، ۱۲]
    م := میانگین(نمره‌ها)
    سرچاپ «میانگین:»، م
    اگر م >= ۱۷:
        سرچاپ «عالی»
    وگرنه م >= ۱۲:
        سرچاپ «قبول»
    وگرنه:
        سرچاپ «مردود»
    پایان
پایان
```

```output
میانگین: 16.25
قبول
```

A Persian-speaking student can read every line of it: `میانگین` (average), `نمره‌ها` (grades, with the zero-width non-joiner Persian writers use), `هر ن در نمره‌ها` (for each `ن` in the grades), `اگر` (if), `وگرنه` (otherwise). Grades out of 20 are how Iranian schools mark, so the example is familiar too. Nothing in it has to be memorized as a foreign symbol.

## The Persian course {#persian-course}

Salam's course has 47 lessons in English on this site and the same 47 lessons in Persian at [salamlang.ir/learn](https://www.salamlang.ir/learn/). It covers the whole language: the first program, variables and types, conditions and loops, functions and lambdas, arrays and collections, structs, enums, generics, packages, memory, compile-time code, calling C and threads, then about twenty lessons on building web pages with Salam's page language, and finally serving and deploying a site with `salam serve`.

Two details make it useful in a classroom. First, every example program in the course is compiled and run when the site is built, and the output shown under each example is the real output; a lesson cannot quietly drift out of date. Second, the Persian lessons are written in Persian Salam with Persian names, so a student reads Persian code from the first lesson on.

The English versions of the first lessons, for comparison: [your first program](/learn/start/), [variables](/learn/variables/), [loops](/learn/loops/) and [functions](/learn/functions/). The [lesson on keywords](/learn/keywords/) lists every keyword in both spellings.

## Moving to English later {#moving-to-english}

The bridge is the point. A student who learned Salam in Persian already knows the English language's concepts, syntax and library; only the spelling differs. The same program written in English, with English names, is a direct mapping:

```salam
func average(scores: int[]): f64:
    mut sum := 0
    each s in scores:
        sum += s
    end
    ret sum as f64 / len(scores)
end

func main:
    scores := [18, 15, 20, 12]
    a := average(scores)
    println "Average:", a
    if a >= 17:
        println "Excellent"
    else a >= 12:
        println "Pass"
    else:
        println "Fail"
    end
end
```

```output
Average: 16.25
Pass
```

There are three ways to make the move:

- **Gradually.** Keep writing Persian code but read the English names in error messages and documentation; the error codes (`E001` and `خطا۰۰۱`) are shared, so looking up one finds the other.
- **With the translator.** `salam translate en` converts a Persian file to English keywords and library names, leaving the student's own names alone. Students can compare their own code side by side with its English form. See [inside salam translate](/blog/salam-translate-persian-english-code/).
- **All at once.** Start a new project in English. Everything learned still applies, and the vocabulary is small:

| Persian         | English       | Meaning                       |
| --------------- | ------------- | ----------------------------- |
| `تابع`          | `func`        | declares a function           |
| `برگشت`         | `ret`         | returns a value               |
| `ناپایا`        | `mut`         | makes a variable changeable   |
| `پایان`         | `end`         | closes a block                |
| `سرچاپ`         | `println`     | prints a line                 |
| `اگر` / `وگرنه` | `if` / `else` | branches                      |
| `هر ... در`     | `each ... in` | loops over a collection       |
| `تا`            | `until`       | loops while a condition holds |
| `تکرار`         | `repeat`      | loops a number of times       |
| `برگردان`       | `as`          | converts a type               |

After that, moving from Salam to Python, Go or C is a change of language, not of spelling, and the concepts transfer the way they do between any two languages.

## Common objections {#objections}

**"Programmers must know English anyway."** Most will, eventually. The argument for native-language learning is about order, not about avoiding English: learn to program first, then learn the English vocabulary of a subject you already understand. A bilingual language makes that order possible without a dead end.

**"It fragments the community."** Only if the code cannot cross over. Because Persian and English Salam are one language, a Persian student's package can be used from an English program, and the reverse, and the translator converts between them.

**"Native-language languages are toys."** Many have been. Salam is a statically typed language that compiles to native code, ships with a large standard library and builds its own compiler. Its limits are those of a young language (a small community, few production deployments, a pre-1.0 language that still changes), not of its Persian spelling.

**"It helps only Persian speakers."** Today, yes: Salam has two language packs, English and Persian. The design, a shared core with per-language spellings, is not specific to Persian, but no other language pack exists yet.

## Advice for teachers {#for-teachers}

- Start in the language your students think in, and say clearly that the English spelling exists and is the same language.
- Use the online editor for the first lessons, so nobody spends the first class installing tools.
- Show students the error messages early and teach them to read the error code; it is their bridge to English material later.
- Introduce `salam translate` once students are comfortable, and let them compare their Persian code with its English form.
- Move to English names when students start using outside material, not before they are ready.

## Frequently asked questions {#faq}

### Can I learn programming in Persian?

Yes. Salam can be written entirely in Persian, and its free course of 47 lessons at salamlang.ir/learn teaches it in Persian, with every example compiled and run. The compiler's error messages are also in Persian.

### Is it better to learn programming in your native language or in English?

It depends on how well you read English. If technical English slows you down, starting in your own language lets you spend your attention on programming itself. If you read English comfortably, starting in English is fine. With Salam you can switch at any time without changing languages.

### Will learning in Persian make it harder to get a programming job?

Not if you move on to English, which Salam is designed for: the concepts, the library and the error codes are the same in both spellings. The skills employers look for, reasoning about programs, debugging and design, do not depend on the spelling of the keywords.

### Is there evidence that native-language programming helps students?

There is evidence that English is a real barrier for many learners, for example in Philip Guo's 2018 study of non-native English speakers learning to program, and broad evidence from education research in favor of teaching in the mother tongue. Controlled studies that isolate the effect of native-language keywords alone are still few.
