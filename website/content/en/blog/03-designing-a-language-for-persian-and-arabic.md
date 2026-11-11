---
title: Beyond translated keywords: the real problems of designing a programming language for Persian and Arabic
seo_title: Designing a Non-English Programming Language for Persian and Arabic
description: Translating keywords is the easy part. Multi-word names, ZWNJ, yeh and kaf, keyword choice, word order and diagnostics in a non-English programming language.
keywords: non-English programming language, Persian keywords, Arabic programming language, Persian programming language, programming language design, localized programming language, ZWNJ, Arabic script code, multilingual programming, native language keywords
category: language
summary: What actually goes wrong when a programming language is written in Persian or Arabic script, from names with spaces to look-alike letters, and the concrete decisions Salam made for each problem.
date: 2026-11-11
---

A non-English programming language is one whose keywords, and ideally its library and error messages, are written in a human language other than English. **The hard part of building one for Persian or Arabic is not choosing the keywords; it is everything around them: names that naturally contain spaces, an invisible joining character, two Unicode encodings for the same letter, words with two meanings, a word order unlike English, and error messages, editors and tools that all have to agree.** Most attempts stop at a keyword table, and the result feels like English with a costume on.

Salam is a general-purpose compiled language with a complete Persian spelling alongside its English one. This article walks through the design problems a Persian-script language runs into, the decisions Salam made for each, and where those decisions have costs. It also places Salam in the longer history of programming languages that are not written in English.

## Why a keyword table is not enough {#beyond-keywords}

A programming language touches its user through more than keywords. Count what a beginner reads in the first hour: keywords and type names, the names of built-in functions and methods, the names of the standard library packages and functions they call, the compiler's error messages, the documentation and the editor's highlighting and completion. If only the first item is translated, the learner meets English in every other place, including the most stressful one, the error message.

Salam's rule is that everything the language defines has two spellings: keywords, types, literal words such as `true`, the entry function, built-in functions and methods, standard library packages and their public functions, compile-time constants, and the compiler's messages, all 1,299 of them. Both spellings go through one lexer into the same tokens. That shared core is what makes the rest of this article possible, because every Persian-specific rule lives in one place: the lexer and the language-pack code. The [bilingual design article](/blog/one-language-two-scripts/) describes the mechanism.

## A short history of non-English programming languages {#history}

The idea is old, and each attempt solved a different part of the problem:

- **ALGOL 68** separated the language from its printed representation, and its report allowed keywords in other natural languages; representations in languages such as Russian and German were produced.
- **Spreadsheets** have localized function names for decades: in a German version of Microsoft Excel, `SUM` is `SUMME`. Hundreds of millions of people have used a localized formula language without thinking of it as programming.
- **1C:Enterprise**, a business platform widely used in Russia, has a scripting language with Russian keywords and English equivalents.
- **Scratch** translates its blocks into many languages, which works well because block-based code has no keywords to type.
- **Qalb** (قلب), created by Ramsey Nasser in 2012, is a Lisp written entirely in Arabic, built partly as a statement about how deeply English is baked into programming tools.
- **Wenyan-lang**, from 2019, writes programs in classical Chinese.
- **Hedy**, created by Felienne Hermans for teaching, introduces Python-like syntax gradually and offers its keywords in many natural languages.
- Mainstream languages moved in a smaller step: Python 3 (PEP 3131), Java, Go and others accept non-ASCII letters in identifiers, so names can be in any script even though keywords stay English.

Two patterns stand out. Projects that only translate keywords tend to stay demonstrations, because the library and the error messages are still English. Projects that go further are usually small, single-purpose languages for teaching. Salam's aim is a full general-purpose and systems language, compiled to native code, where the Persian spelling is complete rather than a teaching subset.

## Choosing Persian keywords {#keyword-choice}

A keyword must be short, unambiguous and readable, and it must not take away a word that people need as a name. In Persian these goals pull against each other more than in English. Some of Salam's choices:

| English  | Persian   | Literal meaning                  |
| -------- | --------- | -------------------------------- |
| `func`   | `تابع`    | function (the mathematical term) |
| `ret`    | `برگشت`   | return, coming back              |
| `main`   | `ریشه`    | root                             |
| `mut`    | `ناپایا`  | not constant, unstable           |
| `struct` | `ساختار`  | structure                        |
| `enum`   | `جداشمار` | separately counted               |
| `match`  | `همخوان`  | in agreement, matching           |
| `defer`  | `دیرکن`   | delay it                         |
| `as`     | `برگردان` | convert, render as               |
| `end`    | `پایان`   | end                              |

The entry function is a good example of a choice with consequences. The obvious translation of `main` is `اصلی`. Salam uses `ریشه` instead, and `تابع اصلی` is an ordinary function: a program that defines it compiles, but has no entry point. The English course warns about this in the first lesson, and the compiler reports "entry point is missing". The upside is that `اصلی` stays free as an everyday word for names such as `صفحه اصلی`.

The cost appears with very short words. The Persian `and` is `و`, a single letter that is also the most common word in the language. Making it a keyword means `و` can never be a variable name; `و۱` or `واحد` are fine. The alternative, a longer word for `and`, would make every condition read unnaturally.

### One word, two meanings {#two-meanings}

Persian has fewer short words for control flow than English, so two Salam keywords are deliberately reused, and the parser tells them apart by position:

- `تا` is `until` (Salam's while loop) at the start of a statement, and `to` inside a `تکرار` (repeat) header.
- `هر` is `each` at the start of a statement, and `by` (the step) inside a `تکرار` header.

This program uses both meanings of both words:

```salam
تابع شمار رقم‌ها(عدد: صحیح): صحیح:
    ناپایا ن := عدد
    ناپایا تعداد := ۰
    تا ن > ۰:
        ن /= ۱۰
        تعداد++
    پایان
    برگشت تعداد
پایان

تابع ریشه:
    تکرار ۰ تا ۲۰ هر ۱۰ در ای:
        سرچاپ «گام»، ای
    پایان
    هر عدد در [۷، ۱۴۰۵، ۲۰۲۶۱۱]:
        سرچاپ عدد، شمار رقم ها(عدد)
    پایان
پایان
```

```output
گام 0
گام 10
گام 20
7 1
1405 4
202611 6
```

Running `salam translate en` on it shows how the compiler reads each `تا` and `هر`: the loop becomes `until`, the repeat header becomes `repeat ۰ to ۲۰ by ۱۰ in ای`, and the last loop becomes `each`. The Persian identifiers and digits stay as they are, and the English file runs with the same output:

```salam
// lang: en
func شمار رقم‌ها(عدد: int): int:
    mut ن := عدد
    mut تعداد := ۰
    until ن > ۰:
        ن /= ۱۰
        تعداد++
    end
    ret تعداد
end

func main:
    repeat ۰ to ۲۰ by ۱۰ in ای:
        println "گام", ای
    end
    each عدد in [۷, ۱۴۰۵, ۲۰۲۶۱۱]:
        println عدد, شمار رقم ها(عدد)
    end
end
```

```output
گام 0
گام 10
گام 20
7 1
1405 4
202611 6
```

A pleasant side effect: English `until` confuses newcomers, because Salam's `until c:` loops while `c` is true. The Persian `تا` reads naturally as "as long as", so the Persian spelling avoids the most common English mistake.

## Multi-word names and the zero-width non-joiner {#multi-word-names}

Persian writes many single concepts as two or more words: `شمار رقم‌ها` (digit count), `جمع کل` (grand total). Forcing them into `snake_case` or gluing them together produces something no Persian reader would write. So Salam allows spaces inside names, in both languages: `func line total(o: Order)` is legal English Salam, and `تابع جمع ردیف(...)` is legal Persian.

Persian adds a second problem: the **zero-width non-joiner** (ZWNJ, U+200C). It separates two parts of a word without a visible space, as in `رقم‌ها` (digits). Writers are inconsistent about it; the same word appears with a ZWNJ, with a space, or glued together, and on screen the first two look nearly the same. Salam's decision:

- a space and a ZWNJ inside a name are **equivalent**: `رقم‌ها` and `رقم ها` are the same name (the program above declares `شمار رقم‌ها` and calls `شمار رقم ها`);
- **glued** forms are different names: `رقمها` is not `رقم ها`, and the compiler's "did you mean" suggestion offers the spaced form;
- keywords that contain a ZWNJ, such as `نادرست‌چاپ` (`printerr`), may be written with a space too, but not glued.

Allowing spaces creates one constraint: a multi-word name may not contain a keyword as one of its words, or the parser could not tell where the name ends. In English that rarely bites; in Persian `بسته` (`package`) and `تا` are also everyday words, so a name such as `بسته بندی` has to be spelled differently.

## Two encodings of the same letter: yeh and kaf {#yeh-and-kaf}

Unicode has separate code points for the Arabic yeh `ي` (U+064A) and the Persian yeh `ی` (U+06CC), and for the Arabic kaf `ك` (U+0643) and the Persian kaf `ک` (U+06A9). In the middle of a word they look identical. Older keyboard layouts, some operating systems and a great deal of existing text produce the Arabic forms, so two programmers can type the "same" name and get different bytes.

A compiler that compares names byte by byte would report an unknown identifier that looks exactly like a defined one, which is the worst kind of error message. Salam's lexer normalizes every name as it reads it: the Arabic yeh and kaf become the Persian letters, a ZWNJ becomes a space, and optional vowel marks (harakat) are dropped. So `كتاب`, `کتاب` and `کِتاب` are one identifier. For data rather than code, the standard library's [arabic package](/std/arabic/) goes further, folding letters, tatweel, vowel marks and digits into a search key.

## Word order and reading direction {#word-order}

Persian is a subject-object-verb language: the verb usually comes last. Programming syntax is mostly operator-first or keyword-first: `if` before the condition, the function name before its arguments, `ret` before the value. Salam does not reorder its grammar for Persian. A Persian statement is the same tree as the English one, with the words swapped.

That keeps the two spellings exactly convertible, and it is why `salam translate` can work token by token. The price is that some Persian lines read like headlines or instructions rather than sentences. The keyword choices soften it: many Persian method names are imperatives that read naturally at the start of a phrase, such as `بیفزا` (add) for `push`, `آزادکن` (free it) for `free`, `دیرکن` (delay) for `defer` and `بشکن` (break) for `break`. Comparison words such as `بزرگتر` (greater) let a condition read as words, which also sidesteps the mirroring of `<` and `>` in right-to-left text, covered in [right-to-left programming](/blog/right-to-left-programming-unicode/).

## Diagnostics in the reader's language {#diagnostics}

The error message is where a translated language most often falls back to English. In Salam each diagnostic has a Persian version, the error code itself is written with Persian digits, and the hints refer to Persian names:

```text
خطا[خطا۰۰۱]: شناسه‌ی 'upper' در یک فایل فارسی باید فارسی باشد
  = راهنما: نام فارسی آن 'بزرگ' است
  = اصلاح: نوشتن 'بزرگ'
```

This says that the identifier `upper` must be Persian in a Persian file and that its Persian name is `بزرگ`, with a fix that `salam check --fix` applies. The codes are shared across languages (`خطا۰۰۱` is `E001`), so a Persian speaker can find English material about the same error. Messages from the tools follow the file too: `salam format` and `salam translate` report their work in Persian when given Persian files.

## Tooling has to agree {#tooling}

A language is also its tools, and every tool needs the Persian rules:

- **Editors.** The plugins for Visual Studio Code and ten other editors highlight both keyword sets, generated from the compiler's own tables so that they cannot drift. The language server inside `salam` provides completion, hover, rename and semantic highlighting that understands multi-word Persian names, and it sends Persian diagnostics and hover text to an editor running in a Persian locale. See the [tools page](/tools/).
- **The formatter.** `salam format` re-indents Persian code and normalizes the spacing around operators, without touching the spelling of names.
- **The online editor** switches its whole layout, including the code area, to right to left for Persian.
- **The translator.** `salam translate` uses the compiler's lexer, so it applies the ZWNJ and yeh rules identically; see [inside salam translate](/blog/salam-translate-persian-english-code/).

Because all of these reuse the compiler's own lexer and tables, a Persian rule is implemented once.

## What about Arabic? {#arabic}

Persian and Arabic share a script, so much of this work carries over: Salam accepts Arabic-Indic digits in literals, folds the Arabic yeh and kaf into the Persian ones in names, and its standard library handles Arabic-script text through the `arabic`, `unicode` and `bidi` packages. But Salam has exactly two language packs today, English and Persian. There is no Arabic keyword set, and an Arabic speaker writing Salam would read Persian keywords, many of which come from Arabic (`تابع` and `صحیح`, for instance) while others are purely Persian (`ساختار`, `پایان`).

An Arabic pack would need its own decisions: keyword choices, the grammatical forms Arabic uses for commands, and translations of every message. The architecture would not need to change, since the compiler already separates the language pack from everything after the lexer.

## Lessons for anyone localizing a language {#lessons}

1. **Translate the whole surface.** Keywords alone leave the learner reading English at the moments that matter most.
2. **Normalize at the lexer.** Equivalences such as space and ZWNJ, or two encodings of one letter, belong in one function that every tool shares.
3. **Prefer exact conversion over natural grammar.** A token-for-token mapping makes translation, shared libraries and shared error codes possible.
4. **Keep error codes language-neutral.** A code is the bridge between communities.
5. **Accept that short words are expensive.** Every keyword removes a word from the pool of names.

The [free course](/learn/) teaches the language in English, and the same 47 lessons exist in Persian on the Persian site.

## Frequently asked questions {#faq}

### Is there a programming language in Persian?

Yes. Salam can be written entirely in Persian, from keywords to standard library calls, and its compiler reports errors in Persian. It is the same language as its English spelling and compiles to native executables.

### Why not just translate the keywords of an existing language?

Because the library, the error messages and the tools would still be English, and because Persian-specific problems such as multi-word names, the zero-width non-joiner and the two encodings of yeh and kaf need support in the lexer, not only in a keyword list.

### Can Salam be written in Arabic?

Not with Arabic keywords. Salam has English and Persian keyword sets. It accepts Arabic letter forms and Arabic-Indic digits, and its standard library processes Arabic text, but a full Arabic language pack does not exist.

### Do Persian names with spaces slow down or complicate compilation?

No. The lexer joins the words of a name into one identifier, and from then on it is an ordinary name. The only restriction is that a multi-word name cannot contain a keyword as one of its words.
