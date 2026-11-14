---
title: Native compilation without a virtual machine or garbage collector: benefits, costs and performance
seo_title: Native Compilation Without a VM or Garbage Collector
description: What native AOT compilation with no garbage collector means: faster start-up, smaller memory, predictable latency, the real costs and Salam's measured speed.
keywords: native compilation, no garbage collector, AOT compilation, ahead-of-time compilation, compiled language without GC, virtual machine vs native code, garbage collection cost, manual memory management, Salam performance
category: compiler
summary: Salam programs run as plain machine code with no virtual machine and no garbage collector. What that buys you, what it costs you, how memory is handled instead, and what the benchmarks do and do not show.
date: 2026-11-11
---

**Native compilation means translating a program into the processor's own machine code before it runs, so that the operating system can execute it directly, with no virtual machine or interpreter in between.** Salam compiles this way, ahead of time, and it also runs without a garbage collector: memory that a program allocates is given back at points the program (or the compiler, on its behalf) decides, not when a collector gets around to it. This combination is what C, C++, Rust and Zig offer, and it is different from Java, C#, Python, JavaScript and Go. This article explains what you gain from it, what you give up, and how Salam's measured performance should be read.

## Native, AOT, JIT and VM: the terms {#definition}

A few terms get mixed up, so here they are side by side:

| Term                            | Meaning                                                                          | Examples                         |
| ------------------------------- | -------------------------------------------------------------------------------- | -------------------------------- |
| Interpreter                     | Reads the program (or its bytecode) and performs it step by step                 | CPython, PHP, `salam exec`       |
| Virtual machine (VM)            | A runtime that executes a portable bytecode, usually with a JIT and a GC         | JVM, .NET CLR                    |
| JIT compilation                 | Compiles bytecode to machine code while the program runs, using what it observes | HotSpot, V8, RyuJIT              |
| AOT (ahead-of-time) compilation | Compiles to machine code before the program runs                                 | C, C++, Rust, Go, Zig, Salam     |
| Garbage collector (GC)          | Finds memory the program can no longer reach and frees it automatically          | Java, C#, Go, JavaScript, Python |

The two axes are independent. Go compiles ahead of time to native code but has a garbage collector and a runtime with its own goroutine scheduler. Java and C# now have AOT options (GraalVM Native Image, .NET Native AOT) that keep the garbage collector. Swift compiles natively and uses reference counting rather than a tracing collector. Salam sits with C, C++, Rust and Zig: native code, ahead of time, no collector.

## What virtual machines and garbage collectors are good for {#vm-and-gc}

It would be wrong to present VMs and garbage collectors as mistakes. They solve real problems:

- **A garbage collector removes whole classes of bugs.** Use after free and double free cannot happen in safe Java, C# or Go code, and memory leaks are limited to data that is still reachable.
- **It makes code simpler to write.** Nobody has to decide who owns a string or when a list is freed.
- **A JIT can optimize for what actually happens.** It sees which branches are hot and which types appear at a call site, and it can specialize code for them.
- **A VM gives portability and tooling.** One bytecode file runs on any machine with the runtime, with rich profilers, debuggers and reflection.

The costs are just as real:

- **Memory headroom.** A tracing collector works best with free space to spare, so a garbage-collected program usually holds noticeably more memory than the data it is using.
- **Pauses and background work.** Modern collectors keep pauses short and do most of their work concurrently, but the work still takes CPU time and makes latency harder to predict at the tail.
- **Start-up and warm-up.** A VM has to start, load classes and, with a JIT, run slowly until hot code is compiled. That matters for command-line tools, serverless functions and anything that runs for milliseconds.
- **A runtime to ship.** The program needs the VM installed, or bundled, which makes deployments larger.

## How Salam produces a native program {#how-salam-compiles}

`salam build` checks the program and then generates machine code through one of three backends: C (handed to a C compiler), LLVM, or Salam's own native code generator with a built-in linker. The article [Three ways to generate code](/blog/salam-code-generation-c-llvm-native/) compares them. The result is an ordinary executable: ELF on Linux, PE on Windows, Mach-O on macOS.

What is inside it? The program's own code, the parts of the standard library it uses, and the C library (glibc or musl on Linux). The start-up work before `main` is small: the program records its arguments, sets up signal handling for its output buffer, and initializes the standard library modules it imports. There is no bytecode loader, no JIT, no collector thread and no task scheduler. The size shows it: a hello-world built with the native backend is 11,488 bytes on x86-64 Linux, or 24,944 bytes as a fully static executable against musl that runs on any x86-64 Linux with nothing installed.

Programs that call C pay nothing extra either. A Salam `extern` function is a direct call into the C function, with no marshaling layer between a managed heap and native memory. The [C interop lesson](/learn/c-interop/) and the article on [calling C from Salam](/blog/salam-c-interop-ffi/) show how.

## Memory without a garbage collector {#memory-without-gc}

If nothing collects garbage, something else has to free memory. In Salam it works like this:

- **Plain values live on the stack.** Numbers, booleans, structs and fixed-size arrays are values: they are copied when assigned and disappear when their scope ends. They cost no allocation and need no freeing.
- **Heap containers are freed explicitly or by the compiler.** A `Vector`, a `HashMap` or a built string lives on the heap. You free it with `.Free()`, usually written as `defer v.Free()` right after creating it, so it happens on every exit path. When a local container is never stored, returned or shared, the compiler inserts the free itself at the end of its scope.
- **The compiler checks the common mistakes.** Using memory after it was freed, or freeing it twice, is reported at compile time when it happens on every path, and as a warning when it happens on some.

Here is what that looks like in a program that reuses one buffer instead of allocating a new one for every batch of readings. `clear()` empties the `Vector` but keeps its storage, so after the first batch there are no allocations at all, and `defer` frees the storage once, when `main` ends:

```salam
func fill readings(buf &: Vector<f64>, hour: int):
    buf.Clear()
    repeat 6 in i:
        buf.Push(20.0 + ((hour * 2 + i % 3) as f64) * 0.5)
    end
end

func mean(values: Vector<f64>): f64:
    mut sum := 0.0
    each v in values:
        sum += v
    end
    ret sum / values.len() as f64
end

func main:
    mut buf := Vector {} as Vector<f64>
    defer buf.Free()
    buf.Reserve(6)
    repeat 3 in hour:
        fill readings(buf, hour)
        println "hour", hour, "mean", mean(buf), "of", buf.len(), "readings"
    end
end
```

```output
hour 0 mean 20.5 of 6 readings
hour 1 mean 21.5 of 6 readings
hour 2 mean 22.5 of 6 readings
```

Run under `salam memcheck`, this program reports `definitely lost: 0 bytes in 0 blocks`. Nothing about its memory use depends on a collector's timing: the buffer exists from the second line of `main` to its last, and that is the whole story. The full rules, including who frees a returned string, are in [the memory management article](/blog/memory-management-in-salam/).

## What you gain from compiling natively without a GC {#benefits}

**Fast start-up.** A Salam program starts like a C program: the loader maps the file and jumps to `main`. That is why native code wins clearly on short programs and command-line tools, where a VM's start-up would dominate.

**A small, predictable memory footprint.** The program holds what it allocated and nothing else. There is no collector headroom to tune and no heap-size flag to get right.

**Predictable latency.** No collector ever stops or slows the program. When a request handler frees its buffers, that happens at a known point in the code, and the time it takes shows up in a profiler where you can see it.

**Simple deployment.** One executable, optionally fully static, copied to a server or a container. The release builds of Salam can also cross-compile: a Windows executable can be built on Linux with the MinGW sysroot embedded in the compiler, and the native backend can write macOS executables as well. The article on [cross-compiling Salam](/blog/cross-compilation-salam/) explains the targets.

**Reach.** Without a runtime to carry, the same compiler can target WebAssembly and even freestanding bare-metal code, where no operating system exists to host a VM. The article on [freestanding Salam](/blog/freestanding-salam-bare-metal/) goes there.

## What it costs {#costs}

The honest side of the ledger:

- **You are responsible for freeing heap memory.** Forget a `free` on a value the compiler cannot prove is local, and the program leaks. A server that leaks a little per request will eventually run out of memory.
- **Mistakes are not all caught.** Salam's compile-time ownership checks catch common use-after-free and double-free bugs without annotations, but they are not a proof. Rust's borrow checker proves memory safety for safe Rust; Salam has no borrow checker, and a bug the analysis does not see can still corrupt memory at runtime. A development build checks indices at runtime, and `--release` removes those checks for speed.
- **Tools replace the safety net.** `salam memcheck` runs a program under Valgrind or AddressSanitizer to find leaks and bad accesses. It is a habit to build, not something you get for free.
- **No runtime specialization.** An AOT compiler optimizes for every possible input. A JIT can, in principle, specialize for the inputs it actually sees, and on some long-running workloads that is an advantage for the JVM or V8.
- **One build per target.** A native executable runs on one operating system and processor family. Cross-compilation makes that cheap, but there is no single portable artifact as with a JAR.

## Performance, measured carefully {#performance}

The project's own benchmark compares 39 programs that use the same algorithm in C, C++, Rust, Go, Python, PHP and Salam. Measured on 27 October 2026, with Salam built by its LLVM backend at `-O3`, the averages were:

| Language | Average time                 |
| -------- | ---------------------------- |
| Salam    | 10.6 ms                      |
| Rust     | 11.3 ms                      |
| C        | 11.7 ms                      |
| C++      | 13.1 ms                      |
| Go       | 14.2 ms                      |
| PHP      | about 9.6 times Salam's time |
| Python   | about 56 times Salam's time  |

Against the fastest of C, C++, Rust and Go on each program, Salam was ahead on 6, level on 27 and behind on 6. It was behind on recursive Fibonacci (3.1 ms against 2.4 ms for C), n-queens, binary search, LCG random numbers, a counting loop and printing a line a million times (8.2 ms against 2.6 ms for Rust).

Read these numbers as **the same range as C and Rust**, not as a ranking. Most of the programs finish in 2 to 30 ms, so process start-up is part of every time, and a plain build with tcc and no optimizer averaged 11.0 ms on the same programs. Gaps under about 5% change from run to run on the shared GitHub Actions virtual machine where the suite ran. The programs are small and do not measure large applications or memory-heavy workloads. The comparison with Go and the interpreted languages is where the absence of a VM and a collector shows most clearly.

The [speed page](/speed/) also has an HTTP benchmark: Salam's `net.http` server against Go's standard library, Fiber, Gin, Echo, nginx, Apache, Node with Express and PHP, on one shared machine. Salam answered more requests per second than Fiber on all 11 routes, between 16% and 52% more on the geometric mean depending on machine load. One machine and one harness, so rerun it on your own hardware before planning capacity with it. The article [Is Salam faster than C and Rust?](/blog/is-salam-faster-than-c-and-rust/) reads both benchmarks in detail.

## Where Salam does use a runtime with a garbage collector {#exceptions}

Two Salam tools are exceptions, by design:

- **The interpreter.** `salam exec` runs programs without a C toolchain by compiling each function to bytecode for a small virtual machine inside the compiler. Its values live in the interpreter's own heap, which a mark-and-sweep collector cleans up. Code written for the compiled language, frees included, runs there unchanged. The article on [salam run, exec and build](/blog/salam-run-vs-salam-build/) covers it.
- **JavaScript output.** `salam js` and the page language compile to JavaScript, which runs under the browser's garbage collector like any other script.

The compiled language, the one `salam build` produces, has neither.

## Frequently asked questions {#faq}

### Does Salam have a garbage collector?

No. Compiled Salam programs free heap memory explicitly, with `.Free()` and `defer`, or through frees the compiler inserts for local containers that never escape. Only the `salam exec` interpreter and JavaScript output use a garbage collector.

### Is native code always faster than a virtual machine?

No. It usually starts faster and uses less memory, but a mature JIT can match or beat ahead-of-time code on some long-running workloads. On Salam's benchmark, Salam is in the same range as C and Rust, ahead of Go on average, and far ahead of Python and PHP.

### What is AOT compilation?

Ahead-of-time compilation translates a program to machine code before it runs, producing an executable. The opposite is just-in-time compilation, which translates while the program runs. Salam, C, Rust, Go and Zig compile ahead of time.

### Is a language without a garbage collector unsafe?

It can be, if nothing checks memory use. Salam reduces the risk with compile-time ownership checks, runtime bounds checks in development builds and `salam memcheck`, but it does not prove memory safety the way Rust's borrow checker does.
