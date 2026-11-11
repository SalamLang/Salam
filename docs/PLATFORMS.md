# 🖥️ Salam Platform & Word-Size Support

This document explains which CPU word sizes (32-bit / 64-bit) and operating
systems Salam supports, how each backend behaves, and how the release pipeline
produces binaries for them.

## 📋 TL;DR

- ✅ **64-bit** (x86-64, arm64): fully supported, every backend, all features.
- ✅ **32-bit** (i686, armhf, wasm32): supported.
  - The **C backend** (`--backend=c`) emits standard C and lets the system C
    compiler resolve pointer and `size_t` width, so it is correct on 32-bit
    by construction.
  - The **LLVM backend** (`--backend=llvm`, and the default when the compiler
    has LLVM linked in) is pointer-width aware and emits 32-bit IR for a
    32-bit `--target` (see below). The i686 and armhf release compilers
    embed LLVM and LLD, so they use it by default.
  - The **native backend** (`--backend=native`) writes i686 and 32-bit ARM
    machine code itself, as well as x86-64, AArch64 and RISC-V 64.

With no `--backend`, a compiler picks native on a Windows on Arm host, LLVM
when it has LLVM linked in, and C otherwise.

## 🔢 Can a 32-bit machine use `i64` and `f64`? - Yes.

"32-bit" refers to the **pointer / word width**, not the widest value you can
compute with. Nothing about 64-bit numeric types is lost on a 32-bit CPU:

| Type            | On a 32-bit target                                                                                                                                                                                                                                            |
| --------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `i64` / `u64`   | Fully supported. Add/sub are inlined as 32-bit register **pairs**; multiply/divide/modulo call the compiler's runtime helpers (`__divdi3`, `__moddi3`, `__udivdi3`, …). Correct, just a few instructions slower than native 64-bit.                           |
| `i128` / `u128` | Fully supported. Add, subtract, multiply and shift are inlined over 64-bit halves. Divide and float conversion call Salam's own helpers in `std/core`, because 32-bit libgcc has no `__udivti3`. No C ABI, so `extern` functions cannot use them here (E160). |
| `f64` (double)  | Fully supported and usually **native** - the FPU (x87 / SSE2 / NEON) operates on 64-bit doubles regardless of pointer width.                                                                                                                                  |
| `i32` / `f32`   | Native.                                                                                                                                                                                                                                                       |

What actually changes on 32-bit is only:

- **pointers are 4 bytes** (not 8), and
- **`size_t` / `uintptr` is 4 bytes** (not 8).

So the only real porting work is making sure the compiler never assumes a
pointer or `size_t` is 64 bits. That is exactly what the changes below address.

## ⚙️ How Each Backend Handles Word Size

### 🔵 C backend (`--backend=c`) - portable by construction

Salam types lower to standard C types: `i64`→`int64_t`, `f64`→`double`,
`sizeof(T)`→C's `sizeof`, pointers→native C pointers. The **system C compiler**
(gcc / tcc / clang) then resolves every width for the actual target. Build the
generated C with a 32-bit compiler and you get correct 32-bit code for free -
no special handling in Salam is required.

### 🟠 LLVM backend (`--backend=llvm`, `salam llvm`) - width-aware

Salam emits textual LLVM IR. Early versions typed every `size_t` position
(`malloc`, `realloc`, `memcpy`, `memmove`, `snprintf`, `strlen`) and the
internal helpers as `i64`, which is wrong on a 32-bit target -
the IR declarations would not match the real libc ABI (most visibly,
`snprintf`'s vararg layout breaks when its `size_t` argument is the wrong
width).

The backend now carries a target **`usize`** width derived from the target
triple:

```sh
salam llvm prog.salam                              # host width (i64 on x86-64)
salam llvm prog.salam --target=i686-linux-gnu      # 32-bit: usize = i32
```

- With no `--target`, `usize` is the host pointer width.
- With a 32-bit triple, all `size_t` positions become `i32`, a matching
  `target triple = "…"` line is emitted, and `llc`/`clang` lower the module to
  real 32-bit machine code (verified: a 32-bit triple produces an
  `ELF 32-bit Intel 80386` object).

Width classification lives in `target_ptr_bits_of()`
(`compiler/llvmgen/llvm_type.salam`); `NewLv` in
`compiler/llvmgen/llvm_api.salam` sets the module's `ptr_bits` and `usize`
from the triple, and the prologue (`emit_prologue()`,
`compiler/llvmgen/llvm_core.salam`) writes the `usize = i32 (32-bit target)`
comment you can check in the `.ll` file.

> ⚠️ **Known gap**: the language has pointer-width integers now (`usize` and
> `size`), but some `std` FFI declarations still spell a C `size_t` as `u64`
> (for example the `malloc`/`memcpy` externs in `std/mem/mem.salam`). If a
> 32-bit LLVM build misbehaves around such a call, try the same program with
> `--backend=c`, which does not depend on those declarations.

### 🟢 Native backend (`--backend=native`)

The native backend writes machine code and the object file itself (ELF,
COFF or Mach-O). It takes the same IR as the LLVM backend, so `usize` and
every other width come out the same way. Targets:

| OS      | Architectures                                        |
| ------- | ---------------------------------------------------- |
| Linux   | x86-64, i686, AArch64, 32-bit ARM (armhf), RISC-V 64 |
| Windows | x86-64, i686, AArch64                                |
| macOS   | x86-64, arm64 (and `universal-apple-macos`)          |

Any other `--target` (wasm32, for example) is rejected with a pointer to
`--backend=llvm`. See [NATIVE_BACKEND.md](NATIVE_BACKEND.md).

## 🧱 Freestanding targets (`*-none`, `*-salamos*`)

Those triples are for programs that run without an operating system: kernels,
unikernels and bootloader payloads. The LLVM backend emits the object with
`-ffreestanding -fno-stack-protector -fno-pic`, links it with `ld.lld` and
never adds libc, crt files or `-lm`. `SALAM_FREESTANDING` and `SALAM_KERNEL`
are defined, and the compiler links its own small runtime from
`std/baremetal` unless `--no-freestanding-runtime` is passed.

```sh
salam build kernel.salam --target=i686-unknown-none \
    --entry=_start --linker-script=linker.ld \
    --link-object=boot.o --output=kernel.elf
```

See [FREESTANDING.md](FREESTANDING.md) for the runtime, the `std/baremetal`
API and a bootable Multiboot example in `tests/kernel/`.

## 📦 Release Matrix

> GitHub Actions workflow: `.github/workflows/compiler-release.yml`

| Job                   | Runner                                                | Release files                                     | Backends in the shipped compiler                                                                   |
| --------------------- | ----------------------------------------------------- | ------------------------------------------------- | -------------------------------------------------------------------------------------------------- |
| `build-linux`         | `ubuntu-24.04`                                        | `linux-x86_64` archives, `salam-linux`            | Embedded static LLVM + LLD, with musl, mingw and wasm32-wasi sysroots for cross builds; C backend  |
| `build-linux-musl`    | `ubuntu-24.04`                                        | `linux-x86_64-musl` archives                      | C backend only: fully static against musl, no embedded LLVM, so no cross builds and no JIT         |
| `build-macos`         | `macos-latest` (arm64)                                | `macos-arm64` archives, `salam-mac`               | Embedded static LLVM; links with the system (Xcode) toolchain                                      |
| `build-windows`       | `windows-2022`, `windows-2025`, `windows-2025-vs2026` | `windows-x86_64` archives, `salam-windows.exe`    | Embedded static LLVM + LLD and sysroots, plus a bundled tcc for the C backend                      |
| `build-windows-i686`  | `ubuntu-24.04`, cross-compiled with llvm-mingw        | `windows-i686` archives, `salam-windows-i686.exe` | Embedded LLVM + LLD when that cross build succeeds, otherwise built without it; bundled 32-bit tcc |
| `build-windows-arm64` | `ubuntu-24.04`, cross-compiled                        | `windows-arm64` archives                          | Native backend with its built-in linker (no LLVM, no tcc)                                          |
| `build-linux-i686`    | `ubuntu-24.04`, `i386/debian` container               | `linux-i686` archives, `salam-linux-i686`         | Embedded LLVM + LLD                                                                                |
| `build-linux-armhf`   | `ubuntu-24.04`, `arm32v7` container under QEMU        | `linux-armhf` archives, `salam-linux-armhf`       | Embedded LLVM + LLD                                                                                |
| `build-linux-aarch64` | `ubuntu-24.04-arm`                                    | `linux-aarch64` archives, `salam-linux-aarch64`   | Embedded LLVM + LLD                                                                                |

"Archives" means `salam-<version>-<name>.zip` and `.tar.gz`, each with
`std/` next to the binary. Only `build-windows-2025-vs2026` publishes the
Windows x86-64 files; the other two Windows runners test. macOS ships for
Apple silicon only; there is no Intel macOS release build. The Windows on
Arm bundle is smoke-tested on a `windows-11-arm` runner
(`smoke-windows-arm64`) before the release.

Every job except `build-linux`, `build-macos` and `build-windows` is
`continue-on-error`, and the release step attaches their files only when
they were built, so a failed 32-bit, musl, ARM or Windows on Arm build does
not block the release of the three core platforms. A versioned release then
stops if a filename the previous release had is missing, unless the release
commit message says `[allow-fewer-assets]`.

The release also carries the standard library on its own
(`salam-<version>-stdlib.zip`/`.tar.gz`, with `stdlib-index.json` and
`stdlib-doc.html`) and, when that job succeeds, the Android app.

### ❓ Which jobs embed LLVM

- **Linux i686, armhf and aarch64** link LLVM 22 and LLD from packages
  built for that architecture (Debian sid's `llvm-22-dev` in an `i386` or
  `arm32v7` container, apt.llvm.org on the arm64 runner), so all three
  embed it.
- **Linux musl** is a fully static C-backend build, so it runs on any x86-64
  Linux, including ones with an older glibc than the flagship build needs.
- **Windows i686** cross-builds a static LLVM + LLD for `i686-w64-mingw32`
  and embeds it. If that build fails, the job builds `salam.exe` without
  LLVM so that a 32-bit Windows compiler still ships, with its bundled
  32-bit tcc.
- **Windows on Arm** ships neither LLVM nor tcc. It uses the native backend
  and links with the compiler's built-in linker against the aarch64 runtime
  files packed next to `salam.exe`.

## ✅ Recommendation

| Goal                         | Recommendation                                                                                                                                                                                            |
| ---------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 🎯 Targeting 32-bit programs | Use the release compiler for that platform, or cross-compile with `--target=` (e.g. `i686-linux-musl`, `arm-linux-musleabihf`). `--backend=c` is the fallback when an LLVM build of a program misbehaves. |
| 🔬 LLVM pipeline on 32-bit   | Pass an explicit `--target=` (e.g. `i686-linux-gnu`, `armv7-linux-gnueabihf`, `wasm32-wasi`) so the IR is emitted with the right `usize` width.                                                           |
| 🏎️ 64-bit users              | Nothing to do.                                                                                                                                                                                            |

## 🔗 Related

- [Getting Started](GETTING_STARTED.md) - build instructions for all platforms
- [LLVM Default Backend](LLVM_DEFAULT_BACKEND.md) - LLVM backend deep-dive
- [Native Backend](NATIVE_BACKEND.md) - the machine-code generator
- [Releases](https://github.com/SalamLang/Salam/releases) - pre-built binaries for all supported platforms
