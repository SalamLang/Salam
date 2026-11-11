---
title: From Persian code to English and back: inside the salam translate tool
seo_title: salam translate: Convert Code Between Persian and English
description: How salam translate converts code between Persian and English: what it rewrites, what it leaves alone, its options, the round trip and its known limits.
keywords: salam translate, translate code Persian English, code translator, Persian to English code, source code translation, Persian programming language, bilingual code, Salam tools, transpile keywords, salam check fix
category: language
summary: salam translate rewrites a Salam program from English to Persian or from Persian to English without touching your own names, comments or strings. Here is what it changes, how it decides, and what to check afterwards.
date: 2026-11-11
---

`salam translate` is the command in the Salam toolchain that converts a source file between the language's two spellings. **It rewrites a Salam program from English to Persian (`salam translate fa`) or from Persian to English (`salam translate en`): the keywords, the entry function, the primitive types, the built-in methods and functions, and the names of standard library packages and functions, while leaving your own identifiers, comments, strings and formatting exactly as they were.** The result is a new file that compiles to the same program.

It is not a natural-language translator and it does not guess. Everything it changes comes from a table in the compiler or from the Persian names declared in the standard library, which is why a round trip gives back the original file. This article shows the tool on a real program, lists what it rewrites and what it does not, explains how it works, and is specific about the cases where you still have to look at the output.

## What salam translate is for {#what-it-does}

Salam is a [bilingual language](/blog/one-language-two-scripts/): a Persian file and an English file are read with different word lists but end up as the same tokens, the same syntax tree and the same executable. That makes a converter possible, and useful in several situations:

- a teacher writes examples in English and hands students the Persian version, or the reverse;
- a student who learned in Persian moves a project to English to share it on GitHub;
- a maintainer who does not read Persian gets a bug report with a Persian program and wants to read the code;
- documentation authors keep one example and publish it in both languages.

The command line is short:

```bash
salam translate fa app.salam            # English to Persian, writes app.fa.salam
salam translate en app.fa.salam         # Persian to English, writes app.fa.en.salam
salam translate fa src/                 # a whole directory, writes src-fa/
salam translate en app.salam --rewrite  # overwrite the file in place
```

## A first translation {#first-translation}

Start with an ordinary English program: two imports, a comment, a struct, a function with a multi-word name, a `Vector`, a loop, a comparison and a standard library call.

```salam
import str
import math

// A point on a plane
struct Point:
    pub x: f64 = 0.0
    pub y: f64 = 0.0
end

func distance from origin(p: Point): f64:
    ret math.Sqrt(p.x * p.x + p.y * p.y)
end

func main:
    mut points := Vector {} as Vector<Point>
    defer points.free()
    points.push(Point { x = 3.0, y = 4.0 })
    points.push(Point { x = 1.0, y = 1.0 })
    each p in points:
        d := distance from origin(p)
        if d >= 2.0 and p.x != p.y:
            println str.Upper("far:"), d
        else:
            println "near:", d
        end
    end
    println "points:", points.len(), true
end
```

```output
FAR: 5
near: 1.4142135623730951
points: 2 true
```

`salam translate fa points.salam` prints `translated points.salam -> points.fa.salam` and writes this file. It is reproduced exactly as the tool wrote it:

```salam
// زبان: فارسی
واردسازی رشته
واردسازی ریاضی

// A point on a plane
ساختار Point:
    همگانی x: اعشار۶۴ = 0.0
    همگانی y: اعشار۶۴ = 0.0
پایان

تابع distance from origin(p: Point): اعشار۶۴:
    برگشت ریاضی.جذر(p.x * p.x + p.y * p.y)
پایان

تابع ریشه:
    ناپایا points := وکتور {} برگردان وکتور<Point>
    دیرکن points.آزادکن()
    points.بیفزا(Point { x = 3.0، y = 4.0 })
    points.بیفزا(Point { x = 1.0، y = 1.0 })
    هر p در points:
        d := distance from origin(p)
        اگر d بزرگتربرابر 2.0 و p.x نابرابر p.y:
            سرچاپ رشته.بزرگ("far:")، d
        وگرنه:
            سرچاپ "near:"، d
        پایان
    پایان
    سرچاپ "points:"، points.طول()، درست
پایان
```

```output
FAR: 5
near: 1.4142135623730951
points: 2 true
```

The output is identical, because the program is identical. Look at what moved and what did not. `import str` became `واردسازی رشته` and `math.Sqrt` became `ریاضی.جذر`, the Persian names the standard library declares. `f64` became `اعشار۶۴`, `Vector` became `وکتور`, `.push` and `.free` became `.بیفزا` and `.آزادکن`, and `main` became `ریشه`. The comparisons became the Persian comparison words `بزرگتربرابر` and `نابرابر`, the commas became the Persian comma `،`, and `true` became `درست`. Meanwhile `Point`, `distance from origin`, `points`, `p` and `d` are unchanged, the comment is still in English, and so are the strings.

A Persian reader would now rename `Point` to `نقطه` and so on. The tool deliberately leaves that to a person, for reasons explained below.

## What the tool rewrites {#what-it-rewrites}

The list comes from the source of the command, `compiler/cli/cli_translate.salam`, and from running it:

| Category | Example, English to Persian |
|---|---|
| Reserved keywords, including `and`, `or`, `not` | `func` to `تابع`, `each` to `هر`, `and` to `و` |
| Literal words | `true`, `false`, `null`, `this` to `درست`, `نادرست`, `پوچ`, `این` |
| The entry function | `main` to `ریشه` |
| Primitive and built-in types in type positions | `int` to `صحیح`, `Vector<str>` to `وکتور<رشته>` |
| Built-in methods after a dot | `.push()` to `.بیفزا()`, `.len()` to `.طول()` |
| Built-in functions | `len(x)` to `طول(x)`, `spawn` to `نخ‌ساز` |
| Standard library imports and members | `import math` to `واردسازی ریاضی`, `math.Sqrt` to `ریاضی.جذر` |
| Compile-time constants | `SALAM_OS_LINUX` to `سلام سیستم لینوکس` |
| Comparison operators | `>=` to `بزرگتربرابر` (Persian), words back to symbols (English) |
| Separators | `,` to `،` and back |
| Guillemet strings | `«...»` to `"..."` when going to English |
| Context words | `static`, `link`, `dynamic`, `use` (embedding) in their positions |
| The page language | element and attribute names in `page:` blocks |
| The language line | `// lang: en` replaced by `// زبان: فارسی` |

Two details are worth knowing. First, comparison symbols are turned into words only when the file parses, because the tool uses the syntax tree to find which `<` and `>` really are comparisons; generic brackets such as `Vector<T>` keep their angle brackets. Going to English, comparison words always become symbols. Second, standard library names are not stored in the compiler. The tool walks the `std/` directory once, reads the `@en` and `@fa` tags of each package, and resolves `str.Upper` to `رشته.بزرگ` from the tag on the function. A package added to the standard library tomorrow is translated without any change to the tool.

## What it leaves alone {#what-it-keeps}

Everything else is copied byte for byte:

- **Your identifiers.** Variable, function, struct, field and enum names stay as written, in whatever script you wrote them.
- **Comments and string contents.** The tool translates code, not prose.
- **Number literals, including their digits.** A Persian file with `۱۲۰` translated to English still says `۱۲۰`, which is valid in an English file too.
- **Whitespace and layout.** Indentation, blank lines and line breaks are the original ones, because the tool copies the text between tokens rather than reformatting.

Leaving identifiers alone is a design decision, not a missing feature. A name such as `total` could be `جمع`, `کل` or `مجموع` in Persian depending on context, and a mechanical choice would often be wrong. Worse, renaming would break code that refers to the name from another file. Mixed names are legal in both languages, so the translated program compiles as it is, and a person can rename at leisure with the editor's rename command, which the [Salam language server](/tools/) provides.

## Options and output files {#options}

From `salam help`:

| Option | Effect |
|---|---|
| `en` or `fa` | The target language, required |
| `--lang=CODE` | The source language to assume when detection is undecided; by default each file is detected from its language line or from which keyword set it matches better |
| `--rewrite` | Overwrite the input files instead of writing new ones |
| `--output=PATH` | An output file for a single input, or an output directory |

Without `--output`, a single file `app.salam` is written as `app.fa.salam` (or `app.en.salam`) next to it, and a directory `src` is copied to `src-fa`. In a directory, every `.salam` file is translated and every other file is copied, so assets and `salam.toml` come along. Hidden files and directories are skipped. A file that is already in the target language is copied unchanged and counted as such in the summary line, for example `translate: 1 translated, 0 unchanged, 0 errors`.

The tool talks in the language of the source: translating a Persian file prints `ترجمه شد ... -> ...` and a Persian summary line. It refuses only one kind of input, a file with lexical errors (an unclosed string, a stray character), and says so: `cannot translate 'x.salam': fix the lexical errors first`. Syntax and type errors do not stop it: it never runs the type checker, so a half-finished file can be translated too.

## The round trip {#round-trip}

Translate the Persian file back with `salam translate en points.fa.salam` and compare:

```bash
salam translate fa points.salam
salam translate en points.fa.salam
diff points.salam points.fa.en.salam
```

The only difference is a first line, `// lang: en`, because the tool always writes a language line into its output and the original had none. Everything else, including the comment, the strings, the spacing and the order of the code, is identical. The project's test suite relies on the same property in a different form: each program in `tests/en/translate/` and `tests/fa/translate/` is translated to the other language, run, and its output compared with the expected output.

## How it works inside {#how-it-works}

The command is about 1,500 lines of Salam in `compiler/cli/cli_translate.salam`, and it reuses the compiler's own front end rather than matching text:

1. **Detect.** The same function the compiler uses decides which language pack the file is written in.
2. **Lex.** The file is split into tokens with that pack. Each token remembers its start and end position in the source text.
3. **Index.** The tool collects the names the file declares itself (so a local variable called `len` is not mistaken for the built-in), the packages it imports and their aliases, and the variables whose type comes from a package.
4. **Walk.** For every token it copies the original text between the previous token and this one, then either the token unchanged or its spelling in the target pack. Keywords map by token kind, which is why `until` and `to` both become `تا` in Persian and come back correctly, since they are different token kinds.
5. **Mark.** It removes any old language line from the header and writes the new one.

Because the text between tokens is never touched, comments and formatting survive by construction. Because the lexer is the compiler's lexer, multi-word names, ZWNJ and Persian digits are handled exactly as the compiler handles them. The [right-to-left programming article](/blog/right-to-left-programming-unicode/) covers those rules.

## Limits, and how to check the result {#limits}

The help text is candid about one trap: a method of your own that has the same name as a built-in container method, for example your own `.get()`, is renamed too at the call site. In practice the compiler accepts the Persian spelling for that call as well, so the program still runs, but read the output before using `--rewrite`.

The other limit is coverage. The built-in method table in the translator lags the compiler's: `lower`, `upper` and `bytes` are not in it in the current release. Here is the English program:

```salam
func main:
    word := "Salam"
    println word.upper(), word.lower(), word.len()
end
```

```output
SALAM salam 5
```

After `salam translate fa`, `.len()` has become `.طول()` but `.upper()` and `.lower()` are still English, and the compiler says so, with the fix:

```text
خطا[خطا۰۰۱]: شناسه‌ی 'upper' در یک فایل فارسی باید فارسی باشد
 --> shout.fa.salam:4:11
  |
4 |     سرچاپ word.upper()، word.lower()، word.طول()
  |           ^
  |
  = راهنما: نام فارسی آن 'بزرگ' است
  = اصلاح: نوشتن 'بزرگ'
```

The message says that the identifier `upper` must be Persian in a Persian file and that its Persian name is `بزرگ`. `salam check --fix shout.fa.salam` applies both fixes and the file then runs:

```salam
تابع ریشه:
    word := "Salam"
    سرچاپ word.بزرگ()، word.کوچک()، word.طول()
پایان
```

```output
SALAM salam 5
```

So the reliable workflow is three commands: `salam translate`, then `salam check --fix` on the output, then `salam check` to confirm there is nothing left. Because a Persian file must use Persian names for everything the language and the standard library define, and an English file English ones, the compiler catches every name the translator missed. A translation cannot silently produce a different program.

## When to use it, and when not to {#use-cases}

It works well for examples, exercises, small tools and whole projects that someone wants to read in the other language. It is the quickest way to see what a Persian keyword means: write a line in English, translate it, and look.

It is not a substitute for writing in a language. A program translated from English to Persian still has English identifiers and comments, so it reads as a hybrid until someone renames things. For teaching material, the better route is to translate once, then edit the names and comments by hand, then run both versions and compare their output. The [course](/learn/) exists in both languages, and its Persian lessons use Persian names throughout, which is the standard a hand-edited translation should aim for.

Related reading: [how a language is designed for Persian and Arabic](/blog/designing-a-language-for-persian-and-arabic/), and [the introduction to Salam](/blog/what-is-salam-programming-language/).

## Frequently asked questions {#faq}

### Does salam translate translate variable names and comments?

No. It changes only words that belong to the language and the standard library. Your own names, comments and string contents are copied byte for byte, because a mechanical translation of them would often be wrong and could break references from other files.

### Can I convert a whole Salam project from Persian to English?

Yes. `salam translate en src/` writes a translated copy of the directory to `src-en/`, translating every `.salam` file and copying everything else. Add `--rewrite` to change the files in place, ideally on a clean version-control checkout so you can review the diff.

### Is the translated program exactly the same program?

It compiles to the same program, and the project tests this by translating test programs and comparing their output. If the translator misses a name, the result does not compile until it is fixed, so it cannot quietly change behavior. Run `salam check --fix` after translating.

### Does salam translate work for languages other than Persian?

Not today. The compiler has two language packs, English (`en`) and Persian (`fa`), and the tool converts between those two.
