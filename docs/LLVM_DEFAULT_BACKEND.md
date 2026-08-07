# Making LLVM the default backend

Status: in progress. This document records what `salam build` does today, what
blocks LLVM from becoming the default, and the plan for each remaining piece.

## Why this is wanted

Two goals, and they are separable:

1. **Better code.** tcc has no optimizer. Measured on a 30M-iteration integer
   loop: tcc compiles in 4.3s and runs in 0.354s; gcc `-O2` compiles in 12.0s
   and runs in 0.139s. LLVM `-O2` lands in the same class as gcc.
2. **No toolchain on the end user's machine.** Today a native `salam build`
   shells out to a system compiler. The goal is a single `salam` binary that
   compiles and links with nothing else installed.

Goal 2 is the harder and more valuable one, and most of its machinery already
exists (see "Self-contained toolchain" below).

## What `salam build` does today

| invocation                         | path                                        |
| ---------------------------------- | ------------------------------------------- |
| `salam build app.salam`            | Salam -> C -> bundled tcc                   |
| `salam build app.salam --cc=gcc`   | Salam -> C -> system gcc, auto `-O2`        |
| `salam build app.salam --target=T` | Salam -> LLVM IR -> object -> lld           |
| `salam llvm app.salam ...`         | Salam -> LLVM IR (explicit, all emit modes) |

The default comes from [`cli.c:38`](../c/src/cli/cli.c#L38) (`out->cc = "tcc"`),
and the cross path is chosen at
[`build.c:489`](../c/src/driver/build.c#L489). tcc is resolved to a _bundled_
copy, which is why the default build works on a clean machine.

## Backend coverage: the actual blocker

LLVM cannot be the default until it compiles what the C backend compiles.
Measured by running `salam llvm` over all 802 files in `tests/en`:

| sweep           | OK  | codegen FAIL | expected-error tests |
| --------------- | --- | ------------ | -------------------- |
| baseline        | 512 | 162          | 127                  |
| after this work | 587 | 87           | 127                  |

Per-category, baseline -> now:

| gap                                      | before | after |
| ---------------------------------------- | ------ | ----- |
| `member X of non-struct/unknown type`    | 405    | 182   |
| `address of an unknown identifier`       | 249    | 10    |
| `method X on type X`                     | 184    | 4     |
| `call to unknown/unsupported function`   | 150    | 24    |
| layout (`LayoutBlock`/`LayoutComponent`) | 50     | 50    |
| `struct literal of unknown type`         | 32     | 32    |

Note `tests/en/errors/` accounts for most of the 127 "expected-error" files;
those are supposed to fail and are not a gap.

### Fixed in this pass

All of these were dispatch gaps, not missing runtime. The stdlib declares its
runtime entry points as Salam `extern:` blocks that carry a body
(`std/fs/fs.salam`'s `salam_file_read`, `std/text/text.salam`'s
`salam_str_split`, ...), and `ll_toplevel` already emits those. Only the
lowering from surface syntax down to them was absent.

- **`File*` methods** (`read`/`readline`/`write`/`seek`/`close`) - 129 errors.
  New `ll_call_file()`, mirroring the C backend's `call_file()`.
- **`str.split`** - 51 errors. Returns `Vector<str>`, built as
  `{ ptr, i32, i32 }` matching `std/collections/vector.salam`.
- **Builtin functions** - 150 errors: `char_code`, `funcptr`, `spawn`,
  `listdir`, `args`, `input`, `lang`, `callhandler`, plus a generic fallback
  through `salam_builtin_lookup()` so the shared `k_builtins` table
  (`join` -> `salam_thread_join`, `strcmp`) reaches both backends from one
  edit.
- **Package-qualified constants** (`dns.TypeA`) - new `ll_pkg_value()`. The
  key detail is that the package must be _touched_ first; `ll_touch_pkg`
  emits its globals and only then can `ll_global_find` see them.

Two reusable helpers came out of this and should be preferred for any further
builtin work:

- `ll_runtime_fn(ll, name, &owner)` - find a `salam_*` runtime builtin in any
  loaded package.
- `ll_call_runtime(ll, n, name, recv, &out)` - call it, with the receiver as
  parameter 0 for method-shaped builtins.

### Still open

Ordered by error count from the post-fix sweep.

1. **Package-qualified struct types** (`opencv.Color {}`, `tcp.Conn`,
   `shell.CmdResult`). The type string arrives already package-mangled
   (`opencv_Color`), but `ll_struct_sym` cannot resolve it because the owning
   package was never touched: a program that uses only a package's _types_,
   never its functions, never triggers `ll_touch_pkg`. Fix: resolve the
   package prefix in `ll_struct_sym`/`ll_struct_lit` and touch it, the same
   way `ll_pkg_value` now does for constants.

2. **Layout DSL** (`LayoutBlock`, `LayoutComponent`) - 50 errors. This is the
   HTML/UI subsystem and has its own generator (`c/src/layout/`). It is not
   reached by `salam build` for ordinary programs; decide explicitly whether
   the LLVM backend should support it or whether `layout-build` stays on the
   C backend.

3. **Struct field holding a function value called as a method**
   (`op.fn(...)` where `fn` is a `func(...)` field) - small, mechanical.

4. **Cross-package name collisions.** `ll_emit_globals` names globals
   `@g.<bare name>` and `ll_global_find` matches on the bare name, so two
   packages exporting the same constant name silently resolve to whichever
   was touched first. The C backend has the same class of bug at link time
   (recorded in the project notes). Both want package-qualified symbol names.

## Self-contained toolchain

This is further along than the codegen work. The pieces:

| piece                                      | flag                    | state              |
| ------------------------------------------ | ----------------------- | ------------------ |
| LLVM codegen in-process                    | `WITH_LLVM=1`           | works              |
| lld in-process (ELF/MinGW/COFF/MachO/Wasm) | `WITH_LLD=1`            | works              |
| static LLVM/lld archives                   | `LLVM_LINK_STATIC=1`    | works              |
| embedded musl sysroot                      | `SALAM_EMBED_MUSL*_DIR` | works (cross)      |
| embedded mingw sysroot                     | `SALAM_EMBED_MINGW_DIR` | works (cross)      |
| embedded third-party libs                  | `SALAM_EMBED_*LIBS_DIR` | works              |
| **native build uses in-process lld**       | -                       | added in this pass |

`make -C c self-contained` wires the cross-compile half together. Verified on
this Windows host against MSYS2's LLVM 22.1.8: a 217 MB `salam.exe` that runs
`--jit` and `-O2 --emit-obj` entirely in-process, with no WSL or clang
shell-out.

### What was missing for native builds

`link_executable()` in `llvm_native.c` shells out to `gcc`/`clang`/`cc`/`tcc`
for any build without `--target`, and `lld_can_link()` returns 0 when the
triple is NULL. So the _cross_ path was self-contained while the _native_
path, the one most users hit, still required a system compiler.

Added: `native_host_link_triple()`, which resolves a host triple that the
in-process linker can accept.

- Windows mingw hosts report `x86_64-w64-windows-gnu`, which the MinGW flavor
  takes as-is.
- Linux hosts report `*-linux-gnu`. Statically linking glibc is not something
  glibc supports cleanly (NSS dlopens at runtime), so the native path retargets
  the embedded musl sysroot and emits a fully static binary. Only when a musl
  sysroot is actually embedded, so a build without one still falls back rather
  than failing at link time.
- Windows MSVC hosts (`*-pc-windows-msvc`) need the COFF flavor and MSVC's own
  libraries; left to the fallback deliberately.
- macOS is absent by design. Linking Mach-O needs libSystem from the installed
  SDK, which cannot be embedded and redistributed. macOS keeps using the
  system toolchain, and that should be stated plainly in release notes rather
  than papered over.

### Remaining work

- **Release builds must set the embed flags.** `SALAM_EMBED_MINGW_DIR` etc.
  are opt-in; a binary built without them still reports "no mingw sysroot".
  Note that changing only the flag does not rebuild `llvm_native.o` - the
  Makefile has no dependency on CPPFLAGS, so release jobs must build clean.
- **Windows sysroot staging.** `make stage-sysroots` reads Linux paths
  (`/usr/x86_64-w64-mingw32`). A Windows host needs the MSYS2 equivalents
  (`/mingw64/lib` plus `/mingw64/lib/gcc/x86_64-w64-mingw32/<ver>`).
- **Binary size.** 217 MB static. `LLVM_COMPONENTS := all` is pulled in
  because `WITH_LLD=1` needs it; trimming to the targets actually shipped is
  the obvious lever if size matters.

## Flipping the default

Applied. `driver_build()` now selects the backend as:

```
LLVM, unless
  - --backend=c was passed, or
  - --cc=PATH was passed (naming a C compiler can only mean the C backend), or
  - this salam was built without WITH_LLVM (nothing to switch to)
```

`--backend=llvm|c` overrides in both directions, and the C backend stays a
supported path rather than being deleted.

### Why this is safe before backend parity is finished

87 of 802 test files still hit an unsupported construct. Flipping the default
naively would regress every one of them, so the switch is paired with an
automatic fallback:

- `driver_llvm()` returns a distinct `SALAM_RC_LLVM_UNSUPPORTED` (7) for
  "module emitted but incomplete", separate from real failures.
- `driver_build()` retries on the C backend for exactly that code. Any other
  non-zero code (bad source, failed link) is reported as-is and never
  retried, so a genuine mistake is not hidden behind a second compile.
- `--backend=llvm` makes it a hard error instead, for CI and for closing the
  remaining gaps.

The user sees a warning naming the unsupported construct, then a successful
build. Nothing that compiled before stops compiling.

### Verification

`tests/en/llvm/` (73 programs with expected-output files), built and _run_,
output compared byte-for-byte:

| backend            | pass | wrong output | build failures |
| ------------------ | ---- | ------------ | -------------- |
| LLVM (new default) | 73   | 0            | 0              |
| C (`--backend=c`)  | 73   | 0            | 0              |

The LLVM runs went through in-process lld and the embedded mingw sysroot, with
no external toolchain involved.

### Remaining order of work

1. Close package-qualified struct types (the largest remaining bucket, 182).
2. Decide the layout-DSL question (50).
3. Get all of `tests/en` to parity, then gate it in `run-tests.sh` so the
   fallback path becomes an error rather than a safety net.
4. Consider making `--release` imply `-O2` through LLVM.
5. Port everything to `compiler/llvm.salam` (see parity note).

## Parity note

`compiler/llvm.salam` is the self-hosted counterpart of
`c/src/llvm/codegen_llvm*.c` and mirrors the same dispatch structure
(`str.Equals(m, "concat")` where the C reads `!strcmp(m, "concat")`). It has
the same gaps, and every fix above needs porting there before the self-hosted
compiler can follow.
