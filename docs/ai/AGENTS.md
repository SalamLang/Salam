# Writing Salam: a guide for AI agents

Salam is a compiled, statically typed language whose keywords exist in English,
Persian and Arabic. This file is the orientation an AI model needs to produce
Salam that compiles on the first try. It documents the rules that are _not_
guessable from other languages — everything here has been verified against the
compiler in this checkout, with the exact diagnostic it produces.

If you have the `salam` MCP server available, use it rather than guessing:
`salam_check` after every edit, `salam_stdlib_symbols` before using any std
package, `salam_find_examples` to see a working usage.

---

## 1. The rule that trips up every newcomer: `until` means `while`

`until COND:` runs the body **while `COND` is true**. It is not a do-while and
it is not "loop until the condition becomes true". It is a plain `while` with a
different spelling.

```salam
mut i := 0
until i < n:        // reads "while i < n"
    // ...
    i += 1
end
```

Getting this backwards produces a loop that never runs, or never stops. When
relocating existing Salam code, copy it verbatim — do not retype loops from
memory.

## 2. Top-level declaration order is enforced

The compiler requires one specific order and rejects anything else. In order:

```
package  →  import  →  extern:  →  globals  →  types  →  private funcs  →  pub funcs
```

| Rule                                                                              | Diagnostic if broken                                                                |
| --------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| `import` must come directly after `package`, before any other top-level statement | `E083: 'import' must appear before any other top-level statement`                   |
| Global variables must precede every function and type definition                  | `E085: global variable 'g' must be declared before any function or type definition` |
| Once the first `pub func` appears, only `pub func`s may follow                    | `E088: function '_b' must appear before 'pub' function 'A'`                         |

The `pub` rule is the one that bites hardest: a private helper written _after_
your first public function will not compile. Put every private helper above the
`// ---- public API ----` line.

`salam format --fix-order` can reorder declarations mechanically.

## 3. Unused anything is an error, not a warning

Salam fails the build on unused imports, variables and functions.

| Code   | Trigger         | Fix                                        |
| ------ | --------------- | ------------------------------------------ |
| `E082` | unused import   | remove it, or prefix the name with `_`     |
| `E059` | unused variable | remove it, or prefix with `_`              |
| `E066` | unused function | call it, mark it `pub`, or prefix with `_` |

Add imports only as you use them.

## 4. Bindings

```salam
x := compute()          // immutable binding, type inferred
mut y := 0              // mutable binding
y = 3                   // reassignment requires `mut` (else E013)
const MAX := 10         // top-level constant
```

There is **no typed local declaration form**. `name: T = value` was removed —
always use `:=` and let the type be inferred, casting with `as` when needed.

`pub const` shares a single link-level namespace across packages: two packages
that both define `pub const KIND` collide at link time. Prefix public constants
with something package-specific.

## 5. Functions

```salam
func add(a: int, b: int): int:
    ret a + b
end

pub func Scale(v: f64, by: f64): f64: ret v * by end   // one-liners are fine

func fill(out &: Vector<str>):                          // `&:` = by reference
    out.push("x")
end
```

`func main:` implicitly returns `i32`, so it must end with `ret 0`, not a bare
`ret`. Note that a bare `ret` passes Salam's own analysis and only fails later
in the C backend, with a raw `'return' with no value` error from gcc — so
`salam_check` will call the file clean while `salam_build` fails. When a check
passes but a build does not, look for this first.

## 6. Strings

Built-in methods on `str` — this is the complete list:

```
len  concat  substr(start, len)  find/search/indexOf  trim
lower  upper  repeat  split  to_int  to_float
```

Anything else lives in the `str` package (`str.StartsWith`, `str.EndsWith`,
`str.Contains`, `str.Equals`, `str.TrimPrefix`, `str.Join`, `str.Replace`,
`str.NewBuilder`/`BufAppend`/`BufStr`/`BufFree`, …). Call
`salam_stdlib_symbols` with package `str` for the current list.

Build strings with a builder rather than repeated `concat` in a loop:

```salam
mut sb := str.NewBuilder()
defer str.BufFree(sb)
str.BufAppend(sb, "hello")
out := str.BufStr(sb)
```

## 7. Collections and generics

```salam
v := Vector {} as Vector<str>
v.push("a")
first := v.get(0)[0]                       // get() returns a slot; [0] reads it
m := HashMap {} as HashMap<str, int>
```

**Nested generics do not work.** `Vector<Vector<T>>` and `HashMap<K, Vector<V>>`
are not usable — flatten the data instead. A cross-package function returning
`Vector<T>` may also need an explicit `as` cast at the call site.

The failure mode is misleading: instantiating `Vector<Vector<int>>` reports
type errors _inside the standard library_ (`return type mismatch: expected
'Vector_i32', got 'i32'` at some `std/collections/vector.salam` line) rather
than at your declaration. Errors pointing into std that you did not touch
almost always mean a nested generic somewhere in your own file.

## 8. Imports

```salam
import str                          // std package
import encoding.json                // nested std package: dotted, unquoted
import fs.file
import mine "my_helpers.salam"      // sibling file, quoted, aliased
```

Nested std packages use dots (`encoding.json`), never quoted slashes. For a
sibling source file, the quoted form binds to the **filename**, so alias it
explicitly (`import mine "my_helpers.salam"`) and call it as `mine.Thing()`.

`import foo` resolves `std/foo/foo.salam` specifically; once that anchor
resolves, the other files in that directory join the same package.

Importing a package links **all** of its public functions' external
dependencies, not only the ones you call.

## 9. Platform conditionals

Platform macros are only valid as an `if` at top level:

```salam
if SALAM_OS_WINDOWS:
    func sep(): str: ret "\\" end
else:
    func sep(): str: ret "/" end
end
```

Available: `SALAM_OS_WINDOWS`, `SALAM_OS_LINUX`, `SALAM_OS_MAC`,
`SALAM_OS_BSD`, `SALAM_OS_FREEBSD`, `SALAM_OS_ANDROID`, `SALAM_OS_WASM`.

## 10. Multilingual source

Every keyword exists in English, Persian and Arabic; `salam_keywords` returns
the full table. Declarations carry name aliases so other-language callers can
use them:

```salam
@en "Trim"
@fa "پیرایش"
@ar "اقتطاع"
pub func Trim(s: str): str: ret s.trim() end
```

Compile non-English source with `--lang=fa` or `--lang=ar`.

---

## Known traps

These are real defects and sharp edges in the current toolchain, not style
advice. Each one silently produces wrong behaviour rather than a diagnostic.

- **`str.Split` can crash on its last element.** On a gcc-linked build,
  `str.Split("a/b/c/d", "/")` reports the right length and correct elements
  `0..n-2`, then segfaults reading the last one. Avoid it in code that must be
  backend-portable; walk the string with `find`/`substr` instead. The same bug
  reaches anything built on it, including `os.shell.Exec` and `io.Lines`.

- **`os.shell.Run` deadlocks on large child output.** It waits for the child
  before draining its pipes, so a child that writes more than the pipe buffer
  (~64KB) blocks forever. For anything that might produce real volume,
  redirect to a file and read it back, or use `os.RunCapture`.

- **The interpreter gets unsigned arithmetic wrong.** `salam exec` silently
  miscomputes `u32`/`u64` operations. Verify crypto, hashing and bit-twiddling
  code with `salam build`/`salam run`, never `salam exec`.

- **`os.Args()` has a broken generic type.** Binding any element to a local
  (`a := argv.get(1)[0]`) corrupts semantic analysis or crashes the compiler.
  Pass argv-derived values _inline_ as call arguments only.

- **Enum values cannot cross a package boundary.** A `pub enum`'s members are
  not usable from another package; use `pub const int` values instead.

- **A file's module is keyed by filename, not package.** Two packages that each
  contain a file called `io.salam` collide at link time. Give every source file
  a name unique across the whole project.

- **`open` and `input` are reserved built-ins.** Do not name anything after them.

- **`input()` cannot report EOF.** It returns `""` both for an empty line and at
  end-of-stream. Drive `getchar()` yourself if you need to tell them apart.

---

## Minimal complete program

```salam
package main

import str

func _greeting(name: str): str:
    ret "Hello, ".concat(name).concat("!")
end

pub func Greet(name: str): str:
    ret _greeting(name)
end

func main:
    println Greet("world")
    println str.FromInt(42)
    ret 0
end
```

```sh
salam check  hello.salam     # via MCP: fastest, no codegen
salam run    hello.salam     # build and execute
salam format hello.salam --check
```
