# Strict values: only numbers and sizes are written bare

Status: accepted design (2026-11-23).

## The rule

In the page language (`page:`, `component`, style attributes, state
prefixed attributes, enum-like HTML attributes), a value is one of:

- a number or a size, written bare: `8`, `1.5`, `50%`, `24px`, `1fr`, `200ms`,
  `45deg`, lists of them (`padding = 10, 14`, `padding = 64, 5vw`);
- `true` or `false`, written bare (they are Salam keywords, never names);
- a string: `"none"`, `"#d6d9e0"`, `"red"`, a raw string, or any
  expression that produces one;
- a variable, constant or any other Salam expression.

Everything else is not written bare. A bare word (`none`, `bold`, `flex`,
`center`, `solid`, `row`, `auto`, `inherit`, `pointer`, `hidden`, `red`,
`transparent`, `current`, `email`, `rtl`) and a bare hex colour (`#fff`,
`#d6d9e0`) is a string value in disguise. After this change the source says
so: the word is quoted. A bare identifier then always means a Salam name.

```salam
text decoration = "none"
color = "#d6d9e0"
border = 1, "solid", "#ccc"
type = "email"
padding = 10, 14
```

## Why

Today `text decoration = none` and `text decoration = none_` look alike but
the first is a keyword and the second a variable. A reader cannot tell
without knowing every keyword table (7000 lines of them). A constant that
is named like a keyword silently changes meaning (W131 already exists for
that). Quoting makes the value kind visible: sizes and numbers are obviously
numbers, strings are obviously strings, bare identifiers are obviously
variables.

## What is accepted and what is rejected

| form                                         | today   | after phase 1 | final (strict) |
| -------------------------------------------- | ------- | ------------- | -------------- |
| `padding = 10, 14`, `width = 50%`, `gap = 8` | ok      | ok            | ok             |
| `width = 24px`, `1.5em`, `1fr`, `200ms`      | ok      | ok            | ok             |
| `opacity = 0.5`, `-4`                        | ok      | ok            | ok             |
| `hidden = true`                              | ok      | ok            | ok             |
| `text decoration = "none"`                   | ok      | ok            | ok             |
| `color = "#d6d9e0"`, `color = "red"`         | ok      | ok            | ok             |
| `color = raw "..."` / raw string             | ok      | ok            | ok             |
| `color = c` (variable, constant)             | ok      | ok            | ok             |
| `text decoration = none`                     | ok      | W299          | E299           |
| `color = red`, `color = transparent`         | ok      | W299          | E299           |
| `color = #fff`, `color = #d6d9e0`            | ok      | W299          | E299           |
| `type = email`, `dir = rtl`                  | ok      | W299          | E299           |
| `border = 1, solid, #ccc`                    | ok      | W299 x2       | E299           |
| `color = on ? red : blue`                    | ok      | W299 x2       | E299           |
| `shadow = [0, 4, 8, black]`                  | ok      | W299          | E299           |

Lists: each item is checked on its own, so `border = 1, "solid", "#ccc"` is
the way to write a list. A group `[...]` is the same. In a ternary both
branches are checked.

## Function calls

A call is not confusable with a variable: `rgb(51, 102, 170)`,
`hsl(210, 50%, 40%)`, `linear(90, red, blue)`, `repeat(3, 1fr)`,
`calc`, `var(...)`, `url(...)`, `image("a.png")`. Calls stay as they are.
The arguments of a call are part of the call: `linear(90, red, blue)` keeps
its bare colour names. A string is not accepted as a colour stop today
(`linear(90, "red", ...)` is E018), and teaching function parameters to take
strings is a separate change. A hex literal inside a call (`linear(90, red, #fff)`)
is therefore also left alone. The warning applies to the value of an
attribute, the items of a list or group, and the branches of a ternary.

## Colours

`red`, `transparent`, `current` and the system colours are keywords: quote
them. Hex literals are lexed as a string already (the lexer emits the
`#3366aa` token as a string value), so `"#3366aa"` is the same token with
quotes; only the source text differs. The `rgb()`, `rgba()`, `hsl()`, `hwb()`,
`lab()`, `lch()`, `oklab()`, `oklch()`, `cmyk()`, `color()`, `mix()` and
`light dark()` forms are calls and stay bare.

## Ambiguity: a variable named like a keyword

An identifier that resolves to a variable, parameter, `each` item or
constant is the variable. Only an identifier that resolves to nothing is a
bare word, and gets the warning. So `none := 3` followed by
`gap = none` is the constant. W131 (a constant hides a layout value) stays,
and after strict mode a constant can never hide a value because values are
never bare.

## Persian files

The quoted form keeps the spelling of the source: `رنگ = "قرمز"` and
`تزئین متن = "هیچ"`. A Persian file accepts both Persian and English
keywords inside the string; an English file accepts only the English ones
(the existing rule in `docs/LAYOUT_SCHEMA.md`). Nothing is translated by the
fix: it only adds the quotes around what was written. Persian digits and
`،` `٪` stay bare as numbers.

## Diagnostics

| code | level | text (en)                                                        |
| ---- | ----- | ---------------------------------------------------------------- |
| W299 | warn  | the word 'none' is a value, not a name; write it in quotes       |
|      | help  | write "none"                                                     |

The fa texts are in `compiler/i18n.salam` (checked by `check-i18n.sh`; the
Persian names in `check-fa-names.sh` are unaffected). With strict values on,
the same condition is E299 with the same text.

## Autofix

The warning carries a safe quick fix (the whole bare word replaced by the
quoted word). Both the LSP quick fix and the CLI use it:

```
salam check --fix main.salam
```

The fix is idempotent: a quoted value no longer warns. It edits only the
source bytes of the value, so it cannot disturb Persian keyword packs or
language detection. `salam fmt` stays cosmetic and does not rewrite values.

## Strict mode

`salam check --strict-values` (and `strict values = true` in `salam.toml`)
turns W299 into E299. It is off by default. The release after the warning has
shipped and the repository is migrated flips the default: bare words are an
error, `--no-strict-values` is not provided. The plan:

1. release N: W299 on by default, fix available, `--strict-values` optional;
2. release N+1: default becomes error; the repository CI already runs with
   strict values;
3. release N+2: the flag goes away; the compiler keyword tables are only
   consulted for strings.

## Editor support

Completion for a value and the snippets insert the quoted form (`"none"`), so
new code is written strict from the start. The LSP reports the same W299 as
the compiler because it runs the same checks.

## Migration

1. compiler: W299 and the quick fix (phase 1, 2);
2. repository: `salam check --fix` over tests, examples, docs, site
   content, book listings, editor and extension samples, one area per PR
   (phase 3); expected outputs do not change because quoting is purely
   syntactic;
3. `--strict-values` and the `salam.toml` setting (phase 4).
