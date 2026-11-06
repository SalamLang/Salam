# Freestanding Salam: kernels and other bare-metal programs

A freestanding build produces a program that does not call an operating system:
no libc, no C runtime start files, no syscalls. Salam can emit one for x86-64,
i686 and aarch64 through its LLVM backend, and the compiler ships a small
runtime that supplies the names the code generator expects.

This is the target to use for an operating-system kernel, a bootloader payload,
a unikernel, a hypervisor component or any project that owns the machine.

## Quick start

```sh
salam build kernel.salam \
    --target=i686-unknown-none \
    --entry=_start \
    --linker-script=linker.ld \
    --link-object=boot.o \
    --output=kernel.elf
```

- `--target` selects the machine and marks the build freestanding. `*-none`
  and the `*-salamos*` triples are freestanding; a `*-uefi` triple is also
  freestanding but keeps the UEFI object format.
- `--entry` is the symbol the linker starts at. It defaults to `_start` for a
  freestanding build. The Salam `main` function is emitted with its normal
  signature (no `argc`/`argv` magic) and the symbol `main`, so a small
  assembly entry point can call it.
- `--linker-script` passes `-T script` to `ld.lld`.
- `--link-object` adds an object file to the link; repeat it for several.
  Use this for the boot stub, interrupt vectors or a board support package.
- `--no-freestanding-runtime` stops the compiler from adding its runtime, for
  a kernel that provides all the symbols itself.

The compiler compiles the LLVM IR to an object and links it with `ld.lld`
(`-m elf_i386`, `-m elf_x86_64`, `-m aarch64elf`, ...). It does not link libc,
crt files or `-lm`.

## What the built-in runtime provides

`std/baremetal/runtime.c` is compiled for the chosen target and linked into the
program. It supplies:

- `malloc`, `calloc`, `realloc`, `free`: a 16 MiB bump allocator, with a small
  header so `realloc` can copy the old contents;
- `memcpy`, `memmove`, `memset`, `memcmp`, `strlen`, `strcmp`, `strncmp`,
  `strstr`, `strtol`, `strtod`, `snprintf`, `printf`, `dprintf`;
- `write`, `isatty`, `fflush`, `atexit`, `signal`, `raise`, `sched_yield`,
  `abort`, `exit`;
- console output: COM1 serial, the VGA text buffer at `0xB8000` and the QEMU
  debug console at port `0xE9`; the VGA buffer prints bytes (ASCII), so use
  serial for UTF-8 text such as Persian;
- the `salam_rt_*` hooks used by `std/baremetal`.

Every hook is weak, so a kernel can replace `salam_rt_write`, `salam_rt_exit`,
`salam_rt_outb` and the others with its own implementation. Keep
`--no-freestanding-runtime` when the replacement also covers libc names such as
`malloc` and `memcpy`.

The default heap is a static 16 MiB array. Set `SALAM_RT_HEAP_SIZE` when
compiling the runtime yourself, or call `malloc` from your own allocator by
replacing the ones in the runtime.

## Compile-time names

A freestanding target defines:

| Name | Meaning |
| --- | --- |
| `SALAM_FREESTANDING` | No host operating system is available |
| `SALAM_KERNEL` | The target is a kernel (`*-none` and `*-salamos-kernel`) |
| `SALAM_OS_NONE` | The generic `*-none` target |
| `SALAM_OS_SALAMOS` | A `*-salamos*` target |

Use them in a top-level `if` so host-only code is not compiled:

```salam
if SALAM_FREESTANDING:
    import baremetal
end
```

The Persian names are `سلام خودایستا`, `سلام هسته` and `سلام سیستم هیچ`.

## `std/baremetal`

```salam
import baremetal

func main:
    baremetal.Clear()
    baremetal.WriteAt(0, 0, "Salam", 0x0F as u8)
    println "serial and VGA"
    baremetal.ExitQemu(0)
end
```

| Function | Purpose |
| --- | --- |
| `Halt()` | Stop the CPU (`cli; hlt` on x86, `wfi` on aarch64) |
| `Exit(code)` | Leave through `salam_rt_exit`; the kernel decides what that means |
| `Out8(port, value)`, `In8(port)` | x86 port I/O |
| `Clear()` | Clear the VGA text screen |
| `WriteAt(row, col, text, color)` | Write one text row directly to VGA memory |
| `SerialWrite(text)` | Write to the serial console without buffering |
| `Panic(message)` | Print a kernel panic line and halt |
| `ExitQemu(code)` | Exit through QEMU's `isa-debug-exit` device (port `0xF4`) |
| `HeapUsed()`, `HeapSize()` | Bump allocator counters in bytes |

Persian programs call the same functions with their `@fa` names, for example
`کرنل.توقف()` and `کرنل.نوشتن جایگاه(...)`.

## A bootable example

`tests/kernel/` contains a complete Multiboot v1 kernel for `i686`:

```sh
SALAM=/path/to/salam sh tests/kernel/build.sh
qemu-system-i386 -kernel tests/kernel/kernel.elf \
    -serial stdio -display gtk \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04
```

The 32-bit Multiboot entry point in `boot.s` sets up a stack, enables SSE and
calls the Salam `main`. QEMU's `-kernel` loader only accepts a 32-bit ELF, so the
demo uses `i686`. The `x86_64-unknown-none` target produces a normal x86-64 ELF
that a 64-bit bootloader (GRUB, Limbo, Limine or your own) can load.

## What is not there

Freestanding means the program owns the machine, so the parts of `std` that
need an operating system are not usable: files, sockets, processes, threads,
wall-clock time and most of `os`, `fs`, `net`, `db` and `gui`. `str`, `mem`,
`conv`, `encoding` and the collection types work. `SALAM_KERNEL` removes the
parts of `std` that cannot build in a kernel.

There is no interrupt, paging or device-management layer in the compiler. A
kernel provides those in Salam, or in a small assembly/C board support package
linked with `--link-object`. The compiler's job is to give that project a
correct, libc-free object and a predictable calling convention.

## Supported triples

| Target | Output | Notes |
| --- | --- | --- |
| `x86_64-unknown-none` | ELF64 | QEMU `-kernel` needs a 32-bit loader; use GRUB/Limine |
| `i686-unknown-none` | ELF32 | Multiboot example, QEMU `-kernel`, 32-bit PCs |
| `aarch64-unknown-none` | ELF64 | PL011 serial at `0x09000000`, `wfi` halt |
| `x86_64-unknown-salamos` | ELF64 | Salam OS userspace-style target |
| `x86_64-unknown-salamos-kernel` | ELF64 | `SALAM_KERNEL` is set |

Other machines need a runtime for their console and memory: pass
`--no-freestanding-runtime` and provide `malloc`, `memcpy`, `write` and the
`salam_rt_*` hooks.
