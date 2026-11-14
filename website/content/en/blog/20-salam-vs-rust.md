---
title: Salam vs Rust: performance, memory management, safety and low-level control
seo_title: Salam vs Rust: Memory Safety, Speed and Control Compared
description: Salam vs Rust, honestly: the borrow checker against manual memory and ownership checks, enums and errors side by side, speed, interop and when to pick each.
keywords: Salam vs Rust, Rust alternative, borrow checker, memory safety, manual memory management, Rust Result question mark, enums with data, systems programming language, no garbage collector, Rust learning curve
category: compare
summary: Rust proves memory safety at compile time and Salam does not. This comparison shows what each language checks, the same programs in both, the measured speed and where each one is the better choice.
date: 2026-11-11
---

Salam and Rust are both compiled, statically typed languages with no garbage collector, enums that carry data, exhaustive pattern matching and native performance. The decisive difference is memory safety: **Rust has a borrow checker that proves at compile time that safe code has no use after free, no double free and no data races. Salam does not.** Salam frees heap memory by hand with `.Free()` and `defer`, catches a set of common mistakes with compile-time checks and warnings, and checks bounds at runtime in development builds. Those checks help, but they are not a proof.

In exchange, Salam is a smaller language with a gentler learning curve, a large standard library in the box and source code you can write in English or Persian. This comparison shows the same programs in both languages and states the trade-offs plainly.

## Salam vs Rust at a glance {#at-a-glance}

|                         | Rust                                                                                    | Salam                                                                                                      |
| ----------------------- | --------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------- |
| Memory management       | Ownership, borrowing and lifetimes, checked by the compiler; `Drop` frees automatically | Manual: `.Free()`, usually through `defer`; compile-time warnings for common mistakes                      |
| Memory safety guarantee | Yes, for safe code; `unsafe` blocks opt out                                             | No guarantee; bounds checks in development builds, ownership checks, AddressSanitizer via `salam memcheck` |
| Data races              | Prevented at compile time by `Send` and `Sync`                                          | Your responsibility: mutexes, atomics, channels                                                            |
| Sum types and matching  | `enum` with data, exhaustive `match`                                                    | `enum` with data, exhaustive `match`                                                                       |
| Errors                  | `Result<T, E>` with `?`                                                                 | `throws`, `try`, `catch`; or your own `Result` enum with postfix `?`                                       |
| Generics                | Traits, monomorphized generics                                                          | Interfaces, generics with bounds                                                                           |
| Code generation         | LLVM (other backends in progress)                                                       | LLVM, C or Salam's own native backend                                                                      |
| Package ecosystem       | crates.io, very large                                                                   | Large standard library, few third-party packages                                                           |
| Stability               | 1.0 in 2015, editions keep old code compiling                                           | Pre-1.0, the language still changes                                                                        |
| Learning curve          | Steep, especially the borrow checker                                                    | Gentle: closer to Go or Python in feel                                                                     |

## The same program in both languages {#side-by-side}

Both languages have sum types: an enum whose members carry their own fields, taken apart with `match`. Here is a small shape calculator in Rust:

```rust
enum Shape {
    Triangle { base: f64, height: f64 },
    Rect { w: f64, h: f64 },
    Square { side: f64 },
}

impl Shape {
    fn area(&self) -> f64 {
        match self {
            Shape::Triangle { base, height } => 0.5 * base * height,
            Shape::Rect { w, h } => w * h,
            Shape::Square { side } => side * side,
        }
    }

    fn name(&self) -> &'static str {
        match self {
            Shape::Triangle { .. } => "Triangle",
            Shape::Rect { .. } => "Rect",
            Shape::Square { .. } => "Square",
        }
    }
}

fn main() {
    let shapes = vec![
        Shape::Triangle { base: 4.0, height: 3.0 },
        Shape::Rect { w: 3.0, h: 4.5 },
        Shape::Square { side: 5.0 },
    ];
    let mut total = 0.0;
    for s in &shapes {
        println!("{} area {}", s.name(), s.area());
        total += s.area();
    }
    println!("total: {}", total);
}
```

And in Salam:

```salam
enum Shape:
    Triangle(base: f64, height: f64)
    Rect(w: f64, h: f64)
    Square(side: f64)
end

func area(s: Shape): f64:
    ret match s:
        Triangle(b, h): 0.5 * b * h end
        Rect(w, h): w * h end
        Square(side): side * side end
    end
end

func main:
    mut shapes := Vector {} as Vector<Shape>
    defer shapes.Free()
    shapes.Push(Shape.Triangle(4.0, 3.0))
    shapes.Push(Shape.Rect(3.0, 4.5))
    shapes.Push(Shape.Square(5.0))
    mut total := 0.0
    each s in shapes:
        println s.name(), "area", area(s)
        total += area(s)
    end
    println "total:", total
end
```

```output
Triangle area 6
Rect area 13.5
Square area 25
total: 44.5
```

The two print the same numbers. In both languages, removing an arm from the `match` is a compile error, because the compiler checks that every member is handled (the [enums lesson](/learn/enums/) covers Salam's version). The visible differences are small: Salam enum values have a built-in `name()`, and it needs `defer shapes.Free()` where Rust's `Vec` frees itself when it goes out of scope.

## Error handling: Result and ? against throws and try {#errors}

Rust returns errors as values of type `Result<T, E>`, and `?` passes an error up to the caller:

```rust
fn parse_port(text: &str) -> Result<u16, String> {
    let n: u16 = text
        .parse()
        .map_err(|e| format!("cannot read {text:?}: {e}"))?;
    if n == 0 {
        return Err(format!("port out of range: {n}"));
    }
    Ok(n)
}

fn main() {
    for arg in ["8080", "0", "http"] {
        match parse_port(arg) {
            Ok(port) => println!("{arg} -> port {port}"),
            Err(e) => println!("{arg} -> error: {e}"),
        }
    }
}
```

Salam's usual tool is a function marked `throws`. `try` passes an error to the caller, and `try:` with `catch` handles it. Errors are values of type `Error` with a kind, a message and a code, so this is still explicit error passing, written differently:

```salam
import conv

func parse port(text: str): int throws:
    n := try conv.ParseInt(text)
    if n < 1 or n > 65535:
        throw "port out of range: " + n
    end
    ret n as int
end

func main:
    each arg in ["8080", "0", "http"]:
        try:
            println arg, "-> port", parse port(arg)
        catch e:
            println arg, "-> error:", e.message
        end
    end
end
```

```output
8080 -> port 8080
0 -> error: port out of range: 0
http -> error: invalid character 'h'
```

A call to a throwing function without `try` does not compile (error `E155`), so an error cannot be silently ignored. Rust gets a similar effect from `#[must_use]` on `Result`, as a warning rather than an error. If you prefer Rust's style, Salam lets you declare `enum Result<T, E>: Ok(value: T), Err(error: E) end` yourself, and a postfix `?` on such a value returns the error early, as in Rust. The [static typing article](/blog/static-typing-in-salam/) covers `T?` optionals, enums and both error styles.

## Memory: the borrow checker against manual freeing {#memory}

This is the section that matters most, so it is worth being precise.

In Rust, every value has one owner, references are checked so that they never outlive what they point to, and memory is freed automatically when its owner goes out of scope. Using a value after it has been moved away is rejected:

```rust
fn main() {
    let names = vec![String::from("Sara")];
    drop(names);
    println!("{}", names.len()); // error[E0382]: borrow of moved value: `names`
}
```

In Salam, freeing is an explicit call, and the compiler tracks some of what happens after it. Using a raw pointer after `mem.Free` is a hard error:

```salam fragment
import mem

func main:
    p := mem.Allocate(16 as u64) as i64*
    p[0] = 42
    mem.Free(p as void*)
    println p[0]
end
```

```text
error[E139]: 'p' is used after its storage was freed on line 6
 --> uaf.salam:7:13
  |
7 |     println p[0]
  |             ^
  |
  = help: a copy of a Vector, HashMap, str or pointer shares the same heap storage; free it only after the last use of every copy
```

The ownership checker also reports freeing twice (`E140`), returning something the function's own `defer` frees (`E141`), the same mistakes on only some paths (warnings `W142` and `W143`), and keeping a pointer or copy across a call that may grow a `Vector` or `HashMap` and move its storage (`W144`). It costs nothing at runtime, and `--ownership=error` turns its warnings into errors.

But it does not cover everything Rust's checker covers. This Salam program, the direct counterpart of the Rust example above, compiles today without a diagnostic:

```salam fragment
func main:
    mut names := Vector {} as Vector<str>
    names.Push("Sara")
    names.Free()
    println names.len()
end
```

In a development build the freed `Vector` reads as empty, so this prints `0`, and a `get` on it would stop the program with a bounds error at runtime. That is a safety net, not a compile-time guarantee. The same applies to threads: nothing in Salam's type system stops two threads from writing the same variable without a lock, which Rust rejects through `Send` and `Sync`. The [concurrency article](/blog/concurrency-in-salam/) shows the tools Salam gives you instead.

The honest summary: **Rust prevents these bugs; Salam detects many of them, catches more at runtime in development builds, and leaves the rest to you, to tests and to `salam memcheck`.** The [memory management article](/blog/memory-management-in-salam/) goes through Salam's model in full.

## Performance {#performance}

Neither language has a garbage collector or a virtual machine, and both compile through LLVM for their main builds, so you should expect them to be close. On the project's benchmark of 39 small programs with the same algorithm in every language, Salam averaged 10.6 ms and Rust 11.3 ms. That is the same range, not a ranking: most programs run for a few milliseconds, start-up is part of each time, and gaps under about 5% move between runs. Rust was clearly faster on a binary search stress test (583.9 ms against 667.7) and on printing a line a million times (2.6 ms against 8.2), and the two were often within a few percent of each other.

One real difference: Salam's development builds keep bounds checks and its `--release` builds remove them, while Rust keeps bounds checks in release builds unless the optimizer proves them unnecessary or you use `unsafe` or iterator patterns that avoid them. The [speed page](/speed/) and [the careful reading of the benchmark](/blog/is-salam-faster-than-c-and-rust/) have the full numbers and caveats.

## Low-level control and C interop {#low-level}

Both languages reach the metal. Rust has raw pointers inside `unsafe`, `#[repr(C)]`, `extern "C"` and `no_std` for kernels and firmware. Salam has pointers (`T*`, `void*`, `null`) available everywhere, `extern:` blocks to [call C](/blog/salam-c-interop-ffi/), `export:` blocks to be called from C, structs passed by value under the platform's C ABI, and freestanding targets such as `x86_64-unknown-none` with a small built-in runtime. Salam's pointers are not fenced off by an `unsafe` keyword, which makes low-level code shorter and also easier to get wrong.

Rust's embedded and kernel story is far more mature, with support in the Linux kernel and a large embedded working group. Salam's freestanding support is young; see [freestanding Salam](/blog/freestanding-salam-bare-metal/) for what exists today.

## Tooling, learning curve and ecosystem {#ecosystem}

Rust's tooling is excellent: Cargo, crates.io, rustfmt, Clippy, rust-analyzer and docs.rs, with a very large library ecosystem and a decade of stability under its edition system. Its learning curve is famously steep, mostly because of ownership and lifetimes, and its compile times are a common complaint on large projects.

Salam ships [one executable](/blog/salam-developer-tools/) that builds, tests, formats, documents, runs a language server, translates between English and Persian source and manages GitHub-hosted packages. Its standard library is unusually broad for a young language (HTTP and WebSocket servers, database drivers, TLS, compression, images, machine learning building blocks), which means many programs need no third-party code at all. The third-party ecosystem, on the other hand, is tiny, the community is small, and the language is pre-1.0, so code written today may need changes after an upgrade.

## When to choose Rust, when to choose Salam {#when}

Choose **Rust** when:

- memory safety must be proven, not tested: security-sensitive code, parsers of untrusted input, long-running infrastructure;
- you need concurrency without data races, enforced by the compiler;
- you depend on crates.io, a mature async ecosystem or embedded support;
- you need long-term stability and a large hiring pool.

Choose **Salam** when:

- you want native speed and no garbage collector with a smaller, more readable language and a short learning curve;
- the work is application code (tools, services, web servers, data processing) where a broad standard library saves time;
- your team accepts manual memory management, backed by tests, development-build checks and `salam memcheck`;
- you want to write or teach in Persian with the same compiler and libraries.

If memory safety is a hard requirement, Rust is the right answer today, and Salam does not claim otherwise.

## Frequently asked questions {#faq}

### Does Salam have a borrow checker?

No. Salam uses manual memory management with `.Free()` and `defer`. Its ownership checker reports common mistakes such as use after free and double free at compile time, but it does not prove the absence of memory errors the way Rust's borrow checker does.

### Is Salam faster than Rust?

On the project's benchmark the two are in the same range: Salam averaged 10.6 ms and Rust 11.3 ms over 39 short programs. Rust was faster on some programs and Salam on others, and the differences are small enough that the benchmark should not be read as a ranking.

### Is Salam easier to learn than Rust?

For most people, yes. Salam has no lifetimes or borrowing rules, and its syntax is closer to Python and Go. The cost of that simplicity is that the compiler catches fewer memory mistakes.

### Can Salam and Rust work together?

Through the C ABI, yes. A Rust library that exports `extern "C"` functions can be called from a Salam `extern:` block, and a Salam library built with `salam build --lib` exposes C functions that Rust can call.
