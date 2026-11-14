---
title: Static typing in Salam: catching bugs before the program runs
seo_title: Static Typing in Salam: Type Inference, Generics and Checks
description: How static typing works in Salam: type inference, strict conversions, generics, enums with data, exhaustive match, optionals and compile-time range checks.
keywords: static typing, type inference, statically typed language, type checking, generics, sum types, exhaustive pattern matching, optional type, compile-time checks, Salam types
category: compiler
summary: Salam checks every type before it generates any code, infers most of them for you, and goes further than types: exhaustive matches, optional values, checked errors and compile-time range checks, with the real compiler messages.
date: 2026-11-11
---

**Static typing means every value's type is known and checked when the program is compiled, not discovered while it runs.** Salam is a statically typed language: each variable, parameter, field and expression has one type, the compiler checks every use against it, and a program with a type error does not build. You rarely have to write the types of local variables, because the compiler infers them from their initial values, but you always write the types of function parameters and results. This article goes through what Salam checks, with the exact messages the compiler prints, and where its checks stop.

## What a statically typed language buys you {#definition}

In a dynamically typed language such as Python or JavaScript, `price * quantity` is checked when that line runs: if `quantity` happens to be a string that day, you find out in production. In a statically typed language such as Go, Rust, Java or Salam, the same mistake is a compile error, found on your machine before the program ever starts.

The trade-offs are well known. Static types catch a whole class of bugs early, make code easier to navigate (an editor can tell you exactly what `user.address` is), and let the compiler generate faster code because it does not have to check types at runtime. They cost some flexibility and, in older languages, a lot of typing. Modern statically typed languages, Salam included, recover most of that convenience with **type inference**.

Salam also uses the type checker as the foundation for more checks than types alone. Before generating any code, the compiler's semantic analysis (described in [the compiler pipeline article](/blog/how-the-salam-compiler-works/)) checks types, mutability, exhaustiveness, definite initialization, unused code, value ranges and memory ownership.

## Type inference for local variables {#type-inference}

**Type inference is the compiler working out a type from the code instead of you writing it.** In Salam, `:=` declares a variable and its type comes from the value:

```salam
func average(values: f64[]): f64:
    mut sum := 0.0
    each v in values:
        sum += v
    end
    ret sum / len(values) as f64
end

func main:
    count := 3
    ratio := 0.75
    big := 5000000000 as i64
    name := "Salam"
    ok := count > 2
    println count, ratio, big, name, ok
    println average([2.0, 4.0, 9.0])
end
```

```output
3 0.75 5000000000 Salam true
5
```

`salam inspect --emit-ast` shows what the compiler inferred: `count` is `i32`, `ratio` is `f64`, `big` is `i64`, `name` is `str`, `ok` is `bool`, and inside `average` the loop variable `v` is `f64`. A few rules are worth knowing:

- An integer literal is an `int`, which is a 32-bit `i32` in Salam (not 64-bit as in Go on most platforms). Use `as i64` when a value needs more room.
- A literal with a decimal point is an `f64`.
- `as` is the one way to state a type for a local: `big := 5000000000 as i64`, `v := Vector {} as Vector<str>`.
- Function parameters and return types are always written out. Inference is local, so a function's signature is its documentation and its contract.

Writing a type annotation on a local, as in many other languages, is not allowed:

```text
error: declarations with a type annotation were removed; use 'name := value' (or 'name := value as Type') (near '=')
```

That syntax (`x: int = 5`) existed in earlier versions and was removed during 2026, one of the changes that come with a pre-1.0 language. The [types lesson](/learn/types/) and the [variables lesson](/learn/variables/) cover the basics.

## Strict conversions: no silent narrowing {#conversions}

Salam converts a value implicitly only when nothing can be lost. An `int` used in arithmetic with an `f64` is widened, so `3.5 + 2` is `5.5`. Anything that could lose information needs an explicit `as`:

```salam fragment
func half(n: i32): i32:
    ret n / 2
end

func main:
    big := 3000000000 as i64
    println half(big)
end
```

```text
error[E012]: argument 1: cannot pass 'i64' to 'i32'
 --> n3.salam:7:13
  |
7 |     println half(big)
  |             ^
  |
```

Conditions must be real booleans. C and JavaScript let an integer stand for true or false; Salam does not:

```text
error[E021]: if condition must be bool, got 'i32'
 --> n4.salam:3:8
  |
3 |     if ok:
  |        ^
  |
```

Strictness extends to things that are not type errors in most languages. An unused variable, parameter, import or function is an error. A branch that can never run is an error. A cast that changes nothing (`[1, 2, 3] as int[3]`) is an error, E093. Reassigning a variable that was not declared `mut` is an error, E013. The [rules lesson](/learn/rules/) lists them; the idea is that code you can read is code where nothing is accidental.

For values that share a representation but not a meaning, `type Meters: int` creates a **distinct type**: it behaves like `int` in arithmetic but never mixes with a plain `int` without a cast. A plain `type Age = int`, by contrast, is only an alias. The [custom types lesson](/learn/custom-types/) shows both.

## Generics and interfaces {#generics}

**Generics let one function or type work over many types while staying fully checked.** A Salam generic function names its type parameters in angle brackets, and the compiler infers them from the arguments. A type parameter can be bounded by an interface, and interfaces are satisfied structurally: a struct with the right public methods implements the interface without declaring it.

```salam
interface Shape:
    func area(): f64
    func name(): str
end

struct Circle:
    pub r: f64
    pub func area(): f64:
        ret 3.14159 * this.r * this.r
    end
    pub func name(): str:
        ret "circle"
    end
end

struct Square:
    pub side: f64
    pub func area(): f64:
        ret this.side * this.side
    end
    pub func name(): str:
        ret "square"
    end
end

func larger<T>(a: T, b: T): T:
    if a > b:
        ret a
    end
    ret b
end

func report<T: Shape>(s: T):
    println s.name(), "has area", s.area()
end

func main:
    println larger(3, 9), larger(2.5, 1.5), larger("apple", "pear")
    report(Circle { r = 1.0 })
    report(Square { side = 2.0 })
    shapes := [Circle { r = 2.0 }, Square { side = 3.0 }] as dyn Shape[2]
    mut total := 0.0
    each s in shapes:
        total += s.area()
    end
    println "total area:", total
end
```

```output
9 2.5 pear
circle has area 3.14159
square has area 4
total area: 21.56636
```

Bounded generics such as `report<T: Shape>` are **monomorphized**: the compiler makes a separate copy for each type used, as C++ templates and Rust generics do, so there is no runtime cost. When you need a mixed collection, `dyn Shape` gives dynamic dispatch through a table of methods, as in the array above. The two arguments of `larger` must have the same type: `larger(2, "x")` fails with `cannot pass 'str' to 'i32'`, and `report(42)` fails because `'i32' does not satisfy 'Shape'` (E002). The [generics lesson](/learn/generics/) goes further.

## Enums with data and exhaustive match {#enums-and-match}

**An enum with data, also called a sum type or tagged union, is a type whose value is exactly one of several named cases, each of which can carry its own fields.** Rust, Swift, Kotlin (through sealed classes) and the ML family have them; Go and C do not. In Salam they are ordinary enums whose members take parameters, and `match` takes them apart:

```salam
enum Payment:
    Cash
    Card(last4: str)
    Transfer(bank: str, reference: int)
end

func describe(p: Payment): str:
    ret match p:
        Cash: "paid in cash" end
        Card(last4): "card ending in " + last4 end
        Transfer(bank, reference): "transfer from " + bank + ", ref " + reference end
    end
end

func main:
    payments := [Payment.Cash, Payment.Card("4242"), Payment.Transfer("Melli", 9081)]
    each p in payments:
        println describe(p)
    end
end
```

```output
paid in cash
card ending in 4242
transfer from Melli, ref 9081
```

The important part is that **`match` must be exhaustive**. Add a fourth payment method later and every `match` that forgot it stops compiling. Here is what happens when a shape enum gains an `Empty` member that one function does not handle:

```text
error[E079]: non-exhaustive match on enum 'Shape': missing member Shape.Empty (or add an 'else' arm)
 --> m1.salam:8:9
  |
8 |     ret match s:
  |         ^
  |
```

This is the single most useful check a type system can give a growing codebase: the compiler finds every place a new case needs handling. The [enums lesson](/learn/enums/) and the [match lesson](/learn/match/) show the remaining forms, including guards and struct patterns.

## Optional values with T? {#optional}

Tony Hoare called the null reference his billion-dollar mistake. Salam's answer for "this value may be absent" is the optional type `T?`, short for `Option<T>` from the `core` package. A field of type `str?` may be left out of a struct literal, plain values are wrapped automatically, and you read the value through `is_some()`, `unwrap_or()` and friends:

```salam
struct User:
    pub name: str
    pub nickname: str?
    pub age: int?
end

func greeting(u: User): str:
    if u.nickname.is_some():
        ret "Hi, " + u.nickname.value
    end
    ret "Hello, " + u.name
end

func main:
    a := User { name = "Sara Ahmadi", nickname = "Sari" }
    b := User { name = "Reza Karimi", age = 31 }
    println greeting(a)
    println greeting(b)
    println b.age.unwrap_or(0), a.age.unwrap_or(0)
end
```

```output
Hi, Sari
Hello, Reza Karimi
31 0
```

The type makes the absence visible in every signature. It is not as airtight as Rust's `Option`, though: reading `.value` without checking `is_some()` first is not a compile error, and on an empty `str?` it gives a null string. Prefer `unwrap_or` when there is a sensible default.

## Errors as values, and checked throws {#errors}

Salam gives you two ways to make failure part of a function's type.

The first is an enum: declare `Result<T, E>` with `Ok` and `Err` members, and the postfix `?` operator unwraps an `Ok` or returns the `Err` early, as in Rust:

```salam
enum Result<T, E>:
    Ok(value: T)
    Err(error: E)
end

func parse digit(s: str): Result<int, str>:
    if s.len() != 1 or not "0123456789".Includes(s):
        ret Result.Err("not a digit: " + s)
    end
    ret Result.Ok(s.to_int())
end

func add digits(a: str, b: str): Result<int, str>:
    x := parse digit(a)?
    y := parse digit(b)?
    ret Result.Ok(x + y)
end

func show(r: Result<int, str>):
    match r:
        Ok(v): println "sum:", v end
        Err(e): println "error:", e end
    end
end

func main:
    show(add digits("4", "5"))
    show(add digits("4", "x"))
end
```

```output
sum: 9
error: not a digit: x
```

The second is `throws`. A function declared `throws` can `throw` an `Error`, and the compiler requires every call to it to be marked: `try` passes the error on, `try ... catch value` supplies a fallback, and a `try:` block handles it. Nothing is unwound behind your back, and `defer` still runs. Forgetting the mark is a compile error, which is what separates this from exceptions in Java, C# or Python:

```text
error[E155]: this call can throw an error: mark it with 'try' to pass the error on, or put it inside a 'try:' block to handle it
 --> th2.salam:9:13
  |
9 |     println parse port("80")
  |             ^
  |
```

## Beyond types: range, overflow and memory checks {#value-checks}

A type tells you a value is an `int`; it does not tell you whether that `int` is a valid index. Since November 2026, Salam's compiler runs a **value-range analysis** on every function (`compiler/semantic/sema_range.salam`, design in `docs/design/RANGES.md`). It tracks intervals for locals and parameters, the lengths of containers, and whether pointers can be null, and it even walks short loops pass by pass. The off-by-one that every programmer writes eventually:

```text
error[E146]: index out of bounds: 'i' is 3, but 'xs' has 3 elements (on pass 4 of the loop on line 4)
 --> r1.salam:5:18
  |
5 |         total += xs[i]
  |                  ^
  |
  = help: valid indices of 'xs' are 0 to 2; 'repeat a to b' includes b itself, so count with 'repeat xs.len() in i:' or 'each i, x in xs:'
```

A factorial loop that outgrows a 32-bit integer, and a division by a variable that is zero:

```text
error[E148]: integer overflow: 'f *= i' gives 6227020800, which does not fit in 'i32' (on pass 13 of the loop on line 3)

error[E150]: division by zero: the divisor 'd' is 0 here
```

An error from this pass (E146, E148, E150, E152) means the line goes wrong every time it runs. A warning (W147 and up) means some value the analysis can see makes it go wrong. Values it knows nothing about, such as a parameter or a function result, are never reported, so it does not guess. `--range-checks=error` turns the warnings into errors and `off` disables the pass.

The same idea applies to memory: an **ownership analysis** reports use after free and double free at compile time, with no annotations. It is covered in [the memory management article](/blog/memory-management-in-salam/).

## Where Salam's type system stops {#limits}

It is worth being exact about what these checks do not do:

- **No borrow checker.** Rust proves at compile time that references never outlive their data. Salam's ownership analysis catches many common mistakes but proves nothing in general, and freeing heap memory is still your job.
- **Runtime checks are removed by `--release`.** A development build checks array and `Vector` indices at runtime and stops with a panic; a release build strips those checks for speed and assumes signed arithmetic does not overflow.
- **Local inference only.** Function signatures must be written out, unlike languages with whole-program inference such as Haskell or OCaml.
- **Optionals can be unwrapped unsafely**, as described above.

These are the honest limits of a young, pre-1.0 language. Within them, the compiler catches more before runtime than most mainstream languages do by default. If you want to try it, the [online editor](https://editor.salamlang.ir) compiles and runs Salam in the browser, and the [install page](/install/) has the full toolchain.

## Frequently asked questions {#faq}

### Is Salam statically or dynamically typed?

Statically typed. Every expression has a type known at compile time, and type errors stop the build. Local variables usually do not need written types, because the compiler infers them.

### Does Salam have type inference?

Yes, for local variables and for the type arguments of generic calls. `count := 3` is an `int`, `x := 0.5` is an `f64`, and `larger(3, 9)` infers `T` as `int`. Function parameters and return types are always written explicitly.

### Does Salam have null?

Pointers can be `null`, mainly for C interop. For ordinary values that may be absent, use the optional type `T?`, which makes the possible absence part of the type, so every signature shows it.

### What is the difference between a static type check and Salam's range checks?

A type check proves a value has the right kind (an integer, a string). Salam's range analysis goes further and tracks which values a variable can hold, so it can report an index that is always out of bounds, an overflow that always happens or a divisor that is always zero, before the program runs.
