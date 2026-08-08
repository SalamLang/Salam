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
| after this work | 675 | **0**        | 127                  |

The 2 remaining are `tests/en/games/pacman/src/{ui,game}.salam`, which are
_library modules_ of a web app - `ui.salam` has no `main` at all. Compiling
them standalone fails identically on the C backend ("no entry point"), and
the real entry point (`index.salam`) builds fine via `salam web`. So they are
not a backend gap: **the LLVM backend now compiles everything the C backend
compiles.**

Per-category, baseline -> now:

| gap                                      | before | after |
| ---------------------------------------- | ------ | ----- |
| `member X of non-struct/unknown type`    | 405    | 0     |
| `address of an unknown identifier`       | 249    | 0     |
| `method X on type X`                     | 184    | 0     |
| `call to unknown/unsupported function`   | 150    | 0     |
| layout (`LayoutBlock`/`LayoutComponent`) | 50     | 0     |
| `struct literal of unknown type`         | 32     | 0     |

Every category is at zero.

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
- **Built-in functions** - 150 errors: `char_code`, `funcptr`, `spawn`,
  `listdir`, `args`, `input`, `lang`, `callhandler`, plus a generic fallback
  through `salam_builtin_lookup()` so the shared `k_builtins` table
  (`join` -> `salam_thread_join`, `strcmp`) reaches both backends from one
  edit.
- **Package-qualified constants** (`dns.TypeA`) - new `ll_pkg_value()`. The
  key detail is that the package must be _touched_ first; `ll_touch_pkg`
  emits its globals and only then can `ll_global_find` see them.

Two reusable helpers came out of this and should be preferred for any further
built-in work:

- `ll_runtime_fn(ll, name, &owner)` - find a `salam_*` runtime built-in in any
  loaded package.
- `ll_call_runtime(ll, n, name, recv, &out)` - call it, with the receiver as
  parameter 0 for method-shaped built-ins.

- **Package-qualified struct types** (`opencv.Color {}`, `tcp.Conn`,
  `rawsock.Socket`) - 437 errors across two categories, now zero. The
  mangled name (`rawsock_Socket`) was resolved by splitting on `_` and
  looking the tail up by _declared_ name, which silently picked the wrong
  type whenever two packages declare the same one - std/net has both a
  `udp.Socket` and a `rawsock.Socket`, and only one wins the bare name in
  the global scope. `ll_sym_qualified` now falls back to scanning for the
  symbol whose _mangled_ name matches, which is unique by construction, and
  touches the owning package - which is also what makes a program that uses
  only a package's types get that package's struct layouts emitted.
- **Package/function name collision** (`std/time` is `package time` _and_
  declares `extern func time(...)`). The global scope holds the package, so
  a bare `time(null)` inside that package resolved to `SYM_PACKAGE` and was
  reported as an unknown function. `ll_call_user` now prefers a function of
  that name from the enclosing package scope.
- **Function-typed struct fields called as methods** (`o.fn(6, 7)`) - no
  `SYM_METHOD` lookup could ever match, so it fell through to the error.
  Routed to `ll_call_indirect`.

### Also fixed (second pass)

- **Raw C function pointers** (`externfunc(...)`, from
  `x as extern func (i32, i32) i32` and std/webview's COM vtable slot
  casts). The LLVM backend had no case for them at all. Unlike a `func(...)`
  closure - env pointer whose first word is the code pointer - an
  externfunc _is_ the code pointer and takes no hidden argument, so it needs
  its own lowering (`ll_call_raw_ptr`), dispatched first exactly as
  `cg_call` has always done. Also taught `ll_func_ret`/`ll_func_params` the
  second prefix. Fixed 4 files.
- **Layout declarations are skipped, not an error.** `AST_LAYOUT_BLOCK` /
  `AST_LAYOUT_COMPONENT` are the HTML/UI DSL, lowered by `c/src/layout/` for
  `layout-build`/`web` and never by a code generator - the C backend emits
  "layout block omitted in general (C) mode" and drops them. Reporting them
  as unsupported made 50 files look like backend gaps that the C backend
  does not compile either. Fixed 42 files.
- **`d.free()` on an interface value** releases the box, not a vtable
  method, so no slot could ever match ("interface 'Shape' has no method
  'free'").
- **Assignment as an expression** (`a = b = c`): store, then reload the
  target so a narrowing target yields what a subsequent read would.
- **Package name shadowed by an extern of the same name.** `ll_sym` searches
  the global scope first, so `time.FormatDate(...)` could resolve `time` to
  std/time's `extern func time(...)` rather than the package, reported as a
  method on type `<null>`. Whether it happened depended on which other
  packages a program pulled in, which is why it surfaced in std/excel and
  not in a two-line test. Now falls back to a package lookup by name, but
  only when the receiver is not a value in scope, so a local that
  legitimately shadows a package name still wins.

### Also fixed: lambda capturing a function value it calls

The last failures, and a **shared front-end bug**, not a backend one. Captures
were recorded only on the `AST_IDENTIFIER` expression path, and a call's
callee never goes through it - so in

```salam
func wire(d: int, dir: func (int)):
    apply((x: int): dir(x) end, d)
end
```

`dir` was left out of the lambda's capture list entirely. Both backends then
failed to find it (`address of an unknown identifier 'dir'` /
`'dir' undeclared`). Passing the same value along _without calling it_
worked, which is what made it look target-specific for so long. Fixed in
`sema_call.c` and `compiler/semantic.salam` by recording the capture on the
indirect-call path with the same condition the expression path uses.

### Still open

**Cross-package name collisions.** `ll_emit_globals` names globals
`@g.<bare name>` and `ll_global_find` matches on the bare name, so two
packages exporting the same constant name silently resolve to whichever was
touched first. The C backend has the same class of bug at link time. Both
want package-qualified symbol names. No test currently trips it.

## Self-contained toolchain

Three separate questions, often conflated:

1. **Does a user compiling a Salam program need a toolchain?** No. This is
   the one that matters for "everybody can use salam", and it is done.
2. **Does someone _building the C compiler from source_ need LLVM?** No -
   it is used when present and skipped when absent.
3. **Can Salam code itself drive static LLVM/lld?** Yes - `import llvm` now
   links and runs. What is left is wiring the self-hosted compiler's
   `NativeRun` to use it. See "The self-hosted compiler" below.

The pieces:

| piece                                      | flag                    | state              |
| ------------------------------------------ | ----------------------- | ------------------ |
| LLVM codegen in-process                    | `WITH_LLVM=1`           | works, now default |
| lld in-process (ELF/MinGW/COFF/MachO/Wasm) | `WITH_LLD=1`            | works, now default |
| static LLVM/lld archives                   | `LLVM_LINK_STATIC=1`    | works, now default |
| embedded musl sysroot                      | `SALAM_EMBED_MUSL*_DIR` | works              |
| embedded mingw sysroot                     | `SALAM_EMBED_MINGW_DIR` | works              |
| embedded third-party libs                  | `SALAM_EMBED_*LIBS_DIR` | works              |
| native build uses in-process lld           | -                       | added              |
| merged `libsalam_llvm.a` for self-host     | `make libsalam-llvm`    | added              |

### Now on by default

A plain `make -C c` previously produced a salam _without_ LLVM; the
self-contained build was opt-in, so the shipped default was the one that
needs a compiler on the end user's machine. That is inverted:

```make
LLVM_CONFIG_OK := $(shell $(LLVM_CONFIG) --version >/dev/null 2>&1 && echo 1)
LLD_STATIC_OK  := $(if $(wildcard $(LLVM_LIBDIR_PROBE)/liblldCommon.a),1)
WITH_LLVM      ?= $(if $(LLVM_CONFIG_OK),1,0)
WITH_LLD       ?= $(if $(LLD_STATIC_OK),1,0)
```

lld is probed separately because `llvm-config` knows nothing about it. A host
missing either still builds, just without the LLVM path - a capability
upgrade, not a new hard build dependency. `WITH_LLVM=0` opts out.

### The stale-flags trap

`CPPFLAGS` is assembled from the `WITH_*`/`SALAM_EMBED_*` switches, but the
compile rules depended only on sources. Adding `SALAM_EMBED_MINGW_DIR` to an
existing build tree therefore rebuilt _nothing_: `llvm_native.o` kept its old
`#ifdef SALAM_HAVE_EMBED_MINGW`-less body, and the resulting salam reported
"no mingw sysroot" while carrying a 16 MB sysroot inside it. This cost real
debugging time here and would silently ship broken releases.

Fixed with a `$(CONFIG_STAMP)` file that records the flag set and that every
object depends on; it is rewritten only when the flags actually change, so
unrelated rebuilds stay incremental. Verified: same flags -> no rebuild,
adding an embed dir -> recompile with the new define.

### stage-sysroots beyond Linux

The recipe hardcoded Debian paths (`/usr/x86_64-w64-mingw32/lib`,
`/usr/lib/gcc/...`), so it only ever worked on a Linux cross-build host. It
now searches MSYS2 layouts (`/mingw64/lib`, `/clang64/lib`, `/ucrt64/lib`)
too, stages the extra import libraries real programs need (ws2*32, crypt32,
bcrypt, iphlpapi, ...) rather than only the bare CRT, and \_skips* a missing
sysroot instead of aborting - so a Windows host can stage mingw without
having musl, and vice versa. Verified on this host: mingw staged from
`/mingw64/lib`, musl skipped cleanly.

### Verified end-to-end

On this Windows host against MSYS2's LLVM 22.1.8, with `gcc`, `clang`, `tcc`
and `ld` all removed from `PATH`:

```
gcc on PATH?  no      clang on PATH? no
tcc on PATH?  no      link/ld?       no
[DRIVER][INFO] using embedded mingw sysroot: ...
[DRIVER][INFO] in-process lld: linking hello.exe.o -> hello.exe (x86_64-w64-windows-gnu)
--- running produced binary ---
12
1
```

Compiled at `-O2` and linked with nothing installed.

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

- **Release jobs must run `make self-contained`,** not plain `make`. The
  LLVM/lld half is now automatic, but the _embedded sysroots_ still come
  from `stage-sysroots`, and without them a cross target reports "no mingw
  sysroot". The stale-flags fix above means this no longer silently
  half-works, but the release workflow still has to ask for it.
- **macOS.** Deliberately not self-contained. Linking Mach-O requires
  libSystem from the installed SDK, which cannot legally be embedded and
  redistributed, so macOS uses the system linker. This is a licensing
  boundary, not a missing feature, and belongs in the release notes rather
  than being papered over.
- **Binary size.** 217 MB static (417 MB for the merged archive).
  `LLVM_COMPONENTS := all` is pulled in because `WITH_LLD=1` needs it;
  trimming to the targets actually shipped is the obvious lever.

## The self-hosted compiler

The goal is that `compiler/*.salam` gets the same in-process LLVM the C
compiler has. Two things blocked it; both are now resolved, and what remains
is pipeline work rather than a design problem.

### Blocker 1: gating the import (resolved - it already worked)

`compiler/llvm.salam`'s header says std/llvm is deliberately not imported
because "condcomp.salam's flag set is a fixed list of 20 OS/arch bools -
there is no user-definable flag to gate an opt-in the way the C's WITH_LLVM
does". That is out of date. Both compilers already accept `-D`:
`cc_push_define` in `condcomp_table.c` and in `compiler/condcomp.salam`, plus
`-DNAME` / `--define=NAME` in the CLI.

Conditional imports work, verified directly:

```salam
if SALAM_HAVE_LLVM:
    import llvm
end
```

Without `-D` the import is pruned; with `-DSALAM_HAVE_LLVM` it is included.
So `NativeRun` can prefer std/llvm when built with the flag and fall back to
the shell-out path otherwise, exactly mirroring the C's `#ifdef`.

### Blocker 2: the LLVM component graph (resolved)

`std/llvm/llvm.salam` said `link static "LLVM"`, which assumes one
monolithic `libLLVM.a`. Its own `TODO(build-pipeline)` recorded the real
problem: LLVM ships ~200 version-dependent component archives, and the C
build resolves them by shelling out to `llvm-config --libs` - something a
`.salam` source file cannot do, since `link` only takes a literal name.

`make -C c libsalam-llvm` now merges the entire graph into one archive via
an `ar -M` script: every archive `llvm-config --libs all` resolves (207 on
LLVM 22), the six lld component archives, salam's own `llvm_native.o` and
`lld_link.o` shims, and on Windows `win_lld_demangle_shim.o`'s `__imp_`
aliases. Verified to define `salam_llvm_native`, `salam_lld_link`,
`LLVMContextCreate`, `LLVMRunPasses` and the ORC entry points.

`std/llvm` now names that one archive plus the host system libraries, which
are not LLVM's to merge:

```salam
link static "salam_llvm"
if SALAM_OS_WINDOWS:
    link dynamic "psapi"  ...  link static "z"  ...
else:
    link dynamic "pthread"  ...
end
```

`make install` places it in `$(PREFIX)/lib` so the bare name resolves.

### Verified: a Salam program with LLVM statically inside it

`--libpath=DIR` (repeatable) was added so an archive that is not installed
yet can still be linked, and threaded through all three link paths
(`link_executable`, `native_link_elf`, `native_link_mingw`). With it, this
program:

```salam
package main
import llvm
func main:
    if llvm.LldAvailable(): println "lld: in-process" end
    ret 0
end
```

builds and runs with no toolchain on `PATH`:

```
$ salam build lvlink.salam --libpath=<build> --libpath=C:/msys64/mingw64/lib
$ ./lvlink.exe
lld: in-process
```

137 MB, statically linked, self-contained. That is the Salam side of the
goal closed: `import llvm` works from ordinary Salam code.

Getting there needed three link-order corrections in `std/llvm`, all of the
"static linking only resolves from archives not yet scanned past" kind:
libstdc++ and libgcc_eh after `libsalam_llvm.a`, and winpthread after those
(mingw's libstdc++ threading layer sits on it).

### Wiring `NativeRun` to std/llvm

Done, with two real blockers resolved on the way. Both were name clashes
between the self-hosted backend and the std package:

- **Package name.** Both were `package llvm`. Sema binds two same-named
  packages to one symbol, so whichever loses is silently unreachable -
  "package function 'LldAvailable' not found". `compiler/llvm.salam` is now
  `package llvmgen`.
- **Filename.** A .salam file's generated C module is keyed by _filename_,
  not package, so `compiler/llvm.salam` and `std/llvm/llvm.salam` both
  produced `salam_mod_llvm.c` and clobbered each other, leaving the
  compiler's own types undeclared at their use sites ("unknown type size").
  The file is now `compiler/llvmgen.salam`. Both importers alias it, so the
  rename is invisible to callers.

The gate itself lives in a third package, `compiler/llvm_bridge.salam`,
because the flag can only be tested at _top level_: a `-D` name that was not
passed is absent from the symbol table, so referencing `SALAM_HAVE_LLVM`
inside a function body is an "unknown identifier" error rather than a false.
The bridge keeps every reference in one top-level `if SALAM_HAVE_LLVM:`
block with an `else` arm supplying identical signatures, and exposes a
primitive-typed API so the gated `std/llvm` types never leak into ungated
modules. `NativeAvailable`/`NativeRun` consult it first and fall through to
the shell-out toolchain when it reports unavailable.

Verified: the self-hosted compiler builds and passes 73/73 with the flag
off, and `salam build ... -DSALAM_HAVE_LLVM` gets all the way through
codegen and per-module compilation with the bridge active.

### Not yet verified

Linking the self-hosted compiler _against_ `libsalam_llvm.a` does not
complete on this Windows host: gcc exits non-zero with no diagnostic while
linking against the 417 MB archive, consistent with resource exhaustion
rather than a code defect (every individual module compiles clean, and the
same archive links fine into a small Salam program - see the `import llvm`
result above). Worth retrying on a machine with more headroom, and the link
command should be logged the way the in-process lld path now logs its argv.

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

## Bugs found along the way

Both surfaced only once std/llvm was compiled into a real program, and both
affect **both** backends.

### 1. `defer` on a binding declared inside a loop body (open)

`SKILL.md` documents defer as running at _scope_ exit. Both backends emit
every defer at _function_ exit (`cg->fn_defers`, `ll->defers`), so a defer on
a loop-local binding lands in the function epilogue where that binding is out
of scope. 20-line repro:

```salam
repeat bases.len() with bi:
    entries := os.ListDir(bases.get(bi)[0])
    defer entries.free()          // <- epilogue cannot see `entries`
    ...
end
```

- C backend: `error: 'entries' undeclared`
- LLVM backend: `address of an unknown identifier 'entries'`

This also accounts for part of the remaining `address of an unknown
identifier` bucket in the sweep. Not fixed here: making defers block-scoped
is a language-semantics change with stdlib-wide blast radius and wants its
own change with tests. `std/llvm/linker.salam` was rewritten to free
explicitly instead, with a comment pointing here.

### 2. `funcptr()` on an extern mangled the symbol (fixed)

`funcptr(printf)` in `std/llvm/orc.salam` emitted
`_Salam_llvm_printf_str` - a name that exists nowhere - because the C
backend mangled unconditionally when it resolved the symbol. An extern keeps
its declared C name. Fixed in `codegen_call.c` and `compiler/codegen.salam`.
The LLVM backend's own `funcptr` lowering, written in this pass, already had
the extern check.

## Parity: the self-hosted compiler

`compiler/llvm.salam` is the self-hosted counterpart of
`c/src/llvm/codegen_llvm*.c` and mirrors the same dispatch structure
(`str.Equals(m, "concat")` where the C reads `!strcmp(m, "concat")`).
Everything above is now ported, and the two compilers agree exactly.

Verified by running the same 802-file sweep with both:

| compiler    | OK  | codegen FAIL | expected-error |
| ----------- | --- | ------------ | -------------- |
| C           | 675 | 0            | 127            |
| self-hosted | 675 | 0            | 127            |

Zero differing files. The self-hosted compiler also passes 73/73 on
`tests/en/llvm/` built and run with output compared byte-for-byte.

### What was ported

| C                                                                                           | self-hosted                                |
| ------------------------------------------------------------------------------------------- | ------------------------------------------ |
| `ll_runtime_fn` / `ll_call_runtime`                                                         | `runtime_fn` / `call_runtime`              |
| `ll_call_file`                                                                              | `call_file`                                |
| `ll_call_vec_str` (split/args/listdir)                                                      | `call_vec_str`                             |
| `ll_pkg_value`                                                                              | `pkg_value`                                |
| built-ins (`char_code`, `funcptr`, `spawn`, `input`, `lang`, `callhandler`, table fallback) | same, in `call_intrinsic`                  |
| `ll_sym_qualified` mangled-name scan                                                        | `sym_qualified` + `scan_mangled`           |
| `ll_call_user` package/function preference                                                  | `call_user`                                |
| func-typed struct field -> indirect call                                                    | `call_method`                              |
| `SALAM_RC_LLVM_UNSUPPORTED` + C fallback                                                    | `RC_LLVM_UNSUPPORTED` + `use_llvm_backend` |
| `--backend=llvm\|c`, `--cc=` implies `c`                                                    | same, in `cli.salam`                       |
| `--libpath=DIR`                                                                             | same, threaded through `Opts.lib_paths`    |
| stray `.ll` cleanup on fallback                                                             | same                                       |
| native build must not report "cross-compilation failed"                                     | same                                       |

Two C-side items have no self-hosted counterpart _yet_, both for the same
reason - `NativeRun` still uses the shell-out toolchain path rather than
`std/llvm`:

- `native_host_link_triple` (in-process lld for a native build). The
  self-hosted equivalent already exists as `std/llvm/linker.salam`'s
  `LinkAuto`; it just is not called from `NativeRun`.
- The lld argv debug log.

Wiring `NativeRun` to prefer `std/llvm` under `-DSALAM_HAVE_LLVM` closes
both, and is the single remaining step for full self-hosted
self-containment.
