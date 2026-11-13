---
title: Three ways to generate code: Salam's C, LLVM and native backends compared
seo_title: Compiler Backends Compared: C, LLVM and Native in Salam
description: How Salam's C backend, LLVM backend and native x86-64 and ARM backend with a built-in linker work, what each costs, and when to choose which one.
keywords: compiler backend, LLVM backend, C as a compilation target, native code generation, built-in linker, Salam backend, code generator, machine code, cross compilation, transpile to C
category: compiler
summary: Salam can turn the same checked program into C, into LLVM IR or straight into machine code. A look inside each backend, real output from all three, and a practical guide to choosing.
date: 2026-11-11
image: /assets/img/blog/salam-code-generation-c-llvm-native-en.png
image_alt: Cover image for "Three ways to generate code: Salam's C, LLVM and native backends compared"
---

A **compiler backend** is the part of a compiler that turns a checked program into something a machine can run. Salam has three backends for native executables: a **C backend** that writes C and hands it to a C compiler, an **LLVM backend** that writes LLVM IR and lets LLVM optimize it, and a **native backend** that writes x86-64, ARM and RISC-V machine code itself and links it with a linker built into the compiler. All three sit behind the same frontend, so a program means the same thing whichever one you pick. This article shows what each produces, why each exists, and when to use which.

## What a compiler backend does {#what-is-a-backend}

The frontend of a compiler (lexer, parser, semantic analysis) answers "is this program valid, and what does it mean?" The backend answers "what instructions should the processor run?" That second question has three hard parts: **lowering** high-level constructs (a `match` on an enum, a string concatenation, a generic call) into simple operations, **optimizing** those operations, and **emitting** them in a format the operating system can load, which also means following each platform's calling conventions and object file format.

Every language project has to decide how much of that to build itself. Some reuse a whole existing compiler by generating C. Many reuse LLVM. A few write everything. Salam does all three, and the [compiler pipeline article](/blog/how-the-salam-compiler-works/) shows where they plug in. Here is the program we will push through each one:

```salam
func sum squares(n: int): int:
    mut total := 0
    repeat 1 to n in i:
        total += i * i
    end
    ret total
end

func main:
    println sum squares(10)
end
```

```output
385
```

## Which backend runs by default {#default}

You choose with `--backend=c`, `--backend=llvm` or `--backend=native`. Without the option, the driver (`use_llvm_backend` in `compiler/driver/driver_proc.salam`) picks:

| Situation                                                                                            | Default backend                                 |
| ---------------------------------------------------------------------------------------------------- | ----------------------------------------------- |
| The `salam` binary has LLVM built-in (the main release builds for Linux, macOS and Windows)          | LLVM                                            |
| The `salam` binary has no LLVM (a compiler built from source without it, or the static musl release) | C                                               |
| Windows on Arm, where neither LLVM nor tcc ships                                                     | Native                                          |
| `--cc=PATH` given                                                                                    | C, since naming a C compiler can only mean that |

If the LLVM backend meets a construct it cannot lower, the build does not fail: the driver prints `salam: falling back to the C backend for this file (build with --backend=llvm to make this an error)` and builds through C instead. Passing `--backend=llvm` explicitly turns that safety net into a hard error, which is what you want in CI.

## The C backend: C as a compilation target {#c-backend}

**Using C as a compilation target means the compiler writes C source code and lets an existing C compiler produce the machine code.** It is one of the oldest tricks in language implementation, and it is still a good one: C compilers exist for nearly every platform, they optimize well, and the generated code is something a human can read.

Salam's C backend lives in `compiler/codegen/`. Every source file and every standard library module it uses becomes a `.c` and `.h` pair in a `.salam-build/` directory. You can keep and read them with `--emit-c`:

```c
static int32_t _Salam_main_square_i32(int32_t n) {
    (void)n;
    return ((int32_t)((uint32_t)(n) * (uint32_t)(n)));
}
```

That is a one-line `square` function after code generation. The name is mangled with the package and parameter types, and the multiplication runs in unsigned arithmetic so that overflow wraps the same way on every C compiler, instead of being undefined behavior as signed overflow is in C.

The driver then finds a C compiler: one bundled next to `salam` if the download includes one (the Windows release bundles tcc), otherwise `clang`, `gcc` or `tcc` from `PATH`. It compiles the modules in parallel, one worker per CPU core, caches the object files so that the next build only recompiles what changed, and links. Before a release, CI builds four groups of test programs through tcc, gcc and clang in a strict mode and fails if the generated C is not clean under any of them.

What the C backend is good for:

- **Portability and transparency.** If a platform has a C compiler, the code can probably run there, and you can read exactly what was generated.
- **Your C tools.** `--cc=` picks a specific C compiler, and `--asan` (AddressSanitizer) works when that compiler is gcc or clang.
- **Features that live only here.** `--protect`, which encrypts string literals in the binary, is implemented in the C backend.

What it costs: you need a C compiler on the machine, and output quality depends on it. tcc compiles very quickly but has no optimizer; gcc and clang optimize but take longer.

## The LLVM backend: optimization and cross-compilation {#llvm-backend}

**An LLVM backend translates the program into LLVM's intermediate representation (IR) and lets the LLVM project optimize it and produce machine code for many processors.** Rust, Swift, Zig and Julia all use [LLVM](https://llvm.org/) this way. Salam's LLVM backend is in `compiler/llvmgen/`, and you can read its output with `salam llvm`:

```bash
salam llvm sum.salam -o sum.ll
```

This is the start of `sum squares` before optimization (`salam llvm` defaults to `-O0`, because the command exists to be read):

```text
define internal i32 @salam_sum_20squares_i32(i32 %arg0) nounwind {
entry:
  %p.n = alloca i32
  %v.total.0 = alloca i32
  ...
L1_rbody:
  %t13 = trunc i64 %t9 to i32
  store i32 %t13, ptr %v.i.8
  %t14 = load i32, ptr %v.total.0
  %t15 = load i32, ptr %v.i.8
  %t16 = load i32, ptr %v.i.8
  %t17 = mul i32 %t15, %t16
  %t18 = add i32 %t14, %t17
  store i32 %t18, ptr %v.total.0
```

The multi-word name `sum squares` becomes `salam_sum_20squares_i32` (the space is encoded as `_20`), and every local is a stack slot that LLVM's optimizer will later promote to registers. A real `salam build` with this backend optimizes at `-O2`, or `-O3` with `--release`.

Unlike the C path, the whole program goes into **one** LLVM module, so the optimizer sees across package boundaries and drops whatever the program never uses. In the main release builds, LLVM and its linker lld are linked into the `salam` binary itself, with sysroots for the cross targets, so the LLVM backend needs no outside tools at all. A build of `salam` without embedded LLVM can still use this backend by calling `clang` from `PATH`.

LLVM is also what makes the reach of the language wide. `--target` cross-compiles to other operating systems and processors (the article on [cross-compiling Salam](/blog/cross-compilation-salam/) goes through the triples), WebAssembly goes through it, and freestanding bare-metal builds require it: ask for one with `--backend=c` and the compiler says `freestanding targets need the LLVM backend`.

The cost is size and build time. A `salam` with LLVM, lld and sysroots inside is a large binary, and LLVM's optimizer takes noticeable time on large programs.

## The native backend: machine code without LLVM or a C compiler {#native-backend}

The native backend (`--backend=native`, source in `compiler/nativegen/`) is Salam's own code generator, written in Salam. It is the largest single part of the compiler, about 52,600 lines, and it targets x86-64 Linux, Windows and macOS, 32-bit x86 Linux and Windows, AArch64 Linux, macOS and Windows, 32-bit ARM Linux and RISC-V 64 Linux. Any other `--target` is rejected with a message that points to `--backend=llvm`.

It makes one clever choice. Rather than lowering the syntax tree a third time, it **reads the textual LLVM IR that `llvmgen` already produced**, parses it into its own compact SSA form, and continues from there with its own passes. Every language feature the LLVM backend handles therefore works here too, and the two backends cannot drift apart on semantics. `docs/NATIVE_BACKEND.md` lists the pipeline: SROA and mem2reg, inlining (including partial inlining of cold paths), tail-recursion elimination, constant folding and jump threading, loop unrolling, loop-invariant code motion, strength reduction, global value numbering, loop and SLP vectorization on x86-64 and AArch64, instruction selection, linear-scan register allocation with live-range splitting, and finally encoders for each instruction set and writers for ELF, COFF and Mach-O objects.

You can see the result with `--emit-asm`, which writes the machine code as assembly next to the executable. For our program, the interesting line is in `main`:

```text
    mov ecx, 385
```

The native backend inlined `sum squares`, evaluated the whole ten-step loop at compile time, and kept only the constant `385` and the code that turns it into decimal digits for printing. No call to `sum squares` and none of its loop is left.

The `--cpu` option chooses an instruction-set level on x86-64: the default baseline, `x86-64-v2` (SSE4.1 vector code) or `x86-64-v3` and `x86-64-v4`, which let loops vectorize with 256-bit AVX2 registers but require a CPU that has them. There is no runtime dispatch, so a build either runs everywhere or needs AVX2 everywhere.

On honest terms, the project's own measurements in `docs/NATIVE_BACKEND.md` put the native backend close to LLVM on the benchmark programs, ahead on some and behind on others, with library-heavy code (image codecs, for example) further behind. It has no AVX-512 or FMA code yet, and armhf and RISC-V builds do not vectorize.

## The built-in linker {#built-in-linker}

Generating machine code is not enough; something has to combine the object files with the C runtime into an executable. The native backend usually does that too, with a linker written in Salam (`compiler/nativelink/`, design in `docs/NATIVE_LINKER.md`). With `--linker=auto`, the default, it is used whenever every input can be found, and it produces:

- fully **static executables against musl** on Linux;
- **position-independent executables against glibc**, for ordinary dynamically linked Linux programs;
- **PE executables and DLLs** for Windows, including icons, manifests and version resources;
- **Mach-O executables** for macOS on x86-64 and arm64, including universal binaries.

`--linker=system` hands the job to `cc` or the MinGW `gcc` instead. `--icf` turns on identical code folding, which merges functions whose machine code came out the same; on the compiler itself it folds pairs such as `Vector<i64>.push` and `Vector<str>.push`.

Here is the effect on file size, for a hello-world program built on x86-64 Linux by a compiler from the main branch in November 2026:

| Build                                         | Linking                       | Size          |
| --------------------------------------------- | ----------------------------- | ------------- |
| `--backend=native`                            | built-in linker, glibc PIE    | 11,488 bytes  |
| `--backend=native --target=x86_64-linux-musl` | built-in linker, fully static | 24,944 bytes  |
| `--backend=llvm`                              | lld, glibc                    | 17,536 bytes  |
| `--backend=c` with clang                      | system linker, glibc          | 363,384 bytes |

The C build is larger because it links whole precompiled standard library modules as separate objects, data included, while the LLVM and native paths compile one program and drop what it never references. The native build's log reports the link itself: `built-in link: 5 objects, 15 live sections, 28 symbols`.

## C vs LLVM vs native: a comparison {#comparison}

|                     | C backend                                               | LLVM backend                                                             | Native backend                                                                                             |
| ------------------- | ------------------------------------------------------- | ------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------- |
| Needs outside tools | a C compiler                                            | none in release builds                                                   | none beyond the C runtime files                                                                            |
| Optimizer           | the C compiler's (tcc: none)                            | LLVM `-O2`/`-O3`                                                         | Salam's own                                                                                                |
| Targets             | wherever a C compiler runs                              | the `--target` triples Salam supports, plus WebAssembly and freestanding | x86-64, i686, AArch64, armhf, RISC-V 64 on Linux; x86-64, i686, AArch64 on Windows; x86-64, arm64 on macOS |
| Read the output     | `--emit-c`                                              | `salam llvm`                                                             | `--emit-asm`                                                                                               |
| Special abilities   | `--asan`, `memcheck`, `--protect`, choice of C compiler | cross-compiling, Wasm, bare metal                                        | `--cpu` levels, built-in linker, `--icf`                                                                   |
| Main cost           | depends on an external toolchain                        | large compiler binary                                                    | newest backend, library code still behind LLVM                                                             |

On the project's [speed page](/speed/), Salam was measured with its LLVM backend at `-O3`: 10.6 ms on average over 39 programs, against 11.7 ms for C and 11.3 ms for Rust, which is the same range rather than a ranking. The same page notes that a plain C-backend build with tcc and no optimizer averaged 11.0 ms on those programs, because most of them finish in a few milliseconds and process start-up is a large share of each time. Backend choice matters most for long-running, compute-heavy code, and much less for short command-line tools.

## Which Salam backend should you use? {#when-to-use}

- **Start with the default.** In a release build from the [install page](/install/) that is LLVM, which gives optimized code and cross-compilation with nothing else installed.
- **Use `--backend=c`** when you want to read or debug the generated C, need AddressSanitizer (`--asan`, `salam memcheck`), want to use a particular C compiler, or are on a platform where only a C compiler is available.
- **Use `--backend=native`** when you want small binaries, static Linux executables, Windows or macOS executables linked by Salam's own linker, AVX2 code through `--cpu`, or builds without LLVM in the loop. It is also the only option on Windows on Arm.
- **Use `--backend=llvm` explicitly** in CI if you want unsupported constructs to fail the build rather than fall back to C.

Whatever you choose, the checks are the same, because they all run before code generation. The [static typing article](/blog/static-typing-in-salam/) covers what those checks catch, and [the article on native compilation](/blog/native-compilation-without-vm-or-gc/) explains what running without a virtual machine means for your program.

## Frequently asked questions {#faq}

### Does Salam transpile to C?

It can. The C backend writes C and calls a C compiler, and it is the default when the `salam` binary was built without LLVM. Release builds default to the LLVM backend, and the native backend writes machine code directly.

### Why have three backends instead of one?

Each solves a different problem. C runs almost anywhere a C compiler exists and is easy to inspect, LLVM brings a mature optimizer and many targets, and the native backend removes outside toolchains and gives the project full control over code generation. They also check each other: the same tests run on all of them.

### Is the native backend production ready?

It is the newest of the three, merged in October 2026. It passes the project's test sweeps on its supported targets, with a few documented exceptions, and builds the compiler itself, but it is younger than LLVM and still behind it on some library-heavy code. For a production build today, LLVM is the conservative choice.

### Can I see the assembly Salam generates?

Yes. `salam build app.salam --backend=native --emit-asm` writes `app.s` in GNU assembler syntax, and `salam llvm app.salam --emit-asm` gives LLVM's assembly for the same program.
