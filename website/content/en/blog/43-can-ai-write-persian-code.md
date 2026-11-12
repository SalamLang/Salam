---
title: Can AI write code in Persian? Generating and validating Persian Salam programs
seo_title: Can AI Write Code in Persian? Generating Persian Salam Programs
description: Persian Salam is a full programming language, but AI models have seen little of it. Where code generation goes wrong and how the compiler and tests catch it.
keywords: AI Persian code, AI code generation Persian, Persian programming language, generate Persian code, Persian Salam, programming in Persian with AI, Farsi code generation, validate generated code, salam translate, Persian compiler errors
category: future
summary: Writing Persian code is a harder test for a code generator than writing English code, because there is so little of it to learn from. What goes wrong, what the Salam compiler catches in Persian, and three practical ways to get Persian programs that compile and do what you meant.
date: 2026-11-11
---

Yes, AI tools can write code in Persian, but only reliably when something checks the result. **Salam is a programming language whose keywords, types, standard library and compiler messages all exist in Persian, so a Persian Salam program is real code that compiles to a native executable, not a translated comment.** That makes it a demanding test for code generators: Persian source code is rare in any training data, the script runs right to left, and small details such as a zero-width non-joiner or an Arabic letter variant change meaning. What makes the task workable is that the Salam compiler checks Persian code as strictly as English code and explains its errors in Persian.

This article shows what Persian Salam looks like, where generated Persian code tends to go wrong, how the compiler and the test runner catch those mistakes, and three practical strategies for getting Persian programs you can trust.

## What Persian code looks like {#persian-code}

A Persian Salam program uses Persian keywords, Persian names and, if you like, Persian digits and the Persian comma. The entry function is `ریشه` ("root"), blocks end with `پایان` ("end"), `سرچاپ` prints a line, and text between the guillemets `«` and `»` is a string, like text between double quotes:

```salam
ساختار کالا:
    همگانی نام: رشته
    همگانی تعداد: صحیح
    همگانی قیمت: صحیح
پایان

تابع جمع سبد(سبد: وکتور<کالا>): صحیح:
    ناپایا جمع := ۰
    هر ک در سبد:
        جمع += ک.تعداد * ک.قیمت
    پایان
    برگشت جمع
پایان

تابع ریشه:
    ناپایا سبد خرید := وکتور {} برگردان وکتور<کالا>
    دیرکن سبد خرید.آزادکن()
    سبد خرید.بیفزا(کالا { نام = «نان»، تعداد = ۲، قیمت = ۴۵ })
    سبد خرید.بیفزا(کالا { نام = «پنیر»، تعداد = ۱، قیمت = ۱۲۰ })
    سبد خرید.بیفزا(کالا { نام = «چای»، تعداد = ۳، قیمت = ۶۰ })
    کل := جمع سبد(سبد خرید)
    سرچاپ «جمع کل:»، کل
    اگر کل > ۳۰۰:
        سرچاپ «ارسال رایگان است»
    وگرنه:
        سرچاپ «هزینه‌ی ارسال اضافه می‌شود»
    پایان
پایان
```

```output
جمع کل: 390
ارسال رایگان است
```

The function name `جمع سبد` ("basket total") is two words, and so is the variable `سبد خرید` ("shopping basket"). Salam allows spaces inside names because Persian writes many single concepts as two words. Numbers in the source are Persian digits; printed numbers come out in ASCII digits, a detail that matters when you check output. The article [one language, two scripts](/blog/one-language-two-scripts/) covers the bilingual design in full.

## Why Persian code is harder for AI models {#why-harder}

Code generators learn from public code. There is far less public code in any non-English programming language than in Python or JavaScript, and Salam's Persian spelling is younger still, so a model has seen very little of it. Several properties of Persian make the gaps easy to fall into:

- **Two-meaning keywords.** `تا` is `until` at the start of a statement and `to` inside a `تکرار` (repeat) header; `هر` is `each` at the start of a statement and the step (`by`) inside `تکرار`. Both are natural Persian, and both are easy to misplace.
- **Built-in names must be Persian.** In a Persian file, `len` is an error; the built-in is `طول`. A model that mixes English built-ins into Persian code is a common failure.
- **Invisible characters.** A zero-width non-joiner (ZWNJ, U+200C) is part of correct Persian spelling, as in `کتاب‌ها`. Inside a name, Salam treats a ZWNJ and a space as the same, but a keyword written with its parts glued together, such as `نادرستسرچاپ` instead of `نادرست‌سرچاپ`, is an unknown identifier.
- **Arabic letter variants.** Persian and Arabic encode yeh and kaf with different code points (`ی`/`ي`, `ک`/`ك`). Text produced on an Arabic keyboard, or by a model trained on Arabic text, mixes them.
- **Reserved short words.** `و` ("and") is a keyword, so it can never be a name.
- **Number formats.** The Persian decimal separator `٫` is not accepted; a decimal point is always `.`, as in `۳.۱۴`.
- **The entry point.** The entry function is `ریشه`. `اصلی`, the literal translation of "main", is an ordinary function name.

None of this is exotic for a Persian speaker, but each item is a place where a model trained mostly on English code can produce something plausible and wrong. The article on [designing a language for Persian and Arabic](/blog/designing-a-language-for-persian-and-arabic/) explains why the language made these choices.

## The compiler as the judge, in Persian {#compiler-in-persian}

The compiler detects the language of a file from its keywords and reports errors in that language, with the same error codes as English (the codes are written with Persian digits). A short program that used the English `len` and declared a variable `ناپایا` (mutable) without ever changing it:

```text
تابع ریشه:
    ناپایا اعداد := [۳، ۵، ۸]
    سرچاپ len(اعداد)
پایان
```

`salam check` reported both problems, each with a fix:

```text
خطا[خطا۰۰۱]: شناسه‌ی 'len' در یک فایل فارسی باید فارسی باشد
 --> bad.salam:3:11
  |
3 |     سرچاپ len(اعداد)
  |           ^
  |
  = راهنما: نام فارسی آن 'طول' است
  = اصلاح: نوشتن 'طول'
خطا[خطا۰۶۹]: متغیر 'اعداد' با کلیدواژه‌ی 'ناپایا' تعریف شده اما هرگز تغییر نکرده است؛ 'ناپایا' را حذف کنید یا آن را 'پایا' تعریف کنید
 --> bad.salam:2:5
  |
2 |     ناپایا اعداد := [۳، ۵، ۸]
  |     ^
  |
  = اصلاح: حذف 'ناپایا'
```

The first says that the identifier `len` must be Persian in a Persian file and that its Persian name is `طول`; the second is error 069, a mutable variable that never changes. `salam check --fix` applies such safe fixes in place. For a reviewer who reads English better, `--error-lang=en` prints the same diagnostics in English for a Persian file, and `--error-lang=fa` does the reverse. A file that names its entry function `اصلی` is caught too: building it stops with "no entry point: define a 'ریشه' function".

Normalization works in the other direction. The compiler treats Arabic and Persian letter variants, and a space or a ZWNJ inside a name, as the same identifier, so mixed input that a model might produce still resolves. String contents are left exactly as written:

```salam
تابع ریشه:
    تعداد کتاب‌ها := ۱۲
    سرچاپ تعداد كتاب ها + ۳
    سرچاپ «یک» == «يک»
پایان
```

```output
15
false
```

The name was declared with Persian `ک` and a ZWNJ and used with Arabic `ك` and a space, and the program still found it. The two strings differ only in their yeh, and the comparison is `false`, which is correct: text data is not rewritten behind your back. The [Unicode and right-to-left article](/blog/right-to-left-programming-unicode/) goes deeper into how the lexer handles this.

## Strategy 1: generate in English, then translate {#translate}

The most reliable way to get a Persian program today is to let the generator work where it is strongest and translate deterministically. `salam translate fa` rewrites English Salam into Persian: keywords, `true`, `false`, `null` and `this`, the entry function, primitive type names and the built-in methods of vectors, maps and files. Everything else, including your own names, comments and strings, is left byte for byte. Given this English program:

```salam
func average(scores: Vector<int>): f64:
    if scores.len() == 0:
        ret 0.0
    end
    mut sum := 0
    each s in scores:
        sum += s
    end
    ret (sum as f64) / (scores.len() as f64)
end

func main:
    mut scores := Vector {} as Vector<int>
    defer scores.Free()
    scores.push(18)
    scores.push(15)
    scores.push(20)
    println "average:", average(scores)
end
```

```output
average: 17.666666666666668
```

`salam translate fa avg.salam` wrote `avg.fa.salam`, which prints exactly the same line:

```salam
// زبان: فارسی
تابع average(scores: وکتور<صحیح>): اعشار۶۴:
    اگر scores.طول() برابر 0:
        برگشت 0.0
    پایان
    ناپایا sum := 0
    هر s در scores:
        sum += s
    پایان
    برگشت (sum برگردان اعشار۶۴) / (scores.طول() برگردان اعشار۶۴)
پایان

تابع ریشه:
    ناپایا scores := وکتور {} برگردان وکتور<صحیح>
    دیرکن scores.آزادکن()
    scores.بیفزا(18)
    scores.بیفزا(15)
    scores.بیفزا(20)
    سرچاپ "average:"، average(scores)
پایان
```

```output
average: 17.666666666666668
```

The structure is now Persian, with `برابر` for `==` and the `// زبان: فارسی` language header, but the names `average`, `scores` and `sum` are still English. Renaming them is the part that needs a human or a model, and it is a much smaller, safer task than writing the whole program in Persian. Translation also works the other way: `salam translate en` on the shopping example above produced English keywords around the original Persian names, and the result printed the same two lines. The [salam translate article](/blog/salam-translate-persian-english-code/) covers the options and limits.

## Strategy 2: write Persian directly, with the reference in context {#direct}

When a model writes Persian directly, the best help is the language reference. `SKILL.md` in the Salam repository has a whole Persian part: every keyword with its English counterpart, the Persian names of types, built-in methods and standard library packages, a Persian syntax guide, a list of traps specific to Persian, and complete Persian programs. It is written to be loaded into a model's context.

The MCP server adds tools for the same job: `salam_keywords` returns the English and Persian keyword table, `salam_translate` converts a program in either direction, `salam_stdlib_symbols` shows a package's API, and `salam_check` reports diagnostics the model can act on. The article on [programming languages in the AI era](/blog/programming-languages-in-the-ai-era/) explains how this check-and-fix loop works and why it matters for a young language.

## Strategy 3: specify the behavior with Persian tests {#tests}

A compiler can only say whether a program is well formed. Whether it does the right thing is a question for tests, and Salam's test runner speaks Persian too. Test directives have Persian spellings, and numbers in them may use Persian digits. Adding two lines to the top of the shopping program turns it into a test:

```text
// انتظار-خط: جمع کل: 390
// انتظار-کد: ۰
```

`انتظار-خط` ("expect line") requires an output line equal to the text, and `انتظار-کد` ("expect code") the exit code. `salam test` passed it; after changing the expected total to 400, it failed and printed the expected line next to the real output. Writing the expected behavior before generating the code, in the same language as the code, is the strongest guard against a program that compiles and is still wrong.

## Where Salam AI fits {#salam-ai}

[Salam AI](/ai/), the project's assistant, has a Persian, right-to-left interface and checks the Salam code it writes with the Salam compiler. It is the natural place to ask for Persian Salam, and the [Salam AI and Salam Agent article](/blog/salam-ai-and-salam-agent/) describes it and the desktop agent in detail. The same strategies apply there: keep tests, read the output, and use translation when you want the most predictable result.

## What we cannot tell you {#limits}

We publish no accuracy figures for any model writing Persian Salam. Such numbers depend on the model, the prompt and the task, and without a public, repeatable evaluation they would be guesses. What we can say is structural: the compiler rejects malformed Persian code with Persian explanations, deterministic translation removes most of the risk from the keywords, and tests decide the rest.

There is also a quality question no tool answers. Good Persian names, ones that read naturally and follow the project's conventions (the Salam standard library uses spaces between the words of Persian names), need a fluent reader. Generated names can be grammatical and still awkward. For teaching, that review is part of the value: a student who learns with the Persian [course](/learn/) can read and correct generated code in their own language, which is the point of [programming in your native language](/blog/programming-in-your-native-language/).

## Frequently asked questions {#faq}

### Can general AI assistants write Persian Salam code?

General assistants have seen very little Salam and less Persian Salam, so they often mix in English built-ins or syntax from other languages. With the compiler's diagnostics, the `SKILL.md` reference or the MCP server in the loop, those mistakes are found and fixed instead of shipped.

### Are compiler errors shown in Persian?

Yes. A Persian file gets Persian diagnostics with the same error codes as English, and `--error-lang` switches the language of the messages for any file.

### Is it better to generate Persian code directly or to translate?

For predictable results, generate or write English Salam and run `salam translate fa`, then rename the identifiers. Writing Persian directly works best with the Persian reference loaded and the compiler checking every draft.

### Does Persian code run slower than English code?

No. Both spellings go through the same compiler and produce the same executable; the shopping program translated to English printed exactly the same output.
