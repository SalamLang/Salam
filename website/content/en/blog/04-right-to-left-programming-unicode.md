---
title: Right-to-left programming: how Salam handles Unicode, Persian digits and bidirectional text
seo_title: Right-to-Left Programming: Unicode and Persian Digits in Salam
description: Right-to-left programming in practice: how Salam handles Persian source, Persian digits, UTF-8 strings as bytes and letters, and mixed-direction bidi text.
keywords: right-to-left programming, Unicode in programming languages, Persian digits, RTL code, bidirectional text, UTF-8 strings, Arabic-Indic digits, bidi algorithm, Persian programming language, zero-width non-joiner
category: language
summary: What it takes for a compiler, a standard library and an editor to handle code written right to left: source encoding, Persian digits, separators, strings as bytes and letters, the bidi algorithm and the traps that remain.
date: 2026-11-11
---

Right-to-left programming means writing source code in a script such as Persian, Arabic or Hebrew, where text runs from the right edge of the line to the left. **For a programming language it involves three separate problems: the compiler must read the script correctly (letters, digits, punctuation), the standard library must process text in that script correctly (length, slicing, normalization, direction), and the editor must display a line that mixes right-to-left words with left-to-right symbols in a readable order.** Salam, which can be written entirely in Persian, has had to answer all three.

This article goes through each layer with runnable programs: how the Salam lexer handles Persian source, how Persian digits work in literals and in output, why a string's length is not its number of letters, and what the `bidi`, `unicode` and `arabic` packages do for programs that handle right-to-left text. It ends with the problems that are not solved.

## What makes right-to-left code different {#what-rtl-means}

A Persian line of code is not simply an English line written backwards. A statement such as `اگر ک > ۱۰:` contains Persian words, a Persian digit sequence, an ASCII operator and an ASCII colon. Unicode stores these characters in the order they were typed (logical order) and leaves the screen order to the **Unicode Bidirectional Algorithm**, usually called the bidi algorithm. It assigns each character a direction class: Persian letters are strongly right to left, Latin letters strongly left to right, digits weakly directional, and spaces and most punctuation neutral, taking the direction of their surroundings.

The compiler only ever sees logical order, so for the compiler right-to-left code is a matter of character sets and rules. The human sees display order, so for the human it is a matter of rendering. Most confusion about "RTL programming" comes from mixing those two views.

## Source files and identifiers {#utf8-source}

Salam source files are UTF-8. The lexer treats every byte outside ASCII as part of an identifier, so names can be written in Persian, Arabic, or other scripts, and a Persian name may contain spaces or a zero-width non-joiner (ZWNJ, U+200C):

- `جمع ردیف`, with a space, and `جمع‌ردیف`, with a ZWNJ, are the same name;
- `جمعردیف`, glued together, is a different name, and the compiler suggests the spaced one when you mistype it;
- the Arabic letters `ي` (U+064A) and `ك` (U+0643) compare equal to the Persian `ی` (U+06CC) and `ک` (U+06A9) in names, so text typed on an Arabic keyboard layout matches;
- optional vowel marks (harakat, such as the kasra) are ignored in names, so `کِتاب` and `کتاب` are one identifier.

The lexer applies these rules as it reads each name: a ZWNJ becomes a space, the Arabic yeh and kaf become the Persian letters, and vowel marks are dropped. It is a small function, but without it a Persian programmer would be fighting their keyboard all day, and because it lives in the lexer, every tool built on the lexer (the formatter, the translator, the language server) follows the same rules. The reasons behind each of these rules are discussed in [designing a language for Persian and Arabic](/blog/designing-a-language-for-persian-and-arabic/).

## Persian digits and Arabic-Indic digits {#persian-digits}

Persian has its own digits, `۰۱۲۳۴۵۶۷۸۹` (U+06F0 to U+06F9), and Arabic uses a different set, `٠١٢٣٤٥٦٧٨٩` (U+0660 to U+0669). The two sets look alike for most values but are different characters. Salam accepts both in number literals, and both can be mixed with ASCII digits. The rules:

- `۱۲۵۰۰`, `٤٠٠` and `12500` are all valid integer literals, in Persian and English files alike.
- The decimal point is always the ASCII `.`: `۳.۵` is valid. The Persian decimal separator `٫` (U+066B) is not accepted, so `۱٫۵` is a compile error rather than a silently different number.
- Numbers are always **printed** with ASCII digits. Converting the output is an explicit call: `str.ToPersianDigits` (Persian name `به رقم فارسی`) and `str.ToEnglishDigits`.
- The string-to-integer conversion `to_int` accepts Persian digits, so input typed on a Persian keyboard parses.

Here is a Persian program that uses all of it:

```salam
واردسازی رشته

تابع ریشه:
    بها := ۱۲۵۰۰
    شمار := 3
    جمع := بها * شمار + ٤٠٠
    سرچاپ جمع
    سرچاپ رشته.به رقم فارسی(«جمع: » + جمع + « ریال»)
    سرچاپ "۲۵۰۰۰".به صحیح() + ۱
    سرچاپ ۳.۵ * ۲
پایان
```

```output
37900
جمع: ۳۷۹۰۰ ریال
25001
7
```

The first line mixes Persian, ASCII and Arabic-Indic digits in one expression. The second converts the finished text to Persian digits for display. Printing ASCII by default is a deliberate choice: a program's output often goes to logs, files, other programs and terminals, and ASCII digits are what all of those expect.

## Separators, quotes and comparison words {#separators}

Three more pieces of punctuation are specific to Persian files:

- **The Persian comma `،`** (U+060C) works everywhere the ASCII comma does: between arguments, parameters, array elements, struct fields, enum members and match patterns. A Persian typist does not have to switch keyboard layouts to write a call.
- **Guillemets `«...»`** are string literals in Persian files, with the same escapes as `"..."`. In an English file they are not delimiters.
- **Comparison words.** Persian files can write comparisons as words: `بزرگتر` (`>`), `کوچکتر` (`<`), `برابر` (`==`), `نابرابر` (`!=`), `بزرگتربرابر` and `کوچکتربرابر`. English has the same words (`gt`, `lt`, `eq`, `neq`, `gte`, `lte`).

The comparison words matter more than they seem. Unicode marks `<`, `>`, `(`, `)`, `[`, `]` and similar characters as **mirrored**: in a right-to-left run, a renderer draws the mirror image of the glyph, so that `(` still opens a parenthetical when reading right to left. That is correct for prose and confusing for code, because a reader can no longer be sure which way an angle bracket points. A word has no direction to mirror. `salam translate fa` writes comparisons as words, and `salam translate en` turns them back into symbols.

## Strings: bytes versus letters {#bytes-vs-letters}

A Salam `str` is a sequence of UTF-8 bytes. Persian letters take two bytes each, so the length of a Persian string in bytes is about twice its number of letters. Salam keeps the byte length as the default (`len`, `substr`, indexing) because it is what files, sockets and C libraries deal in, and provides letter-aware operations next to it:

```salam
import str

func main:
    word := "سلامِ"
    println "bytes:", word.len()
    println "code points:", word.char_count()
    println "graphemes:", str.GraphemeCount(word)
    each ch in "سلام":
        print "[" + ch + "]"
    end
    println ""
    println word.char_substr(0, 2), str.Reverse("سلام")
end
```

```output
bytes: 10
code points: 5
graphemes: 4
[س][ل][ا][م]
سل مالس
```

`سلامِ` ends with a kasra, a vowel mark that is its own code point but is drawn on top of the letter before it. So there are three honest answers to "how long is this word": 10 bytes, 5 code points, 4 user-perceived characters (graphemes). `each` over a string steps by code point, `char_substr` slices by code point, `str.Reverse` reverses code points, and `str.Graphemes` splits at the level a reader sees.

The trap is the byte functions: `substr` with byte offsets can cut a Persian letter in half and produce invalid UTF-8. When you slice text that people wrote, use the `char_` methods or the [str package](/std/str/)'s code point functions. The [strings lesson](/learn/strings/) covers the full set.

## Mixed-direction text and the bidi package {#bidi}

Programs that display text, not just the editor, run into the bidi algorithm. A chat client must decide whether each message is aligned left or right; a program that draws text into an image or a terminal must reorder mixed lines itself, because nothing else will. Salam's standard library has packages for each step:

- [`unicode`](/std/unicode/) gives the bidi class, script and category of any code point, from the Unicode 16 database;
- [`bidi`](/std/bidi/) runs the complete bidirectional algorithm: paragraph level, embedding levels per character, visual order and a reordered string;
- [`arabic`](/std/arabic/) normalizes Arabic-script text for comparison and search.

The most common question is "which way does this line run?", and the usual answer, the one HTML's `dir="auto"` uses, is the direction of the first strong character:

```salam
import bidi

func main:
    lines := [
        "func main:",
        "تابع ریشه:",
        "1405 سال خوبی است",
        "Salam یعنی سلام",
        "x := 42",
    ]
    each line in lines:
        side := bidi.ParagraphLevel(line, bidi.DirAuto) == 1 ? "rtl" : "ltr"
        println side, "|", line
    end
end
```

```output
ltr | func main:
rtl | تابع ریشه:
rtl | 1405 سال خوبی است
ltr | Salam یعنی سلام
ltr | x := 42
```

The third line starts with digits, which are weak, so the Persian word after them decides. The fourth starts with a Latin word and is therefore left to right, even though most of it is Persian. That is exactly how a browser would lay out each line with `dir="auto"`, and it is why a line of Persian code that begins with an English identifier can suddenly display in the wrong direction.

The `bidi` package documents what it does not do, which is useful as a checklist: it does not mirror brackets in its reordered output (you swap them with `unicode.PairedBracket`), it does not shape Arabic letters, and it does not break lines.

## Normalizing Persian and Arabic text {#normalization}

Right-to-left text arrives in more than one spelling. The same Persian word can be typed with the Arabic yeh and kaf, with or without a ZWNJ, with a tatweel stretching it, or with vowel marks. The compiler folds yeh and kaf in identifiers; for data, the `arabic` package builds a comparison key:

```salam
import arabic

func main:
    stored := "برنامه‌نویسی کاربردی"
    typed := ["برنامه‌نويسي كاربردي", "برنامه‌نویسی کاربردی", "برنامهٔ نویسی"]
    key := arabic.SearchKey(stored)
    each t in typed:
        println arabic.SearchKey(t) == key, "|", t
    end
end
```

```output
true | برنامه‌نويسي كاربردي
true | برنامه‌نویسی کاربردی
false | برنامهٔ نویسی
```

The first query is typed with Arabic letters and still matches. The third is a different phrase and does not. Store the key next to the original text, not instead of it; it throws information away on purpose.

## How editors display right-to-left code {#editors}

The compiler does not care how a line looks, but the person reading it does. There are two approaches to displaying Persian code:

1. **Right-aligned, right-to-left paragraphs.** The whole editor runs right to left, and each line is a right-to-left paragraph. Persian keywords read naturally; embedded English names and numbers are laid out left to right inside the line by the bidi algorithm. This is what the [Salam online editor](https://editor.salamlang.ir) does when its language is set to Persian: the page, the code area and the highlighting layer all switch to `dir="rtl"`. The code blocks on this site do the same for Salam code that contains Persian.
2. **Left-aligned lines with bidi inside each line.** Many desktop code editors, VS Code among them, keep every line left-aligned and apply the bidi algorithm within it. Persian code is readable this way, but lines that start with a Latin identifier or a symbol can look scrambled.

The Salam plugins for VS Code and ten other editors highlight both the English and the Persian keyword sets, which are generated from the compiler's own tables; see the [tools page](/tools/). How the text is laid out is up to each editor.

## What is not solved yet {#limits}

- **Invisible bidi controls.** Unicode has control characters (such as U+202E, RIGHT-TO-LEFT OVERRIDE) that change display order without being visible. The "Trojan Source" attack published in 2021 (CVE-2021-42574) used them inside comments and strings to make code display differently from how it compiles. In Salam a stray control character in code is an unknown identifier and fails to compile, but inside comments and string literals the current compiler accepts them without a warning. An editor that shows such characters is the defense today.
- **Output digits are ASCII.** Correct for machines, but every program that shows numbers to Persian readers has to convert them.
- **The decimal separator.** `٫` is rejected, which avoids ambiguity but surprises people who type it.
- **Desktop editor support varies.** A right-to-left code editing experience depends on the editor, and most were designed for left-to-right code.

For the language side of the story, read [one language, two scripts](/blog/one-language-two-scripts/); for the tool that converts between the two spellings, [inside salam translate](/blog/salam-translate-persian-english-code/).

## Frequently asked questions {#faq}

### Can I use Persian digits in Salam code?

Yes. Persian digits (`۰` to `۹`) and Arabic-Indic digits (`٠` to `٩`) are valid in number literals and can be mixed with ASCII digits, in Persian and English files. The decimal point must be the ASCII `.`, and numbers are printed with ASCII digits unless you convert them with `str.ToPersianDigits`.

### Why does a Persian string's length come out twice as long?

`len` counts UTF-8 bytes, and Persian letters take two bytes each. Use `char_count` for the number of code points and `str.GraphemeCount` for the number of characters a reader sees.

### How do I know if a line of text is right to left?

Find its first strong character, a letter with its own direction. `bidi.ParagraphLevel(line, bidi.DirAuto)` returns 1 for right to left and 0 for left to right, following the same rule as HTML's `dir="auto"`.

### Which editors support right-to-left Salam code?

The online editor switches to a full right-to-left layout for Persian. Desktop editors with the Salam plugin highlight Persian keywords and show Persian code, but how they lay out mixed lines depends on the editor itself.
