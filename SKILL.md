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
**`layout:`** blocks compile to HTML/CSS/JS. It can also be run with a
tree-walking interpreter (`salam exec`, pure compute only) and cross-compiled via
LLVM. Source can be written in English or Persian, with the same grammar and
the same compiler rules in both.

This skill has two parts:

- **Part I (§1 to §12): English Salam.** The full language, stdlib, rules and
  porting guide, with English keywords.
- **Part II (§13 to §19): Persian Salam (سلام فارسی).** Every Persian keyword,
  type, method and std package name, the Persian-only rules, and complete
  Persian programs. Read Part I for semantics and Part II for spelling; the
  semantics never differ.

For Persian there is also a complete 25-lesson human tutorial on the official
site: **<https://www.salamlang.ir/learn/>** (one page per topic, every example
compiled and run). There is no English site yet; for English, this file and
`tests/en/` are the reference.

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
each x in xs:  println x  end         // iterate a collection/array
each i, x in xs:  println i, x  end   // index + value (or key, value for a map)
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
- **Closures/lambdas** are first-class typed values: `(x: int) => x * 2`, or a
  block form `(): n = n + 1  ret n  end`. Function-typed parameters:
  `func () int`, `func (int, int) bool`.
  - **Lambdas capture by value.** Each lambda gets its own copy of the outer
    variables at creation time: after `mut n := 1  g := () => n  n = 5`,
    `g()` is still `1`, and a block lambda that does `n = n + 1` changes its
    own copy (it counts across calls), never the caller's `n`. Share state
    through a pointer, a heap collection or a `mut` global.
  - **Never write a return type on a lambda.** `(x: int): int => x` and a
    block `(x: int): int: ... end` fail to parse; the return type is inferred
    (`(x: int): ... ret x * 2 end` is fine).
  - **A bare named function decays to its address**, typed `i64` - the slot
    C-style callback registries take (e.g. the `web` router):
    `web.Get(r, "/", home)`. For a `void*` slot, or to cast to a typed C
    function pointer, use **`&fn`** instead. For a _typed_ Salam callback
    (`func (int) int`), pass a **lambda**: `apply((x: int) => inc(x), 3)`.
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
`str.CharCount(s)` the codepoint count; iterate codepoints with `str.Chars(s)` /
`str.CharAt(s, i)`, classify with `str.IsDigit(code)` etc.

**String/char literal forms** (no interpolation exists; build strings with `+`
or `fmt.Sprintf`):

| Syntax       | Type                     | Escapes (`\n \t \" \\ \xHH \uHHHH \UHHHHHHHH` …) | Multiline | Notes                                                                                                                                                                                                                                                                                                                                |
| ------------ | ------------------------ | ------------------------------------------------ | --------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `"text"`     | `str`                    | yes                                              | no        | normal string; raw newline in source is an error                                                                                                                                                                                                                                                                                     |
| `"""text"""` | `str`                    | yes                                              | **yes**   | triple-double-quote is the _only_ multiline form; still processes escapes                                                                                                                                                                                                                                                            |
| `'c'`        | `char`                   | yes                                              | -         | one raw byte unless escaped, not UTF-8 safe for non-ASCII                                                                                                                                                                                                                                                                            |
| `` `text` `` | `str`                    | **none, fully raw**                              | **yes**   | backtick string; every byte up to the next `` ` `` is taken literally, including `"`, `'`, `\`, and real newlines. **There is no triple-backtick form**; ` ``` ` lexes as an empty backtick string followed by a runaway one, not a multiline literal. Only a literal backtick can't appear inside it (no escape exists for `` ` ``) |
| `u'c'`       | `char` (UTF-8 codepoint) | no                                               | -         | must decode to exactly one Unicode codepoint; use for non-ASCII single chars, e.g. `u'م'`, `u'中'`, `u'€'`                                                                                                                                                                                                                           |
| `u"c"`       | `char` (UTF-8 codepoint) | yes                                              | -         | same as `u'c'` but escapes are processed first, e.g. `u"\U0001F600"`                                                                                                                                                                                                                                                                 |

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
len(a)                                // length builtin
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
    10, 9 => "A"
    8 => "B"
    else => "F"
end
```

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
        Circle(r) => 3.14 * r * r
        Rect(w, h) => w * h
        Empty => 0.0
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
(`Circle(r) if r > 10 => "big"`, `7 if ready:`). The guard sees the arm's
bindings and runs only when the pattern matches; if it is false, matching
continues with the next arm. A guarded arm does not count toward
exhaustiveness, so keep an unguarded arm (or `else`) for that case.

**`Variant<A, B, …>`** is a tagged union (one slot sized to the largest member).
Assign any member type; narrow it back with `match` on **type-name** patterns:

```salam
mut v := 21 as Variant<i32, f64, str>   // initial cast is OK from a *member* type (i32)
v = "offline"                            // then assign member-typed values directly
label := match v:
    i32 n => "int " + n
    f64 f => "float " + f
    str s => "text " + s
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
func draw(s: dyn Shape):  println s.area()  end                  // dynamic dispatch
shapes := [ Circle { r = 1.0 }, Rect { w = 2.0, h = 3.0 } ] as dyn Shape[3]
reg := Vector {} as Vector<dyn Shape>                            // heterogeneous collection
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

---

## 5. Standard library catalog

Import a library by name, never quoted: `import str`, `import math`. Dotted
subpackages: `import db.sqlite`. Your **own files** use `include` with a quoted
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

### Text & formatting

- **`str`**: `Upper Lower Title Trim TrimLeft TrimRight TrimPrefix TrimSuffix
Reverse Repeat Concat Substr Split Join Fields Chars CharAt Contains Find
IndexOf IndexFrom LastIndex Count StartsWith EndsWith Equals EqualFold Compare
Replace Len CharCount FromInt(i64) FromFloat(f64) ToInt ToFloat IsEmpty
IsDigit IsAlpha IsSpace ToEnglishDigits ToPersianDigits ToArabicDigits
DigitValue IsAnyDigit IsPersianDigit IsArabicDigit HasEnglishDigits
HasPersianDigits HasArabicDigits`; plus a `StringBuilder` (`NewBuilder`,
  `BufAppend`, `BufAppendInt`, `BufStr`, `BufFree`).
- **`fmt`**: `Sprintf(tmpl, Vector<str>)` (`{}` placeholders), `Fprintf`,
  `PadLeft PadRight Center`. (`fmt.Int`, `fmt.Float`, `fmt.Bool` build the string
  args you pass to `Sprintf`.)
- **`conv`**: `FormatInt FormatUint FormatHex FormatFloat FormatFloatPrec`.
- **`text`**: `ToUtf16(str): void*`, `FromUtf16(void*): str` (Windows/UTF-16 FFI).
- **`template`**: `Render(tmpl, ctx) RenderHTML NewContext Var EscapeHTML`.

### Numbers

- **`math`**: consts `PI E TAU`; `Sqrt Cbrt Pow(**) Hypot Exp Log Log2 Log10
Sin Cos Tan Asin Acos Atan Atan2 Sinh Cosh Tanh Floor Ceil Round Trunc Abs
Sign Min Max ClampF Lerp Radians Degrees Mod IsNaN IsInf NaN Inf`; integer:
  `MinI MaxI AbsI ClampI Gcd Lcm Factorial Pow10` (+ `*I64` variants).
- **`bigint`**: arbitrary-precision signed integers, one `Int` type (sign +
  base-2^32 magnitude). Build: `Zero One FromInt FromUint Parse ParseBase
ParseOr FromBytes Clone`; inspect: `Sign IsZero BitLen Bit IsOdd IsEven Cmp
CmpAbs Equals Less Greater Min Max`; arithmetic: `Add Sub Mul Sqr Neg Abs
AddInt SubInt MulInt Shl Shr`; division in **both** conventions - `DivMod Div
Rem` truncate toward zero (matching Salam's `/` and `%`), `FloorDivMod
FloorDiv FloorMod` round toward minus infinity, and `Mod` always lands in
  `[0, |m|)`; number theory `Pow Pow10 ModExp Gcd Lcm ExtGcd ModInverse Sqrt
Factorial`; two's-complement bitwise `And Or Xor AndNot Not PopCount
TrailingZeros`; out: `ToStr ToStrBase ToHex ToInt ToIntClamped ToFloat ToBytes
ByteLen`. In-place forms (`Set SetInt AddTo SubFrom MulBy AddIntTo MulIntBy
NegateIn ShlBy ShrBy`) mutate `x &: Int` and free the old value, for loops that
  should not allocate per iteration. **Every returning function hands over a
  fresh value the caller frees** (`defer x.free()`); operands are taken by
  value and are never mutated, so `Mul(x, x)` is fine. Not constant-time - for
  a secret exponent use the blinded routine in `std/ssh/bigint.salam`.
- **`decimal`**: exact fixed-point decimal - **this is the money type**. A
  `Dec` is a `bigint.Int` plus a scale, so `0.1 + 0.2` is exactly `0.3` and
  nothing overflows. Build: `Zero FromInt FromScaled(1999, 2)` → 19.99 `New
Parse ParseOr FromFloat` (lossy, documented); `Add Sub Mul` are **exact and
  never round** (Add/Sub take the larger scale, Mul the sum); `Div` cannot be,
  so it requires a scale _and_ a `Rounding` mode - `Div DivRound DivInt Pow
MulInt AddInt SubInt Percent Sum`. Rounding: `Rounding.{Down,Up,Floor,Ceil,
HalfUp,HalfDown,HalfEven}` with `Rescale Round`(HalfEven) `RoundHalfUp
Truncate Trim`. Compare `Cmp Equals Less Greater Min Max Sign IsZero
IsNegative Scale Unscaled` (1.0 equals 1.00). Out: `ToStr` (**always plain
  notation, never exponential**) `ToStrFixed FormatGrouped ToInt ToFloat`.
  Money splitting that cannot lose a cent: `Split(total, n)` and
  `Allocate(total, ratios)` guarantee the parts sum back to the total exactly
  (100.00 by 3 → 33.34, 33.33, 33.33); free the result with `FreeAll`.
- **`rand`**: `Seed SeedAuto Int Int32 IntN IntRange FloatRange Float Bool
BoolP Choice{Int,Str,Char} Shuffle{Int,Str} Alpha Alnum Digit Text UUID Hash`.
- **`stats`**: descriptive statistics helpers.
- **`matrix`**: dense linear algebra over `f64`, row-major, one `Matrix` type
  for matrices and vectors alike. Construct: `Zeros Ones Full Eye EyeOffset
Diag FromArray RowVector ColVector Arange Linspace Logspace Clone`. Elementwise:
  `Add Sub MulElem DivElem Scale Neg Combine AddScaled` (+ `...InPlace`),
  broadcast via `AddRowVector AddColVector MulRowVector MulColVector`, ufuncs
  `Abs Sqrt Exp Log Sin Cos Round Sign Clip PowElem Chop`. Products: `MatMul
(tiled) TransposeMul MulTranspose Gram MatVec VecMat Dot Outer Kron MatPow
Trace Cross3`. Shape: `Reshape Flatten Row Col SubMatrix Minor Delete/Insert
Row/Col SwapRows Diagonal Triu Tril HStack VStack BlockDiag Rot90 Roll Tile
Pad`. Reduce: `Sum Mean Min Max Var Std Median ArgMin/Max Row/ColSums
Row/ColMeans CumSumRows`. Norms: `NormFrobenius Norm1 NormInf Norm2
NormNuclear Cond1 CondInf Cond2 Normalize`. Factor: `Decompose(LU) Det
LogAbsDet Solve Inverse Adjugate QRDecompose LeastSquares Orthonormalize
Cholesky SolveSPD LDLDecompose Inertia SVDecompose SingularValues MatrixRank
PseudoInverse NullSpace ColumnSpace LowRankApprox EigSym EigVals Hessenberg
CharPoly`. Also `RREF NullSpaceExact SolveGeneral`, matrix functions `Expm
Sqrtm MatrixSign PolyEval CayleyHamilton LogmSym`, iterative `SolveJacobi
SolveGaussSeidel SolveSOR SolveCG PowerIteration Rayleigh`, statistics
  `Covariance Correlation Standardize PCAFit LinearRegression RSquared`,
  predicates `IsSymmetric IsOrthogonal IsPositiveDefinite IsToeplitz ...`,
  named matrices `Hilbert Vandermonde Toeplitz Circulant Companion Pascal
Rotation2D/3D Householder Givens Random RandomSPD RandomOrthogonal`, and
  printing `ToString ToStringPrec Print PrintLabeled ToCSV`. Shape errors
  return a `0x0` matrix rather than panicking.

### AI and machine learning

One family of packages, all pure Salam, seeded and deterministic (a fixed
seed gives the same result on every machine at any thread count). The deep
learning half is `f32` on `std/tensor`; the classical half is `f64` on
`std/matrix`. Shape errors return the empty tensor / `0x0` matrix.

- **`tensor`**: n-d `f32` arrays, contiguous row-major, no views. `Tensor
{data: Vector<f32>, shape: Vector<int>}` with `rank numel dim at set_at at2
set_at2 reshape_ free`; build with `Zeros(shape) Zeros1..4 Ones Full Scalar
FromArray Arange Clone CopyInto` and `Shape1..4` helpers. Elementwise `Add Sub
Mul Div` broadcast in four modes (equal, one element, trailing suffix `[N,D]+[D]`,
  trailing one `[N,1]`), plus `AddScalar Scale Neg Exp Log Tanh Sigmoid Relu Gelu
Sqrt Abs Square PowScalar Clip` and `*InPlace / *Into / AddScaled` forms. Shape:
  `Reshape SqueezeInPlace UnsqueezeInPlace Permute SliceAxis0 Narrow Concat2
Stack2 Pad2D`. Reduce: `SumAll MeanAll MaxAll MinAll ArgMaxAll SumAxis MeanAxis
MaxAxis ArgMaxAxis Softmax LogSoftmax`. Products: `MatMul MatMulInto MatMulAcc
TransposeMul MulTranspose BatchMatMul MatVec Dot Transpose2D` (packed 4x16
  kernel, ~90 GFLOP/s a core on the LLVM backend, 5x on six cores). Conv:
  `Conv2DForward MaxPool2DForward AvgPool2DForward Im2Col Col2ImAcc`. Random:
  `Rng NewRng DeriveSeed FillUniform FillNormal FillBernoulli RandUniform
RandNormal`. Parallel: `SetThreads Threads`. Bridge `ToMatrix FromMatrix`;
  records `WriteTensor ReadTensor`; `AllClose Equal MaxAbsDiff ShapeString`.
  General broadcasting and indexing: `BroadcastShape Expand`, masks `Greater
GreaterEqual Less LessEqual EqualMask NotEqualMask GreaterScalar LessScalar
EqualScalar`, `Where MaskedFill`, gathers `IndexSelect TakeRows Gather
ScatterAddRows OneHot`, and `TopK Tril Triu Cumsum`.
- **`autograd`**: tape-based reverse mode. `NewTape(seed)`, leaves `Input`
  (constant) `Watch` (tape-owned grad) `Param(val, grad)` (accumulates into the
  caller's buffer), recorded ops `Add Sub Mul Div Neg Scale AddScalar Exp Log
Tanh Sigmoid Relu Gelu Sqrt Square PowScalar MatMul BatchMatMul Sum Mean
SumAxis MeanAxis Softmax LogSoftmax Reshape Transpose2D Permute Embedding
Dropout LayerNorm RMSNorm BatchNorm GroupNorm Narrow SliceAxis0 Concat2 RoPE Silu
Conv2D MaxPool2D AvgPool2D`, `Constant` (an untracked tensor on the tape),
  losses `MSELoss
CrossEntropyLogits BCEWithLogits`, then `Backward(tp, loss)`, `Value Grad`,
  `tp.reset()` per step, `tp.free()`. `GradCheck` does central differences.
- **`nn`**: `ParamStore` (owns values and grads; layers hold int handles)
  with `add at zero_grad free`, `Bind`; layers `Linear Conv2D MaxPool2D
AvgPool2D LayerNorm BatchNorm GroupNorm RMSNorm Embedding Dropout LSTM GRU
MultiHeadAttention TransformerEncoderLayer` (`New*` constructors,
  `forward(tp, ps, x)`; `NewTransformerDecoderLayer` sets `attn.causal`, and
  `CausalMask(seq)` is the additive mask it adds), recurrent helper
  `LastStep`, the `Layer` interface with `Sequential` and the
  `ReluLayer GeluLayer TanhLayer` wrappers, `Flatten SinusoidalPositions`,
  init `XavierUniform XavierNormal
KaimingUniform KaimingNormal`, checkpoints `SaveCheckpoint LoadCheckpoint`
  (SLMT, `.gz` aware) and the `*Buf` forms.
- **`optim`**: `SGD` (momentum, nesterov, weight decay) `Adam` `NewAdamW`
  `RMSProp` with `step(ps)`, `ClipGradNorm ClipGradValue`, schedules `StepLR
CosineLR WarmupCosine`.
- **`ml`**: struct-per-estimator over `matrix`; `fit(x, y): bool`, `predict`,
  `predict_proba`, `transform`, `score`, `free`, hyperparameters as fields
  (`ml.KMeans { k = 3, seed = 7 }`). Preprocessing `StandardScaler MinMaxScaler
LabelEncoder OneHotEncoder SimpleImputer PolynomialFeatures`; models
  `LinearReg Ridge Lasso LogisticRegression KNNClassifier KNNRegressor
GaussianNB DecisionTree RandomForest AdaBoost GradientBoosting LinearSVM SVC
LDA QDA KMeans MiniBatchKMeans DBSCAN Agglomerative GaussianMixture PCA
TruncatedSVD TSNE MultinomialNB HistGradientBoosting` (`NewElasticNet(alpha,
l1_ratio)` builds the mixed-penalty `Lasso`), plus the `KDTree` index (`algorithm =
ml.KNN_KDTREE`) and `workers` on KNN and KMeans for parallel queries and
  assignment (identical results at any worker count; the serial path is the
  safe one inside another parallel loop);
  metrics `Accuracy BalancedAccuracy ConfusionMatrix Precision Recall F1
MacroF1 RocAuc RocCurve PrecisionRecallCurve AveragePrecision LogLoss
MatthewsCorrCoef CohenKappa MSE MAE RMSE R2Score ExplainedVariance
MedianAbsoluteError MAPE SilhouetteScore AdjustedRandIndex` (the two curves
  come back as m x 3 matrices: threshold, then the two rates); selection
  `TrainTestSplit KFold StratifiedKFold CrossValScore CrossValScoreParallel
MeanScore GridSearch` (callbacks; a grid is a `Vector<Candidate>`);
  `LabelsToInts CountClasses`.
- **`data`**: `MakeBlobs MakeMoons MakeCircles MakeRegression
MakeClassification` (seeded), `LoadCsv LoadCsvFile` → `Table` (`kinds
to_matrix split_xy cat_levels`), `MatrixDataset.batches` → `BatchIter`
  (`next next_tensor reset`), `LoadIdx LoadMnist ParseIdx`, `LoadImageFolder`,
  `XTensor YLabels`.
- **`npy`**: `ReadNpy ReadNpyFile WriteNpy WriteNpyFile ToMatrix FromMatrix
NpzList NpzRead NpzWrite`.
- **`onnx`**: `LoadOnnx ParseOnnx Run` for inference of MLP/CNN and
  transformer-style graphs: `Conv Gemm MatMul Relu Sigmoid Tanh Softmax
LogSoftmax MaxPool AveragePool GlobalAveragePool BatchNormalization
LayerNormalization InstanceNormalization Reshape Flatten Transpose Concat
Squeeze Unsqueeze Identity Dropout Constant Clip LeakyRelu PRelu Elu Selu
HardSigmoid Softplus Erf Gelu Abs Floor Ceil Round Sign Reciprocal Pow
Min Max Sum Mean ReduceMean ReduceSum ReduceMax ReduceMin Gather Slice
Split Shape Expand Where Equal Greater Less Cast`; anything else fails the
  run with the op named in `model.error`.
- **`gguf`**: `LoadGguf ParseGguf` (metadata, tensor directory),
  `LoadTensorF32` dequantizing `F32 F16 Q8_0 Q4_0 Q4_1`.
- **`llm`**: decoder-only language models (llama family: RMSNorm, RoPE,
  grouped-query attention, SiLU-gated FFN). `Config Model Layer`,
  `RandomModel(cfg, seed)`, `NewKVCache(cfg)` + `Forward(m, cache, tokens)`
  (positions continue from `cache.len`, so prompt-then-token decoding matches
  one-shot), `LogitsRow Greedy`, `Sampler` (`temperature top_k top_p
repeat_penalty`) with `GenerateIds Generate`, tokenizers `NewTokenizer`
  (`TOK_SPM` SentencePiece with `<0xNN>` byte fallback, `TOK_BPE` byte-level)
  with `add_token add_merge finish encode decode`, and the GGUF glue
  `ConfigFromGguf ModelFromGguf TokenizerFromGguf LoadGgufModel`.

### I/O, OS, filesystem

- **`io`**: `ReadFile WriteFile AppendFile Lines WriteLines Input Read Write
ReadAll Readline Seek Close Copy EPrint EPrintln Flush`. `println` already
  gets each line out to a redirected stdout on its own; `io.Flush()` is for
  output written by linked C code, which keeps its own buffer.
- **`os`**: `Args Env Cwd Chdir Exit Pid Run RunCapture Output Exists IsDir
FileSize Stat ReadFile WriteFile AppendFile Copy CopyTree Move Remove RemoveAll
Mkdir MkdirAll Rmdir ListDir ListDirs Walk TempDir Open`.
- **`filepath`**: `Join Dir Base Ext Stem Clean Normalize IsAbs`.
- **`fs`**, **`flag`** (CLI: `New AddStr AddInt AddBool AddFloat Parse GetStr…
Positional Usage`), **`config`**, **`log`** (`Info Warn Error Debug` + `*f`
  variants, `SetLevel ToFile ToStderr`).

### Terminal & TUI

- **`term`**: everything ANSI. **Never hand-roll `\x1b[` sequences** - this
  package is the one place they belong.
  - Terminal: `IsTTY IsTerminal Size Cols Rows` (`WinSize`), `Write WriteErr
Emit Flush Bell SetTitle EnableAnsi` (Windows virtual-terminal mode).
  - Raw mode: `MakeRaw` (no echo, no line editing, no signals) / `MakeCbreak`
    (unechoed keys, Ctrl-C still interrupts) / `Restore`, returning a
    `TermState`. **Always `defer term.Restore(st)` on the next line** - exiting
    without it leaves the user's shell in raw mode.
  - Colour: `Red Green Yellow Blue Magenta Cyan White Gray Bright* On* Bold Dim
Italic Underline Inverse Strike Style Paint Color256 OnColor256 RGB OnRGB`,
    plus `Sgr Seq{Fg,Bg}256 Seq{Fg,Bg}RGB` for raw sequences. All of them
    honour `SetColorMode(ColorAuto|ColorAlways|ColorNever)`, and ColorAuto
    obeys `NO_COLOR`/`CLICOLOR_FORCE`/`TERM=dumb`/isatty - so ONE code path
    serves both a terminal and a pipe. Capability probes: `SupportsColor
Supports256Color SupportsTrueColor`.
  - Escape-aware strings: `Strip Width Truncate PadRight PadLeft Center`. Use
    these, not `str.Len`/`fmt.PadRight`, on anything that may carry escapes -
    byte length counts an invisible `\x1b[31m` as five columns and misaligns
    every coloured table.
  - Cursor/screen, each as a pure `Seq*` builder plus a write-now twin:
    `MoveTo Up Down Left Right Column Home NextLine PrevLine Save/RestoreCursor
Hide/ShowCursor ClearScreen ClearLine ClearToLineEnd ClearBelow ResetLine
Scroll{Up,Down} Insert/DeleteLines SetScrollRegion Enter/ExitAltScreen
Enable/DisableMouse`. Build with `Seq*` and write once per frame; a
    non-positive count yields `""` (terminals read `CSI 0A` as 1).
  - Input: `Decode DecodeMouse` (pure, over the bytes a terminal sent - test
    key handling with no tty), `ReadKey GetKey ReadByte KeyReady ReadPassword`,
    `Key`/`MouseEvent`, `Key*` codes (`KeyUp KeyF1+n KeyChar` + `ctrl/alt/shift`).
  - Progress: `NewBar BarRender BarSet BarAdd BarDraw BarFinish` and
    `NewSpinner NewSpinnerFrames SpinnerFrame SpinnerTick SpinnerStop
SpinnerFree`, plus `FormatBytes FormatDuration`. Off a terminal the redraws
    are skipped and one plain escape-free line is written, so a build script
    needs no `if IsTerminal()` of its own.

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

- **`Vector<T>`** (built-in; also `import collections`): `push pop get(i)
ref(i) set(i,x) len is_empty first last insert remove_at reserve clear iter free`;
  index via `v[i]` (read) / `v[i] = x` (write); free functions `contains index_of
count_of slice clone reverse swap extend`.
- **`HashMap<K,V>`**: `put(k,v) get(k) has(k) remove(k) size is_empty
iter free`; iterate with `each k, v in m:`.
- **`Set<T>`**, **`Stack<T>`** (`push pop peek size is_empty`),
  **`Queue<T>`** (`enqueue dequeue peek size`),
  **`Deque<T>`** (`push_front push_back pop_front pop_back front_val back_val`),
  **`PriorityQueue<T>`** (`push pop peek`), **`LinkedList<T>`**,
  **`Counter<K>`** (`add add_n count distinct`), **`Pair`**, **`CircularList`**.
- **`sort`**: `Sort SortDesc SortBy StableSortBy Sorted IsSorted BinarySearch
LowerBound UpperBound Min Max Reverse Swap` + named algorithms
  (`QuickSort MergeSort HeapSort IntroSort …`). Comparators: `func (T, T) bool`.
- **`option`**: `Some None IsSome IsNone Unwrap UnwrapOr Expect`.

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
  `Schema(v)` derives the same type's **JSON Schema** (2020-12, `$defs` +
  `$ref`, so a self-referential type works) from the same declaration and the
  same markers - the argument is a value only because that is how a generic
  binds, and nothing reads it. A `@doc "..."` marker on a struct, an enum or
  a field becomes that schema's `description`, so what a field means is
  written once next to the field (several values on one marker join with a
  space: `@doc "a" "b"`; a second `@doc` on the same definition is an error).
  `SchemaEnvelope`/`SchemaMerge` are the two halves for callers that want the
  definitions separately.
- **`xml`** (`import encoding.xml`): two models in one package.
  - The **simple tree** is `XMLNode` (`tag attrs children text`) with `Decode
DecodeStrict Valid Encode EncodeIndent EncodeDeclaration Child Children Attr
HasAttr SetAttr AppendChild NewXMLNode Found ToValue FromValue EscapeText
EscapeAttr UnescapeEntities`, plus the pull tokenizer `NewXMLTokenizer` /
    `NextToken` (`XMLTok*` kinds). It ignores mixed-content order and
    namespaces, and is the right size for reading a config file.
  - The **full document model** is `Document`, an arena of nodes addressed by
    `int` handles. It keeps mixed content in order, resolves namespaces,
    expands internal entities and records a line and column for every node.
    Parse with `Parse(text, err)`, `ParseWith(text, opts, err)` or
    `WellFormed(text, err)`; `ParseError` carries `ok message line col offset`
    and `ErrorText` formats it. `ParseOptions` has `preserve_whitespace
keep_comments keep_instructions namespaces expand_entities max_depth`.
    Read with `Root DocumentNode Kind Name LocalName Prefix NamespaceURI Value
Parent ChildCount ChildAt ChildNodes Elements ElementsNamed FirstElementNamed
Descendants Ancestors NextSibling PreviousSibling Depth Path Line Column
AttributeCount AttributeAt HasAttribute AttributeValue AttributeValueNS Text
DirectText EntityCount EntityValue`; build and edit with `NewDocument
CreateElement CreateText CreateCData CreateComment CreateProcessingInstruction
Append InsertBefore RemoveChild SetAttribute RemoveAttribute SetText`; write
    with `Write WriteIndent WriteNode WriteNodeIndent WriteDocument WriteWith
Canonical CanonicalNode`. `Free(d)` releases it and `FreeList` a `NodeList`.
    **Parsing is strict**: a mismatched end tag, a second root, a duplicate
    attribute, `--` inside a comment, `]]>` in character data, an undeclared
    entity or an undeclared namespace prefix is an error with a position, not
    a guess. Entity expansion is bounded by `max_entity_depth` and a
    document-wide `max_entity_expansion`, so a recursive entity and the
    billion-laughs shape are both rejected rather than run.
  - **Searching is XPath 1.0**: `Select(d, context, expr, err): NodeList` (a
    context of `-1` means the document node), plus `SelectFirst SelectString
SelectNumber SelectBoolean Evaluate`, where an `XPathValue` has kind
    `ValueNodeSet`, `ValueString`, `ValueNumber` or `ValueBoolean`. Supported:
    location paths, `//`, `.`, `..`, `@attr`, `*`, `p:*`, the four node tests
    `node() text() comment() processing-instruction()`, all twelve axes
    (`child descendant parent ancestor self descendant-or-self
ancestor-or-self following-sibling preceding-sibling attribute following
preceding`), predicates, the union operator, the boolean, comparison and
    arithmetic operators, and the function library `last position count name
local-name namespace-uri string concat starts-with contains ends-with
substring-before substring-after substring string-length normalize-space
translate boolean not true false number sum floor ceiling round`. Variable
    references and the namespace axis are not implemented. An attribute comes
    back as a **negative handle**: test one with `IsAttributeRef`, read it
    with `ReferencedAttribute`, and `StringValue` / `NodeName` take either
    kind.
  - **Checking is DTD validation**: `ParseDTD(text): DTD` reads element and
    attribute-list declarations, and a document's own subset is in
    `d.internal_subset`. `Validate(d, dtd): Report` and
    `ValidateInternalSubset(d): Report` check the root name, content models
    (`EMPTY`, `ANY`, mixed, and nested groups like `(a,(b|c)+,d?)*`),
    undeclared elements, attribute types (`CDATA ID IDREF IDREFS NMTOKEN
NMTOKENS ENTITY ENTITIES`, enumerations and `NOTATION`), `#REQUIRED` and
    `#FIXED`, ID uniqueness and IDREF resolution. `CheckNamespaces(d)` reports
    unbound prefixes in a tree built by hand, and `ApplyDefaults(d, dtd)`
    fills in declared attribute defaults. A `Report` is `ok` plus `items` of
    `Violation { message line col node }`; `ReportText` formats it, and
    `FreeReport` / `FreeDTD` release them.
- **`yaml`**: parse/query/encode/dump. **`csv`**: `ReadLine(str): Vector<str>`,
  `WriteLine(Vector<str>): str`.
- **`encoding`**: `Base64Encode Base64Decode HexEncode HexDecode URLEncode
URLDecode`.
- **`regex`**: `Compile Match Find Replace ReplaceAll` (`Regex` handle) and
  one-shot `MatchStr FindStr ReplaceStr ReplaceAllStr`. Captures: `FindMatch`
  returns a `MatchResult` value (`.ok .start .stop .count`) to read with
  `Group GroupStart GroupEnd GroupOk GroupByName GroupIndex GroupCount`, plus
  `FindSubmatch FindAll FindAllN FindAllMatches Split SplitN` (all
  `Vector`-returning, so `defer v.free()`) and `$1`/`${name}` rewriting with
  `Expand ReplaceExpand ReplaceAllExpand`; `IsValid` checks a pattern. Syntax:
  `. [] \d \w \s \D \W \S ^ $ | (…) (?:…) (?<name>…) (?P<name>…) * + ? {n,m}`
  and the lazy `*? +? ?? {n,m}?`; no lookaround (an unsupported pattern is
  invalid, and never matches).
- **`crypto`**: `Sha1Hex Sha256Hex Sha512Hex Md5Hex`, `Sha256Bytes/Sha384Bytes/
Sha512Bytes` (+ streaming `Sha256New/Update/Final`), HMAC
  `HmacSha256Hex/HmacSha384Hex/HmacSha512Hex` and the byte-oriented
  `HmacSha256Bytes/HmacSha384Bytes/HmacSha512Bytes` (write into a caller
  buffer) / `HmacSha256Raw/HmacSha384Raw/HmacSha512Raw` (pointer+length, for
  binary keys), `HashPassword VerifyPassword Pbkdf2HmacSha256Hex`,
  `RandomHex RandomToken RandomBytes`.

### Time, memory, testing

- **`time`**: `Now NowMillis NowMicros NowNanos Sleep(ms) Format FormatISO
FormatDate FormatTime Year Month Day Hour Minute Second Weekday Since Until
ElapsedMs`; `DateTime` type.
  - **`Duration`** - a span carried as nanoseconds so it cannot be read back in
    the wrong unit; prefer it to the bare-integer functions above. Build with
    `Nanoseconds Microseconds Milliseconds Seconds Minutes Hours Days SecondsF
ZeroDuration`, read with the methods `nanos micros millis secs seconds minutes
hours days`, combine with `add sub mul div ratio neg abs truncate round`,
    compare with `cmp less equals is_zero is_negative`. `FormatDuration` writes
    Go-style text ("1h30m0s", "1.5s", "150ms") and `ParseDuration`/
    `ParseDurationOr` read it back (`DurationResult { ok, value }`). Bridges:
    `SleepFor SinceDuration DurationBetween AddDuration`.
  - **Parsing** (the inverse of every formatter): `ParseISO` (ISO 8601/RFC 3339,
    with or without an offset), `ParseISOIn ParseISOOr ParseHTTPDate` (RFC
    1123/850/asctime) and `ParseFormat(pattern, s)` (strftime-style `%Y %m %d %H
%M %S %f %b %B %a %A %z %Z %%`), all returning `TimeResult { ok, epoch, nanos }`.
    Also `FormatISOUTC` and `FormatRFC3339` (which write an offset, so they
    round-trip from any machine - `FormatISO` does not), plus `FromLocalParts`
    (libc `mktime`, DST-correct) / `FromUTCParts` (portable `timegm`) and
    `LocalOffsetMinutesAt`. An unzoned string is read as **local** time.
  - **Timers** (all on the monotonic clock): `NewTimer` → `Timer` with
    `expired remaining wait reset stop`; `NewTicker` → `Ticker` with
    `fired wait remaining reset stop advance` + a `dropped` count (a slow loop
    body drops missed ticks instead of firing them back-to-back). Callback
    helpers `AfterFunc Every RepeatEvery WaitUntil` - note a lambda captures
    enclosing locals **by value**, so callbacks share state through top-level
    `mut` globals, exactly like `spawn` workers.
- **`calendar`**: Gregorian/`Jalali` (Persian solar hijri)/`Hijri` (Islamic
  tabular lunar) dates, all pivoting through one Julian Day Number so any
  pair converts directly: `GregorianToJalali JalaliToGregorian
GregorianToHijri HijriToGregorian JalaliToHijri HijriToJalali` +
  `*ToJDN`/`JDNTo*` per calendar. Per-calendar: `IsLeap{Gregorian,Jalali,Hijri}
DaysIn{Gregorian,Jalali,Hijri}Month IsValid{Gregorian,Jalali,Hijri}Date
{Gregorian,Jalali,Hijri}MonthName{En,Fa,Ar} Weekday JalaliWeekday
WeekdayName{En,Fa,Ar} Compare{Gregorian,Jalali,Hijri}`. Reading the system
  clock (mode 1): `Today{Gregorian,Jalali,Hijri} NowClockTime`. From a value
  you already have (mode 2): `Epoch{ToGregorian,ToJalali,ToHijri,ToClockTime}`,
  `{Gregorian,Jalali,Hijri}ToUnixUTC`, `Format{Gregorian,Jalali,Hijri}`
  `FormatClockTime FormatJalaliLongFa FormatHijriLongAr/Fa
FormatGregorianLongEn`. `GregorianDate JalaliDate HijriDate ClockTime` types.
  - **Date arithmetic**, per calendar: `AddDays{...} AddWeeks{...}
AddMonths{...} AddYears{...} DaysBetween{...} StartOfMonth{...}
EndOfMonth{...}` for each of `Gregorian`/`Jalali`/`Hijri`, plus
    `DaysIn{Gregorian,Jalali}Year Weekday{Gregorian,Jalali,Hijri}
NextWeekdayOnOrAfter`. Days are exact and reversible; **months and years
    clamp** to the last valid day (2024-01-31 + 1 month = 2024-02-29), so month
    arithmetic is deliberately not reversible.
  - **Real IANA timezones** (`Zone`), read from the host's tzdata at runtime -
    use these for any zone that observes DST: `LoadZone(name) LocalZone
FreeZone` (a `Zone` owns memory: `defer calendar.FreeZone(z)`), then
    `ZoneOffsetSeconds ZoneOffsetMinutes ZoneIsDST ZoneAbbrevAt`. `TimeZoneAt(z,
epoch)` snapshots a `Zone` into the fixed `TimeZone` below, and `ZoneAt(name,
epoch)` does load-read-free in one call. `ZonedToEpoch` is the DST-aware
    local→UTC inverse. Where there is no tzdata (Windows) `LoadZone` returns
    `ok = false` and `TimeZoneAt` degrades to the fixed table.
  - Fixed-UTC-offset `TimeZone`: `TZUTC TZTehran TZKabul TZRiyadh TZDubai
TZIstanbul TZLondon TZNewYork TZTokyo FixedOffsetZone`, plus DST-aware
    `TZLocal`/`LocalOffsetMinutes` (asks the OS via `time`'s libc
    `localtime()`); `WallClock{Gregorian,Jalali,Hijri,Time}
ZonedGregorianToEpoch ConvertZoned{Gregorian,Time}` convert a date/time between
    zones. ⚠ These are **standard-time** constants: `TZLondon` is UTC+0 and
    `TZNewYork` UTC-5 all year, which is the wrong offset for the months those
    zones are on summer time - prefer `ZoneAt("Europe/London", epoch)`.
- **`mem`**: `Allocate AllocateZeroed AllocateArray Reallocate Free Copy Set
MemMove`; leak tooling `CheckLeaks LiveBytes AllocCount`.
- **`testing`**: `AssertTrue AssertFalse AssertEqInt AssertEqStr AssertEqFloat
AssertEqBool AssertContains AssertNil AssertNotNil AssertMsg Summary()` (call
  `os.Exit(testing.Summary())`).

### Networking & web

- **`http`** (client): `Get Post Put Patch Delete Head Options` (+ `*With` for
  custom `HashMap<str,str>` headers),
  `NewHeaders NewClient Ok GetHeader Headers WithQuery CookieMap`;
  response has `.status`, `.body`.
- **`web`** (server framework): `NewRouter Get/Post/Put/Delete(r, path, fn)
NewServer(port) Use Run Static`; `RunBackground(s)` returns once the port is
  bound and accepts on its own thread, so one process can run several servers,
  each with its own router; handler `func h(ctx: i64)` uses
  `Ctx_html Ctx_json Ctx_text Ctx_param Ctx_query Ctx_form Ctx_body Ctx_method
Ctx_status Ctx_set_header Ctx_redirect`. Also a canvas/DOM JS-interop surface.
- **`net.http.swagger`** (OpenAPI + a documentation page):
  `New(title, version) Describe Server Contact License BearerAuth ApiKeyAuth`;
  `Scan(doc, router)` lists every registered route (`:id` becomes a `{id}`
  path parameter, `*` becomes a catch-all), then
  `At(doc, method, pattern)` selects one and
  `Summary Details Tag OperationId Deprecated Secure Query QueryTyped`
  `QueryEnum Header Cookie PathParam` describe it.
  `Accepts(doc, desc, NewTodo {})` and `Returns(doc, 200, desc, Todo {})`
  take a **value and read its type**, so the payload schemas come from the
  struct declarations by the same compiler pass that derives
  `json.Marshal`'s encoder. Not-JSON payloads:
  `ReturnsText ReturnsHtml ReturnsFile(code, desc, "image/png") ReturnsRaw`
  `AcceptsForm AcceptsRaw`, with
  `TEXT_SCHEMA BINARY_SCHEMA JSON_TYPE TEXT_TYPE HTML_TYPE FORM_TYPE` as the
  usual arguments; `ReturnsNothing` for a 204, and
  `ReturnsHeader(code, "Location", desc)` for a header worth naming.
  Output: `Spec SpecIndent WriteSpec`. `Problems(doc)` lists what is wrong
  with the document (a security scheme nobody declared, a duplicate
  operationId, a body on a GET), `Undocumented(doc)` lists the routes nobody
  has described, and `Missing(doc, r)` lists the routes the router will serve
  that the document does not mention at all (the usual cause: `Mount` ran
  before the last `router.Get`) - print them at startup, or assert on them in
  a test; `Free(doc)` releases it. Serving: `Mount(doc, r, "/docs")` on the
  server the API already runs on, or `Serve(doc, port)` on a port of its own
  (returns once bound, runs on its own thread).
  `MountSpec`/`ServeSpec`/`SpecFromFile` render an OpenAPI document from
  anywhere; `Page(title, spec_url)` is the page itself,
  self-contained with no CDN - an empty `spec_url` means "next to this page".
- **`net.websocket`** (RFC 6455 client and server, `ws://` and `wss://`):
  `Listen Accept AcceptWSS Dial DialWith DialWSS DialWSSWith
DialWSSUnverified`, `SendText SendBinary SendBinaryBytes Ping Close Stop`,
  `Receive` (blocks) / **`Poll(c, ms)`** (returns a `TIMEOUT` message
  instead, which is what a protocol with its own heartbeat needs), plus a
  `Hub` for rooms and broadcast. A binary `Message` carries `.data`/`.len`
  next to `.text`, because a `str` stops at its first NUL.
- **`net.socketio`** (`import net.socketio`): a Socket.IO **client**, wire-
  compatible with socket.io v4 servers - Engine.IO v4 underneath (both
  transports, HTTP long-polling and WebSocket, and the upgrade between
  them), Socket.IO v5 on top.
  - **Connecting**: `Dial(url)` / `DialWith(url, opts)`, or `NewManager` +
    `Open` + `Of(m, "/nsp")` for several namespaces multiplexed on one
    connection. `Connect Disconnect Close CloseManager`. The path in the URL is
    the NAMESPACE, not a resource; the HTTP path is `Options.path`
    (default `/socket.io`) and must match the server's.
  - **Sending**: `Emit EmitStr EmitInt EmitFloat EmitBool EmitJSON
EmitBytes EmitArgs EmitVolatile Send`. An emit made while disconnected is
    held and flushed on connect (`Buffered`); a volatile one is dropped.
  - **Acknowledgements**, both directions: **`EmitAck`** blocks and returns
    the answer (the shape `await socket.emitWithAck` has in JavaScript),
    `EmitWithAck` takes a callback, and `Reply`/`ReplyStr`/`ReplyJSON`
    answer an event the server wants acknowledged (`ev.ack_id`).
  - **Receiving**, two shapes: `On Once Off OffAll OnAny PrependAny OffAny
OnAnyOutgoing PrependAnyOutgoing OffAnyOutgoing` + `Run`/`RunFor`, or
    **`Next(c, ms)`** / **`NextEvent(c, ms)`** to read in a loop - usually
    the better fit, because a Salam lambda captures by value and cannot
    write back to the function that registered it. Handler names include
    the lifecycle ones: `connect disconnect connect_error error ping
upgrade reconnect reconnect_attempt reconnect_error reconnect_failed`.
  - **Arguments**: build with `NewArgs`/`AddStr AddInt AddFloat AddBool
AddNull AddJSON AddValue AddBytes AddBinaryStr`, read with `ArgCount Arg
ArgStr ArgInt ArgFloat ArgBool ArgValue ArgIsBinary ArgBytes` (and the
    same over an acknowledgement: `AckCount AckArg AckStr AckInt AckFloat
AckBool AckValue AckIsBinary AckBytes`). Binary attachments are `Bytes`,
    not `str`, because a `str` stops at its first NUL. `EventFree`/
    `AckFree`. `ArgInt`/`ArgFloat` read a number sent as a string too, which
    is how a server keeps 64 bits intact through JavaScript.
  - **State**: `IsConnected Disconnected Active Recovered Id Sid Namespace
Transport TransportName LastError Refused RefusalReason Buffered Queued
Pending Attempts PingInterval PingTimeout MaxPayload EndpointOf Sockets
ListenerCount HasListeners`. A disconnect reason is one of socket.io's
    own strings - `REASON_SERVER_DISCONNECT REASON_CLIENT_DISCONNECT
REASON_PING_TIMEOUT REASON_TRANSPORT_CLOSE REASON_TRANSPORT_ERROR
REASON_PARSE_ERROR` - so handlers port across unchanged.
  - **`Options`**: `path query auth headers transports reconnection
reconnection_attempts reconnection_delay reconnection_delay_max
randomization_factor timeout auto_connect event_queue ack_timeout retries
trailing_slash remember_upgrade insecure`, with `SetAuth` and the
    `SetReconnection*`/`SetTimeout`/`SetRandomizationFactor` setters for a
    live connection.
  - **Nothing runs in the background**: the connection only advances inside
    `Pump`/`PumpManager`/`Next`/`Run`/`EmitAck`, and a program that stops
    calling them stops answering the server's heartbeat and gets dropped.
    Reconnection (exponential backoff with jitter), offline buffering,
    binary attachments, `retries`, and connection state recovery are all
    handled. Not implemented, deliberately: permessage-deflate compression,
    custom parsers, and client certificates.
  - The wire format is public too, for writing a server or a test against
    it: `EncodePacket DecodePacket EncodeEngine EngineKind EngineBody
SplitPayloads JoinPayloads SplitJSONArray Placeholder PlaceholderNum
ParseURL Origin`.
- **`tcp`** (`Bind Accept Read Write Close Ok ConnOk`),
  **`ssl`**, **`net`**, **`dom`**/**`console`** (browser/JS targets),
  **`webview`** (desktop windows; `-DSALAM_WEBVIEW_CEF` renders with a bundled
  Chromium instead of the OS webview - see `std/webview/native/BUILD.md`).

### JWT and API route protection

- **`jwt`**: JSON Web Tokens (RFC 7519/7515) over HMAC-SHA2, and the HTTP
  layer that uses them - one package, because the HTTP half is only useful
  with the token half and dead-code elimination means a CLI that merely signs
  a token links none of it.
  - Tokens: `NewClaims SetStr/SetInt/SetBool/SetRaw ExpiresIn NotBeforeIn
ClaimsJSON`; `Sign SignHS256/384/512 SignWithKid SignJSON SignatureOf`;
    `Verify VerifyHS256 IsValid DefaultOptions OptionsFor`; `Decode` (parses,
    verifies **nothing**); claim readers `Str Int Bool Raw Has ListHas
HasScope HasAudience SecondsRemaining`; `ErrorText IsSupportedAlg`; error
    codes `JWT_OK JWT_ERR_*`. `Options` carries the policy (`alg issuer
audience subject leeway now require_exp/nbf/iat/sub/jti`). **`alg` comes
    from `Options`, never from the token**; `"none"` and RS*/ES* are rejected
    (no asymmetric support).
  - HTTP (needs `net.http` + `net.router`): `NewGuard` bundles the secret with
    an `Options`; `Require RequireScope RequireClaim RequireSubject` verify at
    the top of a handler and, on failure, write the 401/403 themselves and
    return false. Also `Check` (verify, write nothing), `ParseBearer
BearerToken TokenFrom Unauthorized MissingCredential Forbidden`, issuing
    (`Issue IssueFor IssuePair`) and cookies (`SetSessionCookie
SetSessionCookieInsecure ClearSessionCookie`).

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
    http.Ctx_json(ctx, `{"sub":"` + t.claims.sub + `"}`)
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

- `interface Connection`: `Kind Ok Ping Exec Query Begin LastInsertId Changes
Error Quote Close`
- `interface Rows`: `Ok Next ColumnCount ColumnName ColumnType IsNull Text Int
Int64 Float Close` - forward-only, so the loop is `until r.Next():`, and
  `Close()` is required
- `interface Tx`: `Ok Exec Query Commit Rollback`
- Helpers: `QueryInt QueryInt64 QueryFloat QueryText CountRows Run RunAll InTx
QuoteLiteral`; constants `SQLITE MYSQL POSTGRES` and `TYPE_NULL TYPE_INT
TYPE_FLOAT TYPE_TEXT TYPE_BLOB`
- **Pool** (`NewPool Add Acquire Leased Conn Release Size Idle InUse DropDead
CloseAll Free`): a fixed set of connections leased one at a time. It holds
  connections you opened; it is not a factory. No locking of its own.
- **Migrations** (`NewMigrator NewMigratorIn AddMigration MigrationCount
EnsureTable SchemaVersion IsApplied PendingCount MigrateUp Rollback MigrateTo
FreeMigrator SplitStatements`): each step runs in a transaction with the row
  recording it; steps split on `;` outside quotes, since MySQL rejects
  multi-statement queries.

Each driver keeps its own full surface, and `Conn(d)` boxes one as a
`dyn db.Connection`:

- **`db.sqlite`**:
  `Available Version Open Ok Exec Query Next Text Int Finish Prepare BindText Reset LastInsertId Changes QueryInt Close Conn`.
- **`db.postgres`** (native wire protocol, no libpq; TCP only, SCRAM/MD5/password auth, TLS via `sslmode`): `Available ClientVersion ServerVersion Open OpenFull Ok Ping Reset Close Error Exec ExecCount Query Quote QueryInt QueryText DatabaseName UserName HostName Port Conn`. Placeholders are `$1, $2`, not `?`. There is **no last insert id** - use `INSERT ... RETURNING id`; `LastInsertId()` goes through `lastval()` and is 0 when the session has used no sequence. Results are materialised, so every `Rows` must be closed.
- **`db.mysql`** (MariaDB/MySQL, native protocol, no libmysqlclient; TCP only, native_password and caching_sha2 auth, TLS when the server offers it, `SALAM_MYSQL_SSL_MODE` = DISABLED/REQUIRED/VERIFY_IDENTITY): `Open Ok Close Ping Error Errno Exec Query QueryOk Next Finish Text Int Int64 Float IsNull ColumnCount RowCount ColumnName AffectedRows LastInsertId Begin Commit Rollback Autocommit Escape SetCharset SelectDB QueryInt QueryText Conn`. This API hands every value over as text, so `ColumnType` answers `TYPE_TEXT` where the others report the column's own type; DDL commits implicitly, so `Tx.Rollback()` cannot undo it. **Threading**: a connection
  cannot be used by two threads at once (it segfaults, it does not error), so
  a threaded server gives each request its own connection. Call
  `LibraryInit()` once from `main` before any thread starts, and `ThreadInit()`
  from each thread before its first query - `tests/en/apps/*api/_store.salam`
  is the worked example.
- **`db.redis`** (`connect strings hashes lists sets pubsub`).

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
   is a hard error. Prefix the name with `_` to intentionally keep it
   (`_unused`, `func _helper()`, `_result := …`). Only mark something `mut` if
   you actually reassign it (`unused_mut`).
3. **Reassignment requires `mut`.** `x := 1; x = 2` fails; use `mut x := 1`.
4. **Heap collections must be freed.** `Vector/HashMap/Set/...` allocate; call
   `.free()` (idiom: `defer v.free()` right after creation). Also free elements
   when they own memory.
5. **Integer `/` truncates** toward zero. **Bitwise operators (`& | ^ ~ << >>`)
   are supported on integers** with C precedence and compound forms
   (`&= |= ^= <<= >>=`), but they require integer operands (a bitwise op on a
   float is a compile error).
6. **No exceptions / no try-catch.** Signal failure with a `bool` return, an
   `Option<T>`, or a sentinel value, and check it at the call site.
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

When the compiler complains, fix the code; do not try to suppress the check
(except the deliberate `_` prefix for genuinely-unused names).

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
| lambda / closure             | `(x: int) => expr` or block lambda; type `func (…) R`                             |
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
- `echo` → `print`/`println`. Superglobals (`$_GET`, `$_POST`) → the `web`
  package's `Ctx_query`/`Ctx_form`/`Ctx_body`.
- Exceptions → return `bool`/`Option`; dynamic typing → pick concrete types or
  `Variant`.

### From TypeScript / JavaScript

- `class`→`struct`, `interface`→`interface`, `enum`→`enum`, generics carry over
  (`Array<T>`→`Vector<T>`, `Map`→`HashMap`, `Set`→`Set`, object literal→`struct`
  or `HashMap<str, …>`).
- Arrow functions `(x) => x*2` map almost directly: `(x: int) => x * 2` (add
  types). `Promise`/`async`/`await` have **no equivalent**, so use synchronous
  code, or `spawn`/`join` + `sync` (§8) for real parallelism.
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
    func on_ready(ctx: void*): int:     // C sees `int on_ready(void* ctx)`
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

Rest of `sync`: `CondVar` (`NewCondVar`/`WaitCond`/`WaitCondTimeout`/`Signal`/
`Broadcast` - always re-test your predicate in an `until` loop, wakeups can be
spurious), `RWMutex` (`RLock`/`RUnlock`/`WLock`/`WUnlock`, writer-preferring),
`Semaphore` (`Acquire`/`TryAcquire`/`Release`), `Once` (`Do(o, () => ... end)`),
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
nor the `__sync` family, so under `SALAM_CC_TCC` the `atomic` package falls
back to a mutex per cell - same semantics, higher cost, invisible to callers.
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

Cross-compile by passing an LLVM triple: `salam build app.salam
--target=x86_64-w64-windows-gnu --output=app.exe` (routes through LLVM;
`link dynamic "user32"` → `-luser32`).

---

## 9. Layout DSL (HTML/CSS/JS)

A `layout:` block (either its own `.salam` file built with `salam layout build`,
or embedded after a general program) declares a UI tree that compiles to HTML +
CSS + JS. Elements open with `:` … `end`; **attributes and style properties are
`name = value` pairs** (no commas needed) written inside the element.

```salam
//! mode: layout | title: Web page        // optional editor directive
layout:
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

**Elements** (see `std/layout/elements/`): `layout box header footer nav
section article heading paragraph span bold strong italic font line break list
item link head_link image media iframe canvas table row cell form label input
button script style meta global`.

**Style properties** (`std/layout/style/`): `background color
border border_color border_radius box_shadow box_sizing display position
top right bottom left width height min/max_width min/max_height margin margin_top
padding gap grid_template_columns flex_wrap align_items justify_content
aspect_ratio font_family font_size font_weight line_height text_align
text_decoration text_transform text_shadow letter/opacity overflow cursor
touch_action tap_highlight`.

**Attributes** (`std/layout/attributes/`): identity (`id`, `class`),
forms (`type`, `name`, `value`, `required`, `placeholder`), links/media
(`href`, `src`, `alt`, `target`), i18n (`lang`, `dir`), ARIA, data-attrs, and
`selector`. **Value enums** (`std/layout/values/`): named `colors`,
`units`, `directions`, `input-types`, `languages`, `targets`.

Build: `salam layout build page.salam` → `page.html` + `page.css` + `page.js`;
`--inline` → one self-contained HTML file; multiple files → per-page HTML with
merged `style.css`/`script.js`.

---

## 10. Tooling & verification

```sh
salam exec app.salam                    # run with the interpreter (pure compute, no C toolchain)
salam run app.salam                     # build + run, keep nothing
salam build app.salam --output=app      # native executable (add --keep-temp to inspect the C)
salam obj app.salam                     # object file only
salam build app.salam --target=<triple> # cross-compile via LLVM
salam layout build page.salam [--inline]# layout DSL → HTML/CSS/JS
salam format app.salam                  # reformat in place (--check to verify; --lang=fa for Persian)
salam new name                          # scaffold a project
salam memcheck app.salam                # build with AddressSanitizer and run
salam version                           # print the compiler version (NOT --version)
salam app.salam --emit-tokens | --emit-ast | --emit-symbol   # inspect a stage
```

> **Version check is `salam version`, a subcommand.** There is no `--version`
> or `-v` flag: `salam --version` fails with `unknown command '--version'`.

**Always verify a converted program.** Prefer `salam exec file.salam` for a quick
check of pure logic; use `salam build … --output=…` (or `salam run`) when it uses
FFI, threads, or the full stdlib. Resolve every diagnostic; in Salam most are hard
errors (§6).

> **`salam exec` (the interpreter) is a subset of the compiled language.** Some
> features (notably `Variant` type-pattern `match`, and parts of FFI/threads)
> only work through the C/LLVM backend. If `exec` reports a runtime error like
> "undefined variable 'i64'" on a `match` arm, that feature isn't interpreted:
> verify it with `salam run` / `salam build` instead. Treat the **compiled
> backend as the source of truth**, which is also what a self-hosting port targets.
>
> On this machine the `salam` binary at the repo root is a Linux ELF; run it via
> WSL (e.g. `wsl.exe -e ./salam exec file.salam` from the repo root, using a
> repo-relative path).

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
- **The entry function is `ریشه`**, not `main`: `روال ریشه:` ... `پایان`.
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
  (`روال جمع دو عدد(...)`, `تکه‌ها`). A space and a ZWNJ (U+200C) are the
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
| `func`              | `روال`              | `ret`                     | `برگشت`                       |
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
| `export`            | `برون‌داد`          | `layout`                  | `چیدمان`                      |
| `component`         | `بخش`               | `inline` / `noinline`     | `درخط` / `نادرخط`             |
| `pure`              | `ناب`               | `noret`                   | `نابرگشت`                     |
| `deprecated`        | `بی‌کاره`           | `and` / `or` / `not`      | `و` / `یا` / `وارونه`         |
| `eq` / `neq`        | `برابر` / `نابرابر` | `main` (entry)            | `ریشه`                        |
| `lt` / `gt`         | `کوچکتر` / `بزرگتر` | `lte` / `gte`             | `کوچکتربرابر` / `بزرگتربرابر` |

Context words (keywords only in their position, usable as names elsewhere):
`static` `ایستا` (`ایستا روال`, `پیوند ایستا`), `dynamic` `پویا`
(`پیوند پویا`), `dyn` `پویا` (before a type: `پویا شکل`), `link` `پیوند`,
`framework` `چارچوب`, `use` (struct embedding) `شامل`.
`mut func` is `ناپایا روال`.

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

The free built-in `len(x)` keeps its English name (there is no `طول(x)`
function; use `x.طول()` or `len(x)`).

**Std packages and their functions** have Persian names declared with
`@fa "..."` next to `@en "..."` in `std/`. Import by the Persian name and call
through it; never guess a name, read the `@fa` line in `std/<pkg>/`:

```salam
واردسازی رشته
واردسازی ریاضی
واردسازی سیستم عامل

روال ریشه:
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
(imported as `شبکه.اچ تی تی پی`), `web` `وب`, `db` `دیتابیس`, `dom` `دام`,
`term` `پایانه`, `cli` `خط فرمان`.

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
همگانی روال دوبار(ع: صحیح): صحیح:
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
    همگانی روال جمع(): صحیح:
        برگشت این.ایکس + این.ایگرگ
    پایان
پایان

جداشمار رنگ: قرمز، سبز، آبی پایان

روال دوبرابر(ن: صحیح): صحیح:
    برگشت ن * ۲
پایان

روال ریشه:
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
    هر (ش، م) در آ:                      // index and value
        سرچاپ ش، م
    پایان
    ن := نقطه { ایکس = ۳، ایگرگ = ۴ }
    سرچاپ ن.جمع()، دوبرابر(ن.ایکس)
    متن := همخوان رنگ.سبز:               // match: bare member names
        قرمز، آبی => "گرم یا سرد"
        سبز => "سبز"
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
هر (کلید، مقدار) در نگ:
    سرچاپ کلید، مقدار
پایان

// lambdas: no return type, captured by value
دوبرابر := (ع: صحیح) => ع * ۲
رده := (نمره: صحیح):
    برگشت نمره >= ۱۰ ? "قبول" : "مردود"
پایان
روال به‌کاربردن(ر: روال (صحیح) صحیح، مقدار: صحیح): صحیح:
    برگشت ر(مقدار)
پایان

// guards use اگر
جداشمار شکل:
    دایره(شعاع: اعشار۶۴)
    مستطیل(پهنا: اعشار۶۴، بلندی: اعشار۶۴)
پایان
روال مساحت(ش: شکل): اعشار۶۴:
    برگشت همخوان ش:
        دایره(ر) => ۳.۱۴ * ر * ر
        مستطیل(پ، ب) اگر پ == ب => پ * پ
        مستطیل(پ، ب) => پ * ب
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
    روال رتبه(): صحیح
پایان
کاربست رتبه‌دار بر رشته:
    روال رتبه(): صحیح: برگشت len(این) پایان
پایان
روال بالاتر<ت: رتبه‌دار>(الف: ت، ب: ت): صحیح:
    اگر الف.رتبه() > ب.رتبه():
        برگشت الف.رتبه()
    پایان
    برگشت ب.رتبه()
پایان
روال توصیف(ش: پویا شکل‌دار):             // پویا is dyn
    سرچاپ ش.نام()
پایان

// static members, mut methods, embedding
ساختار شمارنده:
    ن: صحیح = ۰
    همگانی پایا سقف := ۱۰۰
    همگانی ایستا روال تازه(آغاز: صحیح): شمارنده:
        برگشت شمارنده { ن = آغاز }
    پایان
    همگانی ناپایا روال بیفزای():
        این.ن += ۱
    پایان
پایان
ساختار سگ:
    همگانی شامل جانور                     // embeds جانور's fields and methods
    همگانی نژاد: رشته = ""
پایان

// Variant
روال شرح(م: گوناگون<صحیح، رشته>): رشته:
    برگشت همخوان م:
        صحیح ع => "عدد " + ع
        رشته ر => "متن " + ر
    پایان
پایان

// reference parameter, defer, pointers
روال واریز(ح &: حساب، مبلغ: صحیح):
    ح.موجودی += مبلغ
پایان
دیرکن سرچاپ "پاکسازی"
پ := پوچ برگردان صحیح*

// packages: بسته in the library file, فراخوانی in the user
بسته ابزار
همگانی روال چهاربرابر(ع: صحیح): صحیح:
    برگشت ع * ۴
پایان
// ...and in the program:
فراخوانی ابزار "ابزار.salam"
سرچاپ ابزار.چهاربرابر(۵)

// C functions and compile-time branches
درون‌داد:
    روال sqrt(x: اعشار۶۴): اعشار۶۴
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

1. **Entry is `ریشه`.** A Persian file with `روال main` has no entry point.
2. **Enum patterns in `همخوان` are bare member names** (`سبز =>`), not
   `رنگ.سبز =>`; the qualified form is a parse error in a pattern.
3. **`ترابرد` labels are blocks**: each label ends with its own `پایان`, and
   fallthrough continues into the next label unless you `بشکن`.
4. **`و` is reserved** and cannot be a name; pick `و۱`, `واحد`, ...
5. **No `٫` decimal separator**; write `۱۲.۵`.
6. **Output digits are ASCII** and booleans print as `true`/`false`.
7. **`اعشار` is f32.** Use `اعشار۶۴` unless you want single precision
   (`۰.۱ برگردان اعشار` prints `0.10000000149011612`). A float literal is
   already `اعشار۶۴`: casting a literal is allowed, but casting a variable
   to the type it already has is a useless cast (E093).
8. **No typed declarations**, as in English: `ک: صحیح = ۰` is a parse error;
   write `ک := ۰` or `ک := ۰ برگردان صحیح۶۴`.
9. **Top-level order** is the same (§6 rule 8): `بسته`, `واردسازی`,
   `فراخوانی`, `پایا`/`ناپایا` globals, then `ساختار`/`جداشمار`/`گونه`/
   `میانجی`/`کاربست`, then `روال`s, private before `همگانی`.
10. **`پایا` names are one word**: `پایا حد بالا := ۳` is a parse error; use
    `حدبالا` or `حد_بالا`.
11. **Unknown Persian std name?** Read the `@fa` line in `std/<pkg>/*.salam`.
    The English name is not a fallback in a Persian file, and an invented
    translation will not resolve.
12. **Diagnostics are Persian.** The error codes (`E001`, `E087`, ...) are the
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

روال سطح از(نمره: صحیح): سطح:
    اگر نمره >= ۱۷:
        برگشت سطح.زیاد
    وگرنه نمره >= ۱۲:
        برگشت سطح.متوسط
    پایان
    برگشت سطح.کم
پایان

روال برچسب(س: سطح): رشته:
    برگشت همخوان س:
        کم => "ضعیف"
        متوسط => "خوب"
        زیاد => "عالی"
    پایان
پایان

روال ریشه:
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
| Layout DSL `چیدمان`                              | <https://www.salamlang.ir/learn/layout/>       |
| Full Persian/English glossary                    | <https://www.salamlang.ir/learn/keywords/>     |
