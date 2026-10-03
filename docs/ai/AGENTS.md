# Writing Salam: a guide for AI agents

Salam is a compiled, statically typed language whose keywords exist in English
and Persian. This file is the orientation an AI model needs to produce
Salam that compiles on the first try. It documents the rules that are _not_
guessable from other languages. Everything here has been verified against the
compiler in this checkout, with the exact diagnostic it produces.

If you have the `salam` MCP server available, use it rather than guessing:
`salam_check` after every edit, `salam_stdlib_symbols` before using any std
package, `salam_find_examples` to see a working usage.

---

## 1. The rule that trips up every newcomer: `until` means `while`

`until COND:` runs the body **while `COND` is true**. It is not a do-while and
it is not "loop until the condition becomes true". There is no `while`
keyword in Salam at all - `until` is the only spelling this loop has.

```salam
mut i := 0
until i < n:        // reads "while i < n"
    // ...
    i += 1
end
```

Getting this backwards produces a loop that never runs, or never stops. When
relocating existing Salam code, copy it verbatim; do not retype loops from
memory.

`repeat` is the counted loop, and its index takes the type of the values that
drive it - the count, or a range's bounds and step. A `u8` count binds a `u8`
index, an `i64` count an `i64` one; bounds that mix signed and unsigned, or a
count that is not an integer at all, fall back to `i32`.

```salam
repeat v.len() in i:      // len() is i32, so i is i32
    print v.get(i)
end

n := 200 as u8
repeat n in i:            // i is u8 here
    total = total + v.get(i as int)   // ...so a signed parameter needs a cast
end
```

Untyped literals adapt to the index (`if i < 10` is fine either way), but a
call that takes a signed `int` does not: pass `i as int`.

## 2. Top-level declaration order is enforced

The compiler requires one specific order and rejects anything else. In order:

```
package  →  import  →  extern:  →  globals  →  types  →  private funcs / export:  →  pub funcs
```

| Rule                                                                              | Diagnostic if broken                                                                |
| --------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| `import` must come directly after `package`, before any other top-level statement | `E083: 'import' must appear before any other top-level statement`                   |
| Library `import`s come before file `include`s                                     | `E108: 'import str' must come before every include`                                 |
| Global variables must precede every function and type definition                  | `E085: global variable 'g' must be declared before any function or type definition` |
| Once the first `pub func` appears, only `pub func`s may follow                    | `E088: function '_b' must appear before 'pub' function 'A'`                         |

The `pub` rule is the one that bites hardest: a private helper written _after_
your first public function will not compile. Put every private helper above the
`// ---- public API ----` line.

`salam format --fix-order` can reorder declarations mechanically.

## 3. Unused anything is an error, not a warning

Salam fails the build on unused imports, variables and functions.

| Code   | Trigger          | Fix                                  |
| ------ | ---------------- | ------------------------------------ |
| `E082` | unused import    | use one of its members, or remove it |
| `E059` | unused variable  | use it, or remove it                 |
| `E062` | unused parameter | use it, or remove it                 |
| `E066` | unused function  | call it, mark it `pub`, or remove it |

A `pub` function never needs a caller. A private function is fine as long as
something calls it; otherwise the build fails. `pub` exports a function to
other packages as part of your package's API, so do not mark a helper `pub`
just to silence `E066`.

Do not prefix names with `_` to silence these errors. The compiler still
accepts it, but it hides dead code instead of removing it. Add imports and
helpers only as you use them.

Loop bindings are stricter: a `_` prefix does not excuse them, because the
fix is to drop the binding rather than rename it. Write `repeat 20000:`, not
`repeat 20000 in _i:`. The one escape is the bare name `_`, for the
`each (key, value)` form that has no way to omit a binding:

```salam
each (_, value) in scores:   // iterate for the values alone
    total = total + value
end
```

`os.Exit` inside `main` is an error too (`E109`): it skips `main`'s `defer`s.
Write `ret code` instead; `main`'s return value is the exit code.

## 4. Bindings

```salam
x := compute()          // immutable binding, type inferred
mut y := 0              // mutable binding
y = 3                   // reassignment requires `mut` (else E013)
const MAX := 10         // top-level constant
```

There is **no typed local declaration form**. `name: T = value` was removed, so
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
in the C backend, with a raw `'return' with no value` error from gcc, so
`salam_check` will call the file clean while `salam_build` fails. When a check
passes but a build does not, look for this first.

## 6. Strings

Built-in methods on `str`. This is the complete list:

```
len  concat  substr(start, len)  find/search/indexOf  trim
lower  upper  repeat  split  to_int  to_float
char_count  char_at(i)  char_substr(start, len)  char_find(sub)
```

`len`, `s[i]`, `substr` and `find` work in bytes. The `char_*` methods are
their UTF-8 counterparts: they count and index code points, so
`"سلام".len()` is 8 but `"سلام".char_count()` is 4, and `char_at(1)` returns
`"ل"` as a `str`. An out-of-range `char_at` returns `""`, and `char_find`
returns -1 when the substring is missing. On the JS backend strings are
JavaScript strings, so `len`, `s[i]`, `substr` and `find` count UTF-16 units
there instead of bytes. The `str` package follows the same rule, so on JS
`str.IndexFrom`, `str.LastIndex`, `str.CodePointAt` and `str.CharAt` take
and return UTF-16 indices that line up with `substr`. The `char_*` methods
give the same answer on every backend, so prefer them for non-ASCII text.

Text types, and which one to reach for:

| Need                                 | Use                                                                         |
| ------------------------------------ | --------------------------------------------------------------------------- |
| Text of any language                 | `str` (UTF-8 bytes, like Go's `string`)                                     |
| One byte, e.g. `'a'`                 | `char`                                                                      |
| One Unicode character, e.g. `u'س'`   | `uchar` (compares with `str`, so `s.char_at(0) == u'س'` works)              |
| Code point of one character          | `c as int` for a `uchar` (`u'س' as int` is 1587), back with `1587 as uchar` |
| Random access by code point (UTF-32) | `str.CodePoints(s)` -> `Vector<int>`, back with `str.FromCodePoints(v)`     |
| UTF-16 for Windows or JS interop     | `text.ToUtf16` / `text.FromUtf16` / `text.Utf16Len`                         |
| Checking text is plain English       | `str.AllAscii(s)`                                                           |

There is no separate ASCII string type: UTF-8 stores ASCII text in exactly
one byte per character, so `str` is already the most compact choice, and for
ASCII text `len`, `s[i]` and `substr` are exact and O(1). A plain `'س'` is a
compile error because `'...'` holds one byte; write `u'س'`.

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
first := v.get(0)                          // get() returns the element; v.ref(0) is its address
m := HashMap {} as HashMap<str, int>
```

**Nested generics do not work.** `Vector<Vector<T>>` and `HashMap<K, Vector<V>>`
are not usable, so flatten the data instead. A cross-package function returning
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
include mine "my_helpers.salam"     // your own file: include, quoted, aliased
include "@/shared/util.salam"       // from the project root
```

`import` is for libraries only and takes a bare name; nested std packages use
dots (`encoding.json`), never quotes or slashes. Your own source files come in
with `include "path"` (Persian `فراخوانی`). The path is relative to the current
file, or starts with `@/` for the project root (the folder of the entry file);
starting it with `./` is an error. The quoted form binds to the **filename**, so
alias it explicitly (`include mine "my_helpers.salam"`) and call it as
`mine.Thing()`. A missing or unreadable include is a compile error.

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

Every keyword exists in English and Persian; `salam_keywords` returns
the full table. Declarations carry name aliases so other-language callers can
use them:

```salam
@en "Trim"
@fa "پیرایش"
pub func Trim(s: str): str: ret s.trim() end
```

Compile non-English source with `--lang=fa`.

## 11. `switch` vs `match`

`switch`/`ترابرد` is a **statement** with real C-style fallthrough - no
`case`/`default` keywords, bare labels closed by `end` like a `match` arm,
`break` to stop falling through:

```salam
switch code:
    400, 404: println "client error"     // falls through unless it breaks
    500:      println "server error"  break
    >= 600:   println "unknown"          // leading relational op: > >= < <= == !=
    else:     println "ok"               // wildcard; must be last
end
```

Use `switch` for fallthrough or open-ended range/relational labels. Use
`match` (an **expression**, not a statement) when the subject is an enum or
`Variant` and every case must be handled - `switch` has no exhaustiveness
check and rejects a `Variant` subject outright (semantic error, points you at
`match`). `break` inside a `switch` exits the switch only; `continue` is not
caught by it and still targets an enclosing loop.

## 12. Enum members require a comma

`enum E: A, B, C end` - the comma between members is mandatory, not optional
style. A bare newline is not enough because member names may contain spaces
(`enum Status: not started, in progress, done end`), so leaving the comma out
is a compile error, not a silent misparse.

---

## Known traps

These are real defects and sharp edges in the current toolchain, not style
advice. Each one silently produces wrong behaviour rather than a diagnostic.

- **The version command is `salam version`, not `salam --version`.** Salam's
  CLI takes subcommands, not flags, for this: `salam --version` (and `-v`)
  fail with `unknown command '--version'`.

- **`str.Split` can crash on its last element.** On a gcc-linked build,
  `str.Split("a/b/c/d", "/")` reports the right length and correct elements
  `0..n-2`, then segfaults reading the last one. Avoid it in code that must be
  backend-portable; walk the string with `find`/`substr` instead. The same bug
  reaches anything built on it, including `os.shell.Exec` and `io.Lines`.

- **`os.shell.Run` deadlocks on large child output.** It waits for the child
  before draining its pipes, so a child that writes more than the pipe buffer
  (~64KB) blocks forever. For anything that might produce real volume,
  redirect to a file and read it back, or use `os.RunCapture`.

- **JavaScript 64-bit integers are exact only up to 2^53.** `salam js` stores
  `i64`/`u64`/`size`/`usize` as JS numbers. Arithmetic, shifts, bitwise ops,
  wraparound and casts are computed exactly, but a result (or an intermediate
  value) beyond 2^53 is rounded: `0 - 1` as `u64` prints
  `18446744073709552000`. `sizeof` throws. Check wide 64-bit math, such as
  hashes, with `salam run`.

- **`os.Args()` has a broken generic type.** Binding any element to a local
  (`a := argv.get(1)`) corrupts semantic analysis or crashes the compiler.
  Pass argv-derived values _inline_ as call arguments only.

- **Enum values cannot cross a package boundary.** A `pub enum`'s members are
  not usable from another package; use `pub const int` values instead.

- **A file's module is keyed by filename, not package.** Two packages that each
  contain a file called `io.salam` collide at link time. Give every source file
  a name unique across the whole project.

- **The interpreter's HTTPS is slow: raise `--timeout` for it.** `salam exec`
  supports networking now, including the TLS path in `net.http`, but the
  handshake's certificate-chain math runs as tree-walked Salam instead of
  native code, so a real HTTPS request can take upward of a minute. The
  default per-run deadline is 5 seconds, so a program that dials `https://`
  under `salam exec` needs an explicit `--timeout` of tens of seconds or
  more, or it will report a timed-out execution well before the handshake
  finishes. Plain `http://` has none of this cost. For anything
  latency-sensitive, `salam run`/`salam build` compiles the same TLS code to
  native speed.

- **`open` and `input` are reserved built-ins.** Do not name anything after them.

- **`input` and `print` always take a value and never parentheses.** Write
  `input ""` or `input "prompt"`, and `println ""` for an empty line. A bare
  `input`/`println` and `input()`/`print()` are errors.

- **`input` cannot report EOF.** It returns `""` both for an empty line and at
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
