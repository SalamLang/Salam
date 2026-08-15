---
name: write-salam
description: >-
  Convert, rewrite, port, or redesign software from another language (PHP,
  TypeScript/JavaScript, Python, C, Go, Rust, Java, C#, …) into idiomatic,
  full-feature Salam: write new Salam code, and port C to Salam (including
  self-hosting the Salam compiler itself). Use whenever the request mentions
  "Salam" (the programming language / زبان سلام), a `.salam` file, "convert/port/
  rewrite X to Salam", "rewrite the compiler in Salam", the Salam layout DSL, or
  building with the `salam` compiler. This is the authoritative, up-to-date
  reference for Salam's current syntax, its 44-package standard library, its
  strict compiler rules, the layout DSL, and porting low-level C. The tutorial
  books under `books/` are OUT OF DATE. Trust this file, `std/`, and
  `tests/en/` instead.
---

# Writing Salam

Salam is a statically typed, compiled, general-purpose systems language. The
**general language transpiles to C** and builds to a native executable; embedded
**`layout:`** blocks compile to HTML/CSS/JS. It can also be run with a
tree-walking interpreter (`salam exec`, pure compute only) and cross-compiled via
LLVM. Source can be written in English, Persian, or Arabic; **this guide uses
English throughout** (every stdlib symbol also has `@fa`/`@ar` spellings defined
by `@en "Name" @fa "…" @ar "…"` annotations, though you rarely need them).

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
total: int = 250         // explicit type
x: auto = 3.14           // inferred (f64)
const MAX := 100         // compile-time constant
```

`:=` declares (immutable by default). Reassigning a non-`mut` variable is a
**compile error**. `mut` makes it reassignable; `const` is a compile-time value.

### Printing

`print`/`println` (stdout) and `printerr`/`printerrln` (stderr) take
comma-separated arguments and space-join them. They are statements, not calls:
never wrap the whole argument list in parentheses.

Any **struct, array, slice, `Vector` or `HashMap`** can be printed directly:
the compiler derives a stringify function for the type and prints what it
returns, recursing into fields and elements.

```salam
struct Point:  x: int  y: int  end

p := Point {x = 10, y = 15}
println p                                   // Point {x = 10, y = 15}
println [p, Point {x = 20, y = 25}] as Point[2]
                                            // [Point {x = 10, y = 15}, Point {x = 20, y = 25}]
println ["ali", "reza"] as str[2]           // ["ali", "reza"]
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

`+ - * / %`, `**` (power, float result), `== != < > <= >=`, `&& || !`, ternary
`cond ? a : b`, compound `+= -= *= /= %= **=`, `++`/`--`. Integer `/` **truncates**.
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

until i < n:  i = i + 1  end          // ⚠ "until" IS "while"; read the box below
repeat 3:  println "hi"  end          // do 3 times
repeat n with i:  println i  end      // i = 0 .. n-1
repeat 1 to 5:  ...  end              // 1..5 inclusive
repeat 1 to 5 with i:  ...  end       // ...binding the loop variable
repeat 10 to 1 with i:  ...  end      // descending: the *bounds* pick the direction
repeat 0 to 20 by 2:  ...  end        // step; "by" must be POSITIVE even descending
each x in xs:  println x  end         // iterate a collection/array
each (i, x) in xs:  println i, x  end // index + value (or (key,value) for a map)
// break: exit the innermost loop;  continue: next iteration
```

> ### ⚠ `until` means `while`. Never negate a ported condition.
>
> `until C:` runs its body **while `C` is true** and stops when `C` becomes
> false. It is not a do-until and not a "loop until C happens". When porting a
> loop from C/Python/JS/Go, **copy the condition verbatim**:
>
> | source loop              | Salam                                 | NOT                                                 |
> | ------------------------ | ------------------------------------- | --------------------------------------------------- |
> | `while (v != 0)`         | `until v != 0:`                       | ~~`until v == 0:`~~                                 |
> | `while (i < n)`          | `until i < n:`                        | ~~`until i >= n:`~~                                 |
> | `while (v)` (truthy int) | `until v != 0:`                       | ~~`until v:`~~ (no truthiness; needs a real `bool`) |
> | `while (p)` (pointer)    | `until p != null:`                    | ~~`until p == null:`~~                              |
> | `for (;;)`               | `until true:` + `break`               |                                                     |
> | `do { B } while (c);`    | `until true: B  if !c: break end end` |                                                     |
>
> **An inverted `until` fails silently.** `until v == 0:` with a nonzero `v`
> runs **zero times** and produces no error, so the function just returns its
> zero-value/empty result (this is exactly how a bit-length helper silently
> returns 0 and truncates a buffer). The compiler only rejects a _literally_
> constant-false condition (`until false:` → `E068`). So when a loop "did
> nothing", check its condition polarity **first**.
>
> **`repeat a to b` direction is decided at runtime by the bounds**, so
> `repeat n to 1 with i` counts _up_ `0, 1` when `n` is `0` instead of not
> running. Guard the count (`if n >= 1: repeat n to 1 with i: … end end`) when
> the start bound can fall below the end bound.

### Functions

```salam
func add(a: int, b: int): int:  ret a + b  end   // ": type" before the block colon = return type
func greet(name: str):  println "Hi,", name  end // no return type = void
func greet(name: str, prefix: str = "Hello"):     // default arguments allowed
    println prefix, name
end
```

- **Pass by value.** Overloading by parameter types is allowed. Definition order
  doesn't matter.
- **Multi-word names**: identifiers may contain spaces, e.g.
  `func make counter()`, `func is weekend(d: Day)`, `pet name: str`. A call is
  `is weekend(d)`; a field access `dog.pet name`.
- **Modifiers**: `pub inline noinline pure noret deprecated` (combine freely),
  e.g. `pub inline pure func Area(w: f64, h: f64): f64: ret w * h end`.
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
  block form `(): n = n + 1  ret n  end`. Function-typed parameters/vars:
  `func () int`, `func (int, int) bool`.
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

**Primitives:** `i8 i16 i32 i64`, `u8 u16 u32 u64`, `f32 f64`, `bool`, `char`,
`str`, `void`. Aliases: `int`=`i32`, `uint`=`u32`, `float`=`f32`. Literals:
`42` (int), `3.14` (f64), `true`/`false`, `null`.
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
input := `{"name": "salam", "version": 2, "active": true, "pi": 3.5, "tags": ["a", "b"]}`
```

not

```salam
input := "{\"name\": \"salam\", \"version\": 2, \"active\": true, \"pi\": 3.5, \"tags\": [\"a\", \"b\"]}"
```

Only fall back to `"..."` with escaped quotes when the string must also contain
a literal backtick, or when it needs an escape sequence (`\n`, `\uXXXX`, …)
that backtick strings don't process.

**Type aliases:** `type NodeId = int`, `type Bytes = u8*` gives a new name for an
existing type (declared at top level, before functions).

**Casts & typed literals with `as`:**

```salam
b := 250 as int as u8
n := fib(i) as i64
arr := [1, 2, 3] as int[3]
v := Vector {} as Vector<int>
m := HashMap {} as HashMap<str, int>
```

**Arrays (fixed size) & slices:**

```salam
a: int[3] = [1, 2, 3]                 // indexed 0..2
grid: int[2][3] = [[1,2,3],[4,5,6]]   // 2-D
mid := a[1: 3]                        // slice (view), writes through to `a`
whole := a[:]  head := a[: 2]  tail := a[1:]
func sum(view: int[:]): int: ... end  // int[:] = slice parameter
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
struct Stack<T>:  pub items: T[16] = [...]  pub count: int = 0  end  // generic struct

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

`impl` adds interface methods to **any** type, including primitives:

```salam
interface Ranked:  func rank(): int  end
impl Ranked on int:  func rank(): int:  ret this  end  end
impl Ranked on str:  func rank(): int:  ret len(this)  end  end
```

---

## 5. Standard library catalog

Import a package by name: `import str`, `import math`. Dotted subpackages:
`import db.sqlite`. Aliased / local-file imports: `import mx "math.salam"`
(then call `mx.Square(…)`). A file may declare `package <name>` (default `main`).
Only `pub` symbols are importable. **Function names are `PascalCase`; collection
methods are `snake_case`.**

**Everything below is real; signatures come from `std/`.** When unsure of
an exact signature, grep the package file.

### Text & formatting

- **`str`**: `Upper Lower Title Trim TrimLeft TrimRight TrimPrefix TrimSuffix
Reverse Repeat Concat Substr Split Join Fields Chars CharAt Contains Find
IndexOf IndexFrom LastIndex Count StartsWith EndsWith Equals EqualFold Compare
Replace Len CharCount FromInt(i64) FromFloat(f64) ToInt ToFloat IsEmpty
IsDigit IsAlpha IsSpace`; plus a `StringBuilder` (`NewBuilder`, `BufAppend`,
  `BufAppendInt`, `BufStr`, `BufFree`).
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

### I/O, OS, filesystem

- **`io`**: `ReadFile WriteFile AppendFile Lines WriteLines Input Read Write
ReadAll Readline Seek Close Copy EPrint EPrintln`.
- **`os`**: `Args Env Cwd Chdir Exit Pid Run RunCapture Output Exists IsDir
FileSize Stat ReadFile WriteFile AppendFile Copy CopyTree Move Remove RemoveAll
Mkdir MkdirAll Rmdir ListDir ListDirs Walk TempDir Open`.
- **`filepath`**: `Join Dir Base Ext Stem Clean Normalize IsAbs`.
- **`fs`**, **`flag`** (CLI: `New AddStr AddInt AddBool AddFloat Parse GetStr…
Positional Usage`), **`config`**, **`log`** (`Info Warn Error Debug` + `*f`
  variants, `SetLevel ToFile ToStderr`).

### Collections (heap-allocated: call `.free()`, idiom `defer x.free()`)

- **`Vector<T>`** (built-in; also `import collections`): `push pop get(i)
ref(i) set(i,x) len is_empty first last insert remove_at reserve clear iter free`;
  index via `v[i]` (read) / `v[i] = x` (write); free functions `contains index_of
count_of slice clone reverse swap extend`.
- **`HashMap<K,V>`**: `put(k,v) get(k) has(k) remove(k) size is_empty
iter free`; iterate with `each (k, v) in m:`.
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

- **`json`**: `Valid Get GetInt GetFloat Has Minify Indent Escape
Object(members) Array(items) Member/MemberInt/MemberBool/MemberFloat/MemberRaw
Str`.
- **`yaml`**: parse/query/encode/dump. **`csv`**: `ReadLine(str): Vector<str>`,
  `WriteLine(Vector<str>): str`.
- **`encoding`**: `Base64Encode Base64Decode HexEncode HexDecode URLEncode
URLDecode`.
- **`regex`**: `Compile Match Find Replace ReplaceAll` (`Regex` handle) and
  one-shot `MatchStr FindStr ReplaceStr ReplaceAllStr`.
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
NewServer(port) Use Run Static`; handler `func h(ctx: i64)` uses
  `Ctx_html Ctx_json Ctx_text Ctx_param Ctx_query Ctx_form Ctx_body Ctx_method
Ctx_status Ctx_set_header Ctx_redirect`. Also a canvas/DOM JS-interop surface.
- **`tcp`** (`Bind Accept Read Write Close Ok ConnOk`), **`socket`** (WebSocket),
  **`ssl`**, **`net`**, **`dom`**/**`console`** (browser/JS targets),
  **`webview`**/**`webview_cef`** (desktop windows).

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
    if !jwt.Require(ctx, guard(), t): ret end     // 401 already written
    http.Ctx_json(ctx, `{"sub":"` + t.claims.sub + `"}`)
end
```

Worked examples: `tests/en/apps/auth/` (seven programs), the five REST APIs in
`tests/en/apps/*api/`, and `tests/en/webframework/authapi.salam`. Package
tests: `tests/en/stdlib/jwt_demo.salam`.

### Databases (`import db.<engine>`)

- **`db.sqlite`**:
  `Available Version Open Ok Exec Query Next Text Int Finish Prepare BindText Reset LastInsertId Changes QueryInt Close`.
- **`db.mysql`** (MariaDB/MySQL): `Open Ok Close Ping Error Errno Exec Query QueryOk Next Finish Text Int Int64 Float IsNull ColumnCount RowCount ColumnName AffectedRows LastInsertId Begin Commit Rollback Autocommit Escape SetCharset SelectDB QueryInt QueryText`. **Threading**: a connection
  cannot be used by two threads at once (it segfaults, it does not error), so
  a threaded server gives each request its own connection. Call
  `LibraryInit()` once from `main` before any thread starts, and `ThreadInit()`
  from each thread before its first query - `tests/en/apps/*api/_store.salam`
  is the worked example.
- **`db.redis`** (`connect strings hashes lists sets pubsub`).

---

## 6. Compiler rules & top pitfalls

Salam's semantic checker is strict. These are the rules that most often turn a
naive port into compile errors (each corresponds to a case in
`tests/en/errors/`):

1. **`until <cond>` loops WHILE the condition is true.** It is Salam's `while`,
   not a do-until. `until i < n:` iterates for `i` from small to `n`; the C
   `while (v != 0)` is `until v != 0:`, **not** `until v == 0:`. This is the
   single most common porting mistake, and it is _silent_: an inverted
   condition runs the body zero times with no diagnostic (only a literal
   `until false:` is caught, as `E068`). See the box in §2.
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
   then top-level `const`/variable/`type` declarations, then functions. A
   top-level `if` must be a **compile-time constant** condition (see §8).
9. **`pure` functions are checked**: they may not write globals, call impure
   functions, mutate parameters, or `print`. Only mark a function `pure` if it is.
10. **`match` on a `Variant` must be exhaustive** and use valid type/member
    patterns. Enum `match` patterns must be real members.
11. **Types are checked strictly**: no implicit narrowing; use `as`. Ternary
    branches must share a type; a condition must be `bool`. Array-literal length
    must match the declared size.
12. **Dead code is rejected**: an always-false `if/until/repeat/each`, an
    unreachable `ret`, etc. are errors, not warnings.

When the compiler complains, fix the code; do not try to suppress the check
(except the deliberate `_` prefix for genuinely-unused names).

---

## 7. Translating from other languages

General mapping that applies to all source languages:

| Source concept               | Salam                                                                            |
| ---------------------------- | -------------------------------------------------------------------------------- |
| class                        | `struct` with `pub` fields + methods (`this` receiver)                           |
| interface / protocol / trait | `interface` + structural `pub` methods; add to existing types with `impl I on T` |
| subtype polymorphism         | `dyn Interface` (dynamic) or `<T: Interface>` (static)                           |
| generics / templates         | `<T>`, `struct Box<T>`, `func F<T>(…)`                                           |
| dict / map / object          | `HashMap<K,V>` (`put/get/has`)                                                   |
| list / array / vector        | `Vector<T>` (`push/get(i)/set/len`) or fixed `T[n]`                              |
| set                          | `Set<T>`                                                                         |
| tuple / record               | small `struct`, or `Pair`, or `Variant` for sum types                            |
| string ops                   | `str.*` package + `+` concatenation + `len()`                                    |
| exception / error            | `bool` flag, `Option<T>`, or sentinel; **no throw/catch**                        |
| null / nil / None            | `null` (pointers) or `Option.None()`                                             |
| lambda / closure             | `(x: int) => expr` or block lambda; type `func (…) R`                            |
| enum / union                 | `enum` (C-like) or `Variant<…>` (tagged union)                                   |
| module / package / import    | `package name` + `import pkg` (only `pub` exported)                              |
| free function                | top-level `func`; a bare name is its address (`i64`), `&fn` is a `void*`         |
| `while`                      | **`until`** (same meaning!)                                                      |
| `for i in range(n)`          | `repeat n with i:`                                                               |
| `for x in xs`                | `each x in xs:`                                                                  |
| destructor / cleanup         | `defer x.free()`                                                                 |

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
  or `T[:]` slices; `map`→`HashMap`.
- **Rust**: `struct`/`enum`(+data)→`struct`/`Variant`; `trait`→`interface`+
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

`link` REQUIRES an explicit kind before the library name - there is no bare
`link "X"` and no `@link(...)` attribute form, only one way to write this:
`link dynamic "X"` (`-lX`) vs `link static "X"` (`-l:libX.a` on the
LLVM-native/JIT toolchain path; falls back to dynamic on the legacy tcc
path, since tcc's own linker doesn't support that syntax) vs `link
framework "X"` (macOS only, `-framework X`). Persian/Arabic keywords:
`ایستا`/`پویا`/`چارچوب`.

**Concurrency:**

```salam
import sync
mut lock := sync.NewMutex()
mut wg := sync.NewWaitGroup()
sync.Add(wg, 1 as i64)
t := spawn(worker)                    // start a thread from a func value
join(t)                               // wait for it
sync.Lock(lock)  /* critical section */  sync.Unlock(lock)
sync.Wait(wg)  sync.Destroy(lock)  sync.DestroyWaitGroup(wg)
```

**Conditional compilation** (a top-level or inline `if` on a compile-time
constant). Predefined: `SALAM_OS_WINDOWS/MAC/LINUX/UNIX/FREEBSD/ANDROID/WASM`,
`SALAM_ARCH_X64/ARM64/X86/ARM/WASM`, string forms `SALAM_OS`/`SALAM_ARCH`; plus
your own `-DNAME` defines from the build command.

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
salam build app.salam --output=app      # native executable (add --keep-c to inspect the C)
salam obj app.salam                     # object file only
salam build app.salam --target=<triple> # cross-compile via LLVM
salam layout build page.salam [--inline]# layout DSL → HTML/CSS/JS
salam format app.salam                  # reformat in place (--check to verify; --lang=fa for Persian)
salam new name                          # scaffold a project
salam memcheck app.salam                # build with AddressSanitizer and run
salam app.salam --emit-tokens-xml | --emit-ast-xml | --emit-symbol-xml   # inspect a stage
```

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
`|` › `&&` › `||`. So `flags & MASK == MASK` parses as `flags & (MASK == MASK)`, so add
parentheses (`(flags & MASK) == MASK`) exactly as you would in C.

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

| C                                           | Salam                                                                                                |
| ------------------------------------------- | ---------------------------------------------------------------------------------------------------- |
| `typedef struct { … } T;`                   | `struct T: … end` (fields default private → add `pub`)                                               |
| `union { … }` / tagged union                | **`Variant<A, B, …>`**, narrowed by `match` on type patterns                                         |
| `enum { A, B=5 }`                           | `enum E: A, B = 5 end`                                                                               |
| `#define NAME 3` (const)                    | `const NAME := 3`                                                                                    |
| `#define MAX(a,b) …` (macro fn)             | `inline func Max(a: int, b: int): int: … end`                                                        |
| `#ifdef` / `#if` / platform `#ifdef _WIN32` | `@if`-style top-level `if SALAM_OS_WINDOWS:` on compile-time constants (§8)                          |
| `#include "x.h"` / header+source split      | `package` + `import`; export with `pub` (no headers)                                                 |
| function pointer `int (*f)(int)`            | typed value `func (int) int` (pass a lambda); or a bare `fn`/`&fn` for `i64`/`void*` callback tables |
| `void*` / `char*` / `T*`                    | `void*` / `str` or `u8*` / `T*`; deref `p[0]`; `null`                                                |
| `malloc/calloc/realloc/free`                | `mem.Allocate / AllocateZeroed / Reallocate / Free`                                                  |
| `memcpy/memset/memmove`                     | `mem.Copy / mem.Set / mem.MemMove`                                                                   |
| `strlen/strcmp/strcpy/strcat`               | `str.Len / str.Compare / str.Clone / str.Concat` (+ `StringBuilder`)                                 |
| growable array / `realloc` buffer           | `Vector<T>` (`push/get(i)/set/len`), remember `.free()`                                              |
| hash table (symbol table)                   | `HashMap<K, V>`                                                                                      |
| `switch (x) { case … }`                     | `match x: … end`                                                                                     |
| `goto`                                      | not available; restructure with functions/flags/loops                                                |
| `break` / `continue`                        | `break` / `continue` (same, innermost loop only)                                                     |
| `while (c)`                                 | **`until c:`** with the same condition, **never negated** (`while (v != 0)` → `until v != 0:`)       |
| `while (v)` / `while (p)` (truthy)          | `until v != 0:` / `until p != null:` (Salam has no truthiness)                                       |
| `do { B } while (c);`                       | `until true: B  if !c: break end end`                                                                |
| `for (;;)`                                  | `until true:` + `break`                                                                              |
| `for (i = 0; i < n; i++)`                   | `repeat n with i:` (i = 0 .. n-1)                                                                    |
| `for (i = n; i >= 1; i--)`                  | `repeat n to 1 with i:` (descending; guard `n >= 1`, see §2)                                         |
| `for (i = 0; i < n; i += 2)`                | `repeat 0 to n - 1 by 2 with i:` (`by` is always positive)                                           |
| variadic `f(int, ...)`                      | only in `extern`/FFI; pure Salam passes a `Vector`                                                   |
| `static` file-local                         | default (package-private); top level, not `pub`                                                      |
| `static`/global mutable state               | top-level `mut` globals are allowed (`mut g_count := 0`)                                             |
| `const T x`                                 | `const NAME := …` (compile-time) or an immutable `:=` binding                                        |
| `errno` / return-code error handling        | `bool` / `Option<T>` / sentinel + an error record/struct                                             |
| `assert()`                                  | `import testing` (`AssertTrue`, …) or an explicit `if … : print + os.Exit`                           |
| `int8_t … uint64_t`, `size_t`               | `i8…i64`, `u8…u64` (use `u64` for sizes/counts)                                                      |
| bitfields / flag enums                      | bitwise ops on an integer (`flags & MASK`, §12.1), or a small struct of `bool`s                      |

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
| `diag` / `logger` / `xml`             | 0.8k / 0.3k / 0.2k        | errors, logging, `--emit-*-xml`                                     |
| `i18n` / `langpack`                   | 1.6k / 0.4k               | English/Persian/Arabic keyword & symbol tables                      |
| `fmt`                                 | 1.0k                      | source formatter (`salam format`)                                   |

**Suggested port order:** `core` → `token`/`source` → `lexer` → `ast` →
`parser` → `semantic` → one backend (`codegen` C) → `driver`/`cli`, then the
remaining backends and tools. Port module-by-module, keeping the existing C build
runnable, and validate each stage against the current compiler's
`--emit-tokens-xml` / `--emit-ast-xml` / `--emit-symbol-xml` output and the
`tests/` suite. The bit-heavy code (lexer, hasher, codegen) ports directly
now that bitwise operators exist (§12.1).
