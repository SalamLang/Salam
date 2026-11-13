---
title: Salam vs C: simpler code in the same native performance range?
seo_title: Salam vs C: Syntax, Memory, Safety and Speed Compared
description: Salam vs C side by side: the same programs in both, Vector and defer against malloc and free, strings, bounds checks, calling C, speed, and when to pick each.
keywords: Salam vs C, C alternative, C replacement language, manual memory management, defer vs free, bounds checking, undefined behavior, call C from Salam, native performance, systems programming language
category: compare
summary: The same small programs written in C and in Salam, followed by an honest comparison of memory management, strings, safety checks, C interop, performance, tooling and maturity.
date: 2026-11-11
image: /assets/img/blog/salam-vs-c-en.png
image_alt: Cover image for "Salam vs C: simpler code in the same native performance range?"
---

Salam and C are both compiled, statically typed languages without a garbage collector, and on the project's benchmark they finish in the same performance range. The difference is in what you write and what the compiler checks for you. C gives you a small language, a huge ecosystem and complete control, at the price of undefined behavior and manual bookkeeping. Salam keeps manual memory management but adds growable containers, `defer`, bounds checks in development builds, type inference and a compiler that rejects many mistakes C accepts.

This comparison puts the same programs side by side, then goes through memory, strings, safety, interop and performance. Salam can call C directly, so for many projects the question is not "Salam or C" but "which parts in which language".

## Salam vs C at a glance {#at-a-glance}

|                          | C                                                | Salam                                                                                     |
| ------------------------ | ------------------------------------------------ | ----------------------------------------------------------------------------------------- |
| First appeared           | 1972, standardized since 1989 (C23 is current)   | 2024 per its source headers, still pre-1.0                                                |
| Compilation              | Native, through gcc, clang, MSVC and many others | Native, through its own C, LLVM or x86-64/ARM backends                                    |
| Memory                   | `malloc`/`free`, by hand                         | Stack values plus heap containers freed with `.Free()` and `defer`                        |
| Growable arrays and maps | Write your own or pick a library                 | `Vector<T>` and `HashMap<K, V>` built-in, `Set<T>` and more in the standard library       |
| Strings                  | `char *` with a terminating zero byte            | `str`, UTF-8 text with byte and character operations                                      |
| Out-of-bounds access     | Undefined behavior                               | Compile error when provable, runtime panic in development builds                          |
| Signed overflow          | Undefined behavior                               | Wraps in development builds; with `--release` the LLVM backend assumes it does not happen |
| Generics                 | Macros and `void *`                              | Generic functions and types                                                               |
| Error handling           | Return codes and `errno`                         | `throws`, `try` and `catch`, or result enums                                              |
| Standard library         | Small; POSIX and third-party for the rest        | Large: HTTP, databases, JSON, crypto, compression and more                                |
| Ecosystem and community  | Enormous, decades deep                           | Small and young                                                                           |

## The same task in both languages {#side-by-side}

Take a line of comma-separated numbers and report how many there are, their sum, the smallest and the largest. In C, the usual tools are `strtok_r` and `strtol`. `strtok_r` writes into the string it splits, so the literal has to be copied first:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *line = "42,7,19,88,3,61";
    char *copy = strdup(line);
    if (copy == NULL) {
        return 1;
    }
    long sum = 0, lo = 0, hi = 0;
    int count = 0;
    char *save = NULL;
    for (char *tok = strtok_r(copy, ",", &save); tok != NULL;
         tok = strtok_r(NULL, ",", &save)) {
        long n = strtol(tok, NULL, 10);
        sum += n;
        if (count == 0 || n < lo) lo = n;
        if (count == 0 || n > hi) hi = n;
        count++;
    }
    free(copy);
    printf("count: %d sum: %ld min: %ld max: %ld\n", count, sum, lo, hi);
    return 0;
}
```

`strdup` is in C23 and POSIX, and `strtok_r` is POSIX, so on Windows you would reach for `strtok_s` instead. The Salam version uses `str.Split`, which returns a `Vector<str>`:

```salam
import str

func main:
    line := "42,7,19,88,3,61"
    parts := str.Split(line, ",")
    defer parts.Free()
    mut sum := 0
    mut lo := 0
    mut hi := 0
    each i, p in parts:
        n := p.to_int()
        sum += n
        if i == 0 or n < lo: lo = n end
        if i == 0 or n > hi: hi = n end
    end
    println "count:", parts.len(), "sum:", sum, "min:", lo, "max:", hi
end
```

```output
count: 6 sum: 220 min: 3 max: 88
```

Both print the same line. The Salam version is shorter mainly because the container and the string splitting already exist. Notice what stayed the same: there is still a `free`, written once as `defer parts.Free()` right after the allocation, so it runs however the function ends. Salam does not hide memory management; it makes the common pattern short.

## Structs, dynamic arrays and memory {#memory}

C has no growable array, so every C programmer writes one sooner or later. Here is a small, careful version that stores weather readings and reports the average and the hottest city:

```c
#include <stdio.h>
#include <stdlib.h>

struct reading {
    const char *city;
    double celsius;
};

struct readings {
    struct reading *items;
    size_t len;
    size_t cap;
};

static int push(struct readings *v, struct reading r)
{
    if (v->len == v->cap) {
        size_t cap = v->cap ? v->cap * 2 : 4;
        struct reading *items = realloc(v->items, cap * sizeof *items);
        if (items == NULL) {
            return -1;
        }
        v->items = items;
        v->cap = cap;
    }
    v->items[v->len++] = r;
    return 0;
}

int main(void)
{
    struct readings v = {0};
    if (push(&v, (struct reading){"Tehran", 31.5}) != 0 ||
        push(&v, (struct reading){"Tabriz", 24.0}) != 0 ||
        push(&v, (struct reading){"Shiraz", 29.5}) != 0 ||
        push(&v, (struct reading){"Rasht", 26.5}) != 0) {
        free(v.items);
        return 1;
    }
    double total = 0.0;
    struct reading hottest = v.items[0];
    for (size_t i = 0; i < v.len; i++) {
        total += v.items[i].celsius;
        if (v.items[i].celsius > hottest.celsius) {
            hottest = v.items[i];
        }
    }
    printf("readings: %zu\n", v.len);
    printf("average: %g\n", total / (double)v.len);
    printf("hottest: %s\n", hottest.city);
    free(v.items);
    return 0;
}
```

The same program in Salam, with `Vector<Reading>` doing the growing:

```salam
struct Reading:
    pub city: str
    pub celsius: f64
end

func main:
    mut readings := Vector {} as Vector<Reading>
    defer readings.Free()
    readings.Push(Reading { city = "Tehran", celsius = 31.5 })
    readings.Push(Reading { city = "Tabriz", celsius = 24.0 })
    readings.Push(Reading { city = "Shiraz", celsius = 29.5 })
    readings.Push(Reading { city = "Rasht", celsius = 26.5 })
    mut total := 0.0
    mut hottest := readings.Get(0)
    each r in readings:
        total += r.celsius
        if r.celsius > hottest.celsius:
            hottest = r
        end
    end
    println "readings:", readings.len()
    println "average:", total / (readings.len() as f64)
    println "hottest:", hottest.city
end
```

```output
readings: 4
average: 27.875
hottest: Tehran
```

The memory model underneath is close to C's. Numbers, booleans and plain structs live on the stack and are copied on assignment. A `Vector`, a `HashMap` and a string built at runtime live on the heap, and someone has to free them. A function that returns a container or a new string hands ownership to its caller. Copying a `Vector` variable copies its header, not its elements, so two variables can share storage, exactly like two C pointers to one buffer. `.Copy()` makes an independent copy.

What Salam adds is a set of compile-time checks on top of that model. Using a pointer after `mem.Free` is error `E139`, freeing twice is `E140`, returning a value that the function's own `defer` frees is `E141`, and keeping a pointer into a `Vector` across a `push` that may move its storage is warning `W144`. These are checks for common patterns, not a proof that the program is memory safe; the [memory management article](/blog/memory-management-in-salam/) and the [memory lesson](/learn/memory/) go through them. For what the checks miss, `salam memcheck` builds the program with AddressSanitizer and runs it.

## Strings: char pointers and UTF-8 text {#strings}

A C string is a pointer to bytes ending in a zero byte. Its length is found by scanning, its storage is someone's responsibility, and "a character" means a byte. A Salam `str` is UTF-8 text with methods for both views:

- `s.len()` and `str.Substr` count bytes, the same as C's `strlen`;
- `s.CharCount()`, `str.CharSubstr` and `each ch in s` work in Unicode code points, which matters for Persian, Arabic or any text outside ASCII;
- `+` concatenates and converts numbers (`"n=" + 3`), and `str.NewBuilder` builds long text efficiently.

Under the hood a Salam `str` is still zero-terminated, which is why it can be passed straight to C, and also why binary data that contains zero bytes belongs in byte buffers rather than in `str`. Every `+` creates a new heap string, so a long-running loop that concatenates should use a builder and free the result, the same discipline C asks of you with `snprintf` and buffers.

## Safety: undefined behavior and what Salam checks {#safety}

C's biggest cost is undefined behavior: reading past an array, overflowing a signed integer or using freed memory lets the compiler assume it never happens, and the result can be anything. Tools like sanitizers, static analyzers and careful code review keep it under control in good C projects, but the language itself does not help.

Salam closes several of those doors:

- **Bounds checks.** When the compiler can prove an index is out of range, the program does not compile. For a fixed array `a := [1, 2, 3]`, writing `a[3]` is `E168`; for a `Vector` whose length the compiler can track, `v.Get(v.len())` is `E146` with the message "index out of bounds: 'i' is 2, but 'v' has 2 elements". When the index is only known at runtime, a development build checks it and stops with `salam panic: Vector.get: index out of bounds` instead of reading someone else's memory.
- **Integer overflow.** In a development build, signed arithmetic wraps predictably. Range checks at compile time report an overflow, a division by zero or a null dereference that always happens as an error, and one that can happen as a warning.
- **Initialization and types.** Variables are declared with a value, immutable unless declared `mut`, and conversions between types are explicit.

The trade-off is that `--release` removes the runtime bounds checks for speed and lets the optimizer assume signed arithmetic does not overflow, which brings back C's behavior in exchange for C's speed. Many teams ship release builds and test with development builds, which is a reasonable split as long as the tests are good.

## Calling C from Salam {#interop}

Salam was designed to live next to C, not to replace it all at once. An `extern:` block declares C functions, and you call them like Salam functions:

```salam
extern:
    func strlen(s: str): usize
    func labs(n: i64): i64
    func sqrt(x: f64): f64
end

func main:
    greeting := "سلام"
    println "bytes according to C:", strlen(greeting)
    println "letters according to Salam:", greeting.CharCount()
    println "labs(-42):", labs(-42 as i64)
    println "sqrt(2):", sqrt(2.0)
end
```

```output
bytes according to C: 8
letters according to Salam: 4
labs(-42): 42
sqrt(2): 1.4142135623730951
```

The Persian word "salam" is 8 bytes and 4 letters, a small reminder of why the two string views exist. `link dynamic "name"` and `link static "name"` add libraries, structs pass by value following the platform's C ABI, and an `export:` block gives a Salam function a fixed C name so C code can call it back, for example as a `qsort` comparator. Going the other way, `salam build --lib` produces a shared or static library with a C header for the functions you export. The [C interop lesson](/learn/c-interop/) and the [FFI article](/blog/salam-c-interop-ffi/) cover this in detail.

## Performance and tooling {#performance}

On the project's benchmark of 39 programs that use the same algorithm in every language, Salam averaged 10.6 ms and C 11.7 ms, with C built by `gcc -O3` and Salam by its LLVM backend at `-O3`. That is the same range, not a win: most programs run for a few milliseconds, so process start-up is part of each number, and gaps under about 5% change from run to run. C was ahead on programs such as n-queens (35.7 ms against 41.1), recursive Fibonacci (2.4 against 3.1) and an LCG random number loop (17.9 against 19.4). The [speed page](/speed/) has every number, and [the careful reading of the benchmark](/blog/is-salam-faster-than-c-and-rust/) explains why the averages should not be read as a ranking.

On tooling, C gives you a choice of world-class compilers, debuggers, profilers and analyzers, and you assemble them with a build tool such as Make or CMake. Salam ships one executable that builds, tests (`salam test`), formats, generates documentation, runs a language server for editors, launches gdb or lldb (`salam debug`) and runs AddressSanitizer (`salam memcheck`). C's tools are deeper; Salam's are in one place.

## When to choose C, when to choose Salam {#when}

Choose **C** when:

- you target a microcontroller, an unusual CPU or a platform only a C compiler supports;
- you contribute to an existing C codebase, kernel or library;
- you need a stable ABI and a language standard that will not change under you for decades;
- certification, auditing or team experience require it.

Choose **Salam** when:

- you want native code and C-level control but would rather not write containers, string handling and error plumbing yourself (see the [standard library tour](/blog/salam-standard-library-tour/));
- the program is an application (a command-line tool, a server, a service) where a large standard library saves weeks;
- you want compile-time checks and development-build bounds checks without moving to a garbage collector;
- you want to program in Persian, or teach in it, with the same compiler and libraries ([one language, two scripts](/blog/one-language-two-scripts/)).

Keep in mind that Salam is pre-1.0. The language still changes between releases, the community is small, and there are few production deployments to learn from. A practical middle path is to keep proven C libraries in C and write the application around them in Salam.

## Frequently asked questions {#faq}

### Is Salam a replacement for C?

For applications, often yes; for every C use case, no. Salam covers the same native performance range with more help from the compiler, but C remains the language of operating systems, embedded firmware and stable library ABIs, and Salam itself relies on C runtimes on most platforms.

### Is Salam memory safe?

Not in the way Rust is. Salam has bounds checks in development builds, compile-time checks for use after free, double free and similar mistakes, and AddressSanitizer through `salam memcheck`, but you still free heap memory yourself and the compiler does not prove the program safe.

### Can Salam use existing C libraries?

Yes. Declare the functions in an `extern:` block and link the library with `link dynamic` or `link static`. Structs and callbacks follow the C ABI of the target platform.

### Does Salam compile to C?

It can. The C backend generates C and hands it to a C compiler, and `--emit-c` writes that C out so you can inspect it or compile it with your own toolchain. Salam also has an LLVM backend and its own native code generator.
