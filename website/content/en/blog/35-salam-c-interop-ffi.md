---
title: Calling C from Salam: how the FFI opens up the existing C ecosystem
seo_title: Call C From Salam: FFI, extern, Structs and Callbacks
description: How Salam calls C libraries directly: extern declarations, type mapping, structs by value, callbacks with export, linking, dlopen and building C libraries.
keywords: FFI, call C from Salam, C interop, foreign function interface, extern C, C callbacks, link C library, struct by value FFI, dlopen, shared library, export to C
category: build
summary: Salam can call any C function after a one-line declaration, pass structs by value, hand its own functions to C as callbacks, load libraries at run time and even build libraries that C programs call. A practical guide with the rules that keep it safe.
date: 2026-11-11
---

A foreign function interface (FFI) is the mechanism that lets code in one language call functions written in another. Salam's FFI targets C, the language almost every operating system and library exposes: **to call a C function from Salam you declare its signature in an `extern:` block and call it like any Salam function, with no wrapper generator, no glue code and no runtime marshalling layer.** Structs can travel by value, Salam functions can be handed to C as callbacks, and a Salam program can be built into a shared library with a C header.

This matters because no new language can rewrite the world. SQLite, zlib, OpenSSL, raylib, GTK, the Win32 API and thousands of other libraries are written in C or expose a C interface. A language that reaches them cheaply can use decades of existing work on day one. This article covers how Salam does it, with programs you can run, and the rules that keep FFI code from corrupting memory.

## Declaring and calling C functions with extern {#extern}

An `extern:` block lists C functions with their parameter and result types, and no body. The code lives in the C library; Salam only needs to know how to call it. Functions from the C standard library need nothing else.

```salam
extern:
    func printf(format: str, ...): int
    func sqrt(x: f64): f64
    func strlen(s: str): usize
    func atoi(s: str): int
end

func main:
    println "sqrt(2) =", sqrt(2.0)
    println "strlen:", strlen("salam")
    println "atoi:", atoi("1403") + 1
    printf("%-6s|%5d|%8.3f\n", "row", 42, 3.14159)
end
```

```output
sqrt(2) = 1.4142135623730951
strlen: 5
atoi: 1404
row   |   42|   3.142
```

`...` marks a variadic function such as `printf`, and the usual C format rules apply. Output from `printf` and from `println` comes out in the order the program wrote it, because Salam flushes its own output buffer before calling a C function that writes.

With the C backend an extern call becomes an ordinary C call in the generated code, and the LLVM backend (the default in release builds) and the native backend follow the platform's C calling convention directly. There is no per-call bridge of the kind Python's `ctypes` or Java's JNI put between the two sides.

The most important rule comes first: **the compiler trusts these declarations completely.** It does not read C headers. If you declare an `int` where C expects a `double`, nothing warns you and the program reads garbage. Copy every signature from the library's header or manual page.

## Mapping C types to Salam types {#types}

| C                                         | Salam                             |
| ----------------------------------------- | --------------------------------- |
| `int`, `unsigned int`                     | `int` (`i32`), `uint` (`u32`)     |
| `long long`, `int64_t`, `uint8_t`         | `i64`, `i64`, `u8`                |
| `size_t`                                  | `usize`                           |
| `ssize_t`, `intptr_t`                     | `size`                            |
| `double`, `float`                         | `f64`, `f32`                      |
| `const char*` holding text                | `str`                             |
| `char*` or `unsigned char*` holding bytes | `u8*` or `void*`                  |
| `void*`, `T*`                             | `void*`, `T*`                     |
| a function pointer                        | `void*`, or `extern func (...) R` |

A Salam `str` already ends with a zero byte, so it can be passed straight to a C function that wants a string. The reverse direction has a trap: a `str` ends at its first zero byte, so binary data from C (an image, a compressed block, a network packet) must stay a pointer and a length. Text returned by C belongs to the C library; do not free it unless the library's documentation says so. A C function that may return no string, such as `getenv`, returns a null pointer, which you check with `(value as void*) == null`.

## Buffers and memory that C writes into {#buffers}

Many C functions write into memory you give them. Allocate it with the [mem package](/std/mem/), pass the pointer, and free it when you are done; `defer` right after the allocation keeps that from being forgotten.

```salam
import mem

extern:
    func snprintf(buf: void*, size: usize, format: str, ...): int
    func getenv(name: str): str
end

func main:
    buf := mem.AllocateZeroed(32 as u64) as u8*
    defer mem.Free(buf as void*)
    n := snprintf(buf as void*, 32 as usize, "%s-%05d", "INV", 42)
    println n, buf as str

    value := getenv("SALAM_SURELY_NOT_SET")
    if (value as void*) == null:
        println "the variable is not set"
    end
end
```

```output
9 INV-00042
the variable is not set
```

Pointers are indexed like arrays: `buf[i]` reads or writes one element. Adding a number to a typed pointer is a compile error, so you index rather than increment; low-level code that really needs an address offset casts through an integer, which makes the intent visible. The lesson on [working with C](/learn/c-interop/) shows what happens to a buffer with a zero byte in the middle, which is the mistake people make most often.

## Passing structs by value {#structs}

C APIs often take or return small structs by value: a color, a rectangle, a `div_t`. Getting that right is the hard part of most FFIs, because the calling convention decides whether the struct travels in registers or in memory. Salam follows the C ABI of the target (System V and Windows on x86-64, and AArch64), so a Salam struct with the same field order and types matches the C one. The C standard library's `div` and `lldiv` return a struct, which makes a self-contained example:

```salam
extern:
    func div(numerator: int, denominator: int): DivResult
    func lldiv(numerator: i64, denominator: i64): LongDivResult
end

struct DivResult:
    pub quot: int = 0
    pub rem: int = 0
end

struct LongDivResult:
    pub quot: i64 = 0
    pub rem: i64 = 0
end

func main:
    d := div(17, 5)
    println "17 / 5 =", d.quot, "remainder", d.rem
    n := div(0 - 17, 5)
    println "-17 / 5 =", n.quot, "remainder", n.rem
    big := lldiv(1000000000007 as i64, 1000 as i64)
    println "lldiv:", big.quot, big.rem
end
```

```output
17 / 5 = 3 remainder 2
-17 / 5 = -3 remainder -2
lldiv: 1000000000 7
```

The `extern:` block comes before the struct definitions because Salam wants top-level declarations in a fixed order; `salam format` moves them for you. The same rule holds for function pointers of type `extern func (...)`, for exported functions and for callbacks, on every backend. The standard library's [gui.raylib](/std/gui/raylib/) binding relies on it: `ClearBackground(color: Color)` passes a raylib `Color` struct by value, exactly as C does.

## Callbacks: handing Salam functions to C {#callbacks}

Some C functions call you back. `qsort` sorts an array of any type and asks a function of yours to compare two elements. A Salam function that C must call goes in an `export:` block: it becomes a real C function under its plain name, with external linkage, and `&name` is its address.

```salam
import mem

extern:
    func qsort(base: void*, count: usize, width: usize, compare: void*)
end

export:
    func by_value(a: void*, b: void*): int:
        x := (a as i32*)[0]
        y := (b as i32*)[0]
        if x < y:
            ret -1
        else x > y:
            ret 1
        end
        ret 0
    end
end

func main:
    scores := [42, 7, 19, 88, 3]
    n := len(scores)
    p := mem.Allocate((n * 4) as u64) as i32*
    defer mem.Free(p as void*)
    repeat n in i:
        p[i] = scores[i]
    end
    qsort(p as void*, n as usize, 4 as usize, &by_value)
    mut line := ""
    repeat n in i:
        line += " " + p[i]
    end
    println "sorted by C:" + line
end
```

```output
sorted by C: 3 7 19 42 88
```

Exported functions follow a few rules because they are ordinary public C symbols: every one needs a body, there are no generics and no `...` parameters, `pub`, `inline` and `noinline` are not allowed, and the name must be a single word, best kept to Latin letters. The other direction works too: `(&sqrt) as extern func (f64) f64` turns the address of a C function into a typed value you can store and call, and `extern func (...) R` can be a parameter type. On 32-bit Windows, Win32 functions reached through a pointer need `extern stdcall func (...) R`; everywhere else the word changes nothing.

## Linking libraries: static, dynamic and frameworks {#linking}

For any library other than the C standard library, tell the linker which one to use with `link`, written before the extern block. The kind is always explicit:

- `link dynamic "sqlite3"` links the shared library, like `-lsqlite3`;
- `link static "z"` asks for the static library `libz.a`, copied into the program;
- `link framework "Cocoa"` links a macOS framework.

A bare `link "m"` is an error that names the three kinds. Different systems need different libraries, so `link` and `extern` blocks can sit inside a compile-time `if SALAM_OS_WINDOWS:` or `if SALAM_OS_MAC:`, which is folded away on the other platforms. This program asks the installed SQLite library for its version; it needs the SQLite development library on the machine, so it is shown without output:

```salam
link dynamic "sqlite3"

extern:
    func sqlite3_libversion(): str
    func sqlite3_libversion_number(): int
end

func main:
    println "SQLite", sqlite3_libversion(), "(" + sqlite3_libversion_number() + ")"
end
```

The standard library uses exactly this mechanism for [db.sqlite](/std/db/sqlite/), [gui.raylib](/std/gui/raylib/), [opencv](/std/opencv/) and the GTK and WebKit engine of [gui.webview](/std/gui/webview/) on Linux. A third-party package that wraps a C library declares it in `salam.toml` under `[native.<lib>]`, with the system package names for apt, dnf, pacman, brew and winget, an optional prebuilt archive per platform, and the C sources to compile as a last resort; the [packages page](/packages/) explains the workflow.

## Loading libraries at runtime and building libraries for C {#dynamic}

When the library is chosen while the program runs, for plug-ins or optional features, [os.dl](/std/os/dl/) wraps `dlopen` and `dlsym`: `dl.Open` loads a library, `dl.Sym` finds a function, and `dl.Call` calls it with up to eight integer or pointer arguments. Floating point arguments need `CallMixed`, and functions that take a struct by value or a variable argument list cannot be called this way. The package works on Linux, macOS and the BSDs; on Windows `dl.Supported()` is false.

The opposite direction is just as direct. `salam build --lib` builds a shared library (`.so`, `.dylib`, or a `.dll` with an import library) or, with `--lib=static`, a static `.a`, plus a C header listing the functions of the `export:` block:

```bash
$ salam build mathx.salam --lib
built libmathx.so and mathx.h
```

For an `export:` block holding `func mathx_gcd(a: i64, b: i64): i64`, the generated header declares `int64_t mathx_gcd(int64_t a, int64_t b);` inside an `extern "C"` guard for C++, and an ordinary C program calls it:

```c
#include <stdio.h>
#include "mathx.h"

int main(void) {
    printf("gcd(84, 36) = %lld\n", (long long)mathx_gcd(84, 36));
    return 0;
}
```

No `main` is needed in the Salam file; the Salam runtime starts when the library loads. That makes Salam usable for writing a fast extension for a C, C++, Python or PHP program, through whatever C FFI the host language has.

## How Salam's FFI compares {#compare}

| Language | How it calls C                       | What you write                                       |
| -------- | ------------------------------------ | ---------------------------------------------------- |
| Salam    | `extern:` declarations, direct calls | the signature, by hand                               |
| Zig      | `@cImport` reads C headers           | nothing for most headers                             |
| Rust     | `extern "C"` blocks, inside `unsafe` | signatures, often generated by bindgen               |
| Go       | cgo, with a C compiler in the build  | a comment with C code; calls cost more than Go calls |
| Python   | `ctypes` or `cffi` at runtime        | signatures at runtime, with marshalling per call     |

Salam sits between Rust and Zig. Like Rust, you declare the signatures; unlike Zig, the compiler cannot read a C header for you. Unlike Rust, there is no `unsafe` keyword marking the calls, so FFI code looks like any other code and the responsibility is entirely yours. Compared with Go and Python, calls are cheap: there is no cgo-style transition between runtimes and no runtime conversion layer, because there is no separate runtime to leave. With the default backend, `div(17, 5)` in the struct example above appears as `div(17, 5)` in the generated C.

## The rules that keep FFI code safe {#rules}

- **Copy every signature exactly.** The compiler cannot check it, and a wrong one fails silently.
- **Text is `str`; bytes are a pointer and a length.** A `str` stops at its first zero byte.
- **Free what you allocate with `mem`, and nothing that C owns** unless the library's documentation says to.
- **Check pointers C may return as null** before using them.
- **Mind threads.** A C library that is not thread-safe stays that way when Salam calls it; guard it with a mutex from [sync](/std/sync/) if several threads use it. The article on [concurrency in Salam](/blog/concurrency-in-salam/) covers the tools.
- **Use the compiled backends.** `salam exec`, the interpreter, does not support every FFI feature; use `salam run` or `salam build` for code that calls C.
- **Prefer the standard library when it covers the job.** It makes the C calls for you and works the same on every system. TLS, PostgreSQL and MySQL, for example, need no C library at all in Salam.

## Frequently asked questions {#faq}

### Can Salam use C header files directly?

No. Salam does not parse C headers; you declare the functions you use in an `extern:` block. For a large API that is real work, which is why the standard library ships ready-made bindings for common libraries such as SQLite and raylib.

### Is calling C from Salam slow?

No more than calling C from C. With the C backend, an extern call is a plain C call in the generated code, and the LLVM and native backends use the platform's C calling convention directly. There is no marshalling layer in between.

### Can C or Python call Salam code?

Yes. Put the functions in an `export:` block and build with `salam build --lib`. You get a shared or static library and a C header, which any language with a C FFI can use.

### Can Salam call C++ libraries?

Only through a C interface. Like most languages, Salam cannot call C++ classes and templates directly; libraries that offer an `extern "C"` API, or a thin C wrapper you write, work fine. Salam's own OpenCV binding takes the second route with a small shim library.
