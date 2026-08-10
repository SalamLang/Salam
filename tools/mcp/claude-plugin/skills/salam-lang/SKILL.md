---
name: salam
description: Use when writing, reading, debugging or reviewing Salam source (.salam files) - the Salam programming language with English/Persian/Arabic keywords. Covers its enforced declaration ordering, the `until` = `while` rule, stdlib lookup, and the toolchain traps that silently produce wrong behaviour. Triggers on .salam files, "Salam", "salamlang", or any request to compile, check or run Salam code.
---

# Working with Salam

Salam is a compiled, statically typed language whose keywords exist in English,
Persian and Arabic. Its rules differ from mainstream languages in ways that are
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
package → import → extern: → globals → types → private funcs → pub funcs
```

Once the first `pub func` appears, only `pub func`s may follow (`E088`).
Put every private helper above the public section. Globals must precede all
functions and types (`E085`); imports come directly after `package` (`E083`).

**Unused things are errors, not warnings:** unused import `E082`, unused
variable `E059`, unused function `E066`. Prefix with `_` or remove.

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
    ret 0                      // main must return a value
end
```

- No typed local declarations: use `:=`, cast with `as`.
- `&:` marks a by-reference parameter.
- Built-in `str` methods are only: `len concat substr find/search/indexOf trim
lower upper repeat split to_int to_float`. Everything else is in `str`.
- `Vector {} as Vector<str>`, `v.get(i)[0]` to read an element.
- Nested generics (`Vector<Vector<T>>`) do not work — flatten instead. They
  fail with type errors pointing _inside_ std, not at your code.

## Traps that fail silently

- `str.Split` can segfault reading its **last** element on gcc-linked builds.
  Prefer `find`/`substr`.
- `os.shell.Run` deadlocks when the child writes more than ~64KB.
- `salam exec` (the interpreter) miscomputes unsigned `u32`/`u64` arithmetic —
  verify such code with `salam_run`, never `salam_exec`.
- A bare `ret` in `main` passes `salam_check` and fails `salam_build` with a
  raw gcc error. If check passes but build fails, look here first.
- Source files are keyed by **filename**, not package: two files named
  `io.salam` anywhere in one program collide at link time.

## Multilingual source

Keywords exist in all three languages (`salam_keywords` returns the table).
Declarations carry aliases:

```salam
@en "Trim"
@fa "پیرایش"
@ar "اقتطاع"
pub func Trim(s: str): str: ret s.trim() end
```

Compile non-English source with `--lang=fa` or `--lang=ar`.

Full reference: `docs/ai/AGENTS.md`, or the `salam://guide/agents.md` resource.
