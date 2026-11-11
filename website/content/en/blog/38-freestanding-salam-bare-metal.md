---
title: From command-line tools to operating system kernels: freestanding and bare-metal programming in Salam
seo_title: Bare Metal Programming in Salam: Freestanding Builds and Kernels
description: Salam can build freestanding programs with no libc and no operating system. How the bare-metal target works, a kernel that boots in QEMU, and the limits.
keywords: bare metal programming, freestanding programming language, OS kernel development, write a kernel, no libc, Multiboot kernel, QEMU kernel, x86-64 freestanding, aarch64 bare metal, Salam baremetal
category: build
summary: The same compiler that builds Salam web servers can emit a program that owns the machine: no libc, no syscalls, a tiny runtime and a kernel that boots in QEMU. Here is how it works and what you still have to write yourself.
date: 2026-11-11
---

Bare-metal programming means writing code that runs directly on the hardware, with no operating system underneath to provide files, processes or memory. **A freestanding build of Salam produces exactly that kind of program: an executable that calls no operating system, links no libc and no C start-up files, and makes no system calls.** The compiler emits one for x86-64, i686 and AArch64 through its LLVM backend and links a small runtime that supplies the few functions the generated code needs. The repository includes a complete Multiboot kernel written in Salam that boots in QEMU.

This article shows how the freestanding target works, builds and boots a kernel, explains what the runtime provides, and lists clearly what a kernel author still has to write.

## What freestanding means {#what-freestanding-means}

The C standard has long separated a *hosted* implementation, where the whole standard library and an operating system are available, from a *freestanding* one, where almost nothing is. C programmers pass `-ffreestanding`, Rust programmers write `#![no_std]`, and Zig selects a `freestanding` operating system in its target. Salam selects the mode by target triple:

| Target | Output | Notes |
|---|---|---|
| `x86_64-unknown-none` | ELF64 | For a 64-bit bootloader such as GRUB or Limine |
| `i686-unknown-none` | ELF32 | Multiboot, QEMU `-kernel`, 32-bit PCs |
| `aarch64-unknown-none` | ELF64 | PL011 serial at `0x09000000`, `wfi` to halt |
| `x86_64-unknown-salamos` | ELF64 | A Salam OS userspace-style target |
| `x86_64-unknown-salamos-kernel` | ELF64 | Defines `SALAM_KERNEL` |

Any `*-none` or `*-salamos` triple is freestanding. A `*-uefi` triple is freestanding too but keeps the UEFI object format. The compiler turns the program into LLVM IR, compiles it to an object for the target and links it with `ld.lld`, without libc, without `crt` files and without `-lm`. If you have not used the LLVM backend before, the article on [Salam's C, LLVM and native backends](/blog/salam-code-generation-c-llvm-native/) explains where it fits.

## The build command {#build-command}

A freestanding build uses four options on top of `--target`:

```bash
salam build kernel.salam \
    --target=i686-unknown-none \
    --entry=_start \
    --linker-script=linker.ld \
    --link-object=boot.o \
    --output=kernel.elf
```

- `--entry` names the symbol the linker starts at, `_start` by default. Salam's `main` is emitted under its normal name with no `argc`/`argv` handling, so a few lines of assembly can set up a stack and call it.
- `--linker-script` passes `-T script` to `ld.lld`, which is how a kernel decides where each section goes in memory.
- `--link-object` adds an object file, such as the boot stub, interrupt vectors or a board support package. Repeat it as often as needed.
- `--no-freestanding-runtime` leaves the built-in runtime out, for a kernel that supplies every symbol itself.

## A kernel that boots in QEMU {#kernel-in-qemu}

The directory `tests/kernel/` contains a Multiboot v1 kernel for i686. The assembly entry point in `boot.s` sets up a 32 KB stack, enables SSE and calls Salam's `main`. Everything else is ordinary Salam:

```salam
import baremetal

func main:
    baremetal.Clear()

    println "Salam kernel"
    println "no operating system underneath"
    println ""

    mut tick := 0
    until tick < 5:
        print "tick "
        println tick
        tick++
    end

    baremetal.WriteAt(10, 0, "this line went straight to VGA memory", 0x0A as u8)

    println ""
    println "heap used: ", baremetal.HeapUsed(), " bytes"
    println "heap size: ", baremetal.HeapSize(), " bytes"
    println "halting QEMU"

    baremetal.ExitQemu(0)
end
```

`until` in Salam loops while its condition holds, so the loop prints five ticks. `println` keeps working because the runtime routes output to the serial port, the VGA text buffer and QEMU's debug console. Building it with the repository's script and booting it headless with the serial port on standard output:

```bash
SALAM=/path/to/salam sh tests/kernel/build.sh
qemu-system-i386 -kernel tests/kernel/kernel.elf -serial stdio -display none \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04
```

produced, on our test machine, a 24,800-byte ELF32 executable and this output:

```text
Salam kernel
no operating system underneath

tick 0
tick 1
tick 2
tick 3
tick 4

heap used:  80  bytes
heap size:  16777216  bytes
halting QEMU
```

QEMU then exits through its `isa-debug-exit` device. The process exit code is `(code << 1) | 1`, so the kernel's code 0 shows up as exit status 1. With `-display gtk` instead of `-display none` you also see the green line written straight into VGA memory at row 10.

QEMU's `-kernel` loader only accepts a 32-bit ELF, which is why the demo targets i686. The `x86_64-unknown-none` target produces a normal 64-bit ELF that GRUB, Limine or your own loader can start.

## One program, hosted and bare metal {#hosted-and-bare-metal}

A useful property of a freestanding target is that most of your code does not care. The compiler defines `SALAM_FREESTANDING` (no host operating system), `SALAM_KERNEL` (a kernel target) and `SALAM_OS_NONE` or `SALAM_OS_SALAMOS`, and a top-level `if` keeps host-only or kernel-only code apart. The [compile-time lesson](/learn/compile-time/) covers the mechanism. This program computes Fletcher-16 checksums; it imports `baremetal` only when there is no operating system:

```salam
if SALAM_FREESTANDING:
    import baremetal
end

func fletcher16(data: str): int:
    mut a := 0
    mut b := 0
    each c in data.bytes():
        a = (a + (c as int)) % 255
        b = (b + a) % 255
    end
    ret b * 256 + a
end

func main:
    each word in ["abcde", "abcdef", "Salam"]:
        println word, fletcher16(word)
    end
    if SALAM_FREESTANDING:
        baremetal.ExitQemu(0)
    end
end
```

```output
abcde 51440
abcdef 8279
Salam 39919
```

That output is from an ordinary `salam run` on Linux. Built with the kernel's boot stub and linker script for `i686-unknown-none`, the same file became a 22,896-byte kernel, and QEMU printed the same three lines over the serial port. The first two values are the well-known Fletcher-16 test vectors (`0xC8F0` and `0x2057`). Being able to test a parser, an allocator or a checksum with `salam test` on your workstation and then link the identical code into a kernel is one of the more practical reasons to write kernel code in a higher-level language.

The same source also compiles to objects for `x86_64-unknown-none` and `aarch64-unknown-none`. The x86-64 object for this program left exactly six symbols for the runtime to supply: `abort`, `malloc`, `memcpy`, `strlen`, `write` and `salam_rt_exit_qemu`.

## What the built-in runtime provides {#runtime}

`std/baremetal/runtime.c` is compiled for the chosen target and linked in. It is written in C because it plays the role libc plays elsewhere, and it provides:

- `malloc`, `calloc`, `realloc` and `free` over a 16 MiB bump allocator, with a small header so `realloc` can copy the old contents;
- `memcpy`, `memmove`, `memset`, `memcmp`, `strlen`, `strcmp`, `strncmp`, `strstr`, `strtol`, `strtod`, `snprintf`, `printf` and `dprintf`;
- `write`, `isatty`, `fflush`, `atexit`, `signal`, `raise`, `sched_yield`, `abort` and `exit`;
- console output to COM1 serial, the VGA text buffer at `0xB8000` and the QEMU debug console at port `0xE9`.

The VGA buffer shows bytes as ASCII, so Persian or other UTF-8 text should go to the serial console. Every hook is a weak symbol, so a kernel can replace `salam_rt_write`, `salam_rt_exit`, `salam_rt_outb` and the rest with its own versions. A bump allocator never reuses freed memory, which is fine for a demo and for kernels that allocate once at boot, and is the first thing a real kernel replaces: either swap in your own `malloc` or build the runtime with a different `SALAM_RT_HEAP_SIZE`.

## The baremetal package {#baremetal-package}

`std/baremetal` is the Salam face of that runtime:

| Function | Purpose |
|---|---|
| `Halt()` | Stop the CPU (`cli; hlt` on x86, `wfi` on AArch64) |
| `Exit(code)` | Leave through `salam_rt_exit`; the kernel decides what that means |
| `Out8(port, value)`, `In8(port)` | x86 port I/O |
| `Clear()`, `WriteAt(row, col, text, color)` | VGA text screen |
| `SerialWrite(text)` | Unbuffered serial output |
| `Panic(message)` | Print a panic line and halt |
| `ExitQemu(code)` | Leave QEMU through `isa-debug-exit` |
| `HeapUsed()`, `HeapSize()` | Bump allocator counters in bytes |

Like every standard library package, it has Persian names: in Persian source the package is `کرنل`, and `کرنل.توقف()` halts the CPU. A kernel can be written entirely in Persian. This one, built with the same boot stub and linker script, printed its greeting over the serial port and shut QEMU down:

```salam
واردسازی کرنل

تابع ریشه:
    سرچاپ "هسته‌ی سلام"
    کرنل.خروج شبیه ساز(۰)
پایان
```

## What you still have to write {#what-is-missing}

The compiler's job here is narrow: a correct, libc-free object with a predictable calling convention. It does not give you an operating system. In particular:

- **No interrupts, paging or device drivers.** There is no interrupt descriptor table setup, no page-table code and no device layer in the compiler or runtime. You write them in Salam, or in a small assembly or C support package linked with `--link-object`. For port I/O and memory-mapped registers, Salam's pointers and [C interop](/blog/salam-c-interop-ffi/) are the tools.
- **Most of the standard library is unavailable.** Files, sockets, processes, threads and wall-clock time need an operating system, so most of `os`, `fs`, `net`, `db` and `gui` are out. `str`, `mem`, `conv`, `encoding` and the collection types work, and `SALAM_KERNEL` removes the parts of `std` that cannot build in a kernel.
- **Three architectures have a runtime.** Other machines need `--no-freestanding-runtime` and your own `malloc`, `memcpy`, `write` and `salam_rt_*` hooks.
- **Memory is your responsibility, as everywhere in Salam.** There is no garbage collector to disable, which is good news for a kernel, but there is also no borrow checker. Salam has bounds checks in development builds and compile-time ownership warnings; read [memory management in Salam](/blog/memory-management-in-salam/) before relying on them.
- **The demo kernel is a demo.** It proves the toolchain end to end; it has no scheduler, file system or driver model.

## Why write a kernel in Salam {#why}

C is the default language for kernels for good reasons: decades of tooling, compilers on every architecture and a huge body of reference code. Rust has been accepted into the Linux kernel for drivers, and Zig is popular for hobby kernels and firmware. Salam is far younger than all three, and nobody should pick it for a commercial kernel today.

Where Salam is interesting is teaching and experiments. A student can boot a kernel whose `main` reads like the first lesson of the [course](/learn/), keep `println` and vectors working, test the logic on a laptop with `salam test`, and read the compiler that produced it, because the compiler is written in Salam too (see [the self-hosting compiler](/blog/self-hosting-compiler/)). There is no garbage collector or runtime scheduler to remove, which is why the freestanding runtime can be one C file. And because the language has Persian keywords and a Persian `baremetal` package, an operating systems course can be taught with kernel code in Persian, something no mainstream systems language offers.

The same reasons make freestanding builds useful beyond kernels: bootloader payloads, unikernels, hypervisor components and test images for emulators. For targets with a normal operating system, the [cross-compilation article](/blog/cross-compilation-salam/) covers Linux, macOS, Windows and ARM.

## Frequently asked questions {#faq}

### Can Salam run without an operating system?

Yes. A build for a `*-none` target, such as `i686-unknown-none` or `x86_64-unknown-none`, produces a program that links no libc and makes no system calls. The repository's demo kernel boots in QEMU and prints through the serial port and VGA memory.

### Which architectures does freestanding Salam support?

x86-64, i686 and AArch64 have a built-in runtime. Other machines can still be targeted through LLVM if you pass `--no-freestanding-runtime` and supply the memory, string and output functions yourself.

### Does freestanding Salam have a garbage collector to turn off?

No. Salam never had one; heap memory is freed explicitly in every build. In freestanding builds the heap comes from the runtime's 16 MiB bump allocator unless you replace `malloc`.

### Can I write a Linux kernel module in Salam?

Nothing in the repository supports that. Kernel modules depend on the exact kernel's build system, headers and conventions, and the freestanding target is aimed at programs that own the whole machine, not at code loaded into another kernel.
