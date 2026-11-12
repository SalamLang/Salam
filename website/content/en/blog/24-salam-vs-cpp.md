---
title: Salam vs C++: architecture, performance, tooling and systems programming
seo_title: Salam vs C++: Performance, Memory, Generics and Tooling
description: Salam vs C++ compared: variants vs enums with data, concepts vs bounded generics, RAII vs defer, measured speed, tooling, and when C++ is still the right call.
keywords: Salam vs C++, C++ alternative, systems programming language, RAII vs defer, C++ templates vs generics, C++ concepts, std::variant pattern matching, native performance, modern systems language
category: compare
summary: C++ is the default language for performance-critical software and has forty years of libraries behind it. Salam is a young systems language with a smaller surface. This comparison puts the same code side by side and is plain about where each one wins.
date: 2026-11-11
---

Salam vs C++ is a comparison between one of the largest programming languages ever built and a young one that deliberately stays small. **C++ is an ISO-standardized systems language with zero-cost abstractions, RAII, templates and decades of compilers, libraries and tools. Salam is a statically typed systems language that compiles to native code without a garbage collector, with a deliberately smaller feature set, one toolchain and source you can write in English or Persian.** Both produce fast native executables; they differ in how much language you have to learn and maintain to get there.

This article puts the same small tasks side by side, compares how the two handle memory, generics and errors, looks at measured performance, and ends with an honest list of when C++ is still the right choice. Every Salam program shown here is compiled and run when this site is built, and every C++ example compiles with GCC in C++23 mode.

## Salam and C++ at a glance {#at-a-glance}

|                 | C++                                              | Salam                                                           |
| --------------- | ------------------------------------------------ | --------------------------------------------------------------- |
| Standard        | ISO C++ (C++20, C++23, C++26 in progress)        | One open source implementation, pre-1.0                         |
| Compilers       | GCC, Clang, MSVC and others                      | The self-hosting `salam` compiler                               |
| Code generation | Direct to machine code                           | Through C, through LLVM, or its own native backend              |
| Memory          | RAII, smart pointers, manual `new`/`delete`      | Stack values, heap collections freed with `.Free()` and `defer` |
| Generics        | Templates, concepts                              | Generic functions and structs with interface bounds             |
| Polymorphism    | Inheritance, virtual functions, `std::variant`   | Interfaces with `dyn`, enums with data, embedding with `use`    |
| Errors          | Exceptions, `std::expected`                      | `throws`, `try`, `catch`, without stack unwinding               |
| Build           | CMake, Meson, Bazel; vcpkg or Conan for packages | One command: `salam build`, `salam test`, `salam add`           |
| Ecosystem       | Enormous, every domain                           | Large standard library, small community                         |

## Architecture: from source to machine code {#architecture}

A C++ program is compiled one translation unit at a time. Each `.cpp` file includes its headers, the preprocessor pastes them in, templates are instantiated where they are used, and a linker joins the object files. That model is the reason C++ builds scale across thousands of files and the reason they are slow: the same headers are parsed again and again. C++20 modules address this, and support in compilers and build tools has been arriving gradually.

Salam compiles a program together with everything it imports. The compiler, which is written in Salam itself, reads the entry file and every package it uses, checks types and many other rules, and then hands the program to one of three backends: generated C passed to a C compiler (gcc, clang or the bundled tcc), LLVM IR, or its own native backend that writes object files directly. The benchmark discussed below builds Salam through LLVM at `-O3`. The article [Three ways to generate code](/blog/salam-code-generation-c-llvm-native/) explains the trade-offs, and [How the Salam compiler works](/blog/how-the-salam-compiler-works/) follows a program through each stage.

One practical consequence: there are no headers in Salam. Files are included or packages imported, the order of functions does not matter, and a symbol is visible outside its package only when it is marked `pub`.

## Sum types and pattern matching {#sum-types}

A recurring task in systems code is a value that is one of several shapes. Modern C++ uses `std::variant` and `std::visit`, usually with the "overloaded" helper so that each case gets its own lambda:

```cpp
#include <numbers>
#include <print>
#include <variant>
#include <vector>

struct Circle { double r; };
struct Rect { double w, h; };
using Shape = std::variant<Circle, Rect>;

template <class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

double area(const Shape& s) {
    return std::visit(overloaded{
        [](const Circle& c) { return std::numbers::pi * c.r * c.r; },
        [](const Rect& r) { return r.w * r.h; },
    }, s);
}

int main() {
    std::vector<Shape> shapes{Circle{1.0}, Rect{2.0, 3.0}};
    double total = 0.0;
    for (const auto& s : shapes) total += area(s);
    std::println("{:.2f}", total);
}
```

It is type-safe and efficient, and a missing case is a compile error, but it takes a helper template, a variadic `using` declaration and some familiarity with overload resolution to read. In Salam the same structure is an [enum with data](/learn/enums/) and a `match`:

```salam
import conv
import math

enum Shape:
    Circle(r: f64)
    Rect(w: f64, h: f64)
end

func area(s: Shape): f64:
    ret match s:
        Circle(r): math.PI * r * r end
        Rect(w, h): w * h end
    end
end

func main:
    shapes := [Shape.Circle(1.0), Shape.Rect(2.0, 3.0)]
    mut total := 0.0
    each s in shapes:
        total += area(s)
    end
    println conv.FormatFloatPrec(total, 2)
end
```

```output
9.14
```

`match` must be exhaustive, it can bind fields by name, test literals and add guards (`Circle(r) if r > 10:`), and it is an expression, so it returns the area directly.

## Generics: templates and concepts vs bounded generics {#generics}

C++ templates are a compile-time language of their own. They can compute values, choose types and specialize for particular arguments, which is what makes libraries such as Eigen or the standard ranges library possible. Concepts, added in C++20, finally let a template state what it needs from its arguments. Here a concept constrains a function template, and a classic virtual base class handles a mixed collection at runtime:

```cpp
#include <concepts>
#include <memory>
#include <print>
#include <vector>

template <typename T>
concept HasArea = requires(const T& t) {
    { t.area() } -> std::convertible_to<double>;
};

struct Shape {
    virtual ~Shape() = default;
    virtual double area() const = 0;
};

struct Square : Shape {
    double side;
    explicit Square(double s) : side(s) {}
    double area() const override { return side * side; }
};

struct Rect : Shape {
    double w, h;
    Rect(double w, double h) : w(w), h(h) {}
    double area() const override { return w * h; }
};

template <HasArea T>
const T& larger(const T& a, const T& b) {
    return a.area() >= b.area() ? a : b;
}

int main() {
    std::println("{}", larger(Square{3.0}, Square{2.0}).side);

    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.PushBack(std::make_unique<Square>(2.0));
    shapes.PushBack(std::make_unique<Rect>(2.0, 3.0));
    double total = 0.0;
    for (const auto& s : shapes) total += s->area();
    std::println("{}", total);
}
```

Salam has one mechanism for both cases. An `interface` lists methods, a struct satisfies it by having them (no inheritance declaration), a bound such as `<T: HasArea>` gives a generic function that is specialized per type at compile time, and `dyn HasArea` gives dynamic dispatch when the collection is mixed:

```salam
interface HasArea:
    func area(): f64
end

struct Square:
    pub side: f64
    pub func area(): f64: ret this.side * this.side end
end

struct Rect:
    pub w: f64
    pub h: f64
    pub func area(): f64: ret this.w * this.h end
end

func larger<T: HasArea>(a: T, b: T): T:
    if a.area() >= b.area():
        ret a
    end
    ret b
end

func main:
    println larger(Square { side = 3.0 }, Square { side = 2.0 }).side

    shapes := [Square { side = 2.0 }, Rect { w = 2.0, h = 3.0 }] as dyn HasArea[2]
    mut total := 0.0
    each s in shapes:
        total += s.area()
    end
    println total
end
```

```output
3
10
```

What Salam does not have matters as much: no template metaprogramming, no specialization, no variadic templates, no inheritance hierarchies. Code reuse between structs goes through embedding (`use Base`, which promotes the embedded struct's fields and methods) and through interfaces with default methods. Salam does have operator overloading, distinct types such as `type Meters: int`, and compile-time constants and conditional compilation (see [the generics lesson](/learn/generics/) and conditional compilation). If your design depends on heavy compile-time computation in the type system, C++ can express things Salam cannot.

## Resource management: RAII vs defer {#raii-vs-defer}

This is the largest practical difference, and C++ is ahead on it. In C++, a destructor runs when an object goes out of scope, so a `std::vector`, a `std::map` or a file stream cleans up after itself, on every path, including exceptions. Counting words is short:

```cpp
#include <iostream>
#include <map>
#include <sstream>
#include <string>

int main() {
    std::istringstream in("the cat saw the dog and the dog saw the cat");
    std::map<std::string, int> counts;
    for (std::string w; in >> w;) ++counts[w];
    for (const auto& [word, n] : counts) std::cout << word << ' ' << n << '\n';
}
```

Salam has no destructors. Values on the stack (numbers, structs, fixed arrays) need nothing, but heap collections such as `Vector` and `HashMap` must be freed. The idiom is to write `defer x.Free()` right after creating one, so the cleanup sits next to the allocation and runs on every way out of the block:

```salam
import sort

func main:
    text := "the cat saw the dog and the dog saw the cat"
    words := text.split(" ")
    defer words.Free()
    mut counts := HashMap {} as HashMap<str, int>
    defer counts.Free()
    each w in words:
        counts.put(w, counts.GetOr(w, 0) + 1)
    end
    mut keys := Vector {} as Vector<str>
    defer keys.Free()
    each k, _ in counts:
        keys.push(k)
    end
    sort.Sort(keys)
    each k in keys:
        println k, counts.get(k)
    end
end
```

```output
and 1
cat 2
dog 2
saw 2
the 4
```

The Salam version is longer, partly because `HashMap` iterates in hash order and the keys have to be sorted, and partly because every collection needs its `defer`. Forgetting one leaks memory. To make that less likely, the compiler reports some use-after-free and double-free patterns before the program runs, an ownership checker warns about suspicious transfers, and `salam memcheck` runs a program under Valgrind or AddressSanitizer. None of that is a proof. C++ is not memory-safe either (dangling references, iterator invalidation and undefined behavior are all still possible), but RAII removes the most common leak by construction. The article [Memory management in Salam](/blog/memory-management-in-salam/) and the memory lesson go into detail.

On errors, C++ offers exceptions, which unwind the stack and run destructors, and since C++23 `std::expected` for error values. Salam's `throws`, `throw`, `try` and `catch` read like exceptions but are passed back as ordinary return values: nothing is unwound, `defer` statements run, and every call that can fail must be marked with `try` or sit inside a `try` block, so the possible failure points are visible in the source.

## Performance: same range, measured {#performance}

Both languages compile ahead of time to native code with no garbage collector, so you would expect them to be close, and the project's benchmark agrees. On 39 programs that use the same algorithm in every language, measured on 27 October 2026, the geometric means were 10.6 ms for Salam, 11.7 ms for C, 11.3 ms for Rust and 13.1 ms for C++ (GCC at `-O3` for C and C++, Salam through its LLVM backend at `-O3`). Against the fastest of C, C++, Rust and Go on each program, Salam was ahead on 6, level on 27 and behind on 6.

That does not mean Salam is faster than C++. Most of these programs finish in a few milliseconds, so process start-up is part of every time, and gaps under about 5% change between runs on the shared CI virtual machine that produced them. The fair reading is "the same range as C, C++ and Rust" on small compute-bound programs. Salam was behind on recursive Fibonacci, n-queens, a linear congruential generator, a tight counting loop, a binary search stress test and printing a line a million times. The benchmark does not measure large applications, where C++'s mature optimizers, profile-guided optimization and hand-tuned libraries count for a lot. The [speed page](/speed/) publishes every number and the raw data, and [Is Salam faster than C and Rust?](/blog/is-salam-faster-than-c-and-rust/) reads it carefully.

## Tooling and build tools {#tooling}

C++ tooling is deep and fragmented. There are several excellent compilers, world-class debuggers and profilers, sanitizers, static analyzers and IDEs. There is also no standard build tool or package manager: most projects use CMake, some Meson or Bazel, and dependencies come from vcpkg, Conan, the system package manager or a vendored copy. Getting a cross-platform C++ project with dependencies to build cleanly is a skill in itself.

Salam's toolchain is one executable. `salam build` compiles, `salam test` runs the tests, `salam format` formats, `salam check` type-checks without generating code, `salam debug` builds with debug information and starts GDB or LLDB, `salam memcheck` checks memory, and `salam add` fetches a package from GitHub and pins it in a lockfile. Cross-compiling is a `--target` flag. It is simpler because there is less of it: the profilers, analyzers and IDE support around C++ are far more mature. The article [The Salam developer toolchain](/blog/salam-developer-tools/) lists what exists today.

## Systems programming reach {#systems}

C++ runs almost everything performance-sensitive: browsers, game engines, databases, compilers, trading systems, embedded firmware and much of the infrastructure behind cloud services. Its libraries cover every domain, and its interop with C is direct.

Salam reaches further down than its age suggests. It can declare C functions in an `extern:` block and call them directly, export Salam functions to C with `export:`, build shared or static libraries with a generated header (`salam build --lib`), and build freestanding programs and small kernels that boot without an operating system ([freestanding Salam](/blog/freestanding-salam-bare-metal/)). Calling C++ from Salam goes through a C interface, the same way as from most languages: wrap the C++ API in `extern "C"` functions and declare those in Salam. [Calling C from Salam](/blog/salam-c-interop-ffi/) shows how.

What Salam lacks is the depth: no SIMD intrinsics library comparable to what C++ offers, no GPU programming model, far fewer platform SDKs, and very few people who have shipped large Salam systems.

## When to choose which {#choose}

**Choose C++ when:**

- you need an existing C++ library or engine (Unreal, Qt, a specific database or math library);
- the project is large and long-lived, and you need many developers who already know the language;
- you need the most mature optimizers, profilers and platform support, including consoles, GPUs and unusual embedded targets;
- your design relies on templates for compile-time computation.

**Choose Salam when:**

- you want native performance in the same range with a much smaller language to learn and review;
- one toolchain with built-in testing, formatting, packages and cross-compilation matters more than ecosystem breadth;
- the program is a service, a command-line tool or a system component that can live with the standard library and C libraries;
- you want sum types, exhaustive `match` and strict compile-time checks without template machinery, or source in Persian.

## Frequently asked questions {#faq}

### Is Salam a replacement for C++?

For some programs, yes: services, tools and components that need native speed but not the C++ library ecosystem. For large existing C++ code bases, game engines and projects built on C++ libraries, no.

### Is Salam faster than C++?

On the project's 39-program benchmark Salam's average was 10.6 ms against 13.1 ms for C++, but the programs are short and start-up time is part of every measurement. The honest summary is that both are in the same range on small compute-bound programs.

### Does Salam have RAII or destructors?

No. Heap collections are freed explicitly with `.Free()`, usually through `defer` right after they are created. The compiler and `salam memcheck` help find mistakes, but freeing memory is the programmer's job.

### Can Salam call C++ libraries?

Through a C interface. Salam calls C functions declared in an `extern:` block, so a C++ library needs `extern "C"` wrapper functions, which is the same approach most languages use.
