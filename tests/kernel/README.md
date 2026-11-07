# A Salam kernel without an operating system

This directory builds a small Multiboot kernel in Salam and boots it in QEMU.
Nothing from a host operating system is linked in: the generated program talks
to the VGA text buffer, the COM1 serial port and the QEMU debug console through
the freestanding runtime in `std/baremetal`.

## Build

```sh
SALAM=/path/to/salam sh build.sh
```

The script does two things:

1. assembles `boot.s`, a 32-bit Multiboot entry point that sets up a stack and
   calls the Salam `main`;
2. runs `salam build` for the `i686-unknown-none` target. The compiler emits
   LLVM IR without libc, compiles `std/baremetal/runtime.c` for the same target
   and links everything with `ld.lld`.

## Run

```sh
qemu-system-i386 -kernel kernel.elf -serial stdio -display gtk
```

Add `-device isa-debug-exit,iobase=0xf4,iosize=0x04` to let the kernel shut
QEMU down; the process exit code is `(code << 1) | 1`, so code 0 is exit 1.

Prefer a terminal-only run? Add `-display none -debugcon stdio
-global isa-debugcon.iobase=0xe9`.

## How the pieces fit

- `main.salam` is ordinary Salam. `println` and `Vector` keep working.
- `std/baremetal` exposes port I/O, VGA text output, serial output and a
  `Panic`, plus `HeapUsed`/`HeapSize` for the runtime bump allocator.
- The runtime supplies `malloc`, `free`, `memcpy`, `memset`, `strlen`,
  `snprintf`, `write`, `abort` and the other names the code generator uses.
  A kernel can replace any of the weak `salam_rt_*` hooks.
- `SALAM_FREESTANDING`, `SALAM_KERNEL` and `SALAM_OS_NONE` are defined while
  compiling for a `*-none` target, so a project can keep host-only code out
  with a top-level `if SALAM_FREESTANDING:`.

## 64-bit kernels

`--target=x86_64-unknown-none` produces a normal x86-64 ELF object or
executable with the same runtime. QEMU's `-kernel` loader only accepts a 32-bit
ELF, so use a 64-bit bootloader (GRUB, Limine, a custom loader) or keep the
32-bit demo above. The compiler side is the same:

```sh
salam build main.salam --target=x86_64-unknown-none \
    --entry=_start --linker-script=linker64.ld --link-object=boot64.o
```
