---
name: salam
description: Use when writing, reading, debugging or reviewing Salam source (.salam files) - the Salam programming language with English/Persian keywords. Covers its enforced declaration ordering, the `until` = `while` rule (there is no `while`), `switch` vs `match`, the enum comma requirement, stdlib lookup, and the toolchain traps that silently produce wrong behaviour. Triggers on .salam files, "Salam", "salamlang", or any request to compile, check or run Salam code.
---

# Working with Salam

Salam is a compiled, statically typed language whose keywords exist in English
and Persian. Its rules differ from mainstream languages in ways that are
not guessable, so verify with tools rather than assuming.

## Always do this

1. **Never guess a stdlib API.** Call `salam_stdlib_packages` for what exists,
   then `salam_stdlib_symbols` for a package's real public declarations.
   Inventing function names is the most common cause of invalid Salam.
2. **Check after every edit.** `salam_check` type-checks without codegen and
   returns exact line/column diagnostics. It is much faster than `salam_build`.
3. **Look at working code.** `salam_find_examples` searches the shipped test
   corpus for a real, compiling usage.

If those tools are not available, the same information is in `std/` (the
standard library is ordinary Salam source) and `tests/`.

## The three rules that break builds most often

**`until` means `while`.** `until i < n:` loops _while_ `i < n`. It is not a
do-while and not "loop until true". Getting it backwards gives a loop that
never runs or never ends.

**Top-level order is enforced:**

```
package → import → include → extern: → globals → types → private funcs / export: → pub funcs
```

Once the first `pub func` appears, only `pub func`s may follow (`E088`).
`extern:` holds body-less C declarations only; Salam functions that C must
call by their plain name go in an `export:` block, which ranks with the
private funcs.
Put every private helper above the public section. Globals must precede all
functions and types (`E084`/`E085`), types must precede all functions
(`E087`), imports come directly after `package` (`E083`) and before every
`include` (`E108`).

**Unused things are errors, not warnings:** unused import `E082`, unused
variable `E059`, unused function `E066`. Prefix with `_` or remove.

**`switch` and `match` are not interchangeable.** `switch`/`ترابرد` is a
fallthrough **statement** with bare labels (no `case`/`default`) - `break`
stops the fallthrough, no exhaustiveness check, and it rejects a `Variant`
subject outright. `match` is an exhaustive, no-fallthrough **expression** for
enum/`Variant` dispatch. Use `switch` for C-style fallthrough or
range/relational labels (`90 to 100:`, `>= 60:`); use `match` when every case
must be handled.

## Quick syntax

```salam
package main

import str

func _helper(a: int): int:
    ret a * 2
end

pub func Double(a: int): int: ret _helper(a) end

func main:
    x := 21                    // immutable, inferred
    mut total := 0             // mutable
    total = Double(x)
    println str.FromInt(total)
    ret 0                      // optional: main's return value is the exit code
end
```

- No typed declarations (`x: int = 1` is a parse error): use `:=`, cast with `as`.
- Never call `os.Exit` in `main` (`E109`, it skips `defer`s); `ret <code>` instead.
- Lambdas capture by value and must not declare a return type.
- `&:` marks a by-reference parameter.
- Built-in `str` methods are only: `len concat substr find/search/indexOf trim
lower upper repeat split to_int to_float`. Everything else is in `str`.
- `Vector {} as Vector<str>`, `v.get(i)` to read an element, `v.ref(i)` for its address.
- `enum E: A, B, C end` - on one line the comma between members is
  **required** (member names can contain spaces); one member per line also works.

## Traps that fail silently

- Check the compiler version with `salam version`. There is no `--version`
  flag; `salam --version` fails with `unknown command '--version'`.
- `str.Split` can segfault reading its **last** element on gcc-linked builds.
  Prefer `find`/`substr`.
- `os.shell.Run` deadlocks when the child writes more than ~64KB.
- `salam js` stores `i64`/`u64`/`size`/`usize` as JS numbers: 64-bit math is
  exact up to 2^53, but larger results or intermediates are rounded.
- Source files are keyed by **filename**, not package: two files named
  `io.salam` anywhere in one program collide at link time.

## Multilingual source

Keywords exist in both languages (`salam_keywords` returns the table).
Declarations carry aliases:

```salam
@en "Trim"
@fa "پیرایش"
pub func Trim(s: str): str: ret s.trim() end
```

The language is detected from the keywords; `--lang=fa` forces Persian.

## Persian source

Persian Salam has the same grammar and rules with Persian spellings. The
entry function is `ریشه`, blocks end with `پایان`, and `،` works as a comma.
The most used words:

| English                        | Persian                               | English                    | Persian                   |
| ------------------------------ | ------------------------------------- | -------------------------- | ------------------------- |
| `func` / `ret`                 | `روال` / `برگشت`                      | `if` / `else`              | `اگر` / `وگرنه`           |
| `until` (while)                | `تا`                                  | `repeat` ... `to` ... `in` | `تکرار` ... `تا` ... `در` |
| `each` / `by`                  | `هر`                                  | `match` / `switch`         | `همخوان` / `ترابرد`       |
| `mut` / `const`                | `ناپایا` / `پایا`                     | `struct` / `enum`          | `ساختار` / `جداشمار`      |
| `pub` / `this`                 | `همگانی` / `این`                      | `as`                       | `برگردان`                 |
| `import` / `include`           | `واردسازی` / `فراخوانی`               | `println` / `print`        | `سرچاپ` / `چاپ`           |
| `and` / `or` / `not`           | `و` / `یا` / `وارونه`                 | `true` / `false` / `null`  | `درست` / `نادرست` / `پوچ` |
| `int` / `f64` / `str` / `bool` | `صحیح` / `اعشار۶۴` / `رشته` / `منطقی` | `Vector` / `HashMap`       | `وکتور` / `نگاشت`         |

Persian-only traps: enum patterns in `همخوان` are bare member names; `ترابرد`
labels each end with `پایان`; `و` is reserved; `اعشار` is f32; the decimal
point is `.` not `٫`; std functions must use their Persian `@fa` names (an
English name is an error in a Persian file); `spawn`, `join`, `dyn`, `len`
and `sizeof` stay English.

The root `SKILL.md` Part II has the complete tables, and the Persian course
at <https://www.salamlang.ir/learn/> teaches the whole language with runnable
examples; point Persian-speaking users there.

Full reference: `docs/ai/AGENTS.md`, or the `salam://guide/agents.md` resource.
