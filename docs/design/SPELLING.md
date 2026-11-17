# Name spelling fallback (W175)

The standard library names its public functions and methods in PascalCase
(`ParseHeader`, `AddItem`). Older code, tests and examples wrote the same
names in snake_case (`parse_header`, `add_item`), and the standard library kept
a snake_case twin of almost every function so that code kept compiling.

The twins are going away. To keep old code working without them, a call whose
exact name is not defined is looked up under its other spelling.

## What resolves

Only call positions are resolved, and only when the exact name is missing:

| Call                  | Looked up as                               |
| --------------------- | ------------------------------------------ |
| `pkg.parse_header(x)` | `pkg.ParseHeader`                          |
| `build_name()`        | `BuildName`, a function in scope           |
| `value.add_item(3)`   | the method `AddItem` of the value's type   |
| `pkg.ParseHeader(x)`  | `pkg.parse_header` if only that one exists |

The exact name always wins. If both `total` and `Total` exist, each call
reaches the one it names. Builtins (`len`, `char_code`, `print`, ...) are never
redirected, and neither are local variables: a variable named `count` does not
turn `count(...)` into a call of `Count`. Private names (leading underscore)
are not rewritten.

The compiler renames the call in the syntax tree during analysis, so every
backend (C, LLVM, JavaScript, interpreter) sees the canonical name.

## The warning

A snake_case call that resolves to a PascalCase definition reports W175:

```
warning[W175]: 'parse_header' is the old snake_case spelling; write 'ParseHeader'
```

The call still compiles. Writing the PascalCase name, or calling a snake_case
function that really is defined that way, reports nothing.

`--spelling=MODE` controls it:

| Mode             | Effect                   |
| ---------------- | ------------------------ |
| `warn` (default) | W175, the call compiles  |
| `error`          | the same finding is E175 |
| `off`            | silent                   |

## Bootstrap order

The compiler and the standard library are compiled by the previous release.
Nothing in `compiler/` or `std/` may rely on this fallback until a release
that contains it is the seed, so the call sites inside those two trees are
migrated to PascalCase first and the snake_case twins are removed last.
