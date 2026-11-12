---
title: Multilingual programming and the future of native-language code: could Salam's bilingual model work for other languages?
seo_title: Multilingual Programming: Could Salam's Model Work Elsewhere?
description: Multilingual programming in Salam: how language packs let one compiler speak English and Persian, and what a third language such as Arabic would need.
keywords: multilingual programming, localized programming language, non-English programming language, programming in native language, language packs compiler, Persian programming, Arabic programming language, bilingual compiler, localized keywords, Salam translate
category: future
summary: Salam compiles English and Persian source with one front end, one standard library and one set of error codes. This article takes the design apart and asks what it would take to add a third human language, and whether that is a good idea.
date: 2026-11-11
---

Multilingual programming means writing code whose keywords, library names and compiler messages are in a human language other than English, while still compiling to the same programs as everyone else. Salam is one working example: **every Salam keyword, built-in type, standard library name and diagnostic has an English and a Persian spelling, and both compile through the same frontend to the same executable.** The interesting question for the future is whether that model generalizes. Could the same compiler gain an Arabic, Turkish or Hindi spelling? This article explains how Salam's bilingual design (introduced in [One language, two scripts](/blog/one-language-two-scripts/)) actually works in the compiler, then works through what a third language would require. Everything about a third language here is analysis; no third language pack exists or is planned in the repository.

## Localized programming languages before Salam {#history}

Salam is not the first language to drop the assumption that code is written in English. A few well-known precedents show the range of approaches:

- **Spreadsheet formulas.** Microsoft Excel translates its function names per locale, so a German user writes `SUMME` where an English user writes `SUM`. It is probably the most widely used localized programming notation in the world.
- **Scratch** translates its blocks into many languages. Because the program is a set of blocks rather than text, a project can be shown in any language without rewriting it.
- **1C:Enterprise**, used widely in Russia for business software, has Russian keywords with English equivalents.
- **Hedy**, a gradual teaching language from Leiden University, offers its keywords in many natural languages for classroom use.
- **Artistic and research projects** such as Qalb (a Lisp written in Arabic) and Wenyan-lang (a language in classical Chinese) showed that the idea runs into script, direction and culture, not only vocabulary.

Most of these either translate a narrow surface (formula names, blocks) or build a separate language. Salam's approach sits in between: one full general-purpose language, with two complete spellings that share everything below the lexer.

## How Salam's bilingual model works {#how-it-works}

The core idea is that the grammar is shared and only the spellings differ. A Persian `if` statement has the same structure as an English one; only the word changes. Here is a word count in Persian, using the Persian names of the standard library:

```salam
واردسازی رشته

تابع ریشه:
    واژه‌ها := رشته.تفکیک کردن(«سیب انار سیب توت سیب»، « »)
    دیرکن واژه‌ها.آزادکن()
    ناپایا شمار := نگاشت {} برگردان نگاشت<رشته، صحیح>
    دیرکن شمار.آزادکن()
    هر واژه در واژه‌ها:
        شمار.درج(واژه، شمار.دریافت با پیش فرض(واژه، ۰) + ۱)
    پایان
    سرچاپ «سیب:»، شمار.بگیر(«سیب»)
    سرچاپ «انار:»، شمار.بگیر(«انار»)
پایان
```

```output
سیب: 3
انار: 1
```

And the same program in English:

```salam
import str

func main:
    words := str.Split("apple pear apple fig apple", " ")
    defer words.Free()
    mut counts := HashMap {} as HashMap<str, int>
    defer counts.Free()
    each word in words:
        counts.Put(word, counts.GetOr(word, 0) + 1)
    end
    println "apple:", counts.Get("apple")
    println "pear:", counts.Get("pear")
end
```

```output
apple: 3
pear: 1
```

Line for line the two are the same program. `واردسازی` is `import`, `رشته` is the `str` package, `تفکیک کردن` is `Split`, `دیرکن` is `defer`, `نگاشت` is `HashMap`, `هر ... در` is `each ... in`, and `دریافت با پیش فرض` is `GetOr`. Inside the compiler, that mapping happens in layers.

### Layer 1: keyword tables

The keywords live in `compiler/langpack.salam` as parallel tables: a list of token kinds, and for each language pack the spelling of each kind. There are 60 keyword spellings per pack, from `func` and `ret` to `try`, `catch` and `throws`; the [keywords lesson](/learn/keywords/) lists them side by side. The lexer looks up an identifier in the active pack's table and turns it into the same token kind either way, so the parser never sees which language it came from.

One detail shows why this is harder than a word list. Persian does not have a separate word for every English keyword. `تا` serves as both `until` (the while loop) and `to` (in `repeat 1 to 5`), and `هر` serves as both `each` and `by` (the step in `repeat 0 to 20 by 2`). The lexer resolves those by context: inside a `repeat` header `تا` means `to`, elsewhere it means `until`. A natural-language spelling is not always one-to-one with the English one, and the compiler has to carry that context.

### Layer 2: names in the standard library

Keywords are the easy part. A real program spends most of its words on library names: `str.Split`, `HashMap.put`, `os.Args`. Salam handles these with annotations on each public declaration:

```salam fragment
@en "Split"
@fa "تفکیک کردن"
pub pure func Split(s: str, delim: str): Vector<str>: ret s.Split(delim) end
```

The [standard library](/std/) carries more than 21,000 `@fa` annotations, one for every public package, function, type, field and constant. That number is the real cost of a language pack. Translating 60 keywords takes an afternoon; naming every public symbol of a large standard library consistently takes a project. Salam even has a design document, [docs/design/naming-standard.md](https://github.com/SalamLang/Salam/blob/main/docs/design/naming-standard.md), with rules for how Persian names are written (spaces between words, never underscores) and how they stay in step when an English name changes.

### Layer 3: diagnostics

The compiler's message catalog in `compiler/i18n.salam` holds more than 1,300 diagnostic and tool messages, each with a Persian counterpart. Errors keep the same code in both languages, written in Persian digits in Persian, so `E013` and `خطا۰۱۳` are the same error. By default diagnostics follow the source language, and `--error-lang=fa` prints Persian errors for an English program.

### Layer 4: script rules in the lexer

Persian also needs rules that have nothing to do with vocabulary. The lexer accepts Persian digits `۰` to `۹` in number literals and the Persian comma `،` as a separator. Guillemets `«»` delimit strings in Persian files. Names may contain spaces, and the compiler treats a space, a zero-width non-joiner or nothing at all between the parts of a Persian name as the same name. It also treats the Arabic and Persian forms of the letters yeh (`ي`/`ی`) and kaf (`ك`/`ک`) as equal, because keyboards and copied text mix them constantly. Those rules are what make Persian code typeable in practice, and every script would need its own version.

## Detecting the language and moving between packs {#detection}

A file declares its language with a header comment (`// lang: en` or `// زبان: فارسی`); without one, the compiler picks the pack whose keywords the file matches better. The `salam translate` command then rewrites a file between packs: keywords, `true`/`false`/`null`/`this`, the entry function name, primitive type names in type positions and built-in container methods. Identifiers, comments and string contents are left exactly as written.

The translator has limits worth knowing. It moves a fixed list of built-in names, so a standard library method outside that list can stay in the source language. Translating the Persian word count above to English, for example, left `دریافت با پیش فرض` in place, and the compiler then stopped with "identifier must be English in an English file" and the hint "the English name is 'GetOr'". The fix is one word, and the compiler names it, but round trips are not yet fully automatic. The article [From Persian code to English and back](/blog/salam-translate-persian-english-code/) covers the tool in depth.

Packages cross the language line more smoothly. Because every public name carries both spellings, a Persian program can import a package written in English and call it by its Persian names, and the reverse works for packages that declare `@en` names. That is what keeps the two communities on one ecosystem instead of two.

## What a third language pack would require {#third-language}

Suppose someone wanted Salam in Arabic, Turkish or Hindi. Reading the current code, the work falls into seven parts:

| Part                           | Size of the job                | Notes                                                      |
| ------------------------------ | ------------------------------ | ---------------------------------------------------------- |
| Keyword table                  | 60 spellings                   | Including context rules where one word covers two keywords |
| Built-in functions and methods | a few dozen names              | `len`, `push`, `free` and similar, in one compiler table   |
| Standard library names         | over 21,000 annotations        | The dominant cost; needs a naming standard first           |
| Diagnostics                    | over 1,300 messages            | Plus native digits in error codes                          |
| Lexer script rules             | small but subtle               | Digits, punctuation, string quotes, letter variants        |
| Editors and grammars           | 11 editor plugins, tree-sitter | Generated from the compiler's tables, which helps          |
| Course and documentation       | 47 lessons, every std page     | Persian has both; a new pack would start from zero         |

There is also an engineering step before any of that. The pack mechanism is table-driven, but the current code is written for exactly two packs: the compiler checks `LP_EN` and `LP_FA` by name, and the annotations are `@en` and `@fa`. A third language would first need the compiler generalized from "English plus Persian" to "English plus N", which is a contained refactor, not a redesign.

Each candidate language brings its own problems:

- **Arabic** is the closest fit, because it shares the script. The lexer already accepts Arabic-Indic digits in number literals and already normalizes Arabic yeh and kaf. The new work is vocabulary and grammar: Arabic distinguishes gender and dual forms, which makes consistent function names harder than in Persian.
- **Turkish** uses the Latin script, so the lexer work is small, but case rules are not English case rules: the dotted and dotless i break naive lower-casing, which matters for any case-insensitive comparison of names.
- **Chinese and Japanese** write without spaces between words. Salam's multi-word names rely on spaces as separators, so the rules for where a name ends would need rethinking.
- **Hindi** and other Indic scripts combine characters into clusters, which affects how editors count columns for error positions.

## The hard question: is one grammar enough? {#grammar}

Salam's model keeps one grammar for every language. That is what makes translation a token-level operation and keeps one compiler. It also means the grammar follows English word order: the keyword comes first, as in `if condition:` and `repeat 3:`. Persian is a subject-object-verb language, and Salam still puts `اگر` ("if") at the front, which works because Persian conditionals also start with "if". Languages whose natural order differs more would read less naturally under a shared grammar.

The alternative, a grammar per language, would make code read more naturally and break the property that matters most: that any program can be mechanically translated and that every program shares one ecosystem. Salam chose shared grammar, and for a second language with an "if-first" conditional that choice costs little. For a third or fourth language, it is the main design tension to resolve before writing a single keyword.

## Where multilingual programming could go {#future}

This section is opinion and vision. Three directions seem realistic:

1. **Deeper bilingual tooling before more languages.** Closing the translator's gaps, so that a Persian file round-trips to English and back without manual fixes, would make the existing model stronger than a third pack would.
2. **Diagnostics in more languages first.** Error messages are the part of a language that beginners read most, and the message catalog is separate from the grammar. Translating diagnostics is much cheaper than a full pack and helps learners who still write English keywords.
3. **A shared method for adding packs.** If the compiler were generalized to N packs with a documented checklist, a community that wanted its own spelling could do the work itself. That is how localization works in most successful open source projects: the core defines the format, and native speakers supply the content.

None of this is on a published roadmap. What Salam has shown is narrower and still useful: a full, self-hosting, general-purpose language can carry two complete spellings without forking its compiler or its library. The articles on [designing a language for Persian and Arabic](/blog/designing-a-language-for-persian-and-arabic/) and on [right-to-left programming](/blog/right-to-left-programming-unicode/) go into the script-level details, and [programming in your native language](/blog/programming-in-your-native-language/) looks at why anyone would want this.

## Frequently asked questions {#faq}

### What is a localized programming language?

A localized programming language lets you write code with keywords and library names in a human language other than English. Salam does this fully for Persian: keywords, built-in types, the standard library and compiler errors all have Persian spellings that compile to the same program as English.

### Can I mix English and Persian in one Salam file?

Keywords and library names in one file must be in that file's language; using an English library name in a Persian file is an error whose hint gives the Persian spelling. Different files and packages can be in different languages and still work together in one program.

### Does Salam support Arabic?

Not as a keyword language. Salam's two language packs are English and Persian. Because Persian uses the Arabic script, the compiler already handles Arabic letter variants and digits, and the standard library has an [`arabic` package](/std/arabic/) for normalizing Arabic-script text, but there are no Arabic keywords.

### How hard would it be to add another human language to Salam?

The keywords are a small job; the standard library names (over 21,000 for Persian) and the 1,300-plus diagnostics are the large one, along with editor support and a course. The compiler would also need to be generalized from two packs to many before a third could be added.
