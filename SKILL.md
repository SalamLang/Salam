---
name: write-salam
description: >-
  Convert, rewrite, port, or redesign software from another language (PHP,
  TypeScript/JavaScript, Python, C, Go, Rust, Java, C#, …) into idiomatic,
  full-feature Salam: write new Salam code, and port C to Salam (including
  self-hosting the Salam compiler itself). Use whenever the request mentions
  "Salam" (the programming language / زبان سلام), a `.salam` file, "convert/port/
  rewrite X to Salam", "rewrite the compiler in Salam", the Salam layout DSL, or
  building with the `salam` compiler. Covers both English Salam (Part I) and
  Persian Salam, سلام فارسی (Part II: every keyword, type, method and std
  package name). This is the authoritative, up-to-date reference for Salam's
  current syntax, its standard library, its strict compiler rules, the layout
  DSL, and porting low-level C. The tutorial books under `books/` are OUT OF
  DATE. Trust this file, `std/`, `tests/`, and the Persian course at
  https://www.salamlang.ir/learn/ instead.
---

# Writing Salam

Salam is a statically typed, compiled, general-purpose systems language. The
**general language transpiles to C** and builds to a native executable; embedded
**`layout:`** blocks compile to HTML/CSS/JS. It can also be run with an
interpreter (`salam exec`, pure compute only) that compiles each function to
register bytecode on first call (`SALAM_VM=0` falls back to the plain
tree-walker), and cross-compiled via LLVM. Source can be written in English or Persian, with the same grammar and
the same compiler rules in both.

This skill has two parts:

- **Part I (§1 to §12): English Salam.** The full language, stdlib, rules and
  porting guide, with English keywords.
- **Part II (§13 to §19): Persian Salam (سلام فارسی).** Every Persian keyword,
  type, method and std package name, the Persian-only rules, and complete
  Persian programs. Read Part I for semantics and Part II for spelling; the
  semantics never differ.

There is also a 25-lesson human tutorial of the language, in Persian at
**<https://www.salamlang.ir/learn/>** and in English at
**<https://salamlang.github.io/Salam/learn/>** (one page per topic, every
example compiled and run), and a reference page for every std package under
`/std/<slug>/` on both sites (see §5).

> **Source of truth.** When a detail is missing here, read real code:
> `std/<pkg>/*.salam` for exact stdlib signatures, and
> `tests/en/{apps,basics,data,features,games,interop,stdlib,types,webframework}/**`
> for idioms. Do **not** invent APIs.

## Contents

1. Conversion workflow
2. Core syntax crib
3. Types & data
4. Generics, interfaces, polymorphism
5. Standard library catalog
6. Compiler rules & top pitfalls ← **read this before writing**
7. Translating from PHP / TS-JS / Python / C-Go-Rust
8. FFI, concurrency, conditional compilation
9. Layout DSL (HTML/CSS/JS)
10. Tooling & verification
11. When a detail is missing
12. **Porting C → Salam & self-hosting the compiler** (bit ops, unions, the module map)
13. **Persian:** source basics (digits, commas, detection, the entry `ریشه`)
14. **Persian:** keywords
15. **Persian:** types, built-in methods and std package names
16. **Persian:** syntax crib (every construct side by side)
17. **Persian:** rules and traps
18. **Persian:** complete programs
19. **Persian:** the tutorial on salamlang.ir (lesson map)

---

## 1. Conversion workflow

When asked to convert program `X` (in some other language) into Salam:

1. **Understand `X`**: its entry point, data types, control flow, external I/O,
   and dependencies.
2. **Map constructs** to Salam using §7 (class→`struct`, interface→`interface`,
   generics→`<T>`, exceptions→`bool`/`Option`/sentinel, dict→`HashMap`,
   list→`Vector`, etc.).
3. **Pick stdlib packages** from §5 rather than reimplementing (`str`, `json`,
   `http`, `regex`, `math`, `sort`, `collections`, `db`, …).
4. **Write idiomatic Salam** using §2 to §4, obeying the strict rules in §6
   (unused = error, `until` = while, manual `.free()`, integer `/` truncates, no
   exceptions, `mut` to reassign, `pub` to export, top-level ordering).
5. **Verify** with `salam exec file.salam` (interpreter) or `salam build
file.salam --output=app` (§10). Fix every warning; most are hard errors.

Keep the program's structure and names recognizable, but produce _idiomatic_
Salam, not a transliteration.

---

## 2. Core syntax crib

```salam
func main:
    println "Hello, Salam!"          // print + newline; comma args are space-joined
end
```

Blocks open with `:` and close with `end`. `{ … }` braces are also accepted. A
single-statement body may follow the `:` on the same line: `if n < 2: ret n end`.
Comments: `//` to end of line, `/* … */` across lines.

### Declarations & variables

```salam
name := "Sara"           // immutable, type inferred (str)
mut count := 0           // mutable
count += 3 * 4           // compound assignment; count is now 12
total := 250 as i64      // pick a type with `as`
x := 3.14                // f64
const MAX := 100         // compile-time constant
```

`:=` declares (immutable by default). Reassigning a non-`mut` variable is a
**compile error**. `mut` makes it reassignable; `const` is a compile-time value.
**Typed declarations were removed**: `total: int = 250` and `x: auto = 3.14`
are parse errors ("declarations with a type annotation were removed"). Write
`name := value` and use `as` when the inferred type is not the one you want.
`name: Type = value` survives only for struct fields and parameters.
A `const` name must be a single word (`const MAX VALUE := 3` is a parse
error); `mut` globals, locals and functions may still have multi-word names.

### Printing

`print`/`println` (stdout) and `printerr`/`printerrln` (stderr) take
comma-separated arguments and space-join them. They are statements, not calls:
never wrap the whole argument list in parentheses. They always need a value:
a bare `println` is an error, so write `println ""` for an empty line.

`input` reads one line from stdin (without the newline) and follows the same
rules: it always takes a prompt and never parentheses. Write `input ""` or
`input "prompt"` (Persian `ورودی ""` / `ورودی "پیام"`); a bare `input` and
`input()` are errors. The prompt is printed first, with no newline. A
variable works as the prompt (`input label`), but only that name is the
prompt: `input label + "x"` appends `"x"` to the line read. For a computed
prompt, start with a string: `input "> " + label`.

```salam
println ""
name := input "Name: "
line := input ""
```

Any **struct, array, slice, `Vector` or `HashMap`** can be printed directly:
the compiler derives a stringify function for the type and prints what it
returns, recursing into fields and elements.

```salam
struct Point:  x: int  y: int  end

p := Point {x = 10, y = 15}
println p                                   // Point {x = 10, y = 15}
println [p, Point {x = 20, y = 25}]         // [Point {x = 10, y = 15}, Point {x = 20, y = 25}]
println ["ali", "reza"]                     // ["ali", "reza"]
```

Rules worth knowing:

- Nested `str` values are quoted (`"ali"`) so an empty one stays visible; a
  top-level `println s` on a `str` is unquoted as always.
- A `HashMap` prints as `{"a": 1, "b": 2}`; iteration order is the map's.
- Every field is shown, `pub` or not - the derived function is compiler-written
  and is not held to the privacy rule.
- Pointer fields print as `null` or `<ptr>`; they are never followed, so a
  cyclic structure still terminates. Enum fields print as their integer value.
- Give a struct `pub func to_str(): str` to control its own rendering; the
  derived function calls that instead.
- Anything with no derivable form (a `File` handle, a function value) is still
  the old error, and inside a struct renders as `<its type>`.

### Operators

`+ - * / %`, `**` (power, float result), `== != < > <= >=`, the comparison
words `eq neq lt gt lte gte` for `== != < > <= >=` (Persian `برابر نابرابر
کوچکتر بزرگتر کوچکتربرابر بزرگتربرابر`; `کوچکتر برابر` and `بزرگتر برابر`
with a space work too), the logical words
`and or not` (Persian `و یا وارونه`), ternary `cond ? a : b`, compound
`+= -= *= /= %= **=`, `++`/`--`. Integer `/` **truncates**. Power is `**`,
right-associative and tighter than unary minus (`2 ** 3 ** 2 == 512`,
`-2 ** 2 == -4`). `T**` in a type is a pointer to a pointer; after `as`,
`x as i64 ** 2` is a cast then a power. There is no `&&`, `||`, `!` or `^^`:
the compiler rejects them and names the word to write instead.
`^` on its own remains bitwise XOR.
**Bitwise operators** (integer operands only): `& | ^ ~` and shifts `<< >>`, with
compound forms `&= |= ^= <<= >>=`. Precedence follows C: shifts bind tighter than
comparisons; `&` tighter than `^` tighter than `|`, all looser than `==`
(so `1 | 2 & 3 == 3` and `1 << 4 + 1 == 32`). `+` on `str` concatenates and coerces
numbers to text (`"n=" + 3` → `"n=3"`).

### Control flow

```salam
if x > 0:  println "positive"
else x == 0:  println "zero"          // "else <cond>" is else-if
else:  println "negative"
end

until i < n:  i = i + 1  end          // loops WHILE the condition holds - "while" doesn't exist, see box below
repeat 3:  println "hi"  end          // do 3 times
repeat n in i:  println i  end        // i = 0 .. n-1 ("in" binds the index)
repeat 1 to 5:  ...  end              // 1..5 inclusive
repeat 1 to 5 in i:  ...  end         // ...binding the loop variable
repeat 10 to 1 in i:  ...  end        // descending: the *bounds* pick the direction
repeat 0 to 20 by 2:  ...  end        // step with "by"; must be POSITIVE even descending
each x in xs:  println x  end         // iterate a collection/array/your own type
each i, x in xs:  println i, x  end   // index + value (or key, value for a map)
each ch in s:  println ch  end         // a str: one UTF-8 character (code point) per step, ch is a str
each i, b in s.bytes():  println b  end // the bytes of a str: b is a char, i the byte index
// break: exit the innermost loop (or switch, see below); break N: exit N levels;  continue: next iteration
```

> ### ⚠ `until` is Salam's only loop-while keyword. There is no `while`.
>
> `while` does not exist in Salam; `until` is the sole spelling, and it
> **runs its body while `C` is true**, stopping when `C` becomes false. Read
> `until C:` as "loop while C" - not as "loop until C happens" and not as a
> do-until - which is the single most common porting mistake. (Persian's `تا`
> already reads correctly as "while".) When porting a loop from C/Python/JS/Go,
> **copy the condition verbatim**:
>
> | source loop              | Salam                                    | NOT                                                 |
> | ------------------------ | ---------------------------------------- | --------------------------------------------------- |
> | `while (v != 0)`         | `until v != 0:`                          | ~~`until v == 0:`~~                                 |
> | `while (i < n)`          | `until i < n:`                           | ~~`until i >= n:`~~                                 |
> | `while (v)` (truthy int) | `until v != 0:`                          | ~~`until v:`~~ (no truthiness; needs a real `bool`) |
> | `while (p)` (pointer)    | `until p != null:`                       | ~~`until p == null:`~~                              |
> | `for (;;)`               | `until true:` + `break`                  |                                                     |
> | `do { B } while (c);`    | `until true: B  if not c: break end end` |                                                     |
>
> **An inverted `until` fails silently.** `until v == 0:` with a nonzero `v`
> runs **zero times** and produces no error, so the function just returns its
> zero-value/empty result (this is exactly how a bit-length helper silently
> returns 0 and truncates a buffer). The compiler only rejects a _literally_
> constant-false condition (`until false:` → `E068`). So when a loop "did
> nothing", check its condition polarity **first**.
>
> **`repeat a to b` direction is decided at runtime by the bounds**, so
> `repeat n to 1 in i` counts _up_ `0, 1` when `n` is `0` instead of not
> running. Guard the count (`if n >= 1: repeat n to 1 in i: … end end`) when
> the start bound can fall below the end bound.

### Switch

`switch`/`ترابرد` is a **statement** (not an expression like `match`), with
real **C-style fallthrough**: without `break`, execution falls into the next
label's body. There are no `case`/`default` keywords - each label is a bare
value (or comma-list, range, or relational test) closed by its own `end`,
the same shape as a `match` arm:

```salam
switch score:
    100:       println "perfect"  end         // falls through into the next label unless it breaks
    90 to 99:  println "A"  break  end        // break exits the switch (only), not an enclosing loop
    70, 80:    println "round"  break  end    // comma list
    > 60:      println "passing"  break  end  // leading relational operator: > >= < <= == !=
    else:      println "low"  end             // wildcard; must be the LAST case
end
```

- **`break` exits the switch only**; `continue` is not caught by a switch and
  passes straight through to an enclosing loop (`continue` with no enclosing
  loop is still an error, same as everywhere else).
- **`break N` exits N levels** of enclosing loops and switches, so `break 2`
  inside a switch inside a loop leaves the loop too (`break 1` is `break`).
  Deferred statements of every level left still run. N must be between 1 and
  the number of enclosing loops and switches, or it is a compile error.
- **`switch true:`** with boolean labels replaces an if/else chain that tests
  different conditions (`x < 0:`, `x == 0 or name == "zero":`, ...): the first
  true label wins.
- **Enum subjects must be covered** - a `switch` on an enum (or on an alias
  of one, `type Paint = Color`) with no `else` must list every member, or it
  is compile error E103 naming the missing members. Members may be written
  through the enum or the alias (`Paint.Red`, `Color.Green`). For other
  subjects, a `switch` with no matching label and no `else` does nothing.
- **Not usable on a `Variant`** subject - that is a semantic error that tells
  you to use `match`'s type patterns instead (see §3).
- Prefer `switch` for C-style fallthrough or open-ended relational/range
  labels; prefer `match` when the subject is an enum or `Variant` and you want
  the compiler to check every case is handled (see §12.2 for the C
  `switch (x) { case … }` mapping).

### Functions

```salam
func add(a: int, b: int): int:  ret a + b  end   // ": type" before the block colon = return type
func greet(name: str):  println "Hi,", name  end // no return type = void
func greet(name: str, prefix: str = "Hello"):     // default arguments allowed
    println prefix, name
end
```

- **Pass by value.** Overloading by parameter types is allowed. Functions may
  call each other in any order, but every `struct`/`enum`/`type`/`interface`/
  `impl` must come **before the first function** of the file (E087), and
  constants and globals before both (E084/E085).
- **Default values are constants or expressions of globals**; a default may not
  refer to another parameter (`func f(a: int, b: int = a)` is E001 unknown
  identifier `a`). Use an overload instead.
- **Multi-word names**: identifiers may contain spaces, e.g.
  `func make counter()`, `func is weekend(d: Day)`, `pet name: str`. A call is
  `is weekend(d)`; a field access `dog.pet name`.
- **Modifiers**: `pub inline noinline pure noret deprecated` (combine freely),
  e.g. `pub inline pure func Area(w: f64, h: f64): f64: ret w * h end`. They
  work on struct methods too, in the same order:
  `pub inline func len(): int: ret this.count end`.
- **Reference parameters** (`name &: Type`) pass by reference so the callee can
  mutate the caller's value (in/out params, and to avoid copying big structs):

  ```salam
  func bump(c &: Counter):  c.n = c.n + 1  end   // caller's Counter is modified
  func push_all(dst &: Vector<int>, src: Vector<int>): ... end
  ```

  This is how much of the stdlib mutates its argument (`str.BufAppend(b &: StringBuilder, …)`).

- **`defer stmt`** runs at scope exit, LIFO, which is great for cleanup:
  `defer v.free()`.
- **Closures/lambdas** are first-class typed values, always written
  `(params): ... end` with an explicit `ret` for the value:
  `(x: int): ret x * 2 end`, or several statements
  `(): n = n + 1  ret n  end`. Salam has no `=>` at all; it was removed in
  0.5.0. Function-typed parameters: `func () int`, `func (int, int) bool`.
  - **Lambdas capture by value.** Each lambda gets its own copy of the outer
    variables at creation time: after `mut n := 1  g := (): ret n end  n = 5`,
    `g()` is still `1`, and a block lambda that does `n = n + 1` changes its
    own copy (it counts across calls), never the caller's `n`. Share state
    through a pointer, a heap collection or a `mut` global.
  - **Never write a return type on a lambda.** `(x: int): int: ... end`
    fails to parse; the return type is inferred from the `ret`
    (`(x: int): ret x * 2 end`).
  - **A bare named function decays to its address**, typed `i64` - the slot
    C-style callback registries take (e.g. the `net.router` router):
    `router.Get(r, "/", home)`. For a `void*` slot, or to cast to a typed C
    function pointer, use **`&fn`** instead. For a _typed_ Salam callback
    (`func (int) int`), pass a **lambda**: `apply((x: int): ret inc(x) end, 3)`.
  - **A variable may not reuse a function's name.** With a bare name being a
    value, `test := 5` next to `func test` is rejected (E090), in both
    directions, so an identifier always means exactly one thing.

---

## 3. Types & data

**Primitives:** `i8 i16 i32 i64`, `u8 u16 u32 u64`, `usize size`, `f32 f64`,
`bool`, `char`, `str`, `void`. Aliases: `int`=`i32`, `uint`=`u32`,
`float`=`f32`. Literals: `42` (int), `3.14` (f64), `true`/`false`, `null`.
**Pointer-width integers:** `usize` is unsigned (C `size_t`; Persian
`اندازه مثبت`), `size` is signed (C `intptr_t`; Persian `اندازه`). Both are 64
bits on 64-bit targets and 32 bits on 32-bit ones (wasm32, arm32).
`sizeof(T)` returns `usize`.
**Integer bases:** decimal `255`, hex `0xFF`, binary `0b1010`, octal `0o17` (all
verified). **`str` is UTF-8 bytes**: `str.Len(s)` is the byte count,
`str.CharCount(s)` the codepoint count; classify with `str.IsDigit(code)` etc.
Iterate a string with `each`: `each ch in s` gives each code point as a `str`
(`"س"`, `"👋"`), `each i, ch in s` adds the character number `i` (0, 1, 2, ...,
the same index `s.char_at(i)` takes), and `each b in s.bytes()` /
`each i, b in s.bytes()` walk the raw bytes as `char` with the byte index. It is
one linear pass, so prefer it over `repeat s.char_count() in i: s.char_at(i)`,
which rescans from the start on every step. Invalid UTF-8 bytes come out one
byte at a time, so joining every `ch` gives back the original string. A
character is a code point, not a grapheme cluster: `"👋🏽"` is two steps; use
`str.Graphemes(s)` for user-perceived characters. `s.bytes()` is only valid as
the target of `each` (E065 otherwise); read single bytes with `s[i]`.

**String/char literal forms** (values go into text with a ` ``` ` text block and
`${{ expr }}`, see below, or with `+` and `fmt.Sprintf`):

| Syntax       | Type                     | Escapes (`\n \t \" \\ \xHH \uHHHH \UHHHHHHHH` …) | Multiline | Notes                                                                                                                                                                                            |
| ------------ | ------------------------ | ------------------------------------------------ | --------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `"text"`     | `str`                    | yes                                              | no        | normal string; raw newline in source is an error                                                                                                                                                 |
| `"""text"""` | `str`                    | yes                                              | **yes**   | multiline, still processes escapes; for code or long text prefer a ` ``` ` text block                                                                                                            |
| `'c'`        | `char`                   | yes                                              | -         | one raw byte unless escaped, not UTF-8 safe for non-ASCII                                                                                                                                        |
| `` `text` `` | `str`                    | **none, fully raw**                              | **yes**   | backtick string; every byte up to the next `` ` `` is taken literally, including `"`, `'`, `\`, and real newlines. Only a literal backtick can't appear inside it (no escape exists for `` ` ``) |
| `u'c'`       | `char` (UTF-8 codepoint) | no                                               | -         | must decode to exactly one Unicode codepoint; use for non-ASCII single chars, e.g. `u'م'`, `u'中'`, `u'€'`                                                                                       |
| `u"c"`       | `char` (UTF-8 codepoint) | yes                                              | -         | same as `u'c'` but escapes are processed first, e.g. `u"\U0001F600"`                                                                                                                             |

**Prefer backtick strings for any text containing literal `"`**: JSON blobs,
`regex` patterns, shell commands, HTML/CSS fragments, instead of escaping:

```salam
data := `{"name": "salam", "version": 2, "active": true, "pi": 3.5, "tags": ["a", "b"]}`
```

not

```salam
data := "{\"name\": \"salam\", \"version\": 2, \"active\": true, \"pi\": 3.5, \"tags\": [\"a\", \"b\"]}"
```

Only fall back to `"..."` with escaped quotes when the string must also contain
a literal backtick, or when it needs an escape sequence (`\n`, `\uXXXX`, …)
that backtick strings don't process.

**Text blocks** (` ``` `) hold multi-line text, such as generated C, LLVM IR,
SQL, HTML or help text, without `"\n" + "..."` chains. The opening ` ``` ` ends
its line; the closing ` ``` ` starts its own line, and its indentation is
removed from every line, so the block sits indented in the code. The first and
last line breaks are not part of the value. The text is raw like a backtick
string: `\n`, `"` and `'` are kept as typed.

````salam
const OB_SIZE := 65536

func header(name: str): str:
    ret ```
        #define SALAM_OB_SZ ${{ OB_SIZE }}
        #define HALF ${{OB_SIZE / 2}}
        /* built for ${{ name }} */
        static int nl(void) { return '\n' == 10; }
        ```
end
````

- `${{ expr }}` puts the value of any expression into the text: `str`,
  numbers, `bool` and `char`, the same as `+` on a string. Spaces inside are
  free (`${{x}}`, `${{ x }}`), but `$`, `{{` and `}}` must be written without
  spaces in them, so `$ {{x}}` and `} }` are plain text.
- When every injected value is a constant, the whole block folds into one
  literal at compile time; otherwise it becomes a `+` chain.
- `$${{` writes the characters `${{`. A lone `$`, `{{`, `}}` or `$$` is plain
  text, so C, JS and shell code need no escaping.
- An injection must close on the line it opens. Errors: text after the opening
  ` ``` `, a missing closing ` ``` `, a line indented less than the closing
  ` ``` `, an unclosed `${{` and an empty `${{ }}`.
- It works the same in Persian files: `${{ نام }}`, and `salam translate`
  translates the code inside `${{ }}` while keeping the text as it is.
- `std/` and `compiler/` can only use text blocks once a released seed
  compiler understands them (see the seed lag rule).

**Type aliases:** `type NodeId = int`, `type Bytes = u8*` gives a new name for an
existing type (declared at top level, before functions). An alias is the same
type as its target, so values mix freely; type errors that involve a written
alias name it, e.g. `cannot pass 'i32' to 'Age' ('Years' is an alias of 'Age')`.
An alias of an enum reaches its members too: `type Paint = Color` then
`Paint.Red`.

**Casts & typed literals with `as`:**

```salam
b := 250 as int as u8
n := fib(i) as i64
arr := [1, 2, 3]                 // already int[3]; `as int[3]` is E093 "useless cast"
big := [0 as i64, 5, 7]          // the first element picks the element type
v := Vector {} as Vector<int>
m := HashMap {} as HashMap<str, int>
```

**Arrays (fixed size) & slices:**

```salam
a := [1, 2, 3]                        // int[3], indexed 0..2
grid := [[1, 2, 3], [4, 5, 6]]        // int[2][3], 2-D
mid := a[1: 3]                        // slice (view) of a[1] and a[2]; writes through to `a`
whole := a[:]  head := a[: 2]  tail := a[1:]   // omitted bound = that end of `a`
func sum(view: int[]): int: ... end   // int[] = slice parameter, any length
sum(a)  sum([4, 5])  sum(a[1:])       // an array passes as a slice directly, no copy
len(a)                                // length builtin: arrays, slices, str (a Vector uses v.len())
```

**Structs (fields + methods):**

```salam
struct Account:
    pub balance: int = 0              // pub = visible/usable outside; default value
    pub func deposit(n: int):  this.balance = this.balance + n  end
end
mut a := Account { balance = 100 }    // struct literal; omitted fields use defaults
a.deposit(50)
println a.balance
```

Fields and methods are **private by default**; add `pub` to expose. `this` is the
receiver.

**`Self`** is the enclosing type, usable anywhere inside a `struct`, an
`impl ... on T` block or a distinct type's body: as a return type, a parameter
type, a field type, in a cast and as a struct literal. On a generic struct it
means the struct with its own type parameters, so `Self` stands for
`Pair<A, B>` without respelling it. It works in a `static func` too, which is
where constructors live. Persian spells it `خود`. There is no `Self` in an
`interface` (that would be an associated type, which Salam does not have).

```salam
struct Node:
    pub v: int = 0
    pub func With(n: int): Self:          ret Self { v = n }      end
    pub static func Make(n: int): Self:   ret Self { v = n * 2 }  end
end
```

**Static members:** `static func` and `const` inside a struct belong to the type,
not to a value. Call them through the type name, also across packages
(`pkg.Color.Hex(...)`). A static func has no `this`, but it can read and set the
struct's private fields, so it is the place for constructors. On a generic
struct the type parameters come from the arguments or the expected type:
`Box.Of(42)`, `ret Box.Empty()`, `Box.Empty() as Box<f64>`.

```salam
struct Color:
    pub r: int  pub g: int  pub b: int
    pub const Max := 255
    pub static func Gray(v: int): Color:  ret Color { r = v, g = v, b = v }  end
end
w := Color.Gray(Color.Max)
```

**`mut func` (read-only `this`):** once any method of a struct is declared
`mut func`, the compiler checks the whole struct. Its other methods get a
read-only `this` (E105 when one assigns to a field or calls a `mut func` on
`this`), and a `mut func` can only be called on a `mut` binding or a `&:`
parameter (E106). Writes that go through a pointer, slice, `Vector` or
`HashMap` field change the heap data, not the struct, so they stay allowed.
Structs with no `mut func` keep the old rules. `mut` goes right after `pub`
(`pub mut inline func`), is also allowed on interface methods, and cannot be
combined with `pure`.

```salam
struct Counter:
    n: int = 0
    pub func Count(): int:  ret this.n  end     // this is read-only here
    pub mut func Tick():  this.n += 1  end
end
mut c := Counter {}
c.Tick()
```

**Iterating your own type with `each`:** `each` works on any struct that has
one of two shapes, so you do not write an index loop by hand. No interface and
no registration: the compiler checks the method shape.

- **Sequence** - `len()` (or `size()`) returning an integer, plus
  `get(i: int)`. `each x in it` walks `0 .. len()-1`, and `each i, x in it`
  also binds the index.
- **Cursor** - `iter()` returning a struct with `has_next(): bool`, `value()`
  and `next()`. Add `key()` to that cursor and `each k, v in it` binds both;
  without a `key()`, `each` over it takes a single binding.

```salam
struct RangeIter:
    pub cur: int = 0
    pub stop: int = 0
    pub func has_next(): bool:  ret this.cur < this.stop  end
    pub func value(): int:      ret this.cur              end
    pub func next():            this.cur = this.cur + 1   end
end
struct Range:
    pub lo: int = 0
    pub hi: int = 0
    pub func iter(): RangeIter:  ret RangeIter { cur = this.lo, stop = this.hi }  end
end
each v in Range { lo = 2, hi = 5 }:  println v  end      // 2 3 4
```

The method names are the usual built-ins, so a Persian file spells them
`طول`, `بگیر`, `پیمایش`, `داردبعدی`, `مقدار`, `کلید` and `بعدی`. Every std
container now has one of the two shapes, so `each x in set`,
`each x in stack`, `each x in queue`, `each x in deque`, `each x in list` and
`each k, v in ordered_map` all work. `PriorityQueue` is the exception: its
heap order is not its sorted order, so it has no `iter()` - pop from it
instead.

**Embedding (`use`):** composition instead of inheritance. `use Animal` inside a
struct adds a field named `Animal` and promotes its fields and `pub` methods,
so `d.name` means `d.Animal.name` and `d.Describe()` forwards to it. Promoted
methods count for interfaces, `<T: I>` bounds and `dyn I`. The outer struct's
own members win over promoted ones; two embeds that both provide a name are
E107. Literals may set promoted fields directly (`Dog { name = "Rex" }`); an
omitted embed defaults to `Animal {}` when all of its fields have defaults.
`pub use` exposes the embed outside the struct; plain `use` keeps it private.
Print and JSON show it as a nested object. The Persian spelling is `شامل`. A
generic struct embeds with its type arguments (`use Stack<str>`); a pointer
cannot be embedded.

```salam
struct Animal:  pub name: str = ""  pub func Describe(): str:  ret "I am " + this.name  end  end
struct Dog:
    pub use Animal
    pub breed: str = ""
end
d := Dog { name = "Rex", breed = "lab" }
println d.Describe()
```

**Operator overloading:** declare a method whose name is the operator itself,
`func +(o: V): V:`. It is called when the left operand is the struct. `!=` falls
back to `not (a == b)` when there is no `!=`, and `a += b` uses your `+`.
**Unary `-` and binary `-` share the symbol, overloaded by arity**: `func -: V:`
(no parameters, empty parentheses optional) is negation, `func -(o: V): V:` is
subtraction - define both if you need both:

```salam
struct Vec2:
    pub x: f64
    pub y: f64
    pub func +(o: Vec2): Vec2: ret Vec2 { x = this.x + o.x, y = this.y + o.y } end
    pub func -(o: Vec2): Vec2: ret Vec2 { x = this.x - o.x, y = this.y - o.y } end
    pub func -: Vec2: ret Vec2 { x = -this.x, y = -this.y } end   // unary -v
    pub func ==(o: Vec2): bool: ret this.x == o.x and this.y == o.y end
end
```

Only Salam's own operators can be overloaded: `+ - * / % ** == != < > <= >=
not [] []=`. A new symbol (`***`, `+-`, `&`) is an error. The parameter count
decides unary or binary and is checked:

- binary operators take exactly one parameter, the right operand
- `-` takes none (negation, `-v`) or one (subtraction)
- `not` takes none
- `[]` takes one (the index), `[]=` takes two (the index and the value)

**Distinct types (`type X: T`)**: `type Age = int` is a plain alias, the same
type as `int`. `type Meters: int` creates a _new_ type built on `int`. It
supports every `int` operator (the result is `Meters`), can declare its own
operators and methods in a body closed by `end`, and never mixes silently with
plain `int`:

```salam
type Meters: int
    func +(other: Meters): Meters:
        ret ((this as int) + (other as int)) as Meters
    end
    func *(k: int): Meters:             // overload by the operand's type
        ret ((this as int) * k) as Meters
    end
    func -: Meters:                     // unary minus, no parameters
        ret (0 - (this as int)) as Meters
    end
    func km: f64:                       // ordinary method: m.km()
        ret (this as f64) / 1000.0
    end
end

a := 5 as Meters                        // literals adopt the type: a + 3, 3 + a
b := a + 10                             // your operator +
println (a * 4).km(), a < b             // built-in '<' still works
n := a as int                           // conversions are explicit
```

- The base type must be a number, `bool`, `char` or `str`; a struct already
  has its own operators.
- `Meters + int_variable` is an error unless you declare an operator for it.
- `a != b` uses your `==` when there is no `!=`.
- `a += b` uses your `+`.
- An alias of a distinct type (`type Length = Meters`) shares its operators.
- From another file included as `units`, write `units.Meters`; its operators
  work there too.

**Sign runs are errors:** two `+`/`-` operators in a row are rejected, whether
spaced or not. That covers `- -x`, `a - -b`, `a + -b`, `a---b`, `n++ + 1` and
`+x` (Salam has no unary `+`). Put the inner one in parentheses (`a - (-b)`,
`(n++) + 1`) or simplify (`a + b`, just `x`).

**Enums & `match`:**

```salam
enum Day: Mon, Tue, Wed, Thu, Fri, Sat, Sun end   // Mon=0 … Sun=6
enum Color: Red, Green = 5, Blue end               // explicit values (Blue=6)
d := Day.Sat
println d as int                                   // 5

grade := match score / 10:                         // match is an EXPRESSION
    10, 9: "A" end
    8: "B" end
    else: "F" end
end
```

**A match arm is always `pattern: … end`.** The older `pattern => expr` form
was removed in 0.5.0, along with `=>` itself: the token is not part of the
language any more. An arm's block may hold statements, and its value (when
`match` is used as an expression) is the bare expression it ends with.

**A comma (`,` or Persian `،`) is required between enum members** - a bare
newline is not enough, because member names may contain spaces
(`enum Status: not started, in progress, done end`) and a newline alone can't
tell where one multi-word name ends and the next begins. Leaving out the
comma is a compile error (`'end'`/EOF right after a member with no comma
before it).

**Enums with data** (sum types): a member may carry named values. Build one
with `Enum.Member(values...)` (or `Enum.Member` when it has none) and take it
apart in `match` with `Member(a, b)` (use `_` to skip a value) or
`Member whole` to bind the whole case. `match` must cover every member (or
have `else`). Such enums print as `Circle(r = 2)` and compare with `==` when
every value can. They work across packages (`geo.Token.Num(4)`). An enum with
data needs at least two members, and members cannot also have `= value`.
`value.name()` gives the member's name and `Enum.Count()` the number of members.

```salam
enum Shape:
    Circle(r: f64)
    Rect(w: f64, h: f64)
    Empty
end
func area(s: Shape): f64:
    ret match s:
        Circle(r): 3.14 * r * r end
        Rect(w, h): w * h end
        Empty: 0.0 end
    end
end
println area(Shape.Rect(3.0, 4.0))
println Shape.Circle(2.0)          // Circle(r = 2)
```

**Generic enums, `result.Result` and `?`:** an enum with data may take type
parameters (`enum Maybe<T>: Some(v: T), Nothing end`). A generic member is
built where its type is known - returned from a function, or with `as`:
`Maybe.Nothing as Maybe<int>`. `import result` gives
`result.Result<T, E>` (`Ok(value)` / `Err(error)`) plus `result.IsOk`,
`IsErr`, `UnwrapOr(r, fallback)` and `Expect(r, msg)`. Inside a function that
returns such an enum, a postfix `?` unwraps `Ok` and returns any `Err` early
(running `defer`s): it must be a statement's whole value - `x := f()?`,
`x = f()?`, `f()?` or `ret f()?`.

```salam
import result
func parse(s: str): result.Result<int, str>:
    if s == "7": ret result.Result.Ok(7) end
    ret result.Result.Err("bad " + s)
end
func sum(a: str, b: str): result.Result<int, str>:
    x := parse(a)?
    y := parse(b)?
    ret result.Result.Ok(x + y)
end
```

**Struct patterns:** `Point{x = 0, y}` matches a struct (or an enum member,
`Rect{w, h = 1.0}`) by field name: `name` binds the field, `name = expr`
requires it to equal `expr`. A struct pattern with tests acts like a guard,
so keep a final pattern without tests or an `else`.

**Match guards:** any arm may add `if cond` after its patterns
(`Circle(r) if r > 10: "big" end`, `7 if ready:`). The guard sees the arm's
bindings and runs only when the pattern matches; if it is false, matching
continues with the next arm. A guarded arm does not count toward
exhaustiveness, so keep an unguarded arm (or `else`) for that case.

**`Variant<A, B, …>`** is a tagged union (one slot sized to the largest member).
Assign any member type; narrow it back with `match` on **type-name** patterns:

```salam
mut v := 21 as Variant<i32, f64, str>   // initial cast is OK from a *member* type (i32)
v = "offline"                            // then assign member-typed values directly
label := match v:
    i32 n: "int " + n end
    f64 f: "float " + f end
    str s: "text " + s end
end
```

Assign a value whose type is exactly one of the members; **do not** write
`"x" as Variant<…>` (casting a `str` into a Variant fails). Struct fields typed as
a `Variant` coerce a member-typed literal automatically
(`Reading { value = "offline" }`). Variant `match` needs the **compiled** backend,
not `salam exec`; see §10.

**`Option<T>`** (via `import option`) for maybe-absent values:

```salam
o := option.Some(99)
println option.UnwrapOr(o, 0)         // Some/None/IsSome/IsNone/Unwrap/UnwrapOr/Expect
```

**Pointers & `null`:** `T*` is a pointer, `p[0]` dereferences, `null` is the
null pointer. Used with FFI and `mem`: `p := mem.Allocate(8 as u64) as i64*`;
`p[0] = 0`; `mem.Free(p as void*)`.

---

## 4. Generics, interfaces, polymorphism

```salam
func Max<T>(a: T, b: T): T:  if a > b: ret a end  ret b  end   // generic function
struct Stack<T>:  pub items: Vector<T> = Vector {}  pub count: int = 0  end  // generic struct

interface Shape:
    func area(): f64
    func name(): str
end

// A struct satisfies an interface structurally by having matching pub methods:
struct Circle:
    pub r: f64 = 0.0
    pub func area(): f64:  ret 3.14159 * this.r * this.r  end
    pub func name(): str:  ret "circle"  end
end

func describe<T: Shape>(s: T):  println s.name(), s.area()  end   // static bound (monomorphized)
func show<T: Shape + Named>(s: T):  println s.name()  end        // several bounds with '+'
func draw(s: dyn Shape):  println s.area()  end                  // dynamic dispatch
shapes := [ Circle { r = 1.0 }, Rect { w = 2.0, h = 3.0 } ] as dyn Shape[3]
reg := Vector {} as Vector<dyn Shape>                            // heterogeneous collection
```

**`dyn` fields:** a struct field can hold any implementation, e.g.
`pub shape: dyn Shape = Square {}`. Storing a value copies it into a box, so
changing the original afterwards does not reach the field. Copying the
struct shares the box, the same way copying a Vector shares its storage, so a
method that changes state through one copy shows in the other. Release the
box with `field.free()` when you are done with it. `copy()` shares the box
too, while `deep_copy()` boxes a deep copy of whatever value the field holds
(a concrete type that holds a raw pointer is copied as a value, so the
pointer itself is shared).
That works for interfaces declared in your program; for an interface from
another package `deep_copy()` refuses, so give the struct its own
`deep_copy()`.

**One interface can `use` another** - the same keyword structs embed with
(Persian `شامل`). The base's methods, defaults included, become part of the
using interface, so `<T: Solid>`, `dyn Solid` and `impl Solid on T` all see the
full set. Two bases that provide the same method name is E107, and a cycle is
E107 too.

```salam
interface Shape:  func Area(): f64  end
interface Named:  func Name(): str  end
interface Solid:
    use Shape
    use Named
    func Volume(): f64
end
```

**Default methods:** an interface method may carry a body. A struct that
provides all of the interface's methods without bodies gets a copy of every
default it does not define itself, and so does an `impl I on T` block, so
defaults work with `<T: I>`, `dyn I` and direct calls.

```salam
interface Shape:
    func Area(): f64
    func Describe(): str:
        ret "area " + this.Area()
    end
end
```

`impl` adds interface methods to **any** type, including primitives:

```salam
interface Ranked:  func rank(): int  end
impl Ranked on int:  func rank(): int:  ret this  end  end
impl Ranked on str:  func rank(): int:  ret len(this)  end  end
```

**An `impl` block is checked against its interface.** Whatever the block
supplies, the type must end up with every method the interface declares, with
matching signatures, or the `impl` line itself is E002. So an **empty** block
is the way to say "this struct implements this interface" and be told at the
declaration site instead of at some distant `dyn` cast:

```salam
struct Circle:
    pub r: f64 = 1.0
    pub func Area(): f64:  ret 3.14159 * this.r * this.r  end
end
impl Shape on Circle:  end      // asserts conformance; E002 here if it drifts
```

---

## 5. Standard library catalog

Import a library by name, never quoted: `import str`, `import math`. Dotted
subpackages: `import db.sqlite`, `import encoding.json` (called as `json.X`,
the last part of the path; a bare `import json` or `import yaml` does not
find it). Your **own files** use `include` with a quoted
path (Persian `فراخوانی`): `include "util.salam"` (relative to this file),
`include "@/lib/util.salam"` (from the project root, the entry file's folder),
`include mx "math.salam"` (aliased; call `mx.Square(…)`). `import "x.salam"`,
`include str` and a path starting with `./` are all compile errors, and a missing
or unreadable include stops the build. A file may declare `package <name>`
(default `main`).
Only `pub` symbols are importable. **Function names are `PascalCase`; collection
methods are `snake_case`.**

**Everything below is real; signatures come from `std/`.** When unsure of
an exact signature, grep the package file.

**Reference pages.** Every package has a page with guides, a "things that
catch people out" list and a checked example for each function:
`https://www.salamlang.ir/std/<slug>/` (Persian) and
`https://salamlang.github.io/Salam/std/<slug>/` (English), where the slug is
the import path with `/` as `-`: `str`, `db-sqlite`, `net-http`,
`encoding-json`. The index is `/std/` on both sites. The `/learn/` course on
both sites teaches the language only, not the libraries.

**Rules that hold across the whole library:**

- A `str` ends at its first zero byte. Binary data (images, archives, keys,
  packets) passed through any `str` function or field is silently cut there:
  use the `*Bytes`/`Raw`/`ByteBuf`/pointer-and-length forms the package
  offers (`archive.ReadBinaryFile`, `crypto.Sha256Bytes`, `tcp.ReadBytes`,
  `aws.S3GetBytes`, `Message.data`).
- Almost nothing panics. Failure is an empty string, `0`, `-1`, `false`, an
  empty vector, a `0x0` matrix or an `err &:` out-parameter, so check the
  result: a missing file, an unknown JSON key and a server that refused a
  command all look like empty data.
- `int` is 32 bits. Helpers that return `int` (`json.GetInt`,
  `yaml.GetInt`, `conv.Atoi`, `"..".to_int()`) clamp or wrap anything larger;
  use the `i64` form (`conv.ParseInt`, `json.Unmarshal` into an `i64` field)
  for IDs, sizes and timestamps.
- Std functions that return a `str` hand you a fresh string you may free;
  arguments are never modified.
- Packages that keep state in globals are not thread-safe: `regex`, the
  one-shot `http.Get`/`Post` client, `log` settings, `testing` counters,
  `debug`, `grpc.ServeConn`. Values with a `New...` constructor (`sync`,
  `atomic`, `chan`) crash when used as a zero `{}` literal.
- `b := a` on a heap value (`Vector`, `HashMap`, `bigint.Int`,
  `decimal.Dec`, `matrix.Matrix`, a struct holding one) shares the storage;
  take a real copy with `v.copy()`, `collections.clone(v)` or the package's
  `Clone`.

### Text & formatting

- **`str`**: `Upper Lower Title Trim TrimLeft TrimRight TrimPrefix TrimSuffix Reverse Repeat Concat Substr Split Join Fields Chars CharAt Contains Find IndexOf IndexFrom LastIndex Count StartsWith EndsWith Equals EqualFold CompareFold Compare Replace Len CharCount CharSubstr CharFind CodePoints FromCodePoints Graphemes GraphemeCount ReverseGraphemes NFC NFD NFKC NFKD Clone FromInt(i64) FromFloat(f64) ToInt ToFloat IsEmpty IsDigit IsAlpha IsAlnum IsSpace IsPunct ToEnglishDigits ToPersianDigits ToArabicDigits DigitValue IsAnyDigit IsPersianDigit IsArabicDigit HasEnglishDigits HasPersianDigits HasArabicDigits`; plus a `StringBuilder` (`NewBuilder`,
  `BufAppend`, `BufAppendChar`, `BufAppendCp`, `BufAppendInt`, `BufStr`,
  `BufReset`, `BufFree`). `Len`, `Substr`, `IndexOf` count **bytes**;
  `CharCount`, `CharSubstr`, `CharFind` count code points; use those for
  Persian. `Upper`/`Lower` change **ASCII only** (`Upper("café")` is
  `CAFé`; use `unicode.ToUpper` per code point, or `EqualFold` to compare),
  and `Trim` strips only ASCII space, tab, CR and LF (a no-break space
  stays). `ToInt` returns a 32-bit `int` and clamps.
- **`fmt`**: `Sprintf(tmpl, Vector<str>)` (`{}` placeholders), `Fprintf`,
  `PadLeft PadRight Center` (count characters), and `Int Float FloatPrec Bool Hex Uint` to build the string args you pass to `Sprintf`. There are no
  verbs (`%d`, `{:.2}`): format the value first with `FloatPrec`/`PadLeft`.
  A literal `{}` cannot be written in a template. `Hex`/`Uint` take a `u64`:
  `fmt.Hex(n as u64)`.
- **`conv`**: `FormatInt FormatUint FormatHex FormatBool FormatFloat`
  (shortest round-trip) `FormatFloatShort` (6 digits, like `%g`)
  `FormatFloatPrec ParseInt(i64) Atoi(int) ParseFloat ParseBool ParseHex IsNaNF`. Nothing reports bad input: `ParseInt("abc")` is 0, `ParseInt`
  wraps past 64 bits and `Atoi` keeps only 32 (`Atoi("3000000000")` is
  `-1294967296`), `ParseBool` is case-sensitive (`"True"` is false), and a
  comma is not a decimal point (`ParseFloat("1,5")` is 1). Persian digits
  are accepted.
- **`text`**: `ToUtf16(str): void*` (yours to `mem.Free`), `FromUtf16(void*): str`,
  `Utf16Len` (what JavaScript and browsers count), `Utf16UnitAt` (no bounds
  check). For Windows/UTF-16 FFI.
- **`template`**: `Render(tmpl, ctx)` (no escaping) `RenderHTML` (escapes
  `& < > " '`) `NewContext Var EscapeHTML`. Only `{{name}}`,
  `{{#if name}}...{{else}}...{{/if}}` and `{{! comment }}`: no loops,
  filters or dotted paths, and the context holds strings only. Escaping is
  not enough inside `<script>` or for a `href` (`javascript:` passes).
- **`unicode`**: one code point at a time. `Category CategoryName IsLetter IsAlphabetic IsUpper IsLower IsDigit IsNumber IsAlnum IsPunct IsSymbol IsSpace IsControl IsMark ToUpper ToLower ToTitle ToFold Script ScriptName BidiClass PairedBracket CombiningClass Compose GraphemeBreak`. Case maps
  are the simple 1:1 ones (`ß` stays `ß`).
- **`arabic`**: Persian/Arabic normalisation for search. `Normalize(s, flags)` with the flags `FoldLetters StripHarakat StripTatweel FoldDigits FoldAlef FoldTehMarbuta StripZWNJ FoldPresentation`, the presets
  `NormalizePersian SearchKey EqualLoose`, and `IsArabicScript IsHaraka IsTatweel`. `SearchKey` removes the ZWNJ (`هوش‌مند` matches `هوشمند` but
  not `هوش مند`), lowers ASCII only, and runs NFKC, which also rewrites
  `²` and `ﬁ`. Show the original text, never the key.
- **`bidi`**: the Unicode bidirectional algorithm. `ParagraphLevel(s, DirAuto|DirLTR|DirRTL) Levels VisualOrder Reorder`. It does not mirror
  brackets, shape letters or break lines; reorder one line at a time.
- **`i18n`**: `New(fallback) Set Has T Tf Interpolate Plural LoadLocale DetectLocale Free`. `Plural` knows only `.one` (exactly 1) and `.other`;
  an empty translation counts as missing; `DetectLocale` reads
  `SALAM_LOCALE`, then `LANG`, then `LC_ALL`.
- **`types`**: classify text: `IsInt IsFloat IsNumeric IsBool KindOf KindName` (`TypeKind*`). `IsInt` checks characters only (a 20-digit number
  passes, then overflows); only `true`/`false` are bools.

### Numbers

- **`math`**: consts `PI E TAU`; `Sqrt Cbrt Pow(**) Hypot Exp Log Log2 Log10 Sin Cos Tan Asin Acos Atan Atan2 Sinh Cosh Tanh Floor Ceil Round Trunc Abs Sign Min Max ClampF Lerp Radians Degrees Mod IsNaN IsInf NaN Inf`; integer:
  `MinI MaxI AbsI ClampI Gcd Lcm Factorial Pow10` and `MinI64 MaxI64 AbsI64 GcdI64`. `Round` rounds halves toward +infinity (`Round(-2.5)` is -2),
  `Mod` keeps the sign of the first argument like `fmod`, `Factorial` (i64)
  is exact only up to 20.
- **`bigint`**: arbitrary-precision signed integers, one `Int` type (sign +
  base-2^32 magnitude). Build: `Zero One FromInt FromUint Parse ParseBase ParseOr FromBytes Clone`; inspect: `Sign IsZero BitLen Bit IsOdd IsEven Cmp CmpAbs Equals Less Greater Min Max`; arithmetic: `Add Sub Mul Sqr Neg Abs AddInt SubInt MulInt Shl Shr`; division in **both** conventions - `DivMod Div Rem` truncate toward zero (matching Salam's `/` and `%`), `FloorDivMod FloorDiv FloorMod` round toward minus infinity, and `Mod` always lands in
  `[0, |m|)`; number theory `Pow Pow10 ModExp Gcd Lcm ExtGcd ModInverse Sqrt Factorial`; two's-complement bitwise `And Or Xor AndNot Not PopCount TrailingZeros`; out: `ToStr ToStrBase ToHex ToInt ToIntClamped ToFloat ToBytes ByteLen`. In-place forms (`Set SetInt AddTo SubFrom MulBy AddIntTo MulIntBy NegateIn ShlBy ShrBy`) mutate `x &: Int` and free the old value, for loops that
  should not allocate per iteration. **Every returning function hands over a
  fresh value the caller frees** (`defer x.free()`); operands are taken by
  value and are never mutated, so `Mul(x, x)` is fine. Not constant-time - for
  a secret exponent use the blinded routine in `packages/ssh/bigint.salam`.
  `b := a` shares the limbs (an in-place call on `b` then frees what `a`
  points at): `Clone` for a second number. `Div Rem Mod ModExp` by zero
  return 0 (use `DivMod`, which returns false); a negative `Pow` exponent,
  `Sqrt`/`Factorial` of a negative and a `ToStrBase` base outside 2..36
  give 0 / `""`.
- **`decimal`**: exact fixed-point decimal - **this is the money type**. A
  `Dec` is a `bigint.Int` plus a scale, so `0.1 + 0.2` is exactly `0.3` and
  nothing overflows. Build: `Zero FromInt FromScaled(1999, 2)` → 19.99 `New Parse ParseOr FromFloat` (lossy, documented); `Add Sub Mul` are **exact and
  never round** (Add/Sub take the larger scale, Mul the sum); `Div` cannot be,
  so it requires a scale _and_ a `Rounding` mode - `Div DivRound DivInt Pow MulInt AddInt SubInt Percent Sum`. Rounding: `Rounding.{Down,Up,Floor,Ceil, HalfUp,HalfDown,HalfEven}` with `Rescale Round`(HalfEven) `RoundHalfUp Truncate Trim`. Compare `Cmp Equals Less Greater Min Max Sign IsZero IsNegative Scale Unscaled` (1.0 equals 1.00). Out: `ToStr` (**always plain
  notation, never exponential**) `ToStrFixed FormatGrouped ToInt ToFloat`.
  Money splitting that cannot lose a cent: `Split(total, n)` and
  `Allocate(total, ratios)` guarantee the parts sum back to the total exactly
  (100.00 by 3 → 33.34, 33.33, 33.33); free the result with `FreeAll`.
  Pitfalls: each `Mul` adds the scales, so round after every step a
  statement shows; `ToStrFixed FormatGrouped Round Percent` round half to
  **even** (2.345 shows as 2.34, call `RoundHalfUp` first); `Div DivRound DivInt` by zero return 0; `b := a` shares digits (`Clone`).
- **`rand`**: `Seed SeedAuto Int Int32 IntN IntRange Int32Range IntWithLength FloatRange Float Bool BoolP Choice{Int,Str,Char} Shuffle{Int,Str} Bytes TextFrom Alpha Alnum Digit Text UUID Hash`. **Not for secrets** (clock
  seeded, predictable): tokens and keys come from `crypto.RandomToken`/
  `RandomBytes`. `IntN(0)` and a `Choice*` on an empty vector stop the
  program (modulo by zero); `IntRange(lo, hi)` with `lo > hi` returns
  numbers outside the range; `ChoiceChar`/`TextFrom` pick bytes, so Persian
  letters break (use `ChoiceStr` over whole letters); `Bytes` never yields
  0, so it is not key material.
- **`stats`**: `Sum Mean Median Min Max Variance PopVariance Stdev PopStdev Geomean Percentile(v, p)` over `Vector<f64>`. An empty vector gives 0, which
  looks like data; `Variance`/`Stdev` are the sample (n-1) forms.
- **`matrix`**: dense `f64` linear algebra, one row-major `Matrix` for
  matrices and vectors. Build `Zeros Ones Full Eye Diag FromArray RowVector ColVector Arange Linspace Clone`; elementwise `Add Sub MulElem DivElem Scale Neg AddScaled` (+ `...InPlace`), `AddRowVector`-style broadcasts and
  ufuncs `Abs Sqrt Exp Log Round Clip Chop`; products `MatMul TransposeMul MulTranspose MatVec Dot Outer Kron MatPow Trace`; shape `Reshape Row Col SubMatrix HStack VStack Triu Tril`; reduce `Sum Mean Min Max Var Std Median ArgMin ArgMax`; norms `Norm1 Norm2 NormInf NormFrobenius Cond2`; solve and
  factor `Det Solve Inverse LeastSquares Decompose QRDecompose Cholesky SVDecompose EigSym EigVals MatrixRank PseudoInverse NullSpace RREF`,
  `Expm Sqrtm`, iterative `SolveCG SolveJacobi PowerIteration`, statistics
  `Covariance Correlation PCAFit LinearRegression`, named `Hilbert Vandermonde Toeplitz Random`, output `ToString ToStringPrec Print ToCSV`.
  Shape errors and singular systems return a `0x0` matrix rather than
  panicking: check `is_empty()`. Compare with `AllClose`; `free` results in
  loops.

### AI and machine learning

One family of packages, all pure Salam, seeded and deterministic (a fixed
seed gives the same result on every machine at any thread count). The deep
learning half is `f32` on `packages/tensor`; the classical half is `f64` on
`std/matrix`. Shape errors return the empty tensor / `0x0` matrix.

- **`tensor`**: n-d `f32` arrays, contiguous row-major, no views. `Tensor {data: Vector<f32>, shape: Vector<int>}` with `rank numel dim at set_at at2 set_at2 reshape_ free`; build `Zeros(shape) Zeros1..4 Ones Full Scalar FromArray Arange Clone CopyInto Shape1..4`. `Add Sub Mul Div` broadcast
  (equal, scalar, trailing `[N,D]+[D]`, `[N,1]`; `BroadcastShape Expand` for
  the general case), plus `AddScalar Scale Neg Exp Log Tanh Sigmoid Relu Gelu Sqrt Square PowScalar Clip` and `*InPlace`/`*Into` forms; shape `Reshape Permute SliceAxis0 Narrow Concat2 Stack2 Pad2D`; reduce `SumAll MeanAll MaxAll ArgMaxAll SumAxis MeanAxis ArgMaxAxis Softmax LogSoftmax`; products
  `MatMul MatMulInto BatchMatMul MatVec Dot Transpose2D`; conv
  `Conv2DForward MaxPool2DForward Im2Col`; random `NewRng FillUniform FillNormal RandNormal`; masks and gathers `Greater EqualMask Where MaskedFill IndexSelect Gather OneHot TopK Tril Cumsum`; `SetThreads`,
  `ToMatrix FromMatrix WriteTensor ReadTensor AllClose ShapeString`.
- **`autograd`**: tape-based reverse mode. `NewTape(seed)`, leaves `Input`
  (constant) `Watch` (tape-owned grad) `Param(val, grad)` (accumulates into the
  caller's buffer), recorded ops `Add Sub Mul Div Neg Scale AddScalar Exp Log Tanh Sigmoid Relu Gelu Sqrt Square PowScalar MatMul BatchMatMul Sum Mean SumAxis MeanAxis Softmax LogSoftmax Reshape Transpose2D Permute Embedding Dropout LayerNorm RMSNorm BatchNorm GroupNorm Narrow SliceAxis0 Concat2 RoPE Silu Conv2D MaxPool2D AvgPool2D`, `Constant` (an untracked tensor on the tape),
  losses `MSELoss CrossEntropyLogits BCEWithLogits`, then `Backward(tp, loss)`, `Value Grad`,
  `tp.reset()` per step, `tp.free()`. `GradCheck` does central differences.
- **`nn`**: `ParamStore` (owns values and grads; layers hold int handles)
  with `add at zero_grad free`, `Bind`; layers `Linear Conv2D MaxPool2D AvgPool2D LayerNorm BatchNorm GroupNorm RMSNorm Embedding Dropout LSTM GRU MultiHeadAttention TransformerEncoderLayer` (`New*` constructors,
  `forward(tp, ps, x)`; `NewTransformerDecoderLayer` sets `attn.causal`, and
  `CausalMask(seq)` is the additive mask it adds), recurrent helper
  `LastStep`, the `Layer` interface with `Sequential` and the
  `ReluLayer GeluLayer TanhLayer` wrappers, `Flatten SinusoidalPositions`,
  init `XavierUniform XavierNormal KaimingUniform KaimingNormal`, checkpoints `SaveCheckpoint LoadCheckpoint`
  (SLMT, `.gz` aware) and the `*Buf` forms.
- **`optim`**: `SGD` (momentum, nesterov, weight decay) `Adam` `NewAdamW`
  `RMSProp` with `step(ps)`, `ClipGradNorm ClipGradValue`, schedules `StepLR CosineLR WarmupCosine`.
- **`ml`**: struct-per-estimator over `matrix`; `fit(x, y): bool`, `predict`,
  `predict_proba`, `transform`, `score`, `free`, hyperparameters as fields
  (`ml.KMeans { k = 3, seed = 7 }`). Preprocessing `StandardScaler MinMaxScaler LabelEncoder OneHotEncoder SimpleImputer PolynomialFeatures`;
  models `LinearReg Ridge Lasso NewElasticNet LogisticRegression KNNClassifier KNNRegressor GaussianNB MultinomialNB DecisionTree RandomForest AdaBoost GradientBoosting HistGradientBoosting LinearSVM SVC LDA QDA KMeans MiniBatchKMeans DBSCAN Agglomerative GaussianMixture PCA TruncatedSVD TSNE` (+ `KDTree`, `workers` on KNN/KMeans); metrics
  `Accuracy ConfusionMatrix Precision Recall F1 RocAuc RocCurve LogLoss MSE MAE RMSE R2Score SilhouetteScore ...`; selection `TrainTestSplit KFold StratifiedKFold CrossValScore GridSearch`. Labels are a one-column matrix of
  0, 1, 2...; `fit` returns false on bad shapes; fit scalers on the training
  rows only.
- **`data`**: `MakeBlobs MakeMoons MakeCircles MakeRegression MakeClassification` (seeded), `LoadCsv LoadCsvFile` → `Table` (`kinds to_matrix split_xy cat_levels`), `MatrixDataset.batches` → `BatchIter`
  (`next next_tensor reset`), `LoadIdx LoadMnist ParseIdx`, `LoadImageFolder`,
  `XTensor YLabels`.
- **`npy`**: `ReadNpy ReadNpyFile WriteNpy WriteNpyFile ToMatrix FromMatrix NpzList NpzRead NpzWrite`.
- **`onnx`**: `LoadOnnx ParseOnnx Run` for MLP, CNN and transformer-style
  graphs (`Conv Gemm MatMul` activations, pooling, the norms, shape ops,
  reductions, `Gather Slice Where Cast` and more, about 65 ops); anything
  else fails with the op named in `model.error`.
- **`gguf`**: `LoadGguf ParseGguf` (metadata, tensor directory),
  `LoadTensorF32` dequantizing `F32 F16 Q8_0 Q4_0 Q4_1`.
- **`llm`**: decoder-only models (Llama layout: RMSNorm, RoPE, GQA,
  SwiGLU). `Config Model RandomModel NewKVCache(cfg) Forward(m, cache, tokens)` (positions continue from `cache.len`), `LogitsRow Greedy`,
  `Sampler` (`temperature top_k top_p repeat_penalty`) with `GenerateIds Generate`, `NewTokenizer` (`TOK_SPM`, `TOK_BPE`; `encode decode`), and
  `LoadGgufModel ModelFromGguf TokenizerFromGguf`.
- Pitfalls across the family: a shape mismatch silently yields an empty
  tensor (check `is_empty()`); the `*InPlace`/`*Into` forms do not
  broadcast; `tp.reset()` frees every value and gradient the tape made, so
  `tensor.Clone` what you keep; gradients accumulate (`zero_grad` every
  step; `step` does not clear them); set `tape.training = false` to
  evaluate; a checkpoint saves the `ParamStore` only (no BatchNorm running
  stats, no optimiser state); `Adam` defaults to lr 0.001, `SGD` 0.01.
  `gguf`/`llm` read files under 2 GB with `F32 F16 Q8_0 Q4_0 Q4_1` weights
  only (no K-quants such as `Q4_K_M`, no BF16), expand every weight to f32
  (Qwen2.5 0.5B needs about 3 GB), support Llama-layout models (Llama,
  Qwen2, Mistral; not Phi), and `encode` does not recognise special tokens
  like `<|im_start|>` (insert their IDs). `onnx` runs about 65 ops, all in
  f32, with no grouped/dilated `Conv` and no external-data models.

### I/O, OS, filesystem

- **`io`**: `ReadFile WriteFile AppendFile Lines WriteLines Input Read Write ReadAll Readline Seek Close Copy EPrint EPrintln Flush`. A missing file is
  not an error: `ReadFile` gives `""` and `Lines` one empty line (check
  `os.Exists`). Lines split on `\n` only (a Windows `\r` stays), `Readline`
  returns `""` for a empty line and at EOF alike, and everything is `str`,
  so text only. `print`/`println` to a pipe or file are buffered:
  `io.Flush()` pushes the buffer out, and a panic loses what is still in it
  (`printerrln` is unbuffered).
- **`os`**: `Args Env Cwd Chdir Exit Abort Pid Run RunCapture Output Exists IsDir FileSize Stat ReadFile WriteFile AppendFile Copy CopyTree Move Remove RemoveAll Mkdir MkdirAll Rmdir ListDir ListDirs Walk TempDir Open`.
  `Run`/`RunCapture`/`Output` hand the whole string to `/bin/sh` (never
  splice user input in); `RunCapture` captures stdout only (add `2>&1`).
  **`RemoveAll` and `Walk` follow symlinks to directories**, so `RemoveAll`
  deletes the link target's contents and `Walk` never ends on a loop.
  `Move` is a rename and fails (-1) across file systems. `Exit` skips
  `defer` (E109 inside `main`; use `ret`). `Args()[0]` is the program.
- **`os.process`**: `Start(path, args) StartCaptured Wait Kill Terminate ReadStdout ReadStderr Cleanup ExitCode Pid IsRunning`. `IsRunning` is a
  flag, not a live check; `Kill` only signals, so `Wait` after it (zombie);
  `Wait` has no timeout; `Cleanup` a captured run (two temp files).
  **`os.shell`** `Run Exec System` (code, stdout and stderr together),
  **`os.temp`** `File Directory Remove RemoveDir` (`Directory` returns `""`
  on failure; `File` always returns a path), **`os.user`** `Username HomeDir ConfigDir CacheDir` (env based; never for permissions), **`os.platform`**
  `IsLinux IsWindows IsMac IsWasm OS Arch IsX64 IsArm64`, **`os.error`**
  `Last LastCode LastMessage HasError Clear` (the last file/process error).
- **`fs.file`**: `Open Close ReadText ReadAll ReadBytes ReadLines WriteText WriteBytes Append AtomicWrite Size ModTime Stat Copy Move Rename Delete Exists Extension MimeType Md5Hex Sha256Hex`. **Not binary-safe**: every
  read, write and digest goes through `str` and stops at the first NUL
  (`ReadBytes` too); use `archive.ReadBinaryFile`/`WriteBinaryFile` and
  `crypto.Sha256Bytes`. `Exists` is true for directories too; `AtomicWrite`
  resets permissions to the default.
- **`fs.dir`**: `Mkdir MkdirAll Remove RemoveAll List ListDirs ListFiles Walk Copy Move Exists IsDir Size`. `Exists` is true only for a directory;
  `Copy`/`Move` merge into an existing target; `RemoveAll Walk Size Copy`
  follow symlinks. **`fs`** itself: `ModTime AccessedTime CreatedTime Chmod Permissions` (`CreatedTime` is 0 on most Linux file systems).
- **`fs.path`** (`Join Split Clean Normalize IsAbs Abs Rel Parent Dirname Filename Basename Ext Stem IsSafe Validate`) and **`filepath`** (the same
  work, never touching the disk: `Join Dir Base Ext Stem Clean Normalize IsAbs Abs Rel IsSafe Validate Separator`). `Join` does not clean
  (`Join("a/b", "../c")` is `a/b/../c`) and an absolute second part is
  appended, not substituted; `Ext` is only the last extension; check user
  paths with `IsSafe` before using them.
- **`env`**: `Get GetOr Has GetInt GetFloat GetBool Set Unset Mask GetMasked Profile IsProduction IsDevelopment Load Overload LoadDotEnv LoadProfile Expand`. `GetInt` turns `abc` into 0 and `12px` into 12; `GetBool` accepts
  only lower-case `true 1 yes on`; `Load` never overrides a set variable
  (`Overload`/`LoadDotEnv` do).
- **`flag`**: `New(name, desc) AddStr AddInt AddBool AddFloat Parse(fs, args) GetStr GetInt GetBool GetFloat WasSet HelpWanted Positional Usage Error Free`. `Parse` takes the arguments without the program name; a value
  flag takes the next argument whatever it is (`--name --verbose`); a
  negative positional needs `--` first.
- **`cli`** (subcommands): `New Register Dispatch RemainingArgs Usage RequireFlags Confirm Prompt PromptOr`, `CmdHelp CmdVersion CmdUnknown`.
  `Dispatch` wants `RemainingArgs(os.Args())`; it owns `help -h --help version -v --version`.
- **`config`**: INI/env/YAML settings with dotted keys: `LoadINI LoadEnv LoadYAML Parse* Get GetOr GetInt GetFloat GetBool Has Set Remove Keys Section Merge Format* Free`. Nothing reports an error (a missing file is an
  empty config); in INI/env a `#` after a space starts a comment, so quote
  passwords.
- **`log`**: `Debug Info Warn Error Fatal` + `*f` and `*KV` variants, `KV Field FieldInt FieldBool`, `SetLevel SetPrefix SetFormat(FormatText| FormatJSON) ShowTime ToFile ToStdout ToStderr SetRotation CloseFile`. A
  file log is buffered (call `CloseFile` on every exit path); settings are
  unlocked globals (set them at startup); text format does not escape line
  breaks (use JSON for user input).
- **`debug`** `Assert Enter Exit Stack Trace StartTimer StopTimer MemStats`
  (`Assert` runs in release builds too), **`runtime`** `OS Arch CpuCount TotalMemory AvailableMemory UsedMemory MarkStart UptimeMs BuildFlags`
  (machine-wide figures, -1 when unknown, each call on Windows starts
  PowerShell), **`error`** structured errors: `New Newf Wrap WrapKind WithMeta Kind Is Message RootMessage Format Ok IsNil Free`, `Kind*`
  constants, `IsNotFound IsTimeout IsRetryable ...`, `NewRetryPolicy DelayForAttempt ShouldRetry` (you do the sleeping; `Wrap` of a nil error
  makes a real one, so check `Ok` first). **`core`**: `Panic(msg)` (abort,
  exit 134, `defer` does not run).

### Terminal & TUI

- **`term`**: everything ANSI. **Never hand-roll `\x1b[` sequences** - this
  package is the one place they belong.
  - Terminal: `IsTTY IsTerminal Size Cols Rows Write WriteErr Emit Flush Bell SetTitle EnableAnsi`. Raw mode: `MakeRaw` / `MakeCbreak` (Ctrl-C
    still works) / `Restore`; **always `defer term.Restore(st)` on the next
    line**, or the user's shell stays raw.
  - Colour: `Red Green Yellow Blue Magenta Cyan White Gray Bright* On* Bold Dim Italic Underline Inverse Strike Style Paint Color256 RGB OnRGB`, raw
    `Sgr Seq{Fg,Bg}256 Seq{Fg,Bg}RGB`. All honour
    `SetColorMode(ColorAuto|ColorAlways|ColorNever)`; ColorAuto obeys
    `NO_COLOR`/`CLICOLOR_FORCE`/`TERM=dumb`/isatty, so one code path serves a
    terminal and a pipe. Probes `SupportsColor Supports256Color SupportsTrueColor`.
  - Escape-aware `Strip Width Truncate PadRight PadLeft Center`: use these,
    not `len`/`fmt.PadRight`, on coloured text. `Width` counts one column per
    code point (CJK and the ZWNJ are miscounted).
  - Cursor/screen, each a pure `Seq*` builder plus a write-now twin: `MoveTo Up Down Left Right Column Home Save/RestoreCursor Hide/ShowCursor ClearScreen ClearLine ClearBelow Scroll{Up,Down} SetScrollRegion Enter/ExitAltScreen Enable/DisableMouse`; build a frame with `Seq*` and
    write once.
  - Input: `Decode DecodeMouse` (pure, testable without a tty), `ReadKey GetKey ReadByte KeyReady ReadPassword`, `Key`/`MouseEvent`, `Key*` codes.
  - Progress: `NewBar BarSet BarAdd BarDraw BarFinish`, `NewSpinner SpinnerTick SpinnerStop SpinnerFree`, `FormatBytes FormatDuration`; off a
    terminal they write one plain final line.

```salam
import term
func main:
    mut st := term.MakeCbreak(term.StdinFd)
    defer _ok := term.Restore(st)
    term.Emit(term.SeqHideCursor() + term.SeqClearScreen() + term.SeqMoveTo(1, 1))
    println term.Bold(term.Green("ready")), term.Dim("(q to quit)")
    until true:
        k := term.ReadKey()
        if k.code == term.KeyChar and k.ch == "q": break end
        if k.code == term.KeyUp: println "up" end
    end
    term.ShowCursor()
end
```

### Collections (heap-allocated: call `.free()`, idiom `defer x.free()`)

- **`Vector<T>`** (built-in; also `import collections`): `push pop get(i) ref(i) set(i,x) len is_empty first last insert remove_at reserve clear iter copy free`; index via `v[i]` (read) / `v[i] = x` (write); free functions
  `contains index_of last_index_of count_of slice clone reverse swap extend`.
- **`HashMap<K,V>`**: `put(k,v) get(k) get_or(k, def) has(k) remove(k) size is_empty iter copy free`; iterate with `each k, v in m:`. `get` of a
  missing key returns the zero value (a **null** `str` that crashes when
  used), so test `has` or use `get_or`. Iteration is in hash order, which
  changes as the map grows; sort the keys when order shows. `get` returns a
  copy of a struct value: change it and `put` it back.
- **`Set<T>`** (`add has remove size`), **`Stack<T>`** (`push pop peek size is_empty`), **`Queue<T>`** (`enqueue dequeue peek size`),
  **`Deque<T>`** (`push_front push_back pop_front pop_back front_val back_val`),
  **`PriorityQueue<T>`** (`push pop peek`, a **min**-heap on `<`),
  **`LinkedList<T>`**, **`Counter<K>`** (`add add_n count distinct`),
  **`OrderedMap`** (str to str in insertion order: `set get get_or has remove key_at val_at len`), **`Pair`**, **`CircularList`**.
- Pitfalls: `b := a` shares the elements (`b.set(0, x)` changes `a`, a push on
  one can move the other's memory); copy with `v.copy()`. `pop peek dequeue first last` and an index past the end stop the program on an empty
  structure (a `--release` build reads garbage instead).
- **`sort`**: `Sort SortDesc SortBy StableSortBy Sorted SortedBy IsSorted SortIndex BinarySearch LowerBound UpperBound Min Max Reverse Swap` + named
  algorithms (`QuickSort MergeSort HeapSort IntroSort …`). Comparators:
  `func (T, T) bool` (less). **`Sort`, `SortDesc`, `SortBy` and `SortIndex`
  are not stable** (use `StableSortBy`), and `SortBy` goes quadratic on
  many equal keys. `BinarySearch`/`LowerBound` on an unsorted vector answer
  wrongly without error; `Min`/`Max` of an empty vector stop the program.
  Strings sort by code point, which is wrong for Persian (`پ چ ژ ک گ` after
  `و`).
- **`option`**: `Some None IsSome IsNone Unwrap UnwrapOr Expect`, fields
  `.present .value`. Never read `.value` unchecked: on an empty
  `Option<str>` it is a null pointer and printing it segfaults. No `match`
  and no `?` on an Option.
- **`result`**: `Result<T, E>` (`Ok`/`Err`) with `IsOk IsErr UnwrapOr Expect`
  and postfix `?` (error types must match). There is no `Unwrap`, `map` or
  `and_then`; `Expect` does not print the error.

### Data formats

- **`json`** (`import encoding.json`):
  `Valid Get GetInt GetFloat Has Keys Minify Indent Escape`
  `Object(members) Array(items)`
  `Member/MemberInt/MemberBool/MemberFloat/MemberRaw Str`. Typed codecs the
  compiler derives per type:
  `Marshal(v) MarshalIndent Unmarshal(text, out, err) UnmarshalLenient`,
  with `@json "wire"` to rename a field, `@json "-"` to drop it, and
  `@json "" "omitempty"/"optional"/"string"` for the rest. An enum with data
  is written with its member as the key, `{"Circle":{"r":1.5}}`, and a member
  without data as a bare string, `"Empty"`.
  `Schema(v)` derives the type's **JSON Schema** (2020-12, `$defs`/`$ref`)
  from the same declaration and markers (the argument's value is unused); a
  `@doc "..."` marker on a struct, enum or field becomes its `description`
  (one `@doc` per definition); `SchemaEnvelope`/`SchemaMerge` give the parts.
  Also `GetPath(text, "a.b.0.c")`, `Items Unquote`, and the dynamic tree
  `Decode(text, err): value.Value` / `Encode(v, opts)` / `EncodeDefault`.
  Pitfalls: **`GetInt` returns a 32-bit `int` and clamps** (3000000000 reads
  as 2147483647): read IDs and timestamps by `Unmarshal` into an `i64`.
  `Get GetInt Has` see only the top level (`GetPath` for nested). `Unmarshal`
  fails on a missing field, an unknown key or a wrong type;
  `UnmarshalLenient` keeps defaults and skips unknown keys. Reset the
  `encerr.EncodingError` before each call.
- **`xml`** (`import encoding.xml`): two models in one package.
  - **Simple tree** `XMLNode` (`tag attrs children text`): `Decode DecodeStrict Valid Encode EncodeIndent Child Children Attr HasAttr SetAttr AppendChild NewXMLNode ToValue FromValue EscapeText EscapeAttr`, plus the
    tokenizer `NewXMLTokenizer`/`NextToken` (checks nothing; run
    `WellFormed` first). Ignores namespaces and mixed-content order: fine
    for a config file.
  - **Full `Document`** (an arena of `int` node handles, strict parsing with
    line/column errors, namespaces, bounded entity expansion): `Parse(text, err) ParseWith(text, opts, err) WellFormed`, `ParseOptions`
    (`preserve_whitespace keep_comments namespaces expand_entities max_depth`; whitespace-only text is dropped by default). Read `Root Kind Name LocalName NamespaceURI Parent ChildCount ChildAt Elements ElementsNamed FirstElementNamed Descendants NextSibling AttributeValue AttributeValueNS Text Line Column`; edit `NewDocument CreateElement CreateText Append InsertBefore RemoveChild SetAttribute SetText`; write
    `Write WriteIndent WriteNode Canonical`. `Free(d)`, `FreeList(list)`.
  - **XPath 1.0**: `Select(d, context, expr, err): NodeList` (context `-1` =
    document) `SelectFirst SelectString SelectNumber SelectBoolean Evaluate`;
    all axes and the core function library, no variables, no namespace axis.
    Name tests match the name as written, prefix included: use
    `local-name()`/`namespace-uri()` when the prefix may vary. Attributes
    come back as negative handles (`IsAttributeRef ReferencedAttribute`).
  - **DTD validation**: `ParseDTD Validate ValidateInternalSubset CheckNamespaces ApplyDefaults` returning a `Report` (`ok`, `items` of
    `Violation`, `ReportText`, `FreeReport FreeDTD`).
- **`yaml`** (`import encoding.yaml`): `Valid Get GetInt GetFloat GetBool Has Keys Len TypeOf Parse Decode ToJSON Encode Dump DumpWith SplitDocuments`
  plus the `Object Array Member*` builders. No anchors or aliases (`&a`,
  `*a`, `<<:`; an anchored map silently drops the rest of the document);
  `yes`/`no`/`on`/`off` are strings; `GetInt` is 32-bit and wraps; every path
  call reparses, so `Decode` once for many reads.
- **`csv`** (`import encoding.csv`): `ReadLine(str): Vector<str>`,
  `WriteLine(Vector<str>): str`, `ReadAll WriteAll FreeTable ToValue FromValue`. The separator is always a comma; a quoted field with a line
  break cannot be read back (`ReadAll` splits lines first); `"Pen" ,blue`
  (space after the quote) ends the row silently; spaces are kept.
- **`toml`** (`import encoding.toml`): `DecodeStrict Decode Valid EncodeStrict Encode` over `value.Value`. `Decode` never reports errors
  (a typo drops the rest), so use `DecodeStrict` on edited files. Triple
  quoted strings are invalid and dates stay strings; `Encode` loses comments
  and key order.
- **`value`** (`import encoding.value`): the dynamic `Value` every format
  shares: `ValueNull ValueBool ValueInt ValueFloat ValueString NewValueArray NewValueObject Is* As* ArrayLen ArrayGet ArraySet ArrayPush ObjectGet ObjectGetOk ObjectSet ObjectHas ObjectDelete ObjectKeys ValueClone ValueFree Equal`. `As*` never checks the kind (`AsInt` of a string is 0);
  `ObjectGet` of a missing key is null (chains are safe) but `ArrayGet` past
  the end stops the program; a copy shares its map/elements (`ValueClone`);
  keys come back in hash order; `Equal(1, 1.0)` is false.
- **`serde`** `Detect DetectFormat Transcode TranscodeStrict` (detection is a
  guess and almost any text is YAML: pass the format when you know it),
  **`encerr`** the shared `EncodingError` (`NoError` resets it; a reader
  only writes it on failure; `position` counts bytes), **`msgpack`**
  `Marshal Unmarshal UnmarshalBytes` (no bin/ext types, string map keys
  only, trailing bytes ignored), **`binary`** `WriteU32BE ReadI64LE ...`
  (reading past the end yields 0 and sets `r.bad`; `ReadU32*` comes back in a
  signed `int`, so widen with `(v as i64) & 4294967295`; writers truncate
  silently), **`protobuf`** (`.proto` parser, wire helpers `WriteVarint ReadVarint WriteTag ReadTag SkipField ZigZag*` and Salam code generation;
  generated readers do not accept packed repeated fields).
- **`url`** (`import encoding.url`): `Parse(s): URL HostPort Hostname Query SetParam RemoveParam KeepParams Join Domain Subdomain PublicSuffix IsIP`.
  `Parse` never fails or validates (check `scheme` and `host` yourself),
  nothing is lower-cased, and editing one parameter re-encodes the whole
  query. **`query`** (`import encoding.query`): `Get GetAll Has` on the text,
  or `Decode` to a `QueryMap` with `Set Add Remove Retain Encode`; missing
  and empty look the same to `Get`. **`envfile`** (`import encoding.envfile`): `Parse ParseStrict ParseRaw Get GetOr Set Format Expand RefNames`; `Parse` drops bad lines silently (use `ParseStrict`),
  and `${NAME}` only sees lines above it.
- **`encoding`**: `Base64Encode Base64EncodeBytes Base64Decode Base64DecodeStrict Base64DecodeBytes HexEncode HexDecode URLEncode URLDecode`. `Base64Decode`/`HexDecode` return a `str`, so binary is cut at
  the first zero byte (use `Base64DecodeBytes`); `HexDecode` stops at the
  first bad pair without error; URL-safe base64 (`-_`) is damaged by the
  lenient decoder.
- **`compress`**: `ByteBuf` (`NewByteBuf BufPush BufGet BufLen BufFree BufFromStr BufToBase64`), `Compress/Decompress(method)` over `Method*`,
  `DeflateCompress GzipCompress GzipDecompress CRC32 Adler32`. Gzip does not
  verify the CRC; the method is not stored in the output; free every buffer.
- **`archive`**: `ZipCreate ZipCreateFile ZipList ZipExtractAll ZipExtractByName TarCreate TarList TarExtractAll RarList RarExtractAll GzipCompressFile ReadBinaryFile WriteBinaryFile`. No zip64 (under 2 GB),
  ZIP methods 0 and 8 only, CRC not checked, tar names over 100 bytes are
  cut, RAR extracts only stored entries.
- **`excel`**: xlsx, ods and csv workbooks: `Open Save New NewWorkbook NewSheet` with sheet methods `set_string set_number get_cell get_string row_count ...`, `CellRef ParseCellRef SheetToJSON SheetToCSV`. No `.xls`
  (`IsXLS` detects it); the format comes from the file extension; formulas
  are not evaluated; rows and columns count from 0.
- **`regex`**: `Compile Match Find Replace ReplaceAll` (`Regex` handle) and
  one-shot `MatchStr FindStr ReplaceStr ReplaceAllStr`. Captures: `FindMatch`
  returns a `MatchResult` value (`.ok .start .stop .count`) to read with
  `Group GroupStart GroupEnd GroupOk GroupByName GroupIndex GroupCount`, plus
  `FindSubmatch FindAll FindAllN FindAllMatches Split SplitN` (all
  `Vector`-returning, so `defer v.free()`) and `$1`/`${name}` rewriting with
  `Expand ReplaceExpand ReplaceAllExpand`; `IsValid` checks a pattern. Syntax:
  `. [] \d \w \s \D \W \S ^ $ | (…) (?:…) (?<name>…) (?P<name>…) * + ? {n,m}`
  and the lazy `*? +? ?? {n,m}?`. **No `\b`** (it means `b`), **no `(?i)`**,
  no lookaround, no back references; an unsupported pattern is invalid and
  never matches (test with `IsValid`). The engine works on **bytes**: `\w`
  never matches a Persian letter, `[آ-ی]` does not work and `ب+` repeats
  only the last byte (write `(ب)+`). `^`/`$` anchor the whole text, never a
  line. At most 16 groups and `{n}` up to 64; a match over 400000 steps is
  reported as no match. Global state: one thread at a time.
- **`crypto`**: `Sha1Hex Sha256Hex Sha512Hex Md5Hex`, `Sha256Bytes/Sha384Bytes/ Sha512Bytes` (+ streaming `Sha256New/Update/Final`), HMAC
  `HmacSha256Hex/HmacSha384Hex/HmacSha512Hex` and the byte-oriented
  `HmacSha256Bytes/HmacSha384Bytes/HmacSha512Bytes` (write into a caller
  buffer) / `HmacSha256Raw/HmacSha384Raw/HmacSha512Raw` (pointer+length, for
  binary keys), `HashPassword VerifyPassword Pbkdf2HmacSha256Hex`,
  `RandomHex RandomToken RandomBytes SecretEquals`. **`HashPassword` uses
  only 1000 PBKDF2 rounds**; for new systems build the
  `pbkdf2-sha256$<rounds>$<salt>$<hex>` string yourself with
  `Pbkdf2HmacSha256Hex` and 600000 rounds (`VerifyPassword` reads the rounds
  from the string). Compare secrets with `SecretEquals`, never `==`.
  `Sha256Hex` of a `str` hashes only up to the first NUL. `HashStr HashBytes HashInt` are hash-table hashes, not cryptographic.

### Time, memory, testing

- **`time`**: `Now NowMillis NowMicros NowNanos Sleep(ms) Format FormatISO FormatDate FormatTime Year Month Day Hour Minute Second Weekday Since Until ElapsedMs MonoNanos MonoMillis` and `YearUTC MonthUTC DayUTC HourUTC ...`;
  `DateTime` type. An instant is an `i64` of Unix seconds. `Year Month Day Weekday Format FormatDate FormatISO` use the **local** zone (servers
  differ from laptops: use the `...UTC` forms and `FormatISOUTC`/
  `FormatRFC3339` for anything stored); `Now` has whole seconds and follows
  clock changes, so time code with `MonoNanos`; `Days(30)` is not a month;
  `FromUTCParts` rolls over (30 February is 1 March) while the parsers
  reject impossible days. `Sleep` takes milliseconds.
  - **`Duration`** (nanoseconds, so it cannot be read in the wrong unit):
    `Nanoseconds Microseconds Milliseconds Seconds Minutes Hours Days SecondsF ZeroDuration`, methods `nanos millis secs seconds minutes hours days add sub mul div ratio neg abs truncate round cmp less equals is_zero`; `FormatDuration` ("1h30m0s") and `ParseDuration`/
    `ParseDurationOr` (`DurationResult { ok, value }`); `SleepFor SinceDuration DurationBetween AddDuration`.
  - **Parsing**: `ParseISO ParseISOIn ParseISOOr ParseHTTPDate ParseFormat(pattern, s)` (strftime `%Y %m %d %H %M %S %f %b %a %z %Z`),
    all returning `TimeResult { ok, epoch, nanos }`; an unzoned string is
    **local** time. `FromLocalParts` (DST-correct) / `FromUTCParts`,
    `LocalOffsetMinutesAt`.
  - **Timers** (monotonic): `NewTimer` (`expired remaining wait reset stop`),
    `NewTicker` (`fired wait remaining reset stop advance`, a slow body drops
    ticks and counts them in `dropped`), `AfterFunc Every RepeatEvery WaitUntil`; a lambda captures locals by value, so callbacks share state
    through `mut` globals.
- **`calendar`**: Gregorian, `Jalali` (Persian solar) and `Hijri` (tabular
  Islamic) dates through one Julian Day Number: `GregorianToJalali JalaliToGregorian GregorianToHijri HijriToGregorian JalaliToHijri HijriToJalali`, `*ToJDN`/`JDNTo*`; per calendar `IsLeap* DaysIn*Month IsValid*Date *MonthName{En,Fa,Ar} WeekdayName{En,Fa,Ar} Compare*`, the
  system clock `Today{Gregorian,Jalali,Hijri} NowClockTime`, from an epoch
  `EpochTo{Gregorian,Jalali,Hijri,ClockTime} *ToUnixUTC`, text `Format* FormatJalaliLongFa FormatHijriLongAr FormatGregorianLongEn`; arithmetic
  `AddDays* AddWeeks* AddMonths* AddYears* DaysBetween* StartOfMonth* EndOfMonth* NextWeekdayOnOrAfter` (days are exact, **months and years
  clamp**: 2024-01-31 + 1 month = 2024-02-29). Types `GregorianDate JalaliDate HijriDate ClockTime`.
  - Zones: real IANA zones from the host tzdata, `LoadZone(name) LocalZone FreeZone ZoneOffsetSeconds ZoneIsDST ZoneAbbrevAt TimeZoneAt ZoneAt(name, epoch) ZonedToEpoch` (no tzdata on Windows: `ok = false`). The fixed
    `TimeZone` values `TZUTC TZTehran TZKabul TZRiyadh TZDubai TZIstanbul TZLondon TZNewYork TZTokyo FixedOffsetZone TZLocal` with
    `WallClock{Gregorian,Jalali,Hijri,Time} ConvertZoned*` are
    **standard time**: `TZLondon` is UTC+0 all year, so use
    `ZoneAt("Europe/London", epoch)` for DST zones.
  - Pitfalls: `TodayJalali`/`EpochToJalali` use the machine's zone (a UTC
    server is still on yesterday until 03:30 Tehran time): use
    `WallClockJalali(time.Now(), TZTehran())`. `JalaliWeekday` counts from
    Saturday = 0 and takes a JDN, the other `Weekday*` count from Sunday = 0.
    Conversions do not validate (month 13 gives a wrong date): check with
    `IsValidJalaliDate` first.
- **`mem`**: `Allocate AllocateZeroed AllocateArray Reallocate Free Copy Set MemMove`, arenas `NewArena ArenaAlloc ArenaReset ArenaFree`, fixed-size
  pools `NewPool PoolAlloc PoolFree PoolDestroy`; leak tooling `CheckLeaks LiveBytes AllocCount PeakBytes`. Nothing is checked (overruns and double
  frees corrupt silently; `salam memcheck` finds them for `Allocate`
  blocks), every allocator can return null, and memory from a C library
  goes back to that library's free, never `mem.Free`.
- **`testing`**: `AssertTrue AssertFalse AssertEqInt AssertEqStr AssertEqFloat AssertEqBool AssertEqInt64 AssertNeInt AssertNeStr AssertContains AssertNil AssertNotNil AssertMsg Passed Failed Reset Summary()` (`ret testing.Summary()` from `main`). A failure says what differed, not where
  (put the case name in `AssertMsg`); an exit code keeps 8 bits, so 256
  failures read as 0; counters are unlocked globals (assert from the main
  thread).

### Networking & web

- **`net.http`** (`import net.http`, called `http.`), client: `Get Post Put Patch Delete Head Options` (+ `*With(..., headers)` taking a
  `HashMap<str,str>` from `NewHeaders`), `Request(method, url, headers, body)`, then `Status Body Error Ok GetHeader Headers SetCookieLines` (or the
  fields `.status .body .err`); `NewClient()` keeps cookies (`c.Get`,
  `c.Post`, `c.SetHeader`). No redirects are followed and there is **no
  timeout** (use `net.httpx` for one); `Post`/`Put`/`Patch` label the body as
  a form unless you pass `Content-Type`; `Status` 0 means unreachable;
  `Ok` is 2xx; the one-shot calls share package state, so not from several
  threads at once.
- **Server** (`net.http` + `net.router`): `mut r := router.NewRouter()`,
  `router.Get/Post/Put/Patch/Delete(r, "/books/:id", handler)`, `Group Static`; `mut s := http.NewServer(port)`, `http.Use(s, r)`, then `Run(s)`,
  `RunTLS(s, cert, key)` or `RunBackground(s)` (returns once bound, serves on
  its own thread, so one process can run several servers). Limits
  `SetMaxBodyBytes SetMaxHeaderBytes SetKeepAliveIdle EnableAccessLog`. A
  handler is `func h(ctx: i64)` passed by bare name, using `Text`, `Html`,
  `Json`, `SetStatus`, `SetHeader`, `Redirect`, `Param`, `Query`, `Form`,
  `Body`, `Header`, `Method`, `Path`, `Cookies`, `CookieValue`, `HasCookie`,
  `SetCookie`, `ClearCookie` and `Own`. Each of these also takes a
  `Context`, so `c := http.Ctx(ctx)` lets you pass `c` to the same names
  instead of the raw id. Pitfalls: **register every
  route before `Use`** (the server copies the router; a later route is a
  404); **strings read from a request are freed when the handler returns**,
  so `str.Clone` what you keep in a global or session (`Own` hands a
  string you built to the request); router params are not URL-decoded; no
  automatic 405; `Body` is a `str` (binary uploads are cut).
- **`net.router`** alone: `Dispatch` (a `Match`) `FreeMatch ParamGet EnableListing` for routing without a server. **`net.http.middleware`** `Log AllowAll Secure NoCache Cache` (`AllowAll` omits PATCH; `Secure` sets no CSP or
  HSTS), **`net.http.validate`** `Required Length IsEmail Matches Assert`
  (`Required` accepts spaces, `IsEmail` is loose, `Matches` is unanchored,
  and you must `ret` after a failed `Assert`), **`net.http.session`**
  `Configure Start Save Regenerate Destroy Value SetValue CSRFToken CSRFVerify` (call `Save` after changes, `Regenerate` after login; sessions
  live in process memory).
- **`net.httpx`**: raw requests with your own header text: `SendRaw SendRawTimeout SendRawTLS FreeRaw RawText Send SendText HeaderValue`. Send
  `Connection: close` (it reads until the server closes); `SendText` does not
  decode chunked bodies (the raw calls do).
- **`net.http.swagger`** (OpenAPI + a documentation page): `New(title, version) Describe Server BearerAuth ApiKeyAuth`; `Scan(doc, router)` adds
  every registered route, `Route` creates one and `At(doc, method, pattern)`
  selects one (after an `At` that finds nothing the following calls do
  nothing), then `Summary Details Tag OperationId Deprecated Secure Query QueryTyped QueryEnum Header PathParam`. `Accepts(doc, desc, NewTodo {})`
  and `Returns(doc, 200, desc, Todo {})` take a **value and read its type**
  (the schema comes from the struct and its `@json` names), plus `ReturnsText ReturnsHtml ReturnsFile ReturnsRaw ReturnsNothing ReturnsHeader AcceptsForm AcceptsRaw`. `Spec SpecIndent WriteSpec`; check with `Problems Undocumented Missing(doc, r)`; serve with `Mount(doc, r, "/docs")` or
  `Serve(doc, port)`; `Free(doc)`. **`Mount` copies the spec when called**:
  describe every route first, then `Mount`, then `http.Use`. The docs are
  public (CORS `*`).
- **`net.websocket`** (RFC 6455 client and server, `ws://` and `wss://`):
  `Listen Accept AcceptWSS Dial DialWith DialWSS DialWSSWith DialWSSUnverified`, `SendText SendBinary SendBinaryBytes Ping Close Stop`,
  `Receive` (blocks) / **`Poll(c, ms)`** (returns a `TIMEOUT` message), plus
  a `Hub` for rooms and broadcast. A binary `Message` carries `.data`/`.len`
  next to `.text` (that buffer is reused by the next message, so copy it).
  One thread reads one connection; a `Hub` has no lock (wrap it in a
  mutex); unregister with the copy you registered, since `Receive` clears
  `conn.fd` on close.
- **`net.socketio`** (`import net.socketio`): a Socket.IO v4/v5 **client**
  (polling and WebSocket transports). `Dial(url) DialWith(url, opts)`, or
  `NewManager Open Of(m, "/nsp")` for several namespaces; the URL path is the
  NAMESPACE, the HTTP path is `Options.path` (default `/socket.io`). Send
  `Emit EmitStr EmitInt EmitJSON EmitBytes EmitArgs EmitVolatile`; `EmitAck`
  blocks for the answer, `Reply ReplyStr ReplyJSON` answer `ev.ack_id`.
  Receive with `On Once Off OnAny` + `Run`/`RunFor`, or better
  **`Next(c, ms)`/`NextEvent(c, ms)`** in a loop (a lambda captures by
  value). Arguments: `NewArgs AddStr AddInt AddJSON AddBytes`, `ArgCount ArgStr ArgInt ArgValue ArgBytes`, `Ack*`; free with `ArgsFree EventFree AckFree`. **Nothing runs in the background**: the connection only advances
  inside `Pump`/`Next`/`Run`/`EmitAck`, so a program that stops pumping
  misses heartbeats and is dropped. `Next` also returns connection events
  (`EV_CONNECT`, ...). Socket.IO 2 servers are not supported. Wire helpers
  `EncodePacket DecodePacket EncodeEngine` are public for tests.
- **`net.tcp`** (`import net.tcp`): `Bind(port) Accept Dial(host, port) Read Write ReadBytes WriteBytes Close CloseServer Ok ConnOk SetDeadline SetReadDeadline SetNoDelay UpgradeTLS DialTLS`, plus a connection `Pool`.
  An empty `Read` means closed, timed out or failed (and binary is cut at a
  NUL: use `ReadBytes`); `Write` may send part (`WriteBytes` loops); TCP has
  no message boundaries, so frame them; `Bind` listens on every IPv4
  interface. **`net.udp`** `Bind Open Send SendTo Receive ReceiveBytes SetTimeout SetBroadcast JoinMulticast` (IPv4 only; a timeout looks like an
  empty packet; a short buffer drops the rest of the packet).
- **`net.dns`** `Lookup` (system resolver, one IPv4) and `LookupA LookupAAAA LookupMX LookupTXT LookupCNAME LookupPTR QueryServer` (own UDP queries to
  `/etc/resolv.conf` servers, falling back to 8.8.8.8; no TCP retry for
  truncated answers, no DNSSEC). **`net.mime`** `MimeType MimeExt ParseContentType ParamGet ParseMultipart NewMultipartWriter` (about 25
  extensions; everything else is `application/octet-stream`; case
  sensitive). **`net.proxy`** `NewHTTPProxy NewSOCKS5Proxy Dial NewChain DialChain` (no handshake timeout, no IPv6 proxy address, ignores
  `HTTPS_PROXY`). **`net.email`**: `Send` and `BuildMessage`, or drive
  `DialSMTP/DialSMTPTLS Hello StartTLS AuthPlain MailFrom RcptTo Data Quit`;
  `DialIMAP Login Select Fetch`, `DialPOP3 Retr Dele` (`BuildMessage` adds no
  Date/Message-ID and does not encode a non-ASCII subject; IMAP `Login`
  breaks on a password with `"` or `\`). **`net.http2`** frames, HPACK and
  streams (`NewConn ClientHandshake SendHeaders SendData Poll`; one thread
  per `Conn`).
- **`tls`** (TLS 1.2/1.3 in pure Salam, no OpenSSL): `Dial WrapClient WrapServer NewConfig NewServerConfig ConnOk ConnError Read Write ReadBytes WriteBytes Close`, plus `AesGcmSeal`, `AesGcmOpen`, `HkdfSha256`, the `EcdhP256` functions,
  `P256Sign` and `P256Verify`. Check `ConnOk` after `Dial`; no client certificates;
  about 800 KB/s for bulk transfers. **`ssl`** is an OpenSSL-style API over
  it (`NewClientContext SetHostname Wrap Connect Read Write Shutdown`;
  `SetHostname` is required before `Connect`, and `Close` does not close
  the TCP socket).
- **`ssh`**: `Connect`/`Dial` with `NewConfig WithPassword WithKeyFile WithHostKeyMode`, `RunCommand Exec NewSession RequestPty Shell`, SFTP
  `SftpOpen Upload Download ListDir Mkdir Remove Rename Stat`. RSA keys only
  (OpenSSH format, no passphrase; Ed25519 and ECDSA are not read) and the
  server needs an RSA host key; each `Exec` is a fresh shell (a `cd` does not
  carry over). **`ftp`** `Dial DialTLS Login Binary List Upload Download Read Write Mkdir Delete Rename Quit` (call `Binary` before transfers;
  `Read`/`Write` are `str`, so text only; no resume).
- Clients: **`aws`** (S3/SQS/STS and S3-compatible: `NewConfig ForMinio ForR2 S3Put S3Get S3GetBytes S3UploadFile S3DownloadFile S3ListAll S3Delete ...`; `S3Get` is a `str`, so binary needs `S3GetBytes`;
  `S3ListPrefix` returns one page of 1000), **`google`** (service account
  `ParseServiceAccount AccessToken Authorization`), **`telegram.bot`**
  (`NewBot SendMessage SendPhoto Poll SetWebhook InlineKeyboardMarkup ...`;
  every call is a GET, so no local file upload, and `Poll` never returns),
  **`webpush`** (`GenerateKeys KeysFrom Send`; new keys break every
  subscription), **`graphql`** server (`Parse Execute SetResolver HandleHttp`; subscriptions run once; resolvers run field by field, so batch
  your loads), **`grpc`** (`Dial UnaryCall NewServer RegisterMethod Serve`;
  the server is h2c only, one call at a time), **`rpc`** (JSON-RPC 2.0 and
  MessagePack-RPC: `Register NewServer HandleText Call DialClient`).
- Browser/JS targets: **`dom`** (`ById Query QueryAll SetText SetHtml SetStyle OnEvent OffEvent SetTimeout EveryFrame Store*`; a missing element
  is JS null, not 0, so test `Exists(id)`; never `SetHtml` user text),
  **`web.canvas`** (`Of Get FillRect BeginPath MoveTo LineTo Arc Fill FillText DrawImage`; paths pile up until `BeginPath`), **`web.keys`**
  (`Init Track Down Pressed`), **`node`** (Node.js interop: `Require GetProp CallS1 ...`; handles are `i64`, every number is f64). **`web`** itself
  holds the `Color Length Url` value types the layout DSL uses (§9); it is
  not a server.
- **`webview`** (desktop windows: `Create SetTitle SetSize SetHTML Navigate Eval Init Run Destroy IsClosed`; `-DSALAM_WEBVIEW_CEF` renders with a
  bundled Chromium instead of the OS webview, see
  `packages/webview/native/BUILD.md`). Put data into the HTML you pass to
  `SetHTML`; the page cannot call back into Salam, `Eval` right after
  `SetHTML` usually hits the old page, and a failed `Create` returns a window
  that `IsClosed()` already.
- Native audio: **`audio`** holds a `Clip` of interleaved **f32** samples with `rate`, `channels`, `frames` and `bits`.
  Core: `New Sample SetSample Duration Peak RMS GainToDb DbToGain`.
  Editing: `Slice Trim Concat Mix MixAt Gain Scale Normalize Clamp FadeIn FadeOut Reverse Pad Repeat Split TrimSilence`.
  Converting: `ToMono ToStereo ToChannels ToRate ToBits Channel Deinterleave Interleave`.
  Sniffing: `DetectFormat FormatFromExt FormatName FormatIsLossless`.
  `MsbReader` and `MsbWriter` are the **MSB-first** bitstream pair every codec needs; `compress`'s `BitReader` is LSB-first and will not do.
  **`audio.wav`**: `OpenWAV SaveWAV SaveWAVBits EncodeWAV EncodeWAVFloat EncodeMuLaw EncodeALaw DecodeWAV ProbeWAV ReadFormat Chunks FindChunk`, covering PCM 8/16/24/32, IEEE float, A-law and mu-law.
  **`audio.dsp`**: `FFT IFFT FFTVec Magnitude PowerSpectrum` (radix-2, power-of-two lengths only), `MakeWindow ApplyWindow BesselI0`, `LowPass HighPass BandPass Notch Peaking LowShelf HighShelf BiquadStep BiquadProcessAll BiquadResponse`, `ResampleSinc ResampleCubic ResampleTo Correlation DCRemove`.
  Effects and dynamics: `Delay Echo Reverb Chorus Flanger Tremolo CompressorProcess LimiterProcess GateProcess TimeStretch PitchShift`.
  Analysis: `STFT SpectrumAt SpectralCentroid SpectralRolloff SpectralFlatness SpectralFlux FreeSpectrum ZeroCrossingRate DetectPitch DetectOnsets DetectBPM LoudnessLUFS NormalizeLUFS TruePeakDb`.
  Shared sample conversion lives in the core so every codec uses one copy: `DecodePcm EncodePcm DecodeFloatPcm EncodeFloatPcm DecodeLawPcm EncodeLawPcm PcmDepthValid MuLawToPcm PcmToMuLaw ALawToPcm PcmToALaw`.
  **`audio.wav`**: `OpenWAV SaveWAV SaveWAVBits EncodeWAV EncodeWAVFloat EncodeMuLaw EncodeALaw DecodeWAV ProbeWAV ReadFormat Chunks FindChunk`, covering PCM 8/16/24/32, IEEE float, A-law and mu-law.
  **`audio.aiff`**: `OpenAIFF SaveAIFF SaveAIFFBits EncodeAIFF DecodeAIFF ProbeAIFF ReadCommon`. Big-endian, the `sowt` little-endian variant, and the 80-bit extended float sample rate. 8-bit AIFF is **signed**, unlike WAVE.
  **`audio.au`**: `OpenAU SaveAU EncodeAU EncodeAUEncoding DecodeAU ProbeAU ReadHeader`. A 24-byte big-endian header; 8-bit is signed here too.
  **`audio.flac`**: `OpenFLAC SaveFLAC SaveFLACLevel EncodeFLAC EncodeFLACLevel DecodeFLAC ProbeFLAC ReadStreamInfo ReadFrameHeader Blocks VerifyFLAC CRC8 CRC16`. Real lossless compression both ways; the decoder handles constant, verbatim, fixed and LPC subframes and all four channel assignments. `FreeStreamInfo` releases the MD5.
  **`audio.mp3`**: `ParseHeader FindSync CountFrames Headers FrameAt SeekToSecond ReadVbrInfo ProbeMP3 AudioStart IsMP3`. Frame and VBR-header parsing only; **mp3 audio decoding is not implemented**, and `codec.CanDecode(FormatMP3)` says so.
  **`audio.tag`**: `ReadTags ReadTagsFile FreeTags WriteID3v1 WriteID3v2 ReadID3v1 ReadID3v2 ReadVorbisComment WriteVorbisComment ReadFlacTags StripTags AudioStart AudioEnd`. ID3v1, ID3v2.2-2.4 in all four encodings, Vorbis comments and cover art; text always comes back UTF-8. Every string is owned, so call `FreeTags`.
  **`audio.music`**: `NoteToFreq FreqToNote NoteName ParseNote Scale ScaleNotes Chord ChordNotes Transpose Interval Cents JustRatio PythagoreanRatio BeatSeconds`. Persian modes are in **cents**, not semitones: `Dastgah DastgahFreqs NoteToFreqCents KoronCents SoriCents`, because a dastgah does not fall on the twelve-tone grid.
  **`audio.synth`**: `Tone ToneEnvelope Chord NewOsc OscNext WaveValue ADSRAt ADSRTotal NoiseWhite NoisePink NoiseBrown FMTone Additive KarplusStrong NoteAt RenderNotes`.
  **`audio.midi`**: `NewSong NewTrack AddTrack AddNote AddTempo AddProgram AddControl EncodeMIDI DecodeMIDI OpenMIDI SaveMIDI TicksToSeconds Bpm TotalTicks Notes Render FreeSong`. `Render` plays a song through `audio.synth`.
  **`audio.codec`**: `Open Save Decode Encode Probe ProbeFile CanDecode CanEncode`, the format-agnostic entry points. They live in their own package because a parent cannot import its own children; it carries WAV, AIFF, au and FLAC.
  8-, 16- and 24-bit PCM round-trip a `Clip` bit for bit; a true 32-bit integer source loses its low 8 bits to the f32 significand.
  Free every `Clip` with `audio.Free` and every `ByteBuf` with `compress.BufFree`; `Blocks`, `Headers`, `Notes`, `Scale` and friends return vectors the caller frees.
- Native media: **`image`** (`Open Save SaveQuality New Resize Crop DrawText ...`, RGBA; GIF reads frame one; JPEG drops alpha), **`opencv`** (needs the
  `salam_opencv_shim` library and OpenCV 4; `Color` is RGB while a `Mat` is
  BGR; free every `Mat`), **`graph`** (`Dijkstra ShortestPath PageRank Components ...`; unreachable is `GRAPH_INF`, and `Dijkstra` returns false
  on a negative weight), **`llvm`** (build, verify, JIT and link LLVM IR;
  `ParseIR`/`RunJIT` take a `.ll` **path**, call `Verify` before emitting,
  free builder, module, context in that order) and **`llvm.triple`**.

### JWT and API route protection

- **`jwt`**: JSON Web Tokens (RFC 7519/7515) over HMAC-SHA2, and the HTTP
  layer that uses them - one package, because the HTTP half is only useful
  with the token half and dead-code elimination means a CLI that merely signs
  a token links none of it.
  - Tokens: `NewClaims SetStr/SetInt/SetBool/SetRaw ExpiresIn NotBeforeIn ClaimsJSON`; `Sign SignHS256/384/512 SignWithKid SignJSON SignatureOf`;
    `Verify VerifyHS256 IsValid DefaultOptions OptionsFor`; `Decode` (parses,
    verifies **nothing**); claim readers `Str Int Bool Raw Has ListHas HasScope HasAudience SecondsRemaining`; `ErrorText IsSupportedAlg`; error
    codes `JWT_OK JWT_ERR_*`. `Options` carries the policy (`alg issuer audience subject leeway now require_exp/nbf/iat/sub/jti`). **`alg` comes
    from `Options`, never from the token**; `"none"` and RS*/ES* are rejected
    (no asymmetric support). **`require_exp` is off by default**, so a token
    without `exp` is accepted forever unless you set it; the secret is not
    checked for strength; claims are signed, not encrypted.
  - HTTP (needs `net.http` + `net.router`): `NewGuard` bundles the secret with
    an `Options`; `Require RequireScope RequireClaim RequireSubject` verify at
    the top of a handler and, on failure, write the 401/403 themselves and
    return false. Also `Check` (verify, write nothing), `ParseBearer BearerToken TokenFrom Unauthorized MissingCredential Forbidden`, issuing
    (`Issue IssueFor IssuePair`) and cookies (`SetSessionCookie SetSessionCookieInsecure ClearSessionCookie`).

Passwords and secrets live in **`crypto`**, not here: `HashPassword`
`VerifyPassword` (PBKDF2-HMAC-SHA256), `SecretEquals` (constant-time compare),
`RandomHex`.

```salam
func guard(): jwt.Guard:
    mut g := jwt.NewGuard(os.Env("API_SECRET"))
    g.opts.issuer = "api.example.com"
    ret g
end

func me(ctx: i64):
    mut t := jwt.Token { }
    if not jwt.Require(ctx, guard(), t): ret end     // 401 already written
    http.Json(ctx, `{"sub":"` + t.claims.sub + `"}`)
end
```

Worked examples: `tests/en/apps/auth/` (seven programs), the five REST APIs in
`tests/en/apps/*api/`, and `tests/en/webframework/authapi.salam`. Package
tests: `tests/en/stdlib/jwt_demo.salam`.

### Databases (`import db`, `import db.<engine>`)

**`db` itself** is the engine-independent layer: three interfaces plus helpers
that work on any driver. Take a `dyn db.Connection` and the same code runs on
any engine (`tests/en/db/shared_iface.salam` runs one function against sqlite
and postgres and expects identical output).

- `interface Connection`: `Kind Ok Ping Exec Query Begin LastInsertId Changes Error Quote Close`
- `interface Rows`: `Ok Next ColumnCount ColumnName ColumnType IsNull Text Int Int64 Float Close` - forward-only, so the loop is `until r.Next():`, and
  `Close()` is required
- `interface Tx`: `Ok Exec Query Commit Rollback`
- Helpers: `QueryInt QueryInt64 QueryFloat QueryText CountRows Run RunAll InTx QuoteLiteral`; constants `SQLITE MYSQL POSTGRES ELASTICSEARCH OPENSEARCH` and `TYPE_NULL TYPE_INT TYPE_FLOAT TYPE_TEXT TYPE_BLOB`
- **Pool** (`NewPool Add Acquire Leased Conn Release Size Idle InUse DropDead CloseAll Free`): a fixed set of connections leased one at a time. It holds
  connections you opened; it is not a factory. No locking of its own.
- **Migrations** (`NewMigrator NewMigratorIn AddMigration MigrationCount EnsureTable SchemaVersion IsApplied PendingCount MigrateUp Rollback MigrateTo FreeMigrator SplitStatements`): each step runs in a transaction with the row
  recording it; steps split on `;` outside quotes, since MySQL rejects
  multi-statement queries.
- Pitfalls: `QuoteLiteral` only doubles `'` (on MySQL use `c.Quote`, which
  also escapes backslashes); `MigrateUp` compares version numbers, so a
  migration numbered below the highest applied one is skipped silently; a
  `$$` function body is cut apart at its `;` (run it with `c.Exec`).

Each driver keeps its own full surface, and `Conn(d)` boxes one as a
`dyn db.Connection`:

- **`db.sqlite`**:
  `Available Version Open Ok Exec Query Next Text Int Int64 Float IsNull Finish Prepare BindText BindInt BindFloat BindNull Reset LastInsertId Changes QueryInt QueryText Close Conn`. One `Database` is one connection: give each writing thread its own `Open`. There is no BLOB bind (store hex or base64), and `QueryInt`/`QueryText` take no parameters, so use them only on SQL you wrote.
- **`db.postgres`** (native wire protocol, no libpq; TCP only, SCRAM/MD5/password auth, TLS via `sslmode`): `Available ClientVersion ServerVersion Open OpenFull Ok Ping Reset Close Error Exec ExecCount Query Quote QueryInt QueryText DatabaseName UserName HostName Port Conn`. Placeholders are `$1, $2`, not `?`. There is **no last insert id** - use `INSERT ... RETURNING id`; `LastInsertId()` goes through `lastval()` and is 0 when the session has used no sequence. Results are materialised, so every `Rows` must be closed (page large tables with `LIMIT`). No `COPY FROM STDIN`. `Commit` returns -1 when the server rolled the transaction back. `ServerVersion` is major \* 10000 + minor.
- **`db.mysql`** (MariaDB/MySQL, native protocol, no libmysqlclient; TCP only, native_password and caching_sha2 auth, TLS when the server offers it, `SALAM_MYSQL_SSL_MODE` = DISABLED/REQUIRED/VERIFY_IDENTITY): `Open Ok Close Ping Error Errno Exec Query QueryOk Next Finish Text Int Int64 Float IsNull ColumnCount RowCount ColumnName AffectedRows LastInsertId Begin Commit Rollback Autocommit Escape SetCharset SelectDB QueryInt QueryText Conn`. This API hands every value over as text, so `ColumnType` answers `TYPE_TEXT` where the others report the column's own type; DDL commits implicitly, so `Tx.Rollback()` cannot undo it. **Threading**: a connection
  cannot be used by two threads at once (it segfaults, it does not error), so
  a threaded server gives each request its own connection. Call
  `LibraryInit()` once from `main` before any thread starts, and `ThreadInit()`
  from each thread before its first query - `tests/en/apps/*api/_store.salam`
  is the worked example.
- **`db.sql`** (read and check SQL without a database, the sqlc idea with your own DDL as the schema): `DialectOf DialectNamed` give a `Dialect` for `SQLITE MYSQL MARIADB POSTGRES`; `Lint(sql, d)` parses one statement or a whole `;` separated script and returns a `Report` of `Diag` values (`code message help line col offset length`, rendered by `ReportText`/`DiagText`); `ParamCount`/`Placeholders` count and list the placeholders. With a schema: `NewCatalog(d)` then `Apply(cat, ddl)` reads `CREATE TABLE`, `ALTER TABLE`, `DROP` and `CREATE VIEW` into a `Catalog` (so the migrations are the schema), `TableCount TableName HasTable ColumnCount ColumnAt HasColumn` look around it, and `Check(sql, cat)` resolves names: unknown table or column with the nearest real name offered, ambiguous column, INSERT arity, a NOT NULL column with no default left out, a write with no WHERE. `Analyze(sql, cat)` adds what the query takes and returns - `Param { marker ordinal table column kind not_null }` and `ResultCol { name table kind nullable }`, with outer joins making a whole side nullable - and `SalamType(kind, not_null)` names the Salam type to read it into (`option.Option<T>` when it can be null). Codes: `SQL0xx` lexical, `SQL1xx` syntax, `SQL2xx` a dialect rule (`?` in postgres, `AUTO_INCREMENT` in sqlite, `ILIKE` outside postgres, `::` outside postgres, `LIMIT a, b` in postgres, `ON DUPLICATE KEY` outside mysql, `TRUNCATE`/`PRAGMA` in the wrong engine), `SQL3xx` the schema. Messages are English in both language modes; match on `code`. Free with `FreeReport`, `FreeCatalog`, `FreeAnalysis`. Putting values in: the three drivers do not all prepare and bind (only `db.sqlite` has `Prepare`/`Bind*`), so `NewArgs` + `AddInt AddFloat AddText AddBool AddNull` + `Fill(query, args, d, err)` writes the values into the SQL properly - the query is lexed first, so a `?` inside a string literal is left alone, the count has to match (`SQL400`), quotes are doubled, backslashes are doubled on MySQL, and a boolean is `TRUE`/`FALSE` on postgres and `1`/`0` elsewhere. It is substitution, not a prepared statement; `ArgCount` and `FreeArgs` round it out. Code generation, the sqlc half: annotate a `.sql` file with `-- name: GetUser :one` (`:one :many :exec :execrows`), then `ParseQueries(src, err)` and `Generate(pkg, queries, cat, err)` write a row struct and a typed function per query - parameters named and typed from the column each placeholder binds against, nullable columns as `option.Option<T>` read through `IsNull`, every query analysed against the catalog first so a typo stops generation with the query's name. One `dyn db.Connection` signature serves all three engines. The command is `salam sql generate queries --schema=db --output=gen/store.salam --package=store`. The `salam sql check` command runs the same checks over `.sql` files and over every SQL string literal in `.salam` files (`--schema=` a file or a directory; without `--dialect=` each file is read with the engine it imports); a literal assembled from pieces (glued with `+` or `.concat`, or one that simply stops mid-statement) is a fragment, so it is counted in the summary and skipped instead of reported.
- **Official packages** live in `packages/`, not `std/`, and do not ship with Salam: `db.sqlite db.postgres db.mysql db.redis db.meilisearch db.elasticsearch db.typesense db.opensearch`, `audio` and every `audio.*`, `tensor autograd nn optim ml data gguf llm onnx npy`, `net.http.swagger net.http.middleware net.http.session net.http.validate net.http2 net.websocket net.socketio net.email net.proxy`, `grpc rpc graphql ftp ssh aws google webpush telegram.bot node`, `webview opencv excel`. A program that imports one must be in a project (`salam init`) that ran `salam add <name>` (for example `salam add tensor`); otherwise the import fails with that hint. Inside a source checkout they resolve from `packages/` directly. Their API notes below still apply.
- **`db.redis`** (RESP over TCP): `Connect Ok Error Close Ping Auth Select Set SetEx Get Exists Del Expire TTL Incr IncrBy Decr Append Keys HSet HGet HDel HExists HGetAll LPush RPush LPop RPop LLen LRange SAdd SRem SIsMember SMembers Publish`, anything else via `Command` and `Reply*`. Failures come
  back as empty values (`""`, 0, false); `Command` shows the server's
  `REPLY_ERROR` text. A plain `Set` removes a key's TTL; `Auth` sends only
  the password (an ACL user needs `Command`); it can publish but not
  subscribe.
- **`db.meilisearch`** (Meilisearch over HTTP, the whole v1 API): `New WithAgent WithTimeout WithMetadata Ok ErrorString Request RequestAs Health IsHealthy GetVersion GetStats CreateIndex GetIndex ListIndexes UpdateIndex RenameIndex DeleteIndex SwapIndexes GetIndexStats CompactIndex ListFields AddDocuments UpdateDocuments AddDocumentsWith AddDocumentsNdjson AddDocumentsCsv AddDocumentsInBatches AddDocumentsOf GetDocument GetDocumentWith GetDocumentAs GetDocuments DocumentsAs DeleteDocument DeleteDocuments DeleteDocumentsByFilter DeleteAllDocuments Search Query HitsAs MultiSearch FederatedSearch FacetSearch SimilarDocuments GetSettings UpdateSettings ResetSettings Get/Update/Reset<Setting> GetTask GetTasks WaitForTask Wait CancelTasks DeleteTasks GetTaskDocuments CompactTasks GetBatch GetBatches CreateKey GetKey GetKeys UpdateKey DeleteKey TenantToken ListWebhooks CreateWebhook GetWebhook UpdateWebhook DeleteWebhook ListSearchRules GetSearchRule UpdateSearchRule DeleteSearchRule DeleteAllSearchRules ListChatWorkspaces GetChatWorkspace DeleteChatWorkspace GetChatSettings UpdateChatSettings ResetChatSettings ChatCompletion SetStderrLogs NetworkControl RenderTemplate`. Requests go over `net.httpx`: no timeout unless `WithTimeout(c, ms)` (then `err.code == "timeout"`), and a `Client` is a plain value that threads can share. `CompactTasks` leaves the server refusing writes until it is restarted. Struct fields have Persian names too (`ک.میزبان`, `خ.کد`), and a Persian file must use them. Runnable examples for every part of the API live in `tests/en/stdlib/meilisearch/examples` and, with Persian names throughout, `tests/fa/stdlib/meilisearch/examples`. Every call ends with `err &: meilisearch.Error` (empty `code` = success, check with `Ok(err)`). Writes return a `TaskInfo` and apply later; chain `meilisearch.Wait(c, meilisearch.AddDocuments(c, "movies", json, "id", err), err)` when the next step needs the change. Hits and documents are raw JSON strings; decode into structs with `HitsAs`/`GetDocumentAs` (lenient, extra fields like `_formatted` are ignored). `SearchRequest` sends only what you set (-1 numbers, empty strings/lists and false flags are omitted); `filter` takes an expression or a raw JSON array.
- **`db.elasticsearch`** (Elasticsearch 7/8, Elastic Cloud and OpenSearch over `net.httpx`; no outside library; `Distribution(c)` tells them apart and SQL/point in time switch endpoints on their own). `NewClient(Config)` takes several nodes (rotated, a node that refuses connections is skipped for `DEAD_NODE_MS`, 502/503/504 retried), basic auth, `api_key` (encoded or `id:key`), `bearer`, `cloud_id`, `ca_file`/`insecure`, `timeout_ms` and `keep_alive` (one pooled connection per node, on by default); `Connect ConnectAuth ConnectApiKey ConnectCloud` are shortcuts. Documents and query clauses are raw JSON `str`. Documents: `Index Create Get Exists Update Upsert UpdateScript Delete MGet Count` (+`...With(Options)` for `refresh`, `routing`, `if_seq_no`/`if_primary_term`). Search: build a `SearchRequest { query, size, sort, aggs, ... }` from `Match Term Terms Range Bool And Or Not Filtered MultiMatch Knn SortDesc` and `AddAgg(aggs, name, TermsAgg(...))`, then `Search`; read `res.hits`, `AggValue`, `AggBuckets`. Paging: `SearchScroll/Scroll/ClearScroll`, `OpenPit` + `search_after`. Bulk: `NewBulk`, `BulkIndex/BulkUpdate/BulkDelete(b, ...)`, `RunBulk` (check `res.items` when `res.errors`). Admin: `CreateIndex PutMapping Refresh PutAlias SwapAlias Reindex DeleteByQuery UpdateByQuery CatIndices ClusterHealth`, pipelines (`PutPipeline SimulatePipeline`), data streams (`CreateDataStream Rollover`), `PutLifecyclePolicy` (ILM, or ISM on OpenSearch), snapshots, cluster settings, background tasks (`StartReindex StartUpdateByQuery GetTask`), stored scripts and `SearchTemplate`, `SubmitAsyncSearch`, `Explain ValidateQuery FieldCaps TermVectors IndexStats`. SQL: `Sql`/`SqlWith` return `SqlRows` (a `db.Rows` that pages with the cursor), and `Conn(c)` is a read-only `dyn db.Connection` (`Exec` is -1, no transactions). Anything else: `Perform(c, method, path, body)`. Check `Error(c)`/`ErrorType(c)`/`Status(c)` after a call; a `Get` of a missing doc is `found = false` with no error.
- **`db.typesense`** (Typesense over HTTP, 29.x and 30.x): `New NewCluster SetNearestNode AddNode AddHeader Free Ok IsNotFound IsConflict ErrorString Request RequestAs Health ServerVersion Status Stats Metrics CreateSnapshot ClearCache CompactDB SchemaChanges UpdateConfig CreateCollection GetCollection CollectionExists ListCollections UpdateCollection AddFields DropFields CloneCollection TruncateCollection DeleteCollection UpsertAlias GetAlias ListAliases DeleteAlias CreateDocument UpsertDocument EmplaceDocument UpdateDocument GetDocument GetDocumentAs DeleteDocument DeleteByFilter UpdateByFilter ImportDocuments ImportJSONL ImportOf ExportDocuments Search SearchRaw MultiSearch UnionSearch HitsAs Highlight CreateKey SearchOnlyKey GetKey ListKeys DeleteKey ScopedSearchKey GenerateScopedSearchKey UpsertSynonymSet UpsertSynonymItem UpsertSynonym UpsertCurationSet UpsertCurationItem UpsertOverride UpsertPreset UpsertSearchPreset UpsertStopwords ImportStemmingDictionary CreateAnalyticsRule SendEvent FlushAnalytics CreateConversationModel CreateNLSearchModel`. Every call ends with `err &: typesense.Error` (empty `kind` = success, `Ok(err)`; `IsNotFound`/`IsConflict` for 404/409). A client holds several nodes and skips one that fails for `healthcheck_ms`, retrying `retries` times, so a 5xx or a dead node is not an error until every try failed. Build a schema with `Schema { name }` plus `NewField`/`FacetField`/`VectorField`/`EmbeddingField` (`Field.kind` is the type, `sort` is 1/-1/0 for on/off/default). `SearchParams` sends only what you set; anything without a field goes through `Set(p, key, value)`, and a query string over `MAX_GET_QUERY` bytes goes through `/multi_search` by itself. Imports answer 200 even when lines fail: check `ImportResult.failed`. Synonym sets and curation sets are Typesense 30; `UpsertSynonym`/`UpsertOverride` are the 29 per collection forms. `ScopedSearchKey` signs a browser key locally.
- **`db.opensearch`** (OpenSearch over its REST API, native TCP/TLS, no outside library; `db.elasticsearch` covers the shared core API too, pick this one for PPL, k-NN/neural/hybrid, ISM, the security plugin, alerting, ML Commons or AWS SigV4): connect with `Connect ConnectAuth ConnectInsecure ConnectAWS NewClient(Config)` (several nodes with retries and dead node skipping, basic/bearer auth, AWS SigV4 for `SERVICE_ES`/`SERVICE_AOSS`, `ca_file`, `insecure` for a self signed test server). Documents `Index IndexWith Create Get GetSource Exists Update UpdateWith Upsert UpdateScript Delete MGet Count DeleteByQuery UpdateByQuery Reindex`; bulk `NewBulk BulkIndex BulkCreate BulkUpdate BulkUpsert BulkDelete BulkSend BulkFree`; search `Search SearchWith(SearchRequest) SearchRaw SearchParamsWith MSearch SearchScroll Scroll ClearScroll CreatePit DeletePit SearchTemplate Explain Validate`; results `Buckets AggValue AggField Suggestions JsonPath`; query DSL builders return JSON text (`Match Term Terms Range BoolOf(BoolQuery{}) Nested Knn KnnFiltered Neural Hybrid ScriptScore FunctionScore SortDesc Highlight TermsAgg AvgAgg DateHistogramAgg AddAgg SubAggs`); admin `CreateIndex CreateIndexWith PutMapping PutSettings Refresh PutAlias SwapAlias PutIndexTemplate CreateDataStream Rollover CatIndices ClusterHealth WaitForStatus GetTask PutPipeline SimulatePipeline CreateSnapshot`; plugins `Sql SqlWith Ppl SqlExplain` (JDBC rows that page with the cursor), k-NN `KnnIndexBody KnnVectorField KnnWarmup`, ISM `PutPolicy AddPolicy ExplainPolicy DeleteAfterPolicy`, security `WhoAmI PutUser PutRole RoleBody PutRoleMapping ChangePassword`, `PutSearchPipeline NormalizationPipeline`, alerting monitors and `Ml*`; anything else through `Perform`. Every call records `Status Error ErrorType ErrorReason` on the client; `Conn(c)` is a `dyn db.Connection` whose `Query` runs SQL (`Exec` only does SQL `DELETE`, there are no transactions). `Get` of a missing id is `found = false` with an empty `error`. A bulk delete of a missing id counts as ok.

```salam
import db
import db.sqlite

func count(c: dyn db.Connection): int:
    ret db.QueryInt(c, "SELECT COUNT(*) FROM users")
end

func main:
    d := sqlite.Open("app.db")
    defer sqlite.Close(d)
    c := sqlite.Conn(d)
    mut m := db.NewMigrator() defer db.FreeMigrator(m)
    db.AddMigration(m, 1, "users",
        "CREATE TABLE users (id INTEGER PRIMARY KEY, email TEXT NOT NULL)",
        "DROP TABLE users")
    db.MigrateUp(m, c)
    println count(c)
end
```

---

## 6. Compiler rules & top pitfalls

Salam's semantic checker is strict. These are the rules that most often turn a
naive port into compile errors (each corresponds to a case in
`tests/en/errors/`):

1. **`until <cond>` loops WHILE the condition is true; there is no `while`.**
   `until i < n:` iterates for `i` from small to `n`; the C `while (v != 0)` is
   `until v != 0:`, **not** `until v == 0:`. This is the single most common
   porting mistake, and it is _silent_: an inverted condition runs the body
   zero times with no diagnostic (only a literal `until false:` is caught, as
   `E068`). See the box in §2.
2. **Unused = error.** An unused variable, `mut`, parameter, import, or function
   is a hard error. Remove it; a `_` prefix silences the check but leaves dead
   code behind, so keep it for a parameter whose signature is fixed (a
   callback). Only mark something `mut` if you actually reassign it.
3. **Reassignment requires `mut`.** `x := 1; x = 2` fails; use `mut x := 1`.
4. **Heap collections must be freed.** `Vector/HashMap/Set/...` allocate; call
   `.free()` (idiom: `defer v.free()` right after creation). Also free elements
   when they own memory.
5. **Integer `/` truncates** toward zero. **Bitwise operators (`& | ^ ~ << >>`)
   are supported on integers** with C precedence and compound forms
   (`&= |= ^= <<= >>=`), but they require integer operands (a bitwise op on a
   float is a compile error).
6. **No exceptions / no try-catch.** Signal failure with a `bool` return and
   an `err &:` out-parameter (the std style), an `option.Option<T>`
   (`import option`), a `result.Result<T, E>` (`import result`), or a sentinel
   value, and check it at the call site.
7. **Privacy.** Struct fields/methods and package symbols are private by default;
   expose with `pub`. Accessing a private field/method from outside is an error.
8. **Top-level ordering.** Within a file: `package` first, then all `import`s,
   then all `include`s (an `import` after an `include` is E108), then
   top-level `const`/variable declarations (E084/E085), then types
   (`struct enum type interface impl`, E087), then functions. A
   body-less `extern:` block ranks with the imports; an `export:` block ranks
   with the private functions, so it must come before the first `pub func`. A
   top-level `if` must be a **compile-time constant** condition (see §8).
   **No `if` branch may be empty** - not at top level, not in a function, and
   not in an `else if`. Instead of `if X:` with an empty body followed by
   `else:`, negate the condition: `if not X:`.
9. **`pure` functions are checked**: they may not write globals, call impure
   functions, mutate parameters, or `print`. Only mark a function `pure` if it is.
10. **`match` on an enum or a `Variant` must be exhaustive** (or have an
    `else`) - both as a statement and as an expression - and use valid type/member
    patterns. Enum `match` patterns must be real members. **`switch` is the
    fallthrough alternative** - on an enum it must still cover every member
    or have an `else` (E103); it cannot be used on a `Variant` at all (§2,
    §3, §12.2).
11. **Enum members are separated by a comma** (`,` or `،`) **or a newline** -
    on a single line the comma is required, since member names may contain
    spaces.
12. **Types are checked strictly**: no implicit narrowing; use `as`. Ternary
    branches must share a type; a condition must be `bool`. Array-literal length
    must match the declared size.
13. **Dead code is rejected**: an always-false `if/until/repeat/each`, an
    unreachable `ret`, etc. are errors, not warnings.
14. **Useless casts are rejected** (E093): `x as T` where `x` already has type
    `T`, including `[1, 2, 3] as int[3]`.
15. **`main` returns the exit code.** Write `ret 1` in `main` to fail; calling
    `os.Exit` inside `main` is E109 because it skips `main`'s `defer`s. `ret`
    alone (or falling off the end) exits with 0. Other functions may still
    call `os.Exit`.
16. **No typed declarations.** `x: T = v` is a parse error; write
    `x := v as T` (see §2).

When the compiler complains, fix the code; do not try to suppress the check.

---

## 7. Translating from other languages

General mapping that applies to all source languages:

| Source concept               | Salam                                                                             |
| ---------------------------- | --------------------------------------------------------------------------------- |
| class                        | `struct` with `pub` fields + methods (`this` receiver)                            |
| static method / constructor  | `static func` inside the struct, called as `Type.Name(...)`                       |
| interface / protocol / trait | `interface` + structural `pub` methods; add to existing types with `impl I on T`  |
| inheritance / base class     | embed with `use Base` (fields + methods promoted); override by redefining         |
| subtype polymorphism         | `dyn Interface` (dynamic) or `<T: Interface>` (static)                            |
| generics / templates         | `<T>`, `struct Box<T>`, `func F<T>(…)`                                            |
| dict / map / object          | `HashMap<K,V>` (`put/get/has`)                                                    |
| list / array / vector        | `Vector<T>` (`push/get(i)/set/len`) or fixed `T[n]`                               |
| set                          | `Set<T>`                                                                          |
| tuple / record               | small `struct`, or `Pair`, or `Variant` for sum types                             |
| string ops                   | `str.*` package + `+` concatenation + `len()`                                     |
| exception / error            | `result.Result<T, E>` + postfix `?`, or `bool` flag / `Option<T>`; no throw/catch |
| null / nil / None            | `null` (pointers) or `Option.None()`                                              |
| lambda / closure             | `(x: int): ret expr end`; type `func (…) R`                                       |
| enum / union                 | `enum` (C-like, or members with data: `Circle(r: f64)`) or `Variant<…>`           |
| module / package / import    | `package name` + `import pkg` (only `pub` exported)                               |
| free function                | top-level `func`; a bare name is its address (`i64`), `&fn` is a `void*`          |
| `while`                      | **`until`** (no `while` keyword exists - same "loop while true" semantics)        |
| `switch` / `case`            | `switch`: bare labels, no `case`/`default`, C-style fallthrough (§2, §12.2)       |
| `for i in range(n)`          | `repeat n in i:`                                                                  |
| `for x in xs`                | `each x in xs:`                                                                   |
| destructor / cleanup         | `defer x.free()`                                                                  |

### From PHP

- `$var` → plain `name`; PHP arrays split into **`Vector<T>`** (lists) and
  **`HashMap<K,V>`** (assoc arrays); choose per use.
- `class`/`interface`/`trait` → `struct`/`interface`/`impl`. Visibility
  `public`→`pub`; everything else is private by default.
- String interpolation `"$a-$b"` → `a + "-" + b` or `fmt.Sprintf`.
- `echo` → `print`/`println`. Superglobals (`$_GET`, `$_POST`) → the
  `net.http` package's `Query`/`Form`/`Body`.
- Exceptions → return `bool`/`Option`; dynamic typing → pick concrete types or
  `Variant`.

### From TypeScript / JavaScript

- `class`→`struct`, `interface`→`interface`, `enum`→`enum`, generics carry over
  (`Array<T>`→`Vector<T>`, `Map`→`HashMap`, `Set`→`Set`, object literal→`struct`
  or `HashMap<str, …>`).
- Arrow functions `(x) => x*2` become `(x: int): ret x * 2 end` (add types;
  Salam has no `=>`). `Promise`/`async`/`await` have **no equivalent**, so use
  synchronous code, or `spawn`/`join` + `sync` (§8) for real parallelism.
- `let`/`const`→`mut`/`:=`+`const`. `null`/`undefined`→`null`/`Option`.
  `JSON.parse/stringify`→`json.*`. `throw`→`bool`/`Option`.
- Truthiness is gone: conditions must be real `bool`.

### From Python

- `class`→`struct` (`self`→`this`); `__init__` defaults → struct field defaults +
  literal `T { … }`. Duck typing → `interface` + `dyn`/`<T: I>`.
- `dict`→`HashMap`, `list`→`Vector`, `set`→`Set`, `tuple`→`struct`/`Pair`,
  `None`→`null`/`Option.None()`.
- Comprehensions → an explicit `repeat`/`each` loop building a `Vector`.
- `f"{a}"` → `a + …` or `fmt.Sprintf`. `def`→`func` (add types; Salam is static).
  `try/except`→`bool`/`Option`. Integer `/`: Python `//` == Salam `/`; Python `/`
  (true division) needs float operands.

### From C / Go / Rust

- **C**: `struct` maps directly; `malloc/free`→`mem.Allocate/Free`; pointers
  `T*` and `p[0]` carry over; call libc directly via `extern:` (§8). `printf`
  works through FFI, but prefer `println`/`fmt`.
- **Go**: `struct`+methods→same; `interface`→`interface`/`dyn`; goroutines→
  `spawn`; `sync.Mutex/WaitGroup`→`sync.*`; multiple returns → a `struct` or
  out-params via pointers; `error` return → `bool`/`Option`; slices → `Vector`
  or `T[]` slices; `map`→`HashMap`.
- **Rust**: `struct`→`struct`, `enum` with data→`enum` with data (`Circle(r: f64)`); `trait`→`interface`+
  `impl … on …`; `Option`/`Result`→`Option`/`bool`; generics + bounds
  `<T: Trait>`→`<T: Interface>`; ownership/`Drop`→manual `defer x.free()`
  (Salam does not borrow-check). Pattern `match` maps to Salam `match`.

---

## 8. FFI, concurrency, conditional compilation

**FFI (call C directly):**

```salam
link dynamic "sqlite3"                // link an external library (-lsqlite3)
extern:
    func printf(format: str, ...): int   // ... = variadic
    func sqrt(x: f64): f64
    func malloc(size: u64): void*
    func free(ptr: void*)
end
func main:  printf("%d\n", 42)  end
```

C pointer types (`void*`, `u16*`, `T*`) and `null` are available for interop.

**Exporting Salam functions to C (`export:`):** `extern:` only _declares_
symbols defined elsewhere (no bodies). A Salam function that C code, a C
library callback, or the compiler-generated runtime must reach by a fixed name
goes in an `export:` block. It is emitted under its plain name (no mangling)
with external linkage, and it is never dropped as dead code:

```salam
export:
    func on_ready(_ctx: void*): int:    // C sees `int on_ready(void* _ctx)`; `_` because C fixes the signature
        ret 0
    end
end
func main:
    cb := (&on_ready) as extern func (void*) int
    println cb(null)
end
```

Rules: every entry needs a body. No variables, no generics, no `...`. No
`pub`, `inline` or `noinline`: an exported function is always a public,
out-of-line C symbol. Only `deprecated`, `pure` (checked against the body) and
`noret` may modify it. Persian: `درون‌داد:` = `extern:`, `برون‌داد:` =
`export:` (with ZWNJ or a space). A body inside `extern:` is an error that
points you to `export:`.

`link` REQUIRES an explicit kind before the library name - there is no bare
`link "X"` and no `@link(...)` attribute form, only one way to write this:
`link dynamic "X"` (`-lX`) vs `link static "X"` (`-l:libX.a` on the
LLVM-native/JIT toolchain path; falls back to dynamic on the legacy tcc
path, since tcc's own linker doesn't support that syntax) vs `link
framework "X"` (macOS only, `-framework X`). Persian keywords:
`ایستا`/`پویا`/`چارچوب`.

**Concurrency:**

```salam
import sync
mut lock := sync.NewMutex()
mut wg := sync.NewWaitGroup()
sync.Add(wg, 1 as i64)
t := spawn(worker)                    // worker takes no arguments
t2 := spawn(work_on, job)             // ... or exactly one POINTER argument
join(t)                               // wait for it
sync.Lock(lock)  /* critical section */  sync.Unlock(lock)
sync.Wait(wg)  sync.Destroy(lock)  sync.DestroyWaitGroup(wg)
```

`spawn(f, arg)` is how a thread gets its own data; `arg`'s type must be a
pointer, because it travels through the OS's single thread-argument slot.
Without it a worker can only reach globals.

Concurrency pitfalls: a zero `sync.Mutex {}`, `atomic.I64 {}` or an
un-`init`ed `chan.Chan {}` crashes on first use (create with `New...`/`init`,
for globals at the top of `main`); a mutex is not reentrant (locking it
twice, or `Do` inside its own `Once`, hangs); join every thread exactly once
(`thread.JoinAll`, `thread.Collect`; a second `Join` uses freed memory);
`Destroy...` only after the last user is joined; a channel copies a
`Vector` or pointer as a handle, so both threads share it.

Rest of `sync`: `CondVar` (`NewCondVar`/`WaitCond`/`WaitCondTimeout`/`Signal`/
`Broadcast`; `WaitCondTimeout` is true when woken, false on timeout - always re-test your predicate in an `until` loop, wakeups can be
spurious), `RWMutex` (`RLock`/`RUnlock`/`WLock`/`WUnlock`, writer-preferring),
`Semaphore` (`Acquire`/`TryAcquire`/`Release`), `Once` (`Do(o, (): ... end)`),
`SleepMs`, `NowMs` (unspecified epoch - only differences mean anything).
Everything blocks on a condition variable rather than polling.

```salam
import chan
import pool
import atomic
import ctx

mut jobs := chan.Chan {} as chan.Chan<i64>
jobs.init(64 as i64)                          // n >= 1 buffered, 0 = rendezvous
jobs.send(v)                                  // blocks while full; false if closed
mut ok := true
v := jobs.recv(ok)                            // ok = false once closed AND drained
v2 := jobs.recv_timeout(ok, 500 as i64)       // ok = false: timed out OR closed
jobs.send_timeout(v, 500 as i64)              // false if it could not be delivered
jobs.close()   jobs.free()                    // also: try_send/try_recv/len/is_closed

pool.ParallelFor(n, 8 as i64, (i: i64): body(i) end)   // self-scheduling workers
pool.ParallelForAuto(n, (i: i64): body(i) end)         // one worker per CPU
pool.CPUCount()

mut hits := atomic.NewI64(0 as i64)
_ := atomic.Add(hits, 1 as i64)               // Load/Store/Swap/CompareAndSwap
mut stop := atomic.NewFlag(false)
if atomic.TestAndSet(stop): /* exactly one caller wins */ end

root := ctx.Background()                      // free the root LAST
job := ctx.WithTimeout(root, 5000 as i64)     // or WithCancel / WithDeadline
until ctx.IsDone(job):  _ := ctx.Wait(job, 100 as i64)  end
ctx.Cancel(job)   ctx.Err(job)                // "" | "cancelled" | "deadline exceeded"
ctx.Free(job)   ctx.Free(root)
```

`init(0)` makes a rendezvous channel: `send` does not return until a receiver
takes the value, so the two threads meet instead of queueing. `try_send` is
always false on one - there is no way to know a receiver is parked without
waiting.

A `ctx` tree shares one lock; cancelling any node finishes everything below it,
and a child's deadline can only shorten what it inherits, never extend it.

`atomic` is lock-free everywhere except tcc. `atomic_load/store/add/swap/cas`
are **language intrinsics** on an `i64*` cell (a `__atomic_*` built-in in the C
backend, a real `atomicrmw`/`cmpxchg` in LLVM, both seq*cst; plain reads and
writes in JS, which has no threads). `atomic_add` returns the value \_after* the
add, `atomic_swap` the value before it. tcc 0.9.27 has neither the `__atomic`
nor the `__sync` family, so under `SALAM_CC_TCC` the **`atomic` package** falls
back to a mutex per cell - same semantics, higher cost, invisible to callers.
The **bare intrinsics have no such fallback**: they are always a `__atomic_*`
built-in, so on a tcc older than 0.9.28 a program that calls them fails to
build with a `#error` naming `tools/bash/install-tcc.sh`. The pinned tcc
(`tools/ci/tinycc-ref.txt`, 0.9.28rc) has the built-ins, which is why CI is
unaffected; note it does _not_ define `__ATOMIC_SEQ_CST`, so that macro is not
a usable feature test - `__TINYC__ < 928` is.
A declared function of the same name shadows an intrinsic.

There is no `async`/`await`.

**Conditional compilation** (a top-level or inline `if` on a compile-time
constant). Predefined: `SALAM_OS_WINDOWS/MAC/LINUX/UNIX/FREEBSD/ANDROID/WASM`,
`SALAM_ARCH_X64/ARM64/X86/ARM/WASM`, string forms `SALAM_OS`/`SALAM_ARCH`; plus
your own `-DNAME` defines from the build command. The predefined `SALAM_*`
names are also ordinary constant values anywhere else - `switch SALAM_OS:`,
`println SALAM_ARCH`, `is_win := SALAM_OS_WINDOWS`, or inside a runtime
condition such as `if SALAM_OS_MAC and retries > 0:`.

```salam
if SALAM_OS_WINDOWS:  const SEP := "\\"
else:                 const SEP := "/"
end
```

**In Persian these constants have Persian names, and only those work.** Persian
has no upper case, so a leading `سلام` is what marks a name as the language's
own rather than yours; the rest of the words follow it separated by spaces, the
way multi-word names already are elsewhere:

| English                                          | Persian                                          |
| ------------------------------------------------ | ------------------------------------------------ |
| `SALAM_OS_WINDOWS` / `_LINUX` / `_MAC` / `_UNIX` | `سلام سیستم ویندوز` / `لینوکس` / `مک` / `یونیکس` |
| `SALAM_ARCH_X64` / `_ARM64`                      | `سلام معماری ایکس۶۴` / `آرم۶۴`                   |
| `SALAM_OS` / `SALAM_ARCH`                        | `سلام سیستم` / `سلام معماری`                     |
| `SALAM_VERSION` / `SALAM_GIT_COMMIT`             | `سلام نگارش` / `سلام کامیت گیت`                  |
| `defined(X)`                                     | `تعریف‌شده(X)`                                   |

These names are matched the way Persian keywords are, so a ZWNJ, a space, an
Arabic yeh for a Persian one and an Arabic kaf for a keheh are all the same
name: `سلام اشکال‌زدایی حافظه` and `سلام اشکال زدایی حافظه` both name
`SALAM_MEM_DEBUG`. Write whichever reads better.

```salam
اگر سلام سیستم لینوکس:
    سرچاپ سلام سیستم، سلام معماری، سلام نگارش
پایان
```

The two spellings do not mix, and the error names the one you wanted:
`SALAM_OS_LINUX` in a Persian file reports _this file's language writes that
built-in constant as `سلام سیستم لینوکس`_, and the reverse in an English file. Which set
applies is decided per file, by the same language detection the keywords use, so
a Persian program importing English `std/` is fine. Your own `-DNAME` defines
are never translated, in either language.

`SALAM_JS` / `سلام جاوااسکریپت` is the one flag that is only _defined_ on the JS
backend rather than defaulting to false, so test it with `defined` in either
language - `اگر تعریف‌شده(سلام جاوااسکریپت):` - the same as you would in
English. A bare `if SALAM_JS:` is an unknown identifier on a native build.

Cross-compile by passing an LLVM triple: `salam build app.salam
--target=x86_64-w64-windows-gnu --output=app.exe` (routes through LLVM;
`link dynamic "user32"` → `-luser32`).

---

## 9. Layout DSL (HTML/CSS/JS)

A `layout:` block (either its own `.salam` file built with `salam page build`,
or embedded after a general program) declares a UI tree that compiles to HTML +
CSS + JS. Elements open with `:` … `end`; **attributes and style properties are
`name = value` pairs** (no commas needed) written inside the element.

```salam
//! mode: layout | title: Web page        // optional editor directive
page:
    title = "My Page"
    lang = "english"                       // or "en"
    dir = "left to right"                  // or "ltr"
    header:
        background = "blue"
        color = "white"
        padding = "24px"
        heading: size = 1 content = "Welcome to Salam" font_size = "32px" end
    end
    box:                                   // generic container (div)
        padding = "20px"
        paragraph: content = "Generated from Salam." end
    end
    form:
        id = "contactForm"
        input: id = "name" type = "text" required = true end
        button: type = "submit" content = "Submit" end
    end
end
```

**Elements** (`std/layout/elements/`, one `LayoutElement` const per element or
context variant): every current HTML element, with the full list, Persian
names and placement rules in `docs/LAYOUT_ELEMENTS.md`. Common ones: `box`
(div), `paragraph`, `heading` (`size = 1..6`), `span`, `link` (a), `image`
(img), `list`/`ordered list`/`item`, `table`/`row`/`column`, `form`, `input`,
`select`/`option`, `text area`, `button`, `label`, `video`/`audio`/`source`,
`figure`, `details`/`summary`, `dialog`. Every element also accepts its HTML tag
name (`div`, `a`, `img`, `li`, ...), except the table parts that come from
context: write `header`/`main`/`footer` and `column` inside `table`, never
`thead`/`tbody`/`tfoot`/`th`/`td`.

The same name can mean different tags by context. Inside `table`, `header`,
`main` and `footer` become `<thead>`, `<tbody>` and `<tfoot>`. `column` is `<th>`
inside a table header row (or with `heading = true`) and `<td>` everywhere else.
A plain `table: row: column:` emits `<table><tr><td>` with no implicit tbody.

**Attributes are scoped per element.** The same Salam name maps to the right HTML
name for each element: `url` is `href` on `link`/`head link` and `src` on
`image`/`iframe`/`script` (`source`, `href`, `src` and `نشانی` are aliases).
Global attributes (`id`, `class`, `title`, `lang`, `dir`, `hidden`, `tabindex`,
`role`, `content`, `repeat`), ARIA (`aria label` ... and any `aria <name>`), data
attributes (`data view`, or any `data <name>` which becomes `data-<name>`),
and every `on<event>` handler work on all elements. All standard CSS properties
are registered (`background color`, `z index`, ...), with `hover`/`focus`/
`active`/`before`/`after` prefixes (`hover color = "red"`; in Persian `اشاره`,
`تمرکز`, `فشردن`, `پیش‌از`, `پس‌از`).

**CSS values are checked.** Almost every property validates its value
(keyword, length, number, time, angle, color, ...) and accepts Persian keywords
and units: `نمایش = "فلکس"`, `فضا = "۱۰ پیکسل"`, `مدت گذار = "0.3 ثانیه"`.
Shorthands are checked too: `border = "1 solid red"`, `font = "bold 16/1.5
serif"`, `transition = "opacity 0.3s ease"`, `transform = "rotate(45deg)"`,
`box shadow = "0 2 4 black"`. Bare numbers in a shorthand get `px`, except zero.
A wrong value is E018. Only `grid`, `grid template` and `offset` still pass
through unchecked. The full list with status per property is
`docs/CSS_PROPERTIES.md`.

**Values are expressions.** An attribute value can use constants, operators
and parentheses, and a static layout folds it at compile time:
`box: content = 4 * 10 end` prints `40`; `title = SITE + " | Home"` reads a
global `const`; `width = WIDE + "px"`, `margin = -5`, `size = N > 3 ? 2 : 3`.
Word operators (`and`, `or`, `not`, `و`, `یا`) need parentheses:
`content = (A and B)`. Unquoted CSS words keep their hyphens
(`justify content = space-between`, `background = linear-gradient(red, blue)`),
so write subtraction with spaces (`a - b`). A static layout cannot read a
variable or call a Salam function (E129); a bad fold (division by zero,
overflow, `true + 1`) is E130. A constant named like a value word
(`const red := ...` then `color = red`) wins, with W131.

**Runtime layouts.** `layout:` is a static page built at compile time. A
_named_ layout with typed parameters is a runtime page: the compiler builds
the whole document and its CSS at compile time and turns the layout into a
`pure func Name(params): str` that only fills in escaped values. There is no
template interpreter. Call it like any function, e.g. from a router handler:

```salam
import net.http
import rand

component Card(title: str, n: int):          // typed component: also Card(...) -> str fragment
    box: heading: size = 3 content = title end end
end

layout Home(user: str, lucky: int, items: Vector<str>, admin: bool):
    title = "Hi " + user
    paragraph: content = "Lucky number: " + lucky end
    if lucky > 50:
        paragraph: content = "Big!" end
    else:
        paragraph: content = "Small" end
    end
    list:
        each i, it in items:
            item: content = i + ": " + it end
        end
    end
    input: type = "checkbox" checked = admin end   // bool attribute toggled at runtime
    Card: title = user n = lucky end
end

func home_h(ctx: i64):
    http.Html(ctx, Home(http.Query(ctx, "name"), rand.IntN(100) as int, items(), false))
end
```

Rules, all checked at compile time:

- The layout function is `pure`: no I/O, network, random numbers or global
  writes inside the UI (E012). Fetch data in the handler and pass it in.
  Reading a `Vector`/`HashMap` parameter is fine (`len`, `is_empty`, `get`,
  `first`, `last`, `xs[i]`, `has`, `get_or`, `size`, `each x in xs`); a
  method that changes it (`push`, `pop`, `set`, `put`, `remove`, `ref`) is E012.
- Text and attribute values are always HTML-escaped; there is no raw-HTML type.
- A runtime value for a URL attribute must be a `web.Url`, and a runtime CSS
  value must be a `web.Color`, a `web.Length` or a number (E135). Build them
  with `web.ParseUrl(s, ok)`, `web.ParseColor`, `web.ParseLength`, `web.Px(n)`,
  `web.Rgb(r, g, b)`. Their fields are private, so they cannot be forged (E017).
  A runtime CSS value is written as an inline `style`; class CSS stays static.
- Enum-like attributes, `heading size`, `hover ...` styles and `style`/`script`
  bodies must be known at compile time (E132); pick between fixed variants with
  `if`. `if`/`each` need a runtime layout (E133). A fixed `id` inside `each`
  would repeat (E134). A runtime font `url` needs a static `type`.
- Typed components can be called directly (`Card("x", 1)`) and return their
  own `<style>` plus HTML, with class names prefixed by the component name.

**Fonts.** `font`/`قلم` declares a web font and emits only a CSS `@font-face`
rule, never an element. It sits directly under `page`; `name` and `url`
(`source`, `منبع`, `نشانی`) are required. `type` (`ttf`, `otf`, `woff`,
`woff2`, `ttc`) is optional: without it the compiler reads the extension from
the URL, then the magic bytes of a local file next to the source. A remote URL with
neither is E126, and a type that disagrees with the extension or the file is
E127. The compiler never fetches over the network. Optional: `weight`
(`300`, `"100 900"`), `style` (`italic`), `display` (`swap`), `range`
(`"U+0600-06FF"`). Several space-separated URLs become a fallback chain.

```salam
page:
    font: name = "Estedad" url = "fonts/Estedad.woff2" display = swap end
    box: content = "hi" font family = "Estedad, sans-serif" end
end
```

**Safety.** URL-typed attributes reject `javascript:`, `vbscript:` and any
`data:` that is not image/font/audio/video. CSS values cannot contain
`{ } ; < >` or line breaks. A `style`/`script` body cannot contain
`</style` or `</script`. Quoted CSS strings are escaped.

**Everything is checked at compile time**:

| code        | meaning                                                                       |
| ----------- | ----------------------------------------------------------------------------- |
| E004 / E117 | wrong direct parent / missing required ancestor (`item` outside `list`)       |
| E005        | missing required attribute (`link` and `image` need `url`)                    |
| E018        | value has the wrong type (URL, int, uint, float, color, date, MIME type, ...) |
| E111 / E112 | unknown element / unknown attribute, with "did you mean"                      |
| E113 / E114 | attribute not allowed on this element / given twice                           |
| E115 / E116 | child not allowed here / forbidden anywhere inside (link in link)             |
| E118 / E119 | void element with children / element that cannot hold text                    |
| E120        | element allowed only once (`main`, table `header`)                            |
| E121 / E122 | `for`/`aria controls` point at a missing id / duplicate id                    |
| E123        | a std generator (`std/layoutgen`) failed to compile or run                    |
| E125        | element must come first (`caption` in `table`, `legend`, `summary`)           |
| E126 / E127 | font type cannot be detected / declared type disagrees with the file          |
| W128        | a local font file is missing next to the source                               |
| E129 / E130 | value not known at compile time / a constant expression that cannot fold      |
| W131        | a constant hides a layout value word of the same name                         |
| E132 / E133 | value must be static here / `if`/`each` in a static layout                    |
| E134 / E135 | fixed id inside `each` / runtime value of an unsafe type for this attribute   |
| E136        | one `class` given different styles on two elements                            |
| W124        | the schema in `std/layout` itself has a problem (a std bug, not yours)        |

Void elements print `<img ...>` (never `/>`). `page build` exits non-zero
and writes no files on any of these errors.

**Generators.** Most elements use the data-driven default generator. A
`generator` field on an element or on an enum value picks a std generator: a
plain Salam function in `std/layoutgen` that takes a `layoutgen.Node` and returns
a `layoutgen.Output { html, css, js, head, shared_css, shared_js }`. Example:
`input: type = "jalali date" end` emits `<input type="text" class="elm_1">` plus
the JS that validates and normalises Jalali dates. The schema format is
documented in `docs/LAYOUT_SCHEMA.md`.

Build: `salam page build page.salam` → `page.html` + `page.css` + `page.js`;
`--inline` → one self-contained HTML file; multiple files → per-page HTML with
merged `style.css`/`script.js`.

---

## 10. Tooling & verification

```sh
salam check app.salam                   # lex, parse and type-check only: the fast loop after every edit
salam exec app.salam                    # run with the interpreter (pure compute, no C toolchain)
salam run app.salam                     # build + run, keep nothing
salam build app.salam --output=app      # native executable (add --keep-temp to inspect the C)
salam obj app.salam                     # object file only
salam build app.salam --target=<triple> # cross-compile via LLVM
salam page build page.salam [--inline]# layout DSL → HTML/CSS/JS
salam format app.salam                  # reformat in place (--check to verify; --lang=fa for Persian)
salam sql check src --schema=db/      # check every SQL query in .salam/.sql files against the schema
salam sql generate q --schema=db/ --output=gen/store.salam   # typed query functions from annotated .sql
salam new name                          # scaffold a project: name/main.salam + salam.toml
salam init                              # same, in the current directory
salam add github.com/owner/repo         # add a GitHub package (import "github.com/owner/repo"); remove/update/deps too
salam install github.com/owner/tool     # build a program into ~/.salam/bin; salam search <word> finds packages
salam update / salam update pkg@latest  # same-major updates by default; uninstall, deps clean too (docs/PACKAGES.md)
salam build --lib[=static]              # .so/.dylib/.dll (or .a) + NAME.h from the entry file's export: block
salam memcheck app.salam                # build with AddressSanitizer and run
salam version                           # print the compiler version (NOT --version)
salam upgrade [X.Y.Z] [--check]         # upgrade salam and std/ to the newest release (or X.Y.Z)
salam inspect app.salam --emit-tokens | --emit-ast | --emit-symbol   # dump a stage as XML
```

> **Version check is `salam version`, a subcommand.** There is no `--version`
> or `-v` flag: `salam --version` fails with `unknown command '--version'` and
> suggests `salam version`.

**Always verify a converted program.** Run `salam check file.salam` after every
edit (exit 1 on any error, `--error-format=json` for one JSON object per
diagnostic), then `salam exec file.salam` for a quick check of pure logic; use
`salam build … --output=…` (or `salam run`) when it uses FFI, threads, or the
full stdlib. Resolve every diagnostic; in Salam most are hard errors (§6).

> **`salam exec` (the interpreter) is a subset of the compiled language.** Some
> features (notably `Variant` type-pattern `match`, and parts of FFI/threads)
> only work through the C/LLVM backend. If `exec` reports a runtime error like
> "undefined variable 'i64'" on a `match` arm, that feature isn't interpreted:
> verify it with `salam run` / `salam build` instead. Treat the **compiled
> backend as the source of truth**, which is also what a self-hosting port targets.

---

## 11. When a detail is missing

Read the real code; it is the specification:

- Idioms & complete programs: `tests/en/`
  (`basics/`, `features/`, `types/`, `stdlib/`, `interop/`, `apps/`,
  `webframework/`, `games/`).
- Exact stdlib signatures & behavior: `std/<pkg>/*.salam`
  (`grep -rn "pub func" std/<pkg>`).
- The precise semantic rules (what is and isn't allowed): the `.salam` cases in
  `tests/en/errors/`.

---

## 12. Porting C → Salam (and how the compiler itself was self-hosted)

**The Salam compiler itself is now self-hosted**: its ~45,000 lines of C
(formerly `compiler/src/`) were fully ported to Salam; `compiler/` now holds
one flat `.salam` file per former module (`compiler/lexer.salam`,
`compiler/semantic.salam`, `compiler/codegen.salam`, …), and
`compiler/salam` is the tracked bootstrap binary that builds new versions of
itself from `compiler/main.salam`. There is no C source left to read, so this
section is kept as **general guidance for any other large, low-level C→Salam
port** you're asked to do (the same techniques apply), and §12.4's module
map is kept as a historical record of how the compiler's own port was
sequenced. Read §1 to §11 first; the notes below are the C-specific deltas and
the hard limitations you must plan around.

### 12.1 Bitwise operators (needed everywhere in a compiler port)

Salam has **first-class bitwise operators** on integers, mapping 1:1 to C, so the
compiler's bit-heavy code (UTF-8 encoding, hashing, flag sets, `codegen/print_fmt.c`,
`codegen/codegen_type.c`) ports directly with no workarounds:

| C                                   | Salam               | notes                                                      |
| ----------------------------------- | ------------------- | ---------------------------------------------------------- |
| `a & b`                             | `a & b`             | bitwise AND                                                |
| `a \| b`                            | `a \| b`            | bitwise OR                                                 |
| `a ^ b`                             | `a ^ b`             | bitwise XOR                                                |
| `~a`                                | `~a`                | bitwise NOT (unary)                                        |
| `a << n` / `a >> n`                 | `a << n` / `a >> n` | shifts (`>>` is arithmetic on signed, logical on unsigned) |
| `a &= b`, `\|=`, `^=`, `<<=`, `>>=` | same                | compound assignment                                        |

Operands must be integers (a bitwise op on a float is a compile error). **Precedence
follows C**: `*  /  %` › `+  -` › `<<  >>` › `<  <=  >  >=` › `==  !=` › `&` › `^` ›
`|` › `and` › `or`. So `flags & MASK == MASK` parses as `flags & (MASK == MASK)`, so add
parentheses (`(flags & MASK) == MASK`) exactly as you would in C.

`not` (Persian `وارونه`) is a unary prefix that binds tighter than any binary
operator, so `not a == b` means `(not a) == b`; write `not (a == b)` to negate a
comparison. `and`/`or` are `و`/`یا` in Persian.

```salam
mut flags := 0
flags |= 1 << 3          // set bit 3
flags &= ~(1 << 3)       // clear bit 3
on := (flags >> 2) & 1   // test bit 2
hi := (cp >> 6) & 0x3F   // UTF-8 continuation byte, straight from the C
```

> `&` is still also **reference-parameter** (`x &: T`) and **function-address**
> (`&fn`) in their own syntactic positions; the parser disambiguates by context
> (prefix `&fn` vs infix `a & b`), so there is no ambiguity in practice.

Nested generics work too: `Vector<Vector<int>>`'s trailing `>>` is understood as
two closing angle brackets, not a shift.

### 12.2 C construct → Salam

| C                                            | Salam                                                                                                                       |
| -------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| `typedef struct { … } T;`                    | `struct T: … end` (fields default private → add `pub`)                                                                      |
| `union { … }` / tagged union                 | **`Variant<A, B, …>`**, narrowed by `match` on type patterns                                                                |
| `enum { A, B=5 }`                            | `enum E: A, B = 5 end` (members on one line need commas; one per line needs none)                                           |
| `#define NAME 3` (const)                     | `const NAME := 3`                                                                                                           |
| `#define MAX(a,b) …` (macro fn)              | `inline func Max(a: int, b: int): int: … end`                                                                               |
| `#ifdef` / `#if` / platform `#ifdef _WIN32`  | `@if`-style top-level `if SALAM_OS_WINDOWS:` on compile-time constants (§8)                                                 |
| `#include "x.h"` / header+source split       | `package` + `import`; export with `pub` (no headers)                                                                        |
| function pointer `int (*f)(int)`             | typed value `func (int) int` (pass a lambda); or a bare `fn`/`&fn` for `i64`/`void*` callback tables                        |
| `void*` / `char*` / `T*`                     | `void*` / `str` or `u8*` / `T*`; deref `p[0]`; `null`                                                                       |
| `malloc/calloc/realloc/free`                 | `mem.Allocate / AllocateZeroed / Reallocate / Free`                                                                         |
| `memcpy/memset/memmove`                      | `mem.Copy / mem.Set / mem.MemMove`                                                                                          |
| `strlen/strcmp/strcpy/strcat`                | `str.Len / str.Compare / str.Clone / str.Concat` (+ `StringBuilder`)                                                        |
| growable array / `realloc` buffer            | `Vector<T>` (`push/get(i)/set/len`), remember `.free()`                                                                     |
| hash table (symbol table)                    | `HashMap<K, V>`                                                                                                             |
| `switch (x) { case … }` (fallthrough)        | **`switch x: … end`** - bare labels (no `case`/`default`), same fallthrough, `break` exits it (§2)                          |
| `switch` used for exhaustive/tagged dispatch | `match x: … end` instead - exhaustive on enum/`Variant`, no fallthrough (§3)                                                |
| `goto`                                       | not available; restructure with functions/flags/loops                                                                       |
| `break` / `continue`                         | `break` / `continue` (innermost loop **or switch** for `break`, `break N` for N levels; `continue` always targets the loop) |
| `while (c)`                                  | **`until c:`** with the same condition, **never negated** (`while (v != 0)` → `until v != 0:`)                              |
| `while (v)` / `while (p)` (truthy)           | `until v != 0:` / `until p != null:` (Salam has no truthiness)                                                              |
| `do { B } while (c);`                        | `until true: B  if not c: break end end`                                                                                    |
| `for (;;)`                                   | `until true:` + `break`                                                                                                     |
| `for (i = 0; i < n; i++)`                    | `repeat n in i:` (i = 0 .. n-1)                                                                                             |
| `for (i = n; i >= 1; i--)`                   | `repeat n to 1 in i:` (descending; guard `n >= 1`, see §2)                                                                  |
| `for (i = 0; i < n; i += 2)`                 | `repeat 0 to n - 1 by 2 in i:` (`by` is always positive)                                                                    |
| variadic `f(int, ...)`                       | only in `extern`/FFI; pure Salam passes a `Vector`                                                                          |
| `static` file-local                          | default (package-private); top level, not `pub`                                                                             |
| `static`/global mutable state                | top-level `mut` globals are allowed (`mut g_count := 0`)                                                                    |
| `const T x`                                  | `const NAME := …` (compile-time) or an immutable `:=` binding                                                               |
| `errno` / return-code error handling         | `bool` / `Option<T>` / sentinel + an error record/struct                                                                    |
| `assert()`                                   | `import testing` (`AssertTrue`, …) or an explicit `if … : print + os.Exit`                                                  |
| `int8_t … uint64_t`, `size_t`, `ssize_t`     | `i8…i64`, `u8…u64`, `usize` for `size_t`, `size` for `ssize_t`/`intptr_t`/`ptrdiff_t`                                       |
| bitfields / flag enums                       | bitwise ops on an integer (`flags & MASK`, §12.1), or a small struct of `bool`s                                             |

### 12.3 Building blocks the compiler needs (all in Salam today)

- **Reading source**: `io.ReadFile(path)` → `str`; scan bytes/codepoints with
  `str.CharAt`/`str.Chars`/`str.Len`; classify with `str.IsDigit/IsAlpha/IsSpace`.
- **Buffers/output**: `str.StringBuilder` (`NewBuilder`, `BufAppend`,
  `BufAppendInt`, `BufStr`, `BufFree`) for emitting generated C / IR.
- **Tables & lists**: `HashMap` (symbol tables, string interning), `Vector`
  (token streams, AST child lists, scope stacks), `Set`, `Stack`.
- **AST nodes**: a `struct` per node kind, or one node `struct` with a `kind`
  `enum` field plus a `Variant` payload; walk with recursion and `match`.
- **Arenas / lifetime**: `mem.Allocate`/`Free`; or lean on `Vector`/`HashMap`
  ownership + `defer x.free()`. There is no borrow checker, so free deliberately.
- **Diagnostics**: return a `bool`/`Option` up the call chain and collect an
  error list (mirror `src/diag`); no exceptions to unwind.

### 12.4 The compiler's module map (recreate this shape)

The C pipeline is **source → lexer → token → parser → AST → semantic →
codegen** (with `llvm` and `jsgen` as alternative backends), all wired by
`driver`. Approximate sizes (from `compiler/src/`, ~45k LOC total); port roughly
in dependency order:

| Module                                | LOC                       | Role                                                                |
| ------------------------------------- | ------------------------- | ------------------------------------------------------------------- |
| `core`                                | 1.1k                      | shared utilities (arena, strings, containers)                       |
| `source` / `token`                    | 0.1k / 0.4k               | source buffer; token kinds & values                                 |
| `lexer`                               | 1.8k                      | text → tokens (numbers, strings, idents, operators, layout, trivia) |
| `ast`                                 | 0.6k                      | AST node types                                                      |
| `parser`                              | 2.7k                      | tokens → AST (decls, exprs, FFI, layout)                            |
| `semantic`                            | 8.2k                      | name resolution, type checking, the strict rules in §6, const-fold  |
| `condcomp`                            | 0.6k                      | conditional compilation (`SALAM_OS_*`, `-D`)                        |
| `codegen`                             | 4.8k                      | AST → C (the default backend)                                       |
| `llvm`                                | 5.7k                      | AST → LLVM IR (cross-compile backend)                               |
| `jsgen` / `layout` / `minify` / `web` | 2.6k / 2.5k / 0.3k / 0.4k | JS + layout-DSL → HTML/CSS/JS                                       |
| `interp`                              | 3.5k                      | tree-walking interpreter (`salam exec`)                             |
| `driver` / `cli`                      | 4.6k / 0.7k               | command dispatch, build orchestration                               |
| `diag` / `logger` / `xml`             | 0.8k / 0.3k / 0.2k        | errors, logging, `--emit-tokens/-ast/-symbol`                       |
| `i18n` / `langpack`                   | 1.6k / 0.4k               | English/Persian keyword & symbol tables                             |
| `fmt`                                 | 1.0k                      | source formatter (`salam format`)                                   |

**Suggested port order:** `core` → `token`/`source` → `lexer` → `ast` →
`parser` → `semantic` → one backend (`codegen` C) → `driver`/`cli`, then the
remaining backends and tools. Port module-by-module, keeping the existing C build
runnable, and validate each stage against the current compiler's
`--emit-tokens` / `--emit-ast` / `--emit-symbol` output and the
`tests/` suite. The bit-heavy code (lexer, hasher, codegen) ports directly
now that bitwise operators exist (§12.1).

---

## Part II: Persian Salam (سلام فارسی)

Everything in Part I holds for Persian source: same grammar, same types, same
strict rules, same stdlib. Only the spelling changes. This part is the full
spelling reference plus the few things that behave differently because the
source is Persian. The human-facing version of this material is the Persian
course at **<https://www.salamlang.ir/learn/>** (§19 maps its lessons).

## 13. Persian source basics

- **Language detection is automatic** from the keywords in the file; pass
  `--lang=fa` to force it (`salam build app.salam --lang=fa`). Diagnostics are
  printed in Persian for Persian files.
- **Language directive:** a Persian file may state its language with
  `// زبان: فارسی` in its first 8 lines; an English file uses `// lang: en`.
  Mixed forms such as `// lang: fa` or `// زبان: en` are a compile error.
- **The entry function is `ریشه`**, not `main`: `تابع ریشه:` ... `پایان`.
  Its return value is the exit code, as in English (`برگشت ۱` to fail).
- **Digits:** Persian `۰۱۲۳۴۵۶۷۸۹` and Arabic-Indic `٠١٢٣٤٥٦٧٨٩` digits work in
  number literals, mixed freely with ASCII. The decimal point is always `.`
  (`۳.۱۴`); the Persian decimal separator `٫` is **not** accepted.
- **Printed numbers and booleans are ASCII**: `سرچاپ ۱۲` prints `12`, and a
  `منطقی` prints `true`/`false`. Convert digits yourself if the output must
  be Persian.
- **Separators:** the Persian comma `،` works everywhere the ASCII `,` does
  (arguments, parameters, enum members, array literals, struct literals,
  match patterns). A new line also separates call arguments and parameters.
- **Names:** identifiers may be Persian and may contain spaces or ZWNJ
  (`تابع جمع دو عدد(...)`, `تکه‌ها`). A space and a ZWNJ (U+200C) are the
  same inside a name, so `آرگومان ها` and `آرگومان‌ها` are one name. Arabic
  `ي`/`ك` equal Persian `ی`/`ک`.
- **Keywords with ZWNJ** (`نادرست‌چاپ`, `درون‌داد`, `بی‌کاره`, ...) may be
  written with a ZWNJ or a space, but not glued together: `نادرستسرچاپ` is an
  unknown identifier.
- Strings are UTF-8 bytes, as in English: `"سلام".طول()` is `8` (bytes);
  use `.شمارنویسه()` for the letter count (`4`).
- `salam translate fa file.salam` rewrites English source to Persian
  (`translate en` goes back): keywords, `true/false/null/this`, the entry
  function, primitive type names, and the built-in methods of §15.
  Comparisons come out as words in Persian (`بزرگتر`, `برابر`, ...) and as
  symbols in English (`>`, `==`, ...); generic `<T>` keeps its brackets.

## 14. Persian keywords

| English             | Persian             | English                   | Persian                       |
| ------------------- | ------------------- | ------------------------- | ----------------------------- |
| `func`              | `تابع`              | `ret`                     | `برگشت`                       |
| `if`                | `اگر`               | `else`                    | `وگرنه`                       |
| `until` (while)     | `تا`                | `repeat`                  | `تکرار`                       |
| `to` (in repeat)    | `تا`                | `by` (step)               | `هر`                          |
| `each`              | `هر`                | `in`                      | `در`                          |
| `match`             | `همخوان`            | `switch`                  | `ترابرد`                      |
| `break`             | `بشکن`              | `continue`                | `گذر`                         |
| `mut`               | `ناپایا`            | `const`                   | `پایا`                        |
| `type`              | `گونه`              | `struct`                  | `ساختار`                      |
| `enum`              | `جداشمار`           | `interface`               | `میانجی`                      |
| `impl`              | `کاربست`            | `on` (impl X on T)        | `بر`                          |
| `end`               | `پایان`             | `as`                      | `برگردان`                     |
| `import`            | `واردسازی`          | `include`                 | `فراخوانی`                    |
| `package`           | `بسته`              | `pub`                     | `همگانی`                      |
| `true` / `false`    | `درست` / `نادرست`   | `null`                    | `پوچ`                         |
| `this`              | `این`               | `defer`                   | `دیرکن`                       |
| `print` / `println` | `چاپ` / `سرچاپ`     | `printerr` / `printerrln` | `نادرست‌چاپ` / `نادرست‌سرچاپ` |
| `input`             | `ورودی`             | `extern`                  | `درون‌داد`                    |
| `export`            | `برون‌داد`          | `page`                    | `صفحه`                        |
| `component`         | `بخش`               | `inline` / `noinline`     | `درخط` / `نادرخط`             |
| `pure`              | `ناب`               | `noret`                   | `نابرگشت`                     |
| `deprecated`        | `بی‌کاره`           | `and` / `or` / `not`      | `و` / `یا` / `وارونه`         |
| `eq` / `neq`        | `برابر` / `نابرابر` | `main` (entry)            | `ریشه`                        |
| `lt` / `gt`         | `کوچکتر` / `بزرگتر` | `lte` / `gte`             | `کوچکتربرابر` / `بزرگتربرابر` |

Context words (keywords only in their position, usable as names elsewhere):
`static` `ایستا` (`ایستا تابع`, `پیوند ایستا`), `dynamic` `پویا`
(`پیوند پویا`), `dyn` `پویا` (before a type: `پویا شکل`), `link` `پیوند`,
`framework` `چارچوب`, `use` (struct embedding) `شامل`.
`page` `صفحه` is a keyword only at the start of a line that ends with `:`
(`page:`, `صفحه خوشامد(نام: رشته):`); elsewhere the word is a normal name.
`mut func` is `ناپایا تابع`.

**The built-in functions have Persian names, and a Persian file must use
them** - the English spelling is an error (`E001`):

| English          | Persian           |
| ---------------- | ----------------- |
| `len`            | `طول`             |
| `sizeof`         | `اندازه‌گونه`     |
| `args`           | `آرگومان‌ها`      |
| `env`            | `متغیر‌محیطی`     |
| `lang`           | `زبان`            |
| `open`           | `بازکردن`         |
| `listdir`        | `فهرست‌پوشه`      |
| `hash`           | `درهم`            |
| `char_code`      | `کدنویسه`         |
| `char_from_code` | `نویسه‌ازکد`      |
| `strcmp`         | `مقایسه‌رشته`     |
| `spawn`          | `نخ‌ساز`          |
| `join`           | `نخ‌پیوند`        |
| `callhandler`    | `فراخوان‌دستگیره` |
| `atomic_load`    | `اتمی‌بخوان`      |
| `atomic_store`   | `اتمی‌بنویس`      |
| `atomic_add`     | `اتمی‌بیفزا`      |
| `atomic_swap`    | `اتمی‌جابجا`      |
| `atomic_cas`     | `اتمی‌مقایسه`     |

**Stay English in Persian files:** the compile-time constants (`SALAM_OS`,
`SALAM_OS_WINDOWS`, ...) and C names declared in `درون‌داد`.

Two Persian words have two meanings, told apart by position:

- `تا` is `until` at the start of a statement and `to` inside a `تکرار`
  header: `تا ک < ۳:` loops while `ک < ۳`; `تکرار ۱ تا ۵ در ای:` counts 1..5.
  Persian `تا` reads naturally as "while", so the §2 polarity trap is less
  likely, but the rule is the same: the loop runs **while** the condition
  holds.
- `هر` is `each` at the start of a statement and `by` (the step) inside a
  `تکرار` header: `هر ع در لیست:` vs `تکرار ۰ تا ۲۰ هر ۲ در ای:`.

`و` is a reserved word (`and`), so it can never be a name; `و۱` and `وکتور`
are fine.

## 15. Persian types, built-in methods and std package names

**Types:**

| English          | Persian                      | English          | Persian                  |
| ---------------- | ---------------------------- | ---------------- | ------------------------ |
| `void`           | `تهی`                        | `bool`           | `منطقی`                  |
| `char`           | `نویسه`                      | `uchar`          | `یونیکد`                 |
| `str`            | `رشته`                       | `int` (`i32`)    | `صحیح` (`صحیح۳۲`)        |
| `i8` `i16` `i64` | `صحیح۸` `صحیح۱۶` `صحیح۶۴`    | `uint` (`u32`)   | `طبیعی` (`طبیعی۳۲`)      |
| `u8` `u16` `u64` | `طبیعی۸` `طبیعی۱۶` `طبیعی۶۴` | `usize` / `size` | `اندازه مثبت` / `اندازه` |
| `float` (`f32`)  | `اعشار` (`اعشار۳۲`)          | `f64`            | `اعشار۶۴`                |
| `Vector<T>`      | `وکتور<T>`                   | `HashMap<K, V>`  | `نگاشت<K, V>`            |
| `MapIter`        | `پیمایشگرنگاشت`              | `File`           | `پرونده`                 |
| `Variant<...>`   | `گوناگون<...>`               |                  |                          |

Type digits may be Persian or ASCII (`صحیح۶۴` = `صحیح64`). Note that
`اعشار` is **f32**; a float literal such as `۲.۵` is `اعشار۶۴`, so write
`اعشار۶۴` for ordinary floating point.

**Built-in methods** (on `str`, `Vector`, `HashMap`, `File` and iterators):

| English     | Persian       | English       | Persian         |
| ----------- | ------------- | ------------- | --------------- |
| `push`      | `بیفزا`       | `pop`         | `دربیاور`       |
| `get`       | `بگیر`        | `ref`         | `ارجاع`         |
| `set`       | `بنشان`       | `len`         | `طول`           |
| `cap`       | `ظرفیت`       | `free`        | `آزادکن`        |
| `put`       | `درج`         | `has`         | `دارد`          |
| `remove`    | `حذف`         | `size`        | `اندازه`        |
| `iter`      | `پیمایش`      | `has_next`    | `داردبعدی`      |
| `key`       | `کلید`        | `value`       | `مقدار`         |
| `next`      | `بعدی`        | `read`        | `خواندن`        |
| `readline`  | `خواندن خط`   | `write`       | `نوشتن`         |
| `seek`      | `جابجایی`     | `close`       | `ببند`          |
| `concat`    | `پیوست`       | `substr`      | `زیررشته`       |
| `find`      | `بیاب`        | `split`       | `بشکاف`         |
| `trim`      | `پیراست`      | `to_int`      | `به صحیح`       |
| `to_float`  | `به اعشار`    | `char_count`  | `شمارنویسه`     |
| `char_at`   | `نویسه شماره` | `char_substr` | `زیررشته نویسه` |
| `char_find` | `بیاب نویسه`  | `starts_with` | `شروع با`       |
| `ends_with` | `ختم با`      | `includes`    | `دربردارد`      |
| `copy`      | `رونوشت`      | `deep_copy`   | `رونوشت عمیق`   |
| `bytes`     | `بایت ها`     |               |                 |

The free built-in `len(x)` (arrays, slices and `str` only) is spelled
`طول(x)` in a Persian file: `len(x)` there is error E001 ("identifier must
be Persian in a Persian file"). The method form is `x.طول()` and works on a
`وکتور` too, which the free form does not.

**Std packages and their functions** have Persian names declared with
`@fa "..."` next to `@en "..."` in `std/`. Import by the Persian name and call
through it; never guess a name, read the `@fa` line in `std/<pkg>/`:

```salam
واردسازی رشته
واردسازی ریاضی
واردسازی سیستم عامل

تابع ریشه:
    سرچاپ رشته.طول("سلام")، ریاضی.جذر(۱۶.۰)
    سرچاپ سیستم عامل.آرگومان‌ها().طول()
پایان
```

Common packages: `str` `رشته`, `math` `ریاضی`, `os` `سیستم عامل`,
`io` `ورودی خروجی`, `fmt` `قالب بندی`, `conv` `تبدیل`, `time` `زمان`,
`rand` `تصادفی`, `sort` `مرتب سازی`, `json` `جیسون`, `regex`
`عبارت باقاعده`, `collections` `مجموعه ها`, `mem` `حافظه`, `path` `مسیر`,
`fs` `سیستم پرونده`, `file` `پرونده`, `dir` `پوشه`, `sync` `همگام سازی`,
`thread` `نخ`, `chan` `کانال`, `testing` `آزمایش`, `template` `قالب`,
`log` `گزارش`, `result` `نتیجه`, `option` `اختیاری`, `crypto` `رمزنگاری`,
`bigint` `عدد بزرگ`, `net` `شبکه`, `net/http` `اچ تی تی پی`
(imported as `شبکه.اچ تی تی پی`), `web` `وب`, `db` `دیتابیس`,
`db/typesense` `تایپسنس` (imported as `دیتابیس.تایپسنس`),
`db/meilisearch` `میلیسرچ` (imported as `دیتابیس.میلیسرچ`),
`db/opensearch` `اپن سرچ` (imported as `دیتابیس.اپن سرچ`), `dom` `دام`,
`term` `پایانه`, `cli` `خط فرمان`, `audio` `صدا`, `audio/wav` `ویو`
(imported as `صدا.ویو`), `audio/dsp` `پردازش سیگنال` (imported as
`صدا.پردازش سیگنال`), `audio/codec` `کدک` (imported as `صدا.کدک`),
`audio/flac` `فلاک`, `audio/mp3` `ام‌پی‌تری`, `audio/aiff` `ایف`,
`audio/au` `ای‌یو`, `audio/tag` `برچسب صدا`, `audio/music` `موسیقی`,
`audio/synth` `صداساز`, `audio/midi` `میدی` (each imported under `صدا.`).

A few function names, to show the style: `str.Len` `طول`, `str.Split`
`تفکیک کردن`, `str.Join` `بپیوند`, `str.Trim` `پیرایش`, `str.Replace`
`جایگزینی`, `str.Contains` `شامل است`, `str.ToInt` `تبدیل به عدد`,
`str.FromInt` `از عدد`, `math.Sqrt` `جذر`, `math.Abs` `قدرمطلق`,
`math.Pow` `توان`, `math.Min` / `Max` `کمینه` / `بیشینه`, `os.Args`
`آرگومان ها`. **English std names are rejected in a Persian file**
(`رشته.Len(...)` is E001 "identifier must be Persian in a Persian file"), so
look the Persian name up instead of guessing it.

Give your own `pub` API both spellings the same way:

```salam
@en "Twice"
@fa "دوبار"
همگانی تابع دوبار(ع: صحیح): صحیح:
    برگشت ع * ۲
پایان
```

## 16. Persian syntax crib

```salam
پایا بیشینه := ۱۰                        // const (one word)
ناپایا شمار := ۰                         // mut global

ساختار نقطه:
    همگانی ایکس: صحیح = ۰
    همگانی ایگرگ: صحیح = ۰
    همگانی تابع جمع(): صحیح:
        برگشت این.ایکس + این.ایگرگ
    پایان
پایان

جداشمار رنگ: قرمز، سبز، آبی پایان

تابع دوبرابر(ن: صحیح): صحیح:
    برگشت ن * ۲
پایان

تابع ریشه:
    نام := "سارا"                        // immutable
    ناپایا ک := ۰                        // mutable
    ع := ۲.۵ برگردان اعشار۶۴             // cast with برگردان
    اگر ک > ۱۰ و ک < ۲۰:
        سرچاپ "بین"
    وگرنه ک == ۰:                        // else-if
        سرچاپ "صفر"
    وگرنه:
        سرچاپ "دیگر"
    پایان
    تا ک < ۳:                            // while
        ک += ۱
    پایان
    تکرار ۳:                             // three times
        چاپ "*"
    پایان
    تکرار ۱ تا ۵ در ای:                  // 1..5 inclusive
        چاپ ای، ""
    پایان
    تکرار ۰ تا ۲۰ هر ۵ در ای:            // with a step
        چاپ ای، ""
    پایان
    آ := [۱۰، ۲۰]
    هر ش، م در آ:                      // index and value
        سرچاپ ش، م
    پایان
    ن := نقطه { ایکس = ۳، ایگرگ = ۴ }
    سرچاپ ن.جمع()، دوبرابر(ن.ایکس)
    متن := همخوان رنگ.سبز:               // match: bare member names
        قرمز، آبی: "گرم یا سرد" پایان
        سبز: "سبز" پایان
    پایان
    ترابرد ک:                            // switch: each label has its own پایان
        ۳:
            سرچاپ "سه"
            بشکن
        پایان
        وگرنه:
            سرچاپ "?"
        پایان
    پایان
    سرچاپ نام، ع، متن
پایان
```

More forms, each checked with the current compiler:

```salam
// collections
ناپایا و۱ := وکتور {} برگردان وکتور<صحیح>
دیرکن و۱.آزادکن()
و۱.بیفزا(۵)
سرچاپ و۱.بگیر(۰)، و۱.طول()
ناپایا نگ := نگاشت {} برگردان نگاشت<رشته، صحیح>
دیرکن نگ.آزادکن()
نگ.درج("الف"، ۱)
هر کلید، مقدار در نگ:
    سرچاپ کلید، مقدار
پایان

// lambdas: no return type, captured by value
دوبرابر := (ع: صحیح): برگشت ع * ۲ پایان
رده := (نمره: صحیح):
    برگشت نمره >= ۱۰ ? "قبول" : "مردود"
پایان
تابع به‌کاربردن(ر: تابع (صحیح) صحیح، مقدار: صحیح): صحیح:
    برگشت ر(مقدار)
پایان

// guards use اگر
جداشمار شکل:
    دایره(شعاع: اعشار۶۴)
    مستطیل(پهنا: اعشار۶۴، بلندی: اعشار۶۴)
پایان
تابع مساحت(ش: شکل): اعشار۶۴:
    برگشت همخوان ش:
        دایره(ر): ۳.۱۴ * ر * ر پایان
        مستطیل(پ، ب) اگر پ == ب: پ * پ پایان
        مستطیل(پ، ب): پ * ب پایان
    پایان
پایان

// switch on true replaces an if chain
ترابرد درست:
    ک < ۰:
        سرچاپ "منفی"
        بشکن
    پایان
    ک == ۳ یا ک == ۴:
        سرچاپ "سه یا چهار"
        بشکن
    پایان
پایان

// interfaces, impl on a built-in type, generics, پویا (dyn)
میانجی رتبه‌دار:
    تابع رتبه(): صحیح
پایان
کاربست رتبه‌دار بر رشته:
    تابع رتبه(): صحیح: برگشت طول(این) پایان
پایان
تابع بالاتر<ت: رتبه‌دار>(الف: ت، ب: ت): صحیح:
    اگر الف.رتبه() > ب.رتبه():
        برگشت الف.رتبه()
    پایان
    برگشت ب.رتبه()
پایان
تابع توصیف(ش: پویا شکل‌دار):             // پویا is dyn
    سرچاپ ش.نام()
پایان

// static members, mut methods, embedding
ساختار شمارنده:
    ن: صحیح = ۰
    همگانی پایا سقف := ۱۰۰
    همگانی ایستا تابع تازه(آغاز: صحیح): شمارنده:
        برگشت شمارنده { ن = آغاز }
    پایان
    همگانی ناپایا تابع بیفزای():
        این.ن += ۱
    پایان
پایان
ساختار سگ:
    همگانی شامل جانور                     // embeds جانور's fields and methods
    همگانی نژاد: رشته = ""
پایان

// Variant
تابع شرح(م: گوناگون<صحیح، رشته>): رشته:
    برگشت همخوان م:
        صحیح ع: "عدد " + ع پایان
        رشته ر: "متن " + ر پایان
    پایان
پایان

// reference parameter, defer, pointers
تابع واریز(ح &: حساب، مبلغ: صحیح):
    ح.موجودی += مبلغ
پایان
دیرکن سرچاپ "پاکسازی"
پ := پوچ برگردان صحیح*

// packages: بسته in the library file, فراخوانی in the user
بسته ابزار
همگانی تابع چهاربرابر(ع: صحیح): صحیح:
    برگشت ع * ۴
پایان
// ...and in the program:
فراخوانی ابزار "ابزار.salam"
سرچاپ ابزار.چهاربرابر(۵)

// C functions and compile-time branches
درون‌داد:
    تابع sqrt(x: اعشار۶۴): اعشار۶۴
پایان
پیوند پویا "sqlite3"
اگر SALAM_OS_WINDOWS:
    پایا جداکننده := "\\"
وگرنه:
    پایا جداکننده := "/"
پایان

// threads: spawn and join stay English
ر := spawn(کارگر)
join(ر)
```

## 17. Rules and traps specific to Persian

All of §6 applies. In addition:

1. **Entry is `ریشه`.** A Persian file with `تابع main` has no entry point.
2. **`«…»` is a string literal, in Persian files only.** It behaves exactly
   like `"…"`, including `\n` escapes and `+`, so pick whichever reads better:
   `سرچاپ «سلام دنیا»`. It does not nest, for the same reason `"` does not. In
   an English file `«` is not a delimiter at all. Leaving one open reports its
   own error rather than the double-quote one, since the fix is different:
   `رشته‌ی '«' بسته نشده است؛ آن را با '»' ببندید، نه با دابل‌کوتیشن`. A `»`
   with no `«` before it is its own error too, rather than being swallowed
   into the identifier that follows it.
   Text blocks (` ``` `, §6) work the same in a Persian file, and the code
   inside `${{ }}` uses Persian names: `${{ نام }}`, `${{ عدد + 1 }}`,
   `${{ «}}» }}`.
3. **The language's own constants are Persian here** (`سلام سیستم ویندوز`,
   `سلام نگارش`, `تعریف‌شده(…)`); the `SALAM_*` spellings do not work in a
   Persian file. See §8.
4. **Enum patterns in `همخوان` are bare member names** (`سبز:`), not
   `رنگ.سبز:`; the qualified form is a parse error in a pattern.
5. **`ترابرد` labels are blocks**: each label ends with its own `پایان`, and
   fallthrough continues into the next label unless you `بشکن`.
6. **`و` is reserved** and cannot be a name; pick `و۱`, `واحد`, ...
7. **No `٫` decimal separator**; write `۱۲.۵`.
8. **Output digits are ASCII** and booleans print as `true`/`false`.
9. **`اعشار` is f32.** Use `اعشار۶۴` unless you want single precision
   (`۰.۱ برگردان اعشار` prints `0.10000000149011612`). A float literal is
   already `اعشار۶۴`: casting a literal is allowed, but casting a variable
   to the type it already has is a useless cast (E093).
10. **No typed declarations**, as in English: `ک: صحیح = ۰` is a parse error;
    write `ک := ۰` or `ک := ۰ برگردان صحیح۶۴`.
11. **Top-level order** is the same (§6 rule 8): `بسته`, `واردسازی`,
    `فراخوانی`, `پایا`/`ناپایا` globals, then `ساختار`/`جداشمار`/`گونه`/
    `میانجی`/`کاربست`, then `تابع`s, private before `همگانی`.
12. **`پایا` names are one word**: `پایا حد بالا := ۳` is a parse error; use
    `حدبالا` or `حد_بالا`.
13. **Unknown Persian std name?** Read the `@fa` line in `std/<pkg>/*.salam`.
    The English name is not a fallback in a Persian file, and an invented
    translation will not resolve.
14. **Diagnostics are Persian.** The error codes (`E001`, `E087`, ...) are the
    same as in English, so search `tests/en/errors/` by code.

## 18. Complete Persian programs

A command-line program with a struct, a vector, a map and a match:

```salam
واردسازی رشته

جداشمار سطح: کم، متوسط، زیاد پایان

ساختار دانشجو:
    همگانی نام: رشته = ""
    همگانی نمره: صحیح = ۰
پایان

تابع سطح از(نمره: صحیح): سطح:
    اگر نمره >= ۱۷:
        برگشت سطح.زیاد
    وگرنه نمره >= ۱۲:
        برگشت سطح.متوسط
    پایان
    برگشت سطح.کم
پایان

تابع برچسب(س: سطح): رشته:
    برگشت همخوان س:
        کم: "ضعیف" پایان
        متوسط: "خوب" پایان
        زیاد: "عالی" پایان
    پایان
پایان

تابع ریشه:
    ناپایا کلاس := وکتور {} برگردان وکتور<دانشجو>
    دیرکن کلاس.آزادکن()
    کلاس.بیفزا(دانشجو { نام = "سارا"، نمره = ۱۹ })
    کلاس.بیفزا(دانشجو { نام = "علی"، نمره = ۱۳ })
    کلاس.بیفزا(دانشجو { نام = "رضا"، نمره = ۹ })

    ناپایا شمار := نگاشت {} برگردان نگاشت<رشته، صحیح>
    دیرکن شمار.آزادکن()
    ناپایا جمع := ۰
    هر د در کلاس:
        ب := برچسب(سطح از(د.نمره))
        سرچاپ د.نام، د.نمره، ب
        جمع += د.نمره
        قبلی := شمار.دارد(ب) ? شمار.بگیر(ب) : ۰
        شمار.درج(ب، قبلی + ۱)
    پایان
    سرچاپ "میانگین:"، جمع / کلاس.طول()
    سرچاپ "عالی‌ها:"، شمار.بگیر("عالی")
    سرچاپ رشته.طول("پایان")
پایان
```

The same program in English is a direct keyword-for-keyword translation;
`salam translate en` produces it.

## 19. The Persian tutorial on salamlang.ir

The course at <https://www.salamlang.ir/learn/> teaches the whole language in
Persian, one lesson per page, and every example on it is compiled and run
when the site is built. Point Persian-speaking users to the matching lesson:

| Topic                                            | Lesson                                         |
| ------------------------------------------------ | ---------------------------------------------- |
| First program, `چاپ`/`سرچاپ`, comments, `ورودی`  | <https://www.salamlang.ir/learn/start/>        |
| Variables, `ناپایا`, `پایا`, globals             | <https://www.salamlang.ir/learn/variables/>    |
| Types, integer sizes, casts with `برگردان`       | <https://www.salamlang.ir/learn/types/>        |
| Operators, bitwise, ternary, `و`/`یا`/`وارونه`   | <https://www.salamlang.ir/learn/operators/>    |
| Strings and their methods                        | <https://www.salamlang.ir/learn/strings/>      |
| `اگر`/`وگرنه`                                    | <https://www.salamlang.ir/learn/conditions/>   |
| `تا`, `تکرار`, `هر`, `بشکن`, `گذر`               | <https://www.salamlang.ir/learn/loops/>        |
| `همخوان` and `ترابرد`                            | <https://www.salamlang.ir/learn/match/>        |
| Functions, defaults, overloads, multi-word names | <https://www.salamlang.ir/learn/functions/>    |
| Lambdas and function types                       | <https://www.salamlang.ir/learn/lambdas/>      |
| `دیرکن`, `ناب`, `درخط` and other modifiers       | <https://www.salamlang.ir/learn/defer/>        |
| Arrays and slices                                | <https://www.salamlang.ir/learn/arrays/>       |
| `وکتور` and `نگاشت`                              | <https://www.salamlang.ir/learn/collections/>  |
| Structs, methods, `ایستا`, `شامل`                | <https://www.salamlang.ir/learn/structs/>      |
| Enums, enums with data                           | <https://www.salamlang.ir/learn/enums/>        |
| `گونه`, new types, operator overloading          | <https://www.salamlang.ir/learn/custom-types/> |
| Generics, `میانجی`, `کاربست`, `پویا`             | <https://www.salamlang.ir/learn/generics/>     |
| `بسته`, `واردسازی`, `فراخوانی`                   | <https://www.salamlang.ir/learn/packages/>     |
| Compiler rules and error codes                   | <https://www.salamlang.ir/learn/rules/>        |
| Values, references, memory                       | <https://www.salamlang.ir/learn/memory/>       |
| Compile-time `اگر`                               | <https://www.salamlang.ir/learn/compile-time/> |
| C interop, `درون‌داد`, `پیوند`                   | <https://www.salamlang.ir/learn/c-interop/>    |
| `spawn`/`join`                                   | <https://www.salamlang.ir/learn/threads/>      |
| Layout DSL `صفحه`                                | <https://www.salamlang.ir/learn/layout/>       |
| Full Persian/English glossary                    | <https://www.salamlang.ir/learn/keywords/>     |
