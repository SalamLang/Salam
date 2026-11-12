---
title: Memory management in Salam: stack values, heap collections, defer and the ownership checker
seo_title: Memory Management in Salam: defer, Ownership Checks, memcheck
description: Memory management in Salam with no garbage collector: copies and heap collections, defer, automatic drops, the ownership checker and memcheck for leaks.
keywords: memory management, manual memory management, defer, ownership checker, use after free, memory leak, Salam memory, stack vs heap, no garbage collector, AddressSanitizer
category: compiler
summary: A practical guide to memory in Salam: what is copied, what is shared, who frees what, how defer and compiler-inserted drops help, what the ownership checker catches at compile time and how memcheck finds the rest.
date: 2026-11-11
---

**Memory management is how a program obtains memory for its data and gives it back when the data is no longer needed.** Salam has no garbage collector, so this is partly the programmer's job, but it is a much smaller job than in C. Plain values live on the stack and need no care at all. Heap collections such as `Vector` and `HashMap` are freed with `.Free()`, usually through `defer`, and the compiler inserts that call itself for local collections that never leave their scope. On top of that, an ownership checker finds use-after-free and double-free mistakes at compile time, and `salam memcheck` finds leaks at runtime. This article walks through each piece with programs and the compiler's real output.

## The model in one paragraph {#overview}

Numbers, booleans, characters, structs and fixed-size arrays are **values**: assigning one copies it, passing one to a function copies it (unless the parameter is a reference), and it vanishes when its scope ends. `Vector`, `HashMap`, `Set` and the other [collections](/std/collections/), `dyn` boxes, strings built at runtime and blocks from `mem.Allocate` live on the **heap**: the variable holds a small header that points at heap storage, and that storage stays until someone frees it. The rule for the heap is "whoever creates it frees it, or hands it to someone who will". Everything else in this article is tooling around that rule. The [memory lesson](/learn/memory/) teaches the same model step by step.

## Stack values: structs and arrays are copied {#stack-values}

Because structs are values, changing a copy never changes the original. When a function must change the caller's variable, the parameter is declared as a **reference** with `&:`. For a collection, `copy()` makes an independent `Vector` with its own storage, which then needs its own `free()`:

```salam
struct Point:
    pub x: int
    pub y: int
end

func move right(p &: Point, steps: int):
    p.x += steps
end

func main:
    a := Point { x = 1, y = 2 }
    mut b := a
    b.x = 10
    println a.x, b.x

    mut c := a
    move right(c, 5)
    println c.x

    mut cart := Vector {} as Vector<str>
    defer cart.Free()
    cart.Push("bread")
    mut basket := cart.Copy()
    defer basket.Free()
    basket.Push("milk")
    println cart.len(), basket.len()
end
```

```output
1 10
6
1 2
```

Without `copy()`, `basket := cart` would copy only the header. Both names would point at the same items, and a `push` through one of them could move the storage out from under the other. That shared state is the source of most memory bugs in languages without a collector, and it is exactly what the ownership checker watches for (more below). `deep_copy()` goes one step further and also copies containers held inside a container.

## defer: cleanup that runs on every exit {#defer}

**`defer` schedules a statement to run when the current block ends**, however it ends: falling off the end, `ret`, `break` or `continue`. Several `defer`s run in reverse order, last registered first. Go has a function-scoped `defer` and Zig a block-scoped one; Salam's is block-scoped like Zig's, so a `defer` inside a loop body runs at the end of each iteration.

```salam
import mem

func free words(words: Vector<str>):
    each w in words:
        mem.Free(w as void*)
    end
    words.Free()
end

func count words(lines: str[]): int:
    mut total := 0
    each line in lines:
        words := line.Split(" ")
        defer free words(words)
        total += words.len()
    end
    ret total
end

func main:
    defer println "main: last"
    defer println "main: second to last"
    println "words:", count words(["peace be with you", "salam"])
end
```

```output
words: 5
main: second to last
main: last
```

Writing `defer x.Free()` on the line right after `x` is created is the most important habit in Salam. Cleanup sits next to the allocation, where a reviewer can see it, and an early `ret` added later cannot skip it. The [defer lesson](/learn/defer/) has more examples.

Notice `free words`. `line.Split(" ")` returns a `Vector<str>` whose strings are themselves heap allocations, and `free()` on a `Vector` frees its own storage, not what the elements own. So the helper frees each string first, then the vector. That detail is easy to miss, which is why the next tools exist.

## Finding leaks with salam memcheck {#memcheck}

`salam memcheck app.salam` builds the program with debug information and runs it under Valgrind when it is installed, or with AddressSanitizer otherwise, and reports leaks and invalid accesses with Salam filenames and line numbers. Here is its report for an earlier version of `count words`, saved as `d1.salam`, that used a plain `defer words.Free()` and so never freed the word strings (two inner stack frames removed):

```text
==47772== 14 bytes in 3 blocks are definitely lost in loss record 4 of 4
==47772==    at 0x4944E43: calloc (vg_replace_malloc.c:1678)
==47772==    by 0x4003CF9: _Salam_mem_AllocateZeroed_u64 (mem.salam:158)
==47772==    by 0x400C38F: salam_str_split (text.salam:150)
==47772==    by 0x400367F: _Salam_main_count_20words_slice_3cstr_3e (d1.salam:4)
==47772==    by 0x4003588: main (d1.salam:14)
==47772==
salam memcheck: errors detected (exit 1). Review the output above.
```

The trace points at line 4 of that file, the `split` call, through the mangled name of `count words`. With the helper in place, the same command prints `definitely lost: 0 bytes in 0 blocks` and `salam memcheck: no memory errors detected.` Run it on anything meant to run for a long time, such as a server, a daemon or a game loop. `salam build --asan` builds the AddressSanitizer checks into an ordinary executable instead.

## Automatic drop: frees the compiler writes for you {#automatic-drop}

Since October 2026 (PR #2288), the compiler frees a local `Vector` or `HashMap` at the end of its scope when it can see that nothing else could hold on to it. The conditions, from `docs/design/OWNERSHIP.md`: the local starts as an empty literal or as the result of a function that always returns a fresh container; it is declared directly in a block; and it is never reassigned, returned, stored, captured, freed by hand or passed to something that keeps or frees it. Reading it, iterating it, pushing to it and passing it to functions that only read or grow it are all fine.

```salam
func squares(n: int): Vector<int>:
    mut out := Vector {} as Vector<int>
    repeat n in i:
        out.Push(i * i)
    end
    ret out
end

func main:
    sq := squares(5)
    println sq.len(), sq.Get(4)
end
```

```output
5 16
```

There is no `free` in this program, yet it does not leak. The compiler worked out that `squares` always returns a fresh container and that `main` never lets `sq` escape. The C it generates for `main` ends with the free it inserted:

```c
    _Salam_mem_Free_void_ptr((void*)(sq).data);
    return 0;
```

The same analysis covers strings: a chain such as `a + b + c` is built with one allocation instead of one per `+`, so no intermediate strings are left behind, and an immutable local holding a fresh string is freed at the end of its scope when it is only read. A hand-written `free()` or `defer` simply turns the automatic version off for that variable, so existing code never frees twice.

This is not a garbage collector, and not C++ or Rust-style destructors for every type: it covers the simple, very common case of a local collection or string that lives and dies in one scope. Anything stored in a struct, returned or shared is still yours to free.

## Who frees a string {#strings}

A string literal such as `"hello"` is part of the program and is never freed. A string built at runtime lives on the heap. The standard library follows one rule, and your code should too: **a function that returns a `str` returns a new string that the caller owns**, on every path. The caller frees it with `mem.Free(s as void*)` when done:

```salam
import mem
import str

func slug(title: str): str:
    lower := str.Lower(title)
    defer mem.Free(lower as void*)
    ret str.Replace(lower, " ", "-")
end

func main:
    titles := ["Hello World", "Memory In Salam"]
    each t in titles:
        s := slug(t)
        println "/blog/" + s + "/"
        mem.Free(s as void*)
    end
end
```

```output
/blog/hello-world/
/blog/memory-in-salam/
```

`slug` frees its own intermediate (`lower`) and hands its result to the caller, who frees it after use. `salam memcheck` reports no leaks for this program. For text built from many pieces in a loop, a `str.NewBuilder()` avoids creating a new string at every step.

## Pointers and raw memory {#pointers}

For C libraries and low-level code, Salam has raw pointers: `mem.Allocate(bytes)` from the [mem package](/std/mem/) returns a block, `p[i]` reads and writes the i-th element, `mem.Free` gives it back, and `sizeof(T)` gives an element's size.

```salam
import mem

func main:
    n := 5
    p := mem.Allocate(sizeof(i64) * n as u64) as i64*
    defer mem.Free(p as void*)
    repeat n in i:
        p[i] = (i * i) as i64
    end
    println p[0], p[2], p[4]
end
```

```output
0 4 16
```

Pointers are where C's classic bugs live, and the compiler checks what it can. When it knows the block's size, writing past the end is an error, and a block that nothing frees, returns, stores or passes on is a warning:

```text
error[E146]: index out of bounds: the index is 4, but 'p' has 4 elements
 --> ptr3.salam:6:5

warning[W158]: the memory 'p' gets from mem.Allocate on line 4 is never freed: nothing frees, returns, stores or passes it on before the function ends
 --> ptr2.salam:4:5
  |
4 |     p := mem.Allocate(sizeof(i64) * 4) as i64*
  |     ^
  |
  = help: free it with 'defer mem.Free(p as void*)' on the line after the allocation
```

When the size is not a constant the compiler can follow, the check stays silent and the responsibility is yours. Ordinary Salam code rarely needs pointers; reach for a `Vector` first, and see the [C interop lesson](/learn/c-interop/) for the cases that do.

## The ownership checker: compile-time use-after-free detection {#ownership-checker}

The ownership checker (`compiler/semantic/sema_own.salam`) is a flow-sensitive analysis that runs on every function that frees memory or hands it to code that does. It follows each heap buffer through assignments, copies, `defer`s, branches and loops, and it uses **function summaries** that the compiler infers for every function (does it free this parameter, keep it, grow it, return it?) instead of the lifetime annotations Rust asks you to write. Here, a `Vector` is shared by two names, one of them frees it, and the other is used afterwards:

```salam fragment
func main:
    mut a := Vector {} as Vector<int>
    a.Push(1)
    b := a
    a.Free()
    println b.Get(0)
end
```

```text
warning[W163]: 'a' is used after its buffer was moved on line 4; both names now share one buffer, so take 'a.Copy()' where both are needed
 --> o2.salam:5:5
  |
5 |     a.Free()
  |     ^
  |
  = help: use the new name from here on, or copy the Vector or HashMap before handing it over
error[E139]: 'b' is used after its storage was freed on line 5
 --> o2.salam:6:13
  |
6 |     println b.Get(0)
  |             ^
  |
  = help: a copy of a Vector, HashMap, str or pointer shares the same heap storage; free it only after the last use of every copy
```

The codes cover the main cases: **E139** use after free, **E140** double free, **E141** a returned value that the function's own `defer` frees; **W142** and **W143** when only some paths do it; **W144** a copy, `.Ref()` pointer or iterator kept across a call that may move the buffer; **W163** an owned container used after it was handed to another name. `--ownership=warn` is the default, `error` turns the warnings into errors, and `off` disables the checks along with the automatic frees. None of this costs anything at runtime; the analysis only reads the program.

It is important to be precise about what this is. It is **not a borrow checker**. Rust's borrow checker proves that safe code cannot have use-after-free or data races, and rejects programs it cannot prove. Salam's checker reports the mistakes it can see and stays quiet otherwise. Globals, indirect calls through function values and `dyn`, and the elements inside containers are outside its view, as `docs/design/OWNERSHIP.md` lists. It catches a lot, with no annotations and no new syntax, but it is a strong safety net, not a proof.

## How Salam compares {#comparison}

| Language | How heap memory is freed                                            | Compile-time safety                                            |
| -------- | ------------------------------------------------------------------- | -------------------------------------------------------------- |
| C        | `free` by hand                                                      | None built-in                                                  |
| C++      | Destructors (RAII), smart pointers                                  | Partial, mostly by convention                                  |
| Rust     | Ownership and `Drop`                                                | Borrow checker proves memory safety in safe code               |
| Zig      | Explicit allocators and `defer`                                     | Little; safety checks at runtime in debug builds               |
| Go       | Garbage collector                                                   | Memory safe by design                                          |
| Salam    | `free` with `defer`, plus compiler-inserted drops for simple locals | Ownership and range analyses report common errors; not a proof |

Salam's place is between Zig and Rust: closer to Zig's explicit `defer` style, with compile-time analyses that take some of Rust's ideas (ownership, inferred function behavior) without its annotations or its guarantees. The article on [native compilation without a garbage collector](/blog/native-compilation-without-vm-or-gc/) explains what that design buys in speed and footprint, and the [static typing article](/blog/static-typing-in-salam/) covers the range checks that work alongside the ownership checker.

## Frequently asked questions {#faq}

### Does Salam have manual memory management?

Partly. Values on the stack need nothing. Heap collections and runtime strings are freed with `.Free()` or `mem.Free`, usually through `defer`, except for simple local collections and strings that the compiler frees automatically. There is no garbage collector in compiled programs.

### What does defer do in Salam?

It runs a statement when the enclosing block ends, on every path out of it, in reverse order of registration. The usual use is `defer v.Free()` right after creating `v`.

### Can Salam detect use-after-free at compile time?

Many cases, yes. The ownership checker reports a use after free (E139) or a double free (E140) that happens on every path as an error, and the same mistakes on some paths as warnings. It does not catch everything, so `salam memcheck` remains useful.

### How do I find memory leaks in a Salam program?

Run `salam memcheck app.salam`. It uses Valgrind when it is installed and AddressSanitizer otherwise, and points at the Salam line that allocated each leaked block.
