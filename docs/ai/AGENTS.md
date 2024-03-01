# Writing Salam: a guide for AI agents

Salam is a compiled, statically typed language whose keywords exist in English
and Persian. This file is the orientation an AI model needs to produce
Salam that compiles on the first try. It documents the rules that are _not_
guessable from other languages. Everything here has been verified against the
compiler in this checkout, with the exact diagnostic it produces.

If you have the `salam` MCP server available, use it rather than guessing:
`salam_docs` for how a construct is written, `salam_stdlib_search` and
`salam_stdlib_symbols` before using any std function, `salam_find_examples` to
see a working usage, and `salam_check` after every edit.

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

`salam format` reorders declarations mechanically; pass `--no-fix-order` to skip that.

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
`each key, value` form that has no way to omit a binding:

```salam
each _, value in scores:     // iterate for the values alone
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

pub func Scale(v: f64, factor: f64): f64: ret v * factor end   // one-liners are fine

func fill(out &: Vector<str>):                          // `&:` = by reference
    out.push("x")
end
```

`func main:` returns the process exit code: `ret 3` exits with 3, and a `main`
that ends without `ret` (or with a bare `ret`) exits with 0.

Parameter and variable names cannot be keywords. `in`, `to`, `by`, `on`,
`type`, `each`, `match`, `eq`, `lt`, `gt` and the other words in
`salam_keywords` are all reserved, so `func scale(v: f64, by: f64)` does not
parse. `try`, `catch`, `throw` and `throws` are reserved too.

A function that can fail is declared `throws` and fails with `throw`. Every
call to it needs `try` (which passes the error to the caller, so the caller
must be `throws` too) or a surrounding `try: ... catch e: ... end` block;
an unmarked call is `E139`. The error is always the built-in `Error`
(`e.kind`, `e.message`, `e.to_str()`), which needs no import.

```salam
func parse port(text: str): int throws:
    if text == "": throw Error.New(ErrorKind.InvalidArgument, "empty port") end
    ret text.to_int()
end

func main:
    port := try parse port("") catch 8080       // fallback on error
    try:
        println parse port("80") + port
    catch e:
        println "bad port:", e.message
    end
end
```

## 6. Strings

Built-in methods on `str`. This is the complete list:

```
len  concat  substr(start, len)  find/search/indexOf  trim
lower  upper  repeat  split  to_int  to_float
starts_with(prefix)  ends_with(suffix)  includes(sub)
char_count  char_at(i)  char_substr(start, len)  char_find(sub)
```

`starts_with`, `ends_with` and `includes` return `bool`; their Persian names
are `شروع با`, `ختم با` and `دربردارد`.

`len`, `s[i]`, `substr` and `find` work in bytes. The `char_*` methods are
their UTF-8 counterparts: they count and index code points, so
`"سلام".len()` is 8 but `"سلام".char_count()` is 4, and `char_at(1)` returns
`"ل"` as a `str`. An out-of-range `char_at` returns `""`, and `char_find`
returns -1 when the substring is missing. Every backend counts the same way,
the JS one included: `len`, `s[i]`, `substr` and `find` are byte based there
too, and only DOM offsets (UTF-16) need converting at the boundary.

Walk a string with `each`, which is one linear pass:

```salam
each ch in "سلام 👋":        // ch is a str holding one code point
    print ch, "|"
end
each i, ch in "سلام":        // i is the character number: 0, 1, 2, 3
    println i, ch
end
each i, b in "Hi".bytes():   // b is a char (one byte), i the byte index
    println i, b as int
end
```

Persian: `هر ح در متن:` and `هر ب در متن.بایت ها():`. Prefer this to
`repeat s.char_count() in i: s.char_at(i)`, which rescans the string on
every step. Invalid UTF-8 bytes come out one at a time (joining every `ch`
gives back the string), and a character is a code point, not a grapheme
cluster (use `str.Graphemes` for those). `s.bytes()` only works as the
target of `each`.

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

Nested generics work: `Vector<Vector<int>>`, `HashMap<str, Vector<Point>>`
and deeper all compile on every backend.

Assigning a Vector or HashMap (`w := v`) copies its header, not its storage,
so both names share the elements while each keeps its own length (on the JS
backend they share the length too). Use `copy()`
or `deep_copy()` (Persian `رونوشت` and `رونوشت عمیق`) when you need an
independent value:

| Call               | Result                                                                  |
| ------------------ | ----------------------------------------------------------------------- |
| `x.copy()`         | New storage for every container `x` holds directly; elements are shared |
| `x.deep_copy()`    | Fully independent copy, recursing through structs, enums and containers |
| `v.copy(start)`    | Vector or str from `start` to the end                                   |
| `v.copy(s, count)` | At most `count` items from `s`                                          |
| `v.copy(s, n, st)` | Every `st`-th item; a negative `st` walks backwards (Vector only)       |

A negative `start` counts from the end, so `v.copy(-3)` is the last three
items. Ranges clamp instead of panicking. Range arguments only apply to Vector
and str. A struct can define its own `copy()` or `deep_copy()`, and the
recursion uses it. `deep_copy()` refuses a value holding a raw pointer, since
it cannot know what the pointer owns. A `dyn` value is deep-copied through its
concrete type when the interface is declared in your program (a value holding
a raw pointer is then copied as-is); `copy()` shares a `dyn` field's box. Copies are new heap values the caller
frees, like any other Vector or HashMap.

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

Compile non-English source with `--lang=fa`, or start a Persian file with
`// زبان: فارسی` (English files: `// lang: en`). Mixed forms such as
`// lang: fa` are rejected.

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

## 12. Enum members on one line need commas

`enum E: A, B, C end` - on a single line the comma between members is
mandatory, because member names may contain spaces (`enum Status: not
started, in progress, done end`): `enum E: A B end` declares one member named
`A B`. One member per line works without commas:

```salam
enum Color:
    Red
    Green
end
```

---

## Known traps

These are real defects and sharp edges in the current toolchain, not style
advice. Each one silently produces wrong behaviour rather than a diagnostic.

- **The version command is `salam version`, not `salam --version`.** Salam's
  CLI takes subcommands, not flags, for this: `salam --version` (and `-v`)
  fail with `unknown command '--version'`.

- **JavaScript 64-bit integers are exact only up to 2^53.** `salam js` stores
  `i64`/`u64`/`size`/`usize` as JS numbers. Arithmetic, shifts, bitwise ops,
  wraparound and casts are computed exactly, but a result (or an intermediate
  value) beyond 2^53 is rounded: `0 - 1` as `u64` prints
  `18446744073709552000`. `sizeof` throws. Check wide 64-bit math, such as
  hashes, with `salam run`.

- **A file's module is keyed by its filename.** Two included files with the
  same name (`a/io.salam` and `b/io.salam`), or one of your files named after a
  std package you import (`shell.salam` next to `import os.shell`), clash. The
  compiler rejects it with `package name 'io' is claimed by two different
files`; give every source file a name unique across the whole project.

- **A side-effect-free endless loop can crash under clang.** C11 lets a
  compiler assume such a loop terminates, so with the C backend and clang at
  `-O2` an `until true:` loop whose body has no I/O or memory writes can be
  deleted and the program segfaults instead of spinning. Give a busy-wait a
  real effect (a sleep, a print, an atomic) or build it with
  `--backend=llvm`.

- **The interpreter's HTTPS is slow.** `salam exec` supports networking,
  including the TLS path in `net.http`, but the handshake's certificate-chain
  math runs as interpreted Salam instead of native code, so a real HTTPS
  request can take upward of a minute. `salam exec` has no deadline by
  default; a caller that sets one (`--timeout=MS`, `SALAM_EXEC_TIMEOUT_MS`, or
  the MCP server's `SALAM_MCP_RUN_TIMEOUT`) needs it in the tens of seconds
  for HTTPS. Plain `http://` has none of this cost, and `salam run` compiles
  the same TLS code to native speed.

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
salam check  hello.salam     # type-check only, no codegen (salam_check via MCP)
salam check  hello.salam --fix                  # apply the safe compiler fixes in place
salam check  hello.salam --error-format=json    # diagnostics with machine-readable fixes
salam run    hello.salam     # build and execute
salam test                   # run the test_ functions in every *_test.salam file
salam format hello.salam --check
```
