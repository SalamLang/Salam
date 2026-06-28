# Salam platform & word-size support

This document explains which CPU word sizes (32-bit / 64-bit) and operating
systems Salam supports, how each backend behaves, and how the release pipeline
produces binaries for them.

## TL;DR

- **64-bit** (x86-64, arm64): fully supported, both backends, all features.
- **32-bit** (i686, armhf, wasm32): supported.
  - The **C backend** (`salam build`) targets 32-bit correctly and completely -
    it emits standard C and lets the system C compiler resolve pointer/`size_t`
    width. This is the recommended path for 32-bit today.
  - The **LLVM backend** (`salam llvm`) is now **pointer-width aware** and emits
    correct 32-bit IR when given a 32-bit `--target` (see below). The embedded
    LLVM *toolchain* binary is shipped 64-bit-first; the 32-bit LLVM story
    depends on toolchain availability (notes below).

## Can a 32-bit machine use `i64` and `f64`? - Yes.

"32-bit" refers to the **pointer / word width**, not the widest value you can
compute with. Nothing about 64-bit numeric types is lost on a 32-bit CPU:

| Type | On a 32-bit target |
|------|--------------------|
| `i64` / `u64` | Fully supported. Add/sub are inlined as 32-bit register **pairs**; multiply/divide/modulo call the compiler's runtime helpers (`__divdi3`, `__moddi3`, `__udivdi3`, …). Correct, just a few instructions slower than native 64-bit. |
| `f64` (double) | Fully supported and usually **native** - the FPU (x87 / SSE2 / NEON) operates on 64-bit doubles regardless of pointer width. |
| `i32` / `f32` | Native. |

What actually changes on 32-bit is only:

- **pointers are 4 bytes** (not 8), and
- **`size_t` / `uintptr` is 4 bytes** (not 8).

So the only real porting work is making sure the compiler never assumes a
pointer or `size_t` is 64 bits. That is exactly what the changes below address.

## How each backend handles word size

### C backend (`salam build`) - portable by construction

Salam types lower to standard C types: `i64`→`int64_t`, `f64`→`double`,
`sizeof(T)`→C's `sizeof`, pointers→native C pointers. The **system C compiler**
(gcc / tcc / clang) then resolves every width for the actual target. Build the
generated C with a 32-bit compiler and you get correct 32-bit code for free -
no special handling in Salam is required.

### LLVM backend (`salam llvm`) - now width-aware

Salam emits textual LLVM IR. Previously every `size_t`-typed position
(`malloc`, `realloc`, `memcpy`, `memmove`, `snprintf`, `strlen`) and the
internal helpers were hardcoded to `i64`, which is wrong on a 32-bit target -
the IR declarations would not match the real libc ABI (most visibly,
`snprintf`'s vararg layout breaks when its `size_t` argument is the wrong
width).

The backend now carries a target **`usize`** width derived from the target
triple:

```
salam llvm prog.salam --emit-llvm                       # host width (i64 on x86-64)
salam llvm prog.salam --emit-llvm --target=i686-linux-gnu   # 32-bit: usize = i32
```

- With no `--target`, `usize` = the host pointer width, so **64-bit output is
  byte-for-byte identical** to before - no behavior change for existing users.
- With a 32-bit triple, all `size_t` positions become `i32`, a matching
  `target triple = "…"` line is emitted, and `llc`/`clang` lower the module to
  real 32-bit machine code (verified: a 32-bit triple produces an
  `ELF 32-bit Intel 80386` object).

Width classification lives in `ll_target_ptr_bits()`
(`compiler/src/llvm/codegen_llvm_type.c`); the prologue generator is
`ll_emit_prologue()` (`compiler/src/llvm/codegen_llvm.c`).

Note: hash values and `%lld`/`%llu` formatting stay `i64` on every target - that
is correct (64-bit math just runs via the helper routines on 32-bit).

> Known follow-up: full 32-bit correctness for **stdlib** FFI size declarations
> would benefit from a real pointer-width language type (`usize`/`isize`) so
> `std/*` does not hard-code `u64` for sizes. Until then, prefer the C backend
> for production 32-bit builds. The C backend already handles this correctly.

## Release matrix (GitHub Actions: `.github/workflows/build-release.yml`)

| Job | OS / Arch | Word size | Backend in the shipped binary |
|-----|-----------|-----------|-------------------------------|
| `build-linux`        | Linux x86-64   | 64 | Embedded static LLVM + C backend |
| `build-macos`        | macOS (arm64/x64) | 64 | Embedded static LLVM + C backend |
| `build-windows`      | Windows x64    | 64 | Embedded static LLVM + bundled tcc |
| `build-windows-i686` | Windows x86    | 32 | Embedded LLVM if it links on i686, else C backend; bundled 32-bit tcc |
| `build-linux-i686`   | Linux x86      | 32 | C backend + interpreter |
| `build-linux-arm`    | Linux aarch64 / armhf | 64 / 32 | C backend (cross-compiled, smoke-tested under QEMU) |

The three 32-bit/cross jobs are marked `continue-on-error`, and the release
step attaches their artifacts only if present - a flaky 32-bit or cross build
can never block the release of the core 64-bit platforms.

### Why not embedded LLVM everywhere on 32-bit?

- **Linux i686 / ARM**: distro packages ship `llvm-18-dev` for the host
  (64-bit) only; there is no ready 32-bit/cross static LLVM-18 to embed in CI.
  The C backend covers these fully.
- **Windows i686**: MSYS2 *does* package `mingw-w64-i686-llvm`, so the job
  attempts an embedded-LLVM static build. Linking the **full static LLVM** can
  overflow the 32-bit address space, so the job falls back to a C-backend build
  to guarantee a usable 32-bit `salam.exe` (still self-contained, with the
  bundled 32-bit tcc).

## Recommendation

- **Targeting 32-bit programs?** Use `salam build` (C backend). It is correct
  and complete on every platform today.
- **Need the LLVM pipeline on 32-bit?** Pass an explicit `--target=` (e.g.
  `i686-linux-gnu`, `armv7-linux-gnueabihf`, `wasm32-wasi`) so the IR is emitted
  with the right `usize` width.
- **64-bit users:** nothing changes.
