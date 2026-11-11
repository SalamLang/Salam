---
title: Cross-compilation in Salam: building for Linux, macOS, Windows and ARM from one machine
seo_title: Cross-Compilation in Salam: Linux, macOS, Windows and ARM
description: Cross-compile Salam programs for Windows, macOS, Linux, ARM and 32-bit x86 from one machine: the --target flag, the backends behind it and what needs a sysroot.
keywords: cross compilation, cross-compile Windows Linux macOS, Salam cross compile, cross-compile ARM, musl static binary, 32-bit cross compilation, universal macOS binary, Windows on Arm, target triple, Salam --target
category: compiler
summary: The --target flag, the three code generators behind it, the sysroots and linker built into the compiler, and an honest list of which targets are run in CI and which are only built.
date: 2026-11-11
---

Cross-compilation means building an executable on one kind of machine that runs on another: a Windows `.exe` built on Linux, an ARM binary built on an x86-64 laptop, a macOS program built in a Linux CI job. In Salam you ask for it with one flag, `--target`, followed by a target triple such as `x86_64-w64-windows-gnu` or `aarch64-linux-musl`. The compiler picks a code generator, finds a C runtime for that target, links the result and hands you a file for the other machine.

This article explains what happens behind that flag, which targets work out of the box, which need extra files, how to write code that compiles cleanly for every platform, and where the project's own testing stops.

## What cross-compilation is, and why it matters {#what}

A compiler that only builds for the machine it runs on forces you to own every platform you ship to, or to rent CI runners for each one. A cross-compiler removes that constraint. You keep one development machine and one build job, and you produce the Windows, macOS and Linux artifacts from it.

Three things have to be right for this to work:

1. **Code generation** for the target CPU: x86-64, 32-bit x86, AArch64, 32-bit ARM, RISC-V or WebAssembly.
2. **A C runtime and system libraries** for the target operating system, often called a sysroot: musl or glibc on Linux, the mingw-w64 runtime on Windows, `libSystem` on macOS.
3. **A linker** that understands the target's object and executable formats: ELF, PE/COFF or Mach-O.

Go is well known for making this easy with `GOOS` and `GOARCH`, and Zig ships C runtimes for many targets inside its toolchain. Rust can target most platforms but usually needs `rustup target add` plus a linker and sysroot for the other platform. Salam sits closer to the Go and Zig end: the release build of the compiler carries its own linker and the most common sysroots. (The comparisons with [Go](/blog/salam-vs-go/) and [Zig](/blog/salam-vs-zig/) cover the rest of those languages.)

## The --target flag and target triples {#target}

A target triple names the CPU, the vendor or OS family, and the ABI. These are the ones you will use most often with Salam:

| Triple | Produces |
|---|---|
| `x86_64-linux-musl` | static 64-bit Linux executable |
| `aarch64-linux-musl` | static 64-bit ARM Linux executable (Raspberry Pi 4 and 5, ARM servers) |
| `arm-linux-musleabihf` | static 32-bit ARM Linux executable (ARMv7, hard float) |
| `i686-linux-musl` | static 32-bit x86 Linux executable |
| `x86_64-w64-windows-gnu` | 64-bit Windows `.exe` |
| `i686-w64-windows-gnu` | 32-bit Windows `.exe` |
| `aarch64-w64-windows-gnu` | Windows on Arm `.exe` |
| `arm64-apple-macos`, `x86_64-apple-macos` | macOS executable for Apple silicon or Intel |
| `universal-apple-macos` | one macOS file holding both architectures |
| `wasm32-wasi` | WebAssembly module for a WASI runtime (see [Salam and WebAssembly](/blog/salam-webassembly-and-javascript/)) |

The command is the same for all of them:

```bash
salam build app.salam --target=x86_64-w64-windows-gnu
salam build app.salam --target=aarch64-linux-musl --output=app-arm64
```

When the target is Windows, the default output name gets `.exe` added. Triples for bare-metal targets (`x86_64-unknown-none`, `i686-unknown-none`, `aarch64-unknown-none`) build freestanding programs with no operating system; those have their own article, [freestanding Salam](/blog/freestanding-salam-bare-metal/).

## Which code generator does the work {#backends}

Salam has three backends, and the article on [code generation](/blog/salam-code-generation-c-llvm-native/) covers them in depth. For cross-compilation they behave differently:

- **LLVM** (`--backend=llvm`, and the path `--target` takes by default for most triples) handles any triple LLVM knows. It is pointer-width aware: for a 32-bit triple every `size_t` position in the generated IR becomes 32 bits, as described in `docs/PLATFORMS.md`. The release builds for Linux x86-64, macOS and 64-bit Windows embed LLVM and the LLD linker and run them in-process. A compiler built from source without the embedded LLVM falls back to `clang`, `llc` and `lld` on your `PATH`.
- **Native** (`--backend=native`) is Salam's own machine-code generator. According to `docs/NATIVE_BACKEND.md` it writes x86-64, 32-bit x86, AArch64, 32-bit ARM (ARMv7-A with VFPv3-D16) and RISC-V 64 code, as ELF, COFF or Mach-O objects, and refuses any other target with a pointer to `--backend=llvm`. It needs neither LLVM nor a C compiler.
- **C** (`--backend=c`) emits portable C and lets a C compiler resolve every width. It is the path `docs/PLATFORMS.md` recommends for production 32-bit builds, and `--emit-c` gives you the generated C to compile with any cross C compiler you like. The browser build of the compiler itself is made this way: it is emitted as C for `wasm32-unknown-emscripten` and compiled with Emscripten.

> [!NOTE]
> Older lines in `docs/PLATFORMS.md` and in `salam --help` still describe the native backend as x86-64 only. `docs/NATIVE_BACKEND.md` and `docs/NATIVE_LINKER.md` are the current references: the backend has since gained i686, armhf, RISC-V and AArch64 code generators.

## Sysroots and the built-in linker {#sysroots}

The part of cross-compilation that usually hurts is not code generation but linking: finding `crt1.o`, `libc.a` and the right import libraries for a platform you do not have. Salam's release build for Linux x86-64 is built with the musl sysroot for x86-64 Linux, the mingw-w64 sysroots for 64-bit and 32-bit Windows and the WASI sysroot embedded inside the `salam` executable, and unpacks them into a cache directory the first time a build needs them. The release workflow smoke-tests this by cross-compiling to `x86_64-linux-musl` and `x86_64-w64-windows-gnu` with `PATH` pointing at a directory that does not exist, so no outside tool can be found.

The linker is written in Salam and lives inside the compiler. Its design document lists the outputs it writes:

| Target | Output | C runtime |
|---|---|---|
| `x86_64-linux-musl`, `aarch64-linux-musl`, `i686-linux-musl`, `arm-linux-musleabihf` | static ELF | bundled musl |
| `x86_64-linux-gnu`, `aarch64-linux-gnu` | dynamic ELF PIE | glibc |
| `x86_64-w64-windows-gnu`, `i686-w64-windows-gnu` | PE | mingw-w64 |
| `aarch64-w64-windows-gnu` | PE (ARM64) | llvm-mingw (UCRT) |
| `x86_64-apple-macos`, `arm64-apple-macos` | Mach-O PIE, ad-hoc signed | `libSystem` |

For glibc targets other than the host, the driver looks for a glibc sysroot under `/usr/aarch64-linux-gnu` (and the equivalents) or in `SALAM_GLIBC_SYSROOT`. Two targets need something you install yourself: Windows on Arm needs the llvm-mingw runtime (found through `aarch64-w64-mingw32-clang` on `PATH` or `SALAM_LLVM_MINGW`), and RISC-V needs a sysroot for the C library you link against.

Here is what a Linux x86-64 machine produced for one small program, using the native backend and the built-in linker:

```bash
salam build hello.salam --backend=native --target=x86_64-w64-windows-gnu --output=hello-win64
salam build hello.salam --backend=native --target=aarch64-linux-musl --output=hello-arm64
salam build hello.salam --backend=native --target=i686-linux-musl --output=hello-i686
salam build hello.salam --backend=native --target=arm64-apple-macos --output=hello-mac
salam build hello.salam --backend=native --target=universal-apple-macos --output=hello-mac-universal

file hello-*
# hello-arm64:         ELF 64-bit LSB executable, ARM aarch64, statically linked
# hello-i686:          ELF 32-bit LSB executable, Intel i386, statically linked
# hello-mac:           Mach-O 64-bit arm64 executable
# hello-mac-universal: Mach-O universal binary with 2 architectures: x86_64, arm64
# hello-win64.exe:     PE32+ executable for MS Windows (console), x86-64
```

## Linux, Windows and macOS targets {#platforms}

### Linux: static musl or dynamic glibc {#linux}

On Linux you choose between two C libraries. A `*-linux-musl` triple gives a statically linked executable with no runtime dependencies: copy it to any Linux machine with the same CPU architecture and it runs, whether that machine uses glibc, musl (Alpine) or something else. That is usually what you want for command-line tools, containers built `FROM scratch` and servers you deploy by copying a file.

A `*-linux-gnu` triple links dynamically against glibc, which you need when the program loads a system shared library that only exists as a `.so`, such as a GUI toolkit or a database client library. The native backend's i686, ARM and RISC-V code uses absolute addresses, so for those targets the dynamic executables load at a fixed address rather than being position independent; x86-64 and AArch64 builds are PIEs.

### Windows: x64, 32-bit and Windows on Arm {#windows}

`x86_64-w64-windows-gnu` and `i686-w64-windows-gnu` produce ordinary PE executables against the mingw-w64 runtime. A few options exist only for Windows targets and need `--backend=native` or `--linker=builtin`:

- `--subsystem=windows` builds a GUI program that opens no console window;
- `--icon=app.ico` embeds the program icon;
- `--manifest=FILE` embeds an application manifest;
- `--version-info=KEY=VALUE` adds a version resource (version, company, description, product, copyright).

A project can set the same values in a `[windows]` table in `salam.toml`, and `salam bundle` packages a desktop app as a zip or an NSIS installer, among other formats; the article on [desktop apps](/blog/desktop-gui-apps-with-salam/) covers that workflow. On 32-bit Windows, Win32 functions use the stdcall convention; the compiler handles this for the functions it knows, and an `extern stdcall func` type covers the ones you reach through a pointer.

On a Windows on Arm machine with no `--backend` given, the compiler uses the native backend by default, because neither LLVM nor tcc ships for that host.

### macOS and universal binaries {#macos}

The built-in linker writes Mach-O executables for Apple silicon and Intel Macs, with the ad-hoc code signature that arm64 macOS requires before it runs anything. `--target=universal-apple-macos` compiles and links the program once per architecture and joins the two into one universal file. The deployment target comes from the triple (`arm64-apple-macos13`), from `MACOSX_DEPLOYMENT_TARGET`, or defaults to macOS 12.

Building for macOS on Linux works without Apple's SDK. Without an SDK, a library or framework you ask for still gets its load command at the standard system path, and imports are bound by flat lookup, so the dynamic loader finds them in whichever loaded image exports them. With an SDK available (`SDKROOT`, or Xcode's command-line tools on a Mac), the linker reads the `.tbd` stubs and binds every import to the library that exports it.

## ARM and 32-bit targets {#arm-32bit}

"32-bit" refers to the width of pointers and `size_t`, not to the widest number you can compute with. On a 32-bit target `i64` and `u64` still work; addition and subtraction become register pairs and division calls runtime helpers. `i128` and `u128` work too, with divisions done by Salam's own helpers because 32-bit libgcc lacks them. `f64` runs on the hardware floating-point unit on all of these targets. What changes is that pointers and `usize` are 4 bytes, and that is exactly what the backends are careful about.

This program prints the same four lines whether it is built for x86-64 or for `i686-linux-musl`:

```salam
func main:
    big := 9000000000 as i64
    println "i64 is", sizeof(i64), "bytes on every target"
    println "9e9 * 3 =", big * 3
    println "9e9 / 7 =", big / 7
    wide := (big as u128) * (big as u128)
    println "u128 square:", wide
end
```

```output
i64 is 8 bytes on every target
9e9 * 3 = 27000000000
9e9 / 7 = 1285714285
u128 square: 81000000000000000000
```

Two details matter on ARM. AArch64 and RISC-V hardware return a value instead of trapping on integer division by zero, so the native backend keeps an explicit check before each division by a non-constant there, even in release builds. And the armhf target is the Debian armhf baseline (ARMv7-A, VFPv3-D16, no NEON), so the native backend's vectorizers stay off on it.

## Writing code that builds everywhere {#portable}

Most Salam code is portable without effort, because the standard library hides the operating system. When you do need a platform difference, use compile-time constants such as `SALAM_OS_WINDOWS`, `SALAM_OS_MAC`, `SALAM_OS_LINUX`, `SALAM_ARCH_ARM64` and the string forms `SALAM_OS` and `SALAM_ARCH`. A top-level `if` on them chooses declarations; branches that do not apply are removed before type checking.

```salam
if SALAM_OS_WINDOWS:
    const CONFIG_DIR := "C:\\ProgramData\\notes"
else SALAM_OS_MAC:
    const CONFIG_DIR := "/Library/Application Support/notes"
else:
    const CONFIG_DIR := "/etc/notes"
end

func main:
    println "built for", SALAM_OS, "on", SALAM_ARCH
    println "settings in", CONFIG_DIR
end
```

Built for `x86_64-w64-windows-gnu` and `i686-w64-windows-gnu` and run under Wine, it prints `built for windows on x64` and `built for windows on x86`, followed by the Windows path. Persian programs use the same flags under Persian names, such as `سلام سیستم ویندوز`.

Removing dead branches before type checking has a consequence you should know about: a mistake inside the Windows branch is invisible when you check on Linux. `salam check` reports no errors for a call to a function that does not exist inside `if SALAM_OS_WINDOWS:`, while `salam obj app.salam --target=x86_64-w64-windows-gnu` reports it immediately. `salam obj` stops at the object file, so it needs no linker or sysroot for the other platform. Run it for each target you ship in CI and you have a cheap per-platform type check. The [compile-time lesson](/learn/compile-time/) covers the full set of flags.

## How the targets are tested {#testing}

A cross-compiler is only as good as the testing of what it produces, and here a young project should be specific rather than reassuring.

- The **native backend gate** in CI cross-builds test programs and the compiler itself on Linux for macOS arm64, macOS x64, Windows x64 and Linux arm64, then runs them on real GitHub-hosted machines of each kind; the compiler built that way must reproduce the reference LLVM IR on each host. A separate job builds and runs the tests on a `windows-11-arm` runner.
- The i686 Linux and armhf builds are checked by running the main test groups natively (i686) and under `qemu-arm` (armhf), with a documented list of exceptions; the i686 Windows build is run under Wine.
- The compiler itself is published for Linux x86-64, x86-64 musl, AArch64 and i686, macOS arm64, and Windows x86-64, i686 and ARM64.
- The `docs/PLATFORM_NOTES.md` file for the networking stack separates what was run from what was only read: AArch64 under qemu and Windows under Wine were run; the macOS objects were built but not run in that round; FreeBSD, OpenBSD and NetBSD support was checked by reading headers only.

The BSDs have compile-time constants but no target that is built and run in CI, so treat them as unsupported for now.

## Limits worth knowing {#limits}

- **Shared system libraries.** A static musl build cannot load a library that only exists as a shared object on the target. Programs that [call C libraries](/blog/salam-c-interop-ffi/) such as GTK, WebKit or a database client need the target's own libraries, which usually means building on that platform or providing a full sysroot.
- **Toolchain availability.** Builds of the compiler made from source may not embed LLVM or the sysroots, so cross-compiling there needs `clang`, `lld` and the sysroots installed. Windows on Arm and RISC-V need extra runtime files even in release builds.
- **Not every combination is run.** Building is not the same as testing. When a platform matters to you, run your own tests on it, under qemu, Wine or real hardware.

## Frequently asked questions {#faq}

### Can I build a Windows .exe on Linux with Salam?

Yes. `salam build app.salam --target=x86_64-w64-windows-gnu` produces a 64-bit Windows executable, and `i686-w64-windows-gnu` a 32-bit one. Release builds of the compiler (see the [install page](/install/)) carry the mingw-w64 runtime and the linker, so nothing else needs to be installed.

### How do I make a static Linux binary?

Use a musl triple such as `x86_64-linux-musl` or `aarch64-linux-musl`. The result is statically linked and runs on any Linux machine with that CPU architecture, regardless of its C library.

### Does Salam support 32-bit platforms?

Yes. i686 Linux and Windows and 32-bit ARM Linux are supported targets, and `i64`, `u64`, `i128` and `f64` all work there. `usize` and pointers are 32 bits wide on those targets.

### Can Salam build macOS apps without a Mac?

It can build and sign macOS executables, including universal binaries, on Linux. For linking against specific frameworks with exact bindings, and for testing what you built, a Mac (or a macOS CI runner) is still the right tool.
