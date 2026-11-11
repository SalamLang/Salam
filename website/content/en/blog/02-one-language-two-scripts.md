---
title: One language, two scripts: how Salam lets you program in Persian and English with the same compiler
seo_title: Programming in Persian and English: Salam's Bilingual Design
description: Programming in Persian and English with Salam: one compiler, one standard library, error messages in both languages, and twin programs with real output.
keywords: programming in Persian, bilingual programming language, Persian programming language, Persian code, write code in Persian, Salam Persian, Farsi programming language, multilingual compiler, Persian error messages, salam translate
category: language
summary: Salam is one language with two spellings. This article shows how a Persian file and an English file reach the same compiler, share one standard library, report errors in the reader's language and call each other's packages.
date: 2026-11-11
---

Programming in Persian with Salam means writing a complete program, from the entry function to the standard library calls and the error messages you read back, in Persian script, and getting exactly the same native executable an English program would produce. **Salam is a bilingual programming language: every keyword, built-in type, built-in method, standard library package and compiler diagnostic has an English spelling and a Persian spelling, and both spellings are the same language underneath.** There is no Persian dialect, no separate compiler and no preprocessor that rewrites Persian into English text before compiling.

This article explains how that works in practice: how the compiler tells the two apart, what exactly has a Persian name, how errors are reported, how packages written in one language are used from the other, and where the design stops.

## What "bilingual programming language" means in Salam {#what-bilingual-means}

Many projects that offer "programming in your native language" translate the keywords and stop. The program still calls `printf` or `len`, the library documentation is in English and the first compile error arrives in English. A beginner who reads Persian comfortably but English slowly is back to translating in their head after the first five minutes.

Salam takes the opposite approach. The English and Persian forms are two **language packs** for one grammar. The lexer reads a file with one pack or the other, turns the words into the same tokens, and from that point on the parser, the type checker, the code generators and the runtime see no difference. A Persian `اگر` and an English `if` become the same token; a Persian `رشته` and an English `str` are the same type.

The practical consequences:

- one compiler binary, `salam`, builds both;
- one standard library serves both, with Persian names declared next to the English ones;
- every diagnostic is printed in the language of the file;
- a tool, `salam translate`, converts a file from one spelling to the other;
- an English program can use a package written in Persian, and the other way round.

## The same program in Persian and in English {#twin-programs}

Here is a small program in English: an enum, a struct, a function with a two-word name, an array of struct literals, an `each` loop and a `match` expression.

```salam
enum Box: Small, Medium, Large end

struct Order:
    pub item: str
    pub price: int
    pub count: int
end

func line total(o: Order): int:
    ret o.price * o.count
end

func box for(total: int): Box:
    if total >= 500:
        ret Box.Large
    else total >= 200:
        ret Box.Medium
    end
    ret Box.Small
end

func main:
    orders := [
        Order { item = "Tea", price = 120, count = 2 },
        Order { item = "Bread", price = 45, count = 3 },
        Order { item = "Dates", price = 300, count = 1 },
    ]
    mut sum := 0
    each o in orders:
        t := line total(o)
        println o.item, t
        sum += t
    end
    packing := match box for(sum):
        Small: "a small box" end
        Medium: "a medium box" end
        Large: "a large box" end
    end
    println "Total:", sum, "in", packing
end
```

```output
Tea 240
Bread 135
Dates 300
Total: 675 in a large box
```

And the same program written by a Persian speaker. The structure is identical; only the words change. Note the Persian digits in the number literals, the Persian comma `،` between arguments and fields, and the `«...»` quotes, which are string literals in Persian files.

```salam
جداشمار جعبه: کوچک، میانه، بزرگ پایان

ساختار سفارش:
    همگانی کالا: رشته
    همگانی بها: صحیح
    همگانی شمار: صحیح
پایان

تابع جمع ردیف(س: سفارش): صحیح:
    برگشت س.بها * س.شمار
پایان

تابع جعبه برای(جمع: صحیح): جعبه:
    اگر جمع >= ۵۰۰:
        برگشت جعبه.بزرگ
    وگرنه جمع >= ۲۰۰:
        برگشت جعبه.میانه
    پایان
    برگشت جعبه.کوچک
پایان

تابع ریشه:
    سفارش‌ها := [
        سفارش { کالا = «چای»، بها = ۱۲۰، شمار = ۲ }،
        سفارش { کالا = «نان»، بها = ۴۵، شمار = ۳ }،
        سفارش { کالا = «خرما»، بها = ۳۰۰، شمار = ۱ }،
    ]
    ناپایا کل := ۰
    هر س در سفارش‌ها:
        ج := جمع ردیف(س)
        سرچاپ س.کالا، ج
        کل += ج
    پایان
    ظرف := همخوان جعبه برای(کل):
        کوچک: «جعبه‌ی کوچک» پایان
        میانه: «جعبه‌ی میانه» پایان
        بزرگ: «جعبه‌ی بزرگ» پایان
    پایان
    سرچاپ «جمع:»، کل، «در»، ظرف
پایان
```

```output
چای 240
نان 135
خرما 300
جمع: 675 در جعبه‌ی بزرگ
```

A few word pairs to read the Persian version: `تابع` is `func`, `ریشه` (root) is the entry function `main`, `برگشت` is `ret`, `ناپایا` is `mut`, `هر ... در` is `each ... in`, `همخوان` is `match`, `همگانی` is `pub`, `سرچاپ` is `println` and `پایان` is `end`. The numbers in the output are printed with ASCII digits in both versions; Salam prints numbers the same way whatever the source language, and the `str` package converts them to Persian digits when you want that (see [right-to-left programming in Salam](/blog/right-to-left-programming-unicode/)).

## How the compiler knows which language a file is in {#language-detection}

There is no project setting and no file extension for Persian. The compiler decides per file, in this order:

1. **A language line.** If one of the first eight lines is the comment `// lang: en` or `// زبان: فارسی`, that decides it. A mixed form such as `// lang: fa` is a compile error, so the marker cannot be half-translated by accident.
2. **Keyword counting.** Otherwise the lexer tokenizes the file with each language pack and counts how many keywords each one recognizes. The pack that finds more keywords wins. A Persian file full of `تابع` and `پایان` has almost no English keywords, so in practice the guess is never close.
3. **An explicit flag.** `salam build app.salam --lang=fa` forces the Persian pack.

Detection is per file, which is what makes mixing possible: a project can contain English and Persian files side by side, and each is read with its own pack.

`salam new` follows the same idea. `salam new shop --lang=fa` creates a project whose `main.salam` starts with `تابع ریشه:` and prints `"سلام، دنیا!"`, and the messages that `salam new` prints are themselves in Persian.

## What has a Persian name {#what-is-translated}

The table lists everything that changes with the language of the file, and what does not.

| Part of the language | English | Persian |
|---|---|---|
| Keywords | `func`, `if`, `else`, `until`, `repeat`, `each`, `match`, `end` | `تابع`, `اگر`, `وگرنه`, `تا`, `تکرار`, `هر`, `همخوان`, `پایان` |
| Entry function | `main` | `ریشه` |
| Literals | `true`, `false`, `null`, `this` | `درست`, `نادرست`, `پوچ`, `این` |
| Primitive types | `int`, `str`, `f64`, `bool` | `صحیح`, `رشته`, `اعشار۶۴`, `منطقی` |
| Collections | `Vector<T>`, `HashMap<K, V>` | `وکتور<T>`, `نگاشت<K, V>` |
| Built-in methods | `push`, `len`, `free`, `split` | `بیفزا`, `طول`, `آزادکن`, `بشکاف` |
| Built-in functions | `len`, `sizeof`, `spawn` | `طول`, `اندازه‌گونه`, `نخ‌ساز` |
| Logic and comparison words | `and`, `or`, `not`, `gt`, `eq` | `و`, `یا`, `وارونه`, `بزرگتر`, `برابر` |
| Standard library | `str.Split`, `math.Sqrt` | `رشته.تفکیک کردن`, `ریاضی.جذر` |
| Compile-time constants | `SALAM_OS_LINUX` | `سلام سیستم لینوکس` |
| Diagnostics | `error[E001]: unknown identifier` | `خطا[خطا۰۰۱]: شناسه‌ی ناشناخته` |

What stays the same in both: operators and punctuation (`+`, `:=`, `==`, `[ ]`), the names you choose for your own variables, functions and types, string contents, and the names of C functions declared in an `extern` block, because those are names in a C library.

The standard library is the part that takes the most work, and it is complete. Each of the 194 package directories in `std/` declares a Persian name next to its English one, and the public functions carry the same pair of tags:

```salam fragment
@en "ToPersianDigits"
@fa "به رقم فارسی"
pub func ToPersianDigits(s: str): str: ret _convert_digits(s, _DIGIT_PERSIAN_BASE) end
```

A Persian program imports a package by its Persian name and calls the Persian names. This one uses `رشته` (the `str` package) to split a sentence and find its longest word:

```salam
واردسازی رشته

تابع بلندترین(واژه‌ها: وکتور<رشته>): رشته:
    ناپایا بهترین := ""
    هر و۱ در واژه‌ها:
        اگر رشته.طول(و۱) > رشته.طول(بهترین):
            بهترین = و۱
        پایان
    پایان
    برگشت بهترین
پایان

تابع ریشه:
    جمله := «برنامه‌نویسی به زبان مادری»
    واژه‌ها := رشته.تفکیک کردن(جمله، " ")
    دیرکن واژه‌ها.آزادکن()
    سرچاپ «شمار واژه‌ها:»، واژه‌ها.طول()
    سرچاپ «بلندترین:»، بلندترین(واژه‌ها)
    سرچاپ رشته.بزرگ("salam")
پایان
```

```output
شمار واژه‌ها: 4
بلندترین: برنامه‌نویسی
SALAM
```

`رشته.تفکیک کردن` is `str.Split`, `رشته.بزرگ` is `str.Upper` and `دیرکن` is `defer`. The library code that runs is the same code an English program calls.

## Error messages in both languages {#diagnostics}

A compiler's error messages are the part of a language that a beginner reads most, so Salam translates them too. The compiler carries 1,299 messages, each with a Persian version, and picks the language from the file. Here is the same mistake, an undefined variable, in an English program:

```text
error[E001]: unknown identifier 'y'; did you mean 'x'?
 --> main.salam:3:13
  |
3 |     println y
  |             ^
  |
  = fix: Change to 'x'
```

And in a Persian program, where even the error code is written in Persian digits:

```text
خطا[خطا۰۰۱]: شناسه‌ی ناشناخته 'کتابها'؛ منظورتان 'کتاب ها' بود؟
 --> main.salam:5:11
  |
5 |     سرچاپ کتابها
  |           ^
  |
  = اصلاح: تغییر به 'کتاب ها'
```

The error codes are the same in both (`E001` is `خطا۰۰۱`), so a Persian speaker can search for the English documentation of a code and an English-speaking maintainer can read a bug report written in Persian. `--error-lang=fa` prints Persian diagnostics for an English file, which is useful in a classroom where the teacher's examples are in English.

The diagnostics also know about the two languages. Calling `رشته.Len` in a Persian file is an error that says the identifier must be Persian and gives the Persian name, `طول`, as the fix; calling `str.طول` in an English file gets the mirror message. `salam check --fix` applies those fixes in place.

## Mixing packages written in either language {#mixing-packages}

Because the language is chosen per file, a project can mix the two. The standard library is the everyday example: it is written in English, and every Persian program uses it through its `@fa` names. Your own packages work the same way. Here is a small library written in Persian, `abzar.salam`, which gives its public function both names:

```salam fragment
بسته ابزار

@en "Quadruple"
@fa "چهاربرابر"
همگانی تابع چهاربرابر(ع: صحیح): صحیح:
    برگشت ع * ۴
پایان
```

An English program includes it under an alias of its choice and calls the English name:

```salam fragment
include tools "abzar.salam"

func main:
    println tools.Quadruple(5)
end
```

That program prints `20`. The rule inside one file is consistent: names that belong to the language or to a package with two spellings must be written in the file's language, while the names you invent yourself can be in any script. An English file may contain a variable called `قیمت`, and a Persian file may call a function named `parse`; the compiler treats both as ordinary identifiers.

The [packages lesson](/learn/packages/) covers `package`, `import` and `include` in detail.

## Converting a file: salam translate {#translate}

`salam translate fa app.salam` writes `app.fa.salam` next to the original, with the keywords, types, entry function, built-in methods and standard library names in Persian, and `salam translate en` goes back. It works on the token stream, so comments, strings, formatting and your own identifiers are left untouched, and a round trip returns the original code unchanged, apart from a language line the tool adds at the top. The tool has its own article: [inside salam translate](/blog/salam-translate-persian-english-code/).

## Persian-specific details {#persian-details}

A few things exist only because Persian is written differently from English:

- **Digits.** Persian `۰۱۲۳۴۵۶۷۸۹` and Arabic-Indic `٠١٢٣٤٥٦٧٨٩` digits are accepted in number literals and can be mixed with ASCII digits. The decimal point is always `.`; the Persian decimal separator `٫` is not accepted.
- **The Persian comma** `،` works everywhere `,` does: arguments, parameters, array and struct literals, enum members and match patterns.
- **Guillemets.** `«...»` is a string literal in a Persian file, equivalent to `"..."`.
- **Multi-word names.** Identifiers may contain spaces or the zero-width non-joiner (ZWNJ), and the two are interchangeable: `کتاب ها` and `کتاب‌ها` are the same name. The Arabic letters `ي` and `ك` are treated as the Persian `ی` and `ک`, so code typed on an Arabic keyboard layout still matches.

The reasons behind these rules are the subject of [designing a language for Persian and Arabic](/blog/designing-a-language-for-persian-and-arabic/).

## Where the bilingual design stops {#limits}

Honest limits, as of the current release:

- **Two languages only.** The compiler has exactly two language packs, English and Persian. There is no Arabic, Urdu or Turkish keyword set, even though the lexer accepts Arabic letter variants and digits.
- **Output is not localized automatically.** Numbers and booleans print in ASCII (`12`, `true`) in both languages. Persian digits in output are a library call away, not a default.
- **The translator does not cover everything.** A few newer built-in string methods are missing from its table; the compiler then reports the leftover English name with the Persian fix, as shown in the translate article.
- **Most published code is English.** The compiler, the standard library and most examples on GitHub are written in English. A Persian learner will meet English code sooner or later, which is why the translator exists.
- **Salam is pre-1.0.** The language still changes between releases, in both spellings.

The [introduction to Salam](/blog/what-is-salam-programming-language/) covers the rest of the language, and the [free course](/learn/) teaches it from the first program.

## Frequently asked questions {#faq}

### Can I write a whole program in Persian with Salam?

Yes. Keywords, types, the entry function (`ریشه`), built-in methods, every standard library package and the compiler's messages all have Persian names. A Persian file needs no English words except the names of C functions you declare yourself.

### Is the Persian version slower or limited compared with English?

No. Both spellings are turned into the same tokens by the lexer, so the parser, the type checker and the code generators never see which language you wrote in. The executables are the same.

### Do I need to choose one language for a whole project?

No. The language is detected per file, so one project can contain English and Persian files. A package written in Persian can be used from an English program and the other way round, as long as its public names carry both spellings.

### Is Salam an Arabic programming language as well?

Not in the same sense. There is no Arabic keyword set. Persian and Arabic share a script, and Salam accepts Arabic-Indic digits and the Arabic forms of yeh and kaf in Persian names, and its standard library has an `arabic` package for normalizing Arabic-script text.
