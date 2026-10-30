# Built-in linker

`--backend=native` writes its own object files, but turning them into an
executable still needs an outside linker: `cc` on Linux, mingw `gcc` for the
Windows target, or the lld embedded in LLVM builds of the compiler. This
document designs a linker written in Salam and shipped inside the compiler,
so that a native build needs no tool beyond `salam` itself.

## Goals

- `salam build --backend=native --target=x86_64-linux-musl` produces a static
  executable with no external program, using the musl sysroot the compiler
  already downloads (`crt1.o`, `crti.o`, `crtn.o`, `libc.a`,
  `libclang_rt.builtins-x86_64.a`).
- The result runs identically to the same objects linked by `cc -static`, on
  every test in the suite.
- Links fast: a hello world in a few milliseconds and the whole compiler in
  well under a second. Linking should never be the slow step of a native
  build.
- Errors read like compiler errors: an undefined symbol names the symbol and
  every object that wants it; a duplicate names both definitions.
- Deterministic output: the same inputs give byte-identical executables.

## Non-goals for the first phase

- Shared libraries. A program that links a system `.so` (sqlite, a database
  client, a GUI toolkit) keeps using the system linker. The driver decides per
  build (see "Driver integration").
- Linker scripts, versioned symbols, symbol interposition, `--wrap`.
- Debug information was left out of the first phase. It is supported now:
  see "Debug sections" under Writing.

## What the inputs contain

A census of the bundled musl `libc.a` (1342 members) and the CRT objects
shows a small, well-behaved surface:

| Feature | Seen |
|---|---|
| Relocations | `R_X86_64_PC32`, `PLT32`, `64`, `32`, `REX_GOTPCRELX` (about 10,000 in all) |
| Sections | `.text`, `.text.*`, `.rodata`, `.rodata.*` including `.rodata.cst*`, `.data*`, `.bss*`, `.init`/`.fini` (only in `crti.o`/`crtn.o`), notes |
| Symbols | global, local and weak (about 290 weak), no COMMON, no IFUNC, no TLS (musl keeps errno in the thread descriptor) |
| Absent | `.init_array`, `.fini_array`, `.eh_frame`, COMDAT groups |

Our own objects add `.init_array` (global initialisers and the flush hooks),
`.fini_array`, `.eh_frame`, and `R_X86_64_GOTPCRELX` for data that lives in
libc. Static user libraries (`link "foo" static`) are ordinary archives of the
same kind.

The linker supports exactly this set and rejects anything else with a clear
message naming the object, section and relocation, so a new input shape is
found by a test failure, never by a silent wrong binary.

## Pipeline

```
objects + archives
   -> read (ELF64 relocatable, ar with GNU symbol index)
   -> resolve symbols (archive members pulled in on demand)
   -> collect live sections (garbage collection from the roots)
   -> lay out output sections and segments
   -> assign symbol addresses, synthesise linker-defined symbols
   -> apply relocations (relaxing GOT loads where possible)
   -> write the executable
```

### Reading

- **ar**: the global header, then members. The `/` member is the GNU symbol
  index (big-endian count, offsets, names) and `//` holds long member names.
  Members are parsed lazily: the index says which member defines a symbol, and
  only those members are read.
- **ELF64 relocatable**: section headers, `.symtab`, `.strtab`, and the
  `.rela.*` sections. Only `ET_REL`, `EM_X86_64` and little-endian inputs are
  accepted.
- Inputs are memory-mapped or read whole; section data is referenced by
  offset, never copied until the output is written.

Our own object can skip the disk entirely: the driver can hand the linker the
in-memory sections, symbols and relocations that `nativegen` already holds.
The file path stays as the general interface for user `.o` and `.a` inputs.

### Symbol resolution

The standard Unix model, with archives scanned to a fixed point (as if every
archive were inside `--start-group`, which avoids order-dependent link
failures):

1. Add every symbol of every explicit object. A global definition clashing
   with another global definition is an error. A global replaces a weak one;
   between two weaks the first one wins.
2. While some global undefined symbol has a definition in an archive index,
   load that member and add its symbols.
3. Any undefined symbol left that is only weakly referenced resolves to 0.
   Anything else is an "undefined symbol" error listing its referencing
   objects.

Entry symbol: `_start` from `crt1.o`.

### Garbage collection

musl is built with one function per section, so collecting unreachable
sections keeps a hello world small. The roots are the entry symbol, every
`.init_array`, `.fini_array`, `.init` and `.fini` section, and `.eh_frame`
(kept whole in the first phase). A section is live if a live section has a
relocation against a symbol defined in it.

### Layout

A static, non-PIE executable based at `0x400000`, with three `PT_LOAD`
segments so that no page is both writable and executable:

| Segment | Flags | Contents |
|---|---|---|
| 1 | R | ELF and program headers, `.rodata*`, `.eh_frame`, `.eh_frame_hdr` |
| 2 | R X | `.init`, `.text*`, `.fini` |
| 3 | R W | `.init_array`, `.fini_array`, `.data*`, `.got`, `.bss*` |

Plus `PT_GNU_STACK` (non-executable stack) and `PT_GNU_EH_FRAME` for
`.eh_frame_hdr`. Segments are aligned to the page size and file
offsets are congruent to addresses modulo the page size. Input sections keep
their alignment, are placed in input order, and same-named `.rodata.cst*`
constants may be merged later as an optimisation.

`.init` and `.fini` are concatenated in link order: `crti.o`'s prologue, any
middle pieces, then `crtn.o`'s epilogue. That is the order `cc` uses.

### Linker-defined symbols

Listing every symbol that musl and the CRT objects reference but no archive
member defines gives exactly what the linker must provide:

- `__init_array_start`, `__init_array_end`, `__fini_array_start`,
  `__fini_array_end` (weak references, read by `__libc_start_init` and
  `exit`)
- `_DYNAMIC` (weak, resolves to 0 in a static link)
- `_GLOBAL_OFFSET_TABLE_` (a strong reference; it names the start of `.got`,
  which exists even when empty)

`main` comes from our object, `_init`/`_fini` from `crti.o`, and the complex
multiplication helpers (`__muldc3`, `__mulsc3`, `__mulxc3`) from
`libclang_rt.builtins-x86_64.a`, which is always passed as the last archive.
`__bss_start`, `_end` and `__ehdr_start` are defined as well, for tools and
for a future runtime that wants its own extent, but nothing in libc needs
them.

### Relocations

With `S` the symbol address, `A` the addend and `P` the address being
patched:

| Type | Value | Check |
|---|---|---|
| `R_X86_64_64` | `S + A` | none |
| `R_X86_64_32` | `S + A` | must fit in 32 unsigned bits |
| `R_X86_64_PC32`, `R_X86_64_PLT32` | `S + A - P` | must fit in 32 signed bits |
| `R_X86_64_GOTPCRELX`, `R_X86_64_REX_GOTPCRELX` | see below | |

In a static executable every symbol has a final address, so a GOT load is
relaxed in place when the instruction allows it: `mov reg, [rip + GOT(sym)]`
(opcode `8b`) becomes `lea reg, [rip + sym]` (opcode `8d`), and the
displacement becomes `S + A - P`. Forms that cannot be rewritten (`call` or
`jmp` through the GOT, or non-`mov` users) get a real 8-byte `.got` entry
holding `S`, and the relocation becomes PC-relative to that entry. This is
the same relaxation lld and GNU ld perform.

Every overflow is an error naming the symbol, the referencing section and
the offset.

### Writing

The ELF header (`ET_EXEC`, entry at `_start`), the program headers, then the
segment contents with file padding. A minimal section header table
(`.text`, `.rodata`, `.data`, `.bss`, `.symtab`, `.strtab`, `.shstrtab`) is
written so that `objdump`, `gdb` and `perf` can name functions. It can be
stripped with `--strip`.

### Debug sections

Non-allocated `.debug_*` sections are kept from every object given on the
command line that contributes at least one live section; archive members'
debug sections are dropped (library debug info is not needed to debug the
program, and toolchain libraries ship DWARF 5 forms the linker does not
rewrite). Input sections with the same name are
concatenated in input order into one output section, written after the
loaded image and before `.symtab`, with no address. A symbol in a debug
section resolves to its offset inside that output section, which is what the
`R_X86_64_32` references from `.debug_info` to `.debug_abbrev` and
`.debug_line` need (the `ABS32`/`ABS64` types of i386, ARM, AArch64 and
RISC-V are applied the same way). A relocation against code that garbage collection
removed is written as 0, the usual tombstone. `--strip` drops them along
with the section headers.

PE output places each merged `.debug_*` section after `.reloc` as a
discardable section, names it through a COFF string table that
`PointerToSymbolTable` points at, resolves `IMAGE_REL_AMD64_SECREL` to the
offset inside the merged section and `ADDR64` to the virtual address
without a base relocation.

When objects carry CodeView (`.debug$S`, and `.debug$T` holding only the
pointer, struct, field list, array, enum, procedure, argument list and
function ID records the native backend writes, which is what `-g` gives on Windows targets), the PE linker also writes `<name>.pdb`
next to the executable (`nl_pdb.salam`) and points a debug directory entry
with an `RSDS` record at it, so WinDbg, Visual Studio and
`llvm-symbolizer` find functions and lines without the DWARF. The PDB is an
MSF file with 4 KiB blocks holding:

- the info stream (GUID, age and a `/names` entry);
- the TPI stream, with every object's `.debug$T` records appended and their
  type indices shifted (in the records and in the module's symbols), plus
  its hash stream (names for structs and enums, JamCRC for the rest), which
  debuggers use to resolve forward references; the IPI stream, with the
  `LF_FUNC_ID` records split out of the same `.debug$T` sections (each
  object gets a map from its own indices to the TPI or IPI ones, applied to
  the records, to the module's symbols, including `S_INLINESITE`, and to its
  inlinee line entries);
- DBI, with module info, section contributions, a section map, file info
  and a section header stream;
- `/names`;
- one stream per object, with its symbols, their parent and end offsets
  filled in, and its C13 line and checksum subsections;
- the globals (`S_PROCREF`), publics (`S_PUB32` with an address map) and
  symbol record streams.

Relative source names are made absolute with the link directory, as `lld`
does. The GUID is a hash of the image, taken before the `RSDS` record is
filled in.

## Driver integration

A new option `--linker=auto|builtin|system`:

- `builtin` always uses the built-in linker and fails clearly on inputs it
  does not support (for example a shared library).
- `system` keeps today's behaviour (`cc`, mingw `gcc`, or embedded lld).
- `auto` (the eventual default) picks `builtin` when the target is
  `x86_64-linux-musl`, every `link` directive names a static archive or
  object, and no framework is requested; otherwise it picks `system`.

`auto` now picks `builtin` whenever every input is found: static for
x86-64 musl, a PIE for x86-64 glibc, and a PE for x86-64 Windows. It falls
back to `system` otherwise. `SALAM_LINKER` sets the default mode, which
is how the test runner forces one or the other.

## Code layout

A new package `compiler/nativelink/`, independent of `nativegen` except for
the in-memory object hand-off:

| File | Role |
|---|---|
| `nl_ar.salam` | ar archives, GNU symbol index, long names |
| `nl_elfin.salam` | ELF64 relocatable reader |
| `nl_resolve.salam` | symbol table, archive fixed point, weak rules |
| `nl_gc.salam` | section liveness |
| `nl_layout.salam` | output sections, segments, addresses, synthetic symbols |
| `nl_reloc.salam` | relocation application and GOT relaxation |
| `nl_elfout.salam` | executable writer |
| `nl_api.salam` | `Link(inputs, options) -> result` with diagnostics |

## Testing

- **Differential**: build every test of the suite with `--backend=native`
  twice, once linked by `cc -static` against the same musl files and once by
  the built-in linker. Outputs must match. Executables should also match
  section for section when GC is off, which pins layout bugs down to a
  section.
- **Structural**: `readelf -lSW` checks on segments, alignment and flags in a
  small test of its own, and `ld.so`-free execution (a static binary that
  runs at all has a correct entry, stack and relocations).
- **Negative**: undefined symbol, duplicate definition, unsupported
  relocation and overflow cases, each with an expected message.
- **Speed**: link time reported by `--log-level=info`, tracked for hello world
  and for the self-hosted compiler.

## Phase 2: dynamic executables against glibc

Status: implemented in `nl_dyn.salam` (shared libraries, linker scripts,
versions) and `nl_dynout.salam` (PLT, GOT, dynamic relocations, `.dynamic`,
`.gnu.hash`, version tables). With `--backend=native` the whole suite passes
when every program is linked this way (1309 of 1309, including the sqlite
tests that load a system `.so`), so `--linker=auto` uses it on x86-64 Linux.
`.eh_frame_hdr` is written for both static and dynamic output (see
"`.eh_frame_hdr`" below).

This is the phase that removes `cc` from ordinary Linux builds, including
programs that link system shared libraries (sqlite, database clients).

### What `cc` does today

For one of our objects, `cc -v` shows the link it delegates to `collect2`:

- output: a position-independent executable (`-pie`), with `-z now`,
  `-z relro`, `--hash-style=gnu`, `--eh-frame-hdr`, `--build-id`, and the
  interpreter `/lib64/ld-linux-x86-64.so.2`
- objects: `Scrt1.o`, `crti.o`, `crtbeginS.o`, our object, `crtendS.o`,
  `crtn.o`
- libraries: `-lm -lgcc --as-needed -lgcc_s -lc -lgcc`, where `libc.so` and
  `libm.so` are linker scripts:
  `GROUP ( libc.so.6 libc_nonshared.a AS_NEEDED ( ld-linux-x86-64.so.2 ) )`
  and `GROUP ( libm.so.6 AS_NEEDED ( libmvec.so.1 ) )`

### Output shape

A PIE (`ET_DYN`, linked at address 0), matching what the system produces, so
security tooling and ASLR see nothing unusual:

| Segment | Contents |
|---|---|
| `PT_PHDR`, `PT_INTERP` | program headers, interpreter path |
| R | headers, `.interp`, `.gnu.hash`, `.dynsym`, `.dynstr`, `.gnu.version`, `.gnu.version_r`, `.rela.dyn`, `.rela.plt`, `.rodata*`, `.eh_frame_hdr`, `.eh_frame` |
| R X | `.init`, `.plt`, `.text*`, `.fini` |
| R W (RELRO) | `.init_array`, `.fini_array`, `.dynamic`, `.got` |
| R W | `.got.plt`, `.data*`, `.bss*` |
| `PT_DYNAMIC`, `PT_GNU_RELRO`, `PT_GNU_EH_FRAME`, `PT_GNU_STACK` | |

### New pieces

- **Linker scripts**: a small parser for `GROUP(...)`, `INPUT(...)` and
  `AS_NEEDED(...)`, enough for glibc's `libc.so`/`libm.so`. Anything else is
  an error naming the script. An absolute path in a script that does not
  exist on its own is looked up under the sysroot, the way GNU ld's `--sysroot`
  does, so a cross glibc unpacked anywhere links.
- **Shared library reader**: from each `.so`, the `DT_SONAME`, the dynamic
  symbol table (`.dynsym`, `.dynstr`) and the version tables
  (`.gnu.version`, `.gnu.version_d`). A symbol defined there resolves an
  undefined reference and records which library and version it came from.
  `AS_NEEDED` libraries get a `DT_NEEDED` only if a symbol was taken from
  them.
- **Symbol versions**: glibc exports several versions of some symbols
  (`memcpy@GLIBC_2.2.5` and `memcpy@@GLIBC_2.14`). An unversioned reference
  binds to the default (`@@`) version, which is recorded in
  `.gnu.version_r` as a requirement of `libc.so.6`, exactly as `ld` does.
- **PLT**: each function called through `R_X86_64_PLT32` and defined in a
  shared library gets a 16-byte stub `jmp *[rip + GOT(sym)]`. Because the
  output is linked with `-z now` semantics (`DF_BIND_NOW`), the stubs do
  not need lazy-binding trampolines and `.got.plt` is filled by
  `R_X86_64_JUMP_SLOT` at load time.
- **GOT**: `GOTPCRELX` loads of symbols defined in the executable are
  relaxed to `lea` as in phase 1; loads of shared-library data get a GOT slot
  with `R_X86_64_GLOB_DAT`. Our code never takes the address of library data
  directly, so copy relocations are never needed; an input that would need
  one is reported as unsupported.
- **Position independence**: every absolute 64-bit address in the output
  (`R_X86_64_64` in `.init_array`, `.fini_array`, `.eh_frame` and data)
  becomes an `R_X86_64_RELATIVE` dynamic relocation. `R_X86_64_32` against a
  local address cannot be made position independent and is an error, which
  matches `ld -pie`.
- **`.dynamic`**: `DT_NEEDED` (in command-line order), `DT_GNU_HASH`,
  `DT_SYMTAB`, `DT_STRTAB`, `DT_STRSZ`, `DT_SYMENT`, `DT_RELA`, `DT_RELASZ`,
  `DT_RELAENT`, `DT_JMPREL`, `DT_PLTRELSZ`, `DT_PLTREL`, `DT_PLTGOT`,
  `DT_INIT_ARRAY(SZ)`, `DT_FINI_ARRAY(SZ)`, `DT_VERSYM`, `DT_VERNEED(NUM)`,
  `DT_FLAGS` (`DF_BIND_NOW`), `DT_FLAGS_1` (`DF_1_NOW | DF_1_PIE`), and
  `DT_DEBUG` for debuggers.
- **`.gnu.hash`**: built over the exported symbols. A PIE exports nothing by
  default, so the table is minimal unless a program asks to export symbols
  for `dlopen` plugins.
- **`.eh_frame_hdr`**: a sorted table of FDE start addresses, needed by the
  unwinder in a dynamic process; the same code serves phase 1.
- **Startup files**: `Scrt1.o`, `crti.o`, `crtn.o` from the C library
  directory. `crtbeginS.o`/`crtendS.o` only register transactional memory
  clones and the `.ctors` list, which our objects never use; the first
  version leaves them out and the differential test proves it.
  `libgcc.a`/`libgcc_s` supply compiler helpers (`__udivti3` and similar)
  that our code generator never calls; they are searched only when a symbol
  is still undefined.

### Testing the dynamic linker

The same differential method as phase 1: every test linked by `cc` and by
the built-in linker, outputs compared, plus `readelf --dyn-syms -V -d` and
`LD_DEBUG=bindings` checks on a few binaries to confirm each symbol binds to
the same library and version as with `cc`.

## Phase 3: PE executables for Windows

Status: implemented in `nl_coff.salam` (COFF objects, COMDAT, weak
externals, commons) and `nl_pe.salam` (grouped section layout, linker
symbols, base relocations, PE32+ writer). With the built-in linker,
`tools/bash/native-windows-check.sh` passes all 111 programs under wine and
the 42 benchmarks built with `--release` produce the same output as on
Linux, so `--linker=auto` uses it for `x86_64-w64-windows-gnu`. The mingw
files come from the user's library paths, the compiler's sysroot (embedded
in release builds, or the download cache), then `/usr/x86_64-w64-mingw32`.

The goal is the same as on Linux: `--target=x86_64-w64-windows-gnu` builds
need no mingw `gcc`, only the mingw-w64 runtime libraries.

### What mingw `gcc` links

For one of our objects the link is `crt2.o crtbegin.o <object> -lmingw32
-lgcc -lgcc_eh -lmoldname -lmingwex -lmsvcrt -lkernel32 -ladvapi32 -lshell32
-luser32 ... crtend.o`, laid out by the default script `i386pep.x`.

### PE inputs

- **COFF objects** (`machine 0x8664`, and `0x14c` and `0xAA64` since phase
  4): section table with long names in the
  string table (`/123`), 18-byte symbols with auxiliary records, 10-byte
  relocations whose addend is stored in the section bytes.
- **Archives** use the same GNU `ar` format as on Linux.
- **Import libraries** in the mingw-w64 tree are long-form `dlltool`
  objects: a head object with the `.idata$2` descriptor, one object per
  symbol (`.idata$4` lookup entry, `.idata$5` IAT slot, `.idata$6`
  hint/name, `.idata$7` name pointer, and a `jmp` thunk in `.text`), and a
  tail with the DLL name. Sorting the grouped `.idata$N` sections produces a
  valid import table. Short import members (as written by LLVM tools, and
  used by llvm-mingw) are turned into objects of the same shape; see "ARM64
  PE and short import members" below.
- **COMDAT** sections (`IMAGE_SCN_LNK_COMDAT`) keep the first definition
  of their leader symbol; associative COMDATs follow their leader.
- **Weak externals** (storage class 105) fall back to their default symbol.
- **Common symbols** are allocated in `.bss` with the largest size seen,
  unless the linker defines the name itself (`__CTOR_LIST__`).
- `.drectve`, `.debug$*` and other `IMAGE_SCN_LNK_REMOVE` sections are
  dropped.

### PE layout

Following `i386pep.x`, image base `0x140000000`, section alignment `0x1000`,
file alignment `0x200`:

| Section | Contents |
|---|---|
| `.text` | `.text`, sorted `.text$*`, `.text.*` |
| `.data` | `.data`, sorted `.data$*` |
| `.rdata` | `.rdata`, sorted `.rdata$*`, the pseudo-relocation list, the `-1`/`0` framed `.ctors` and `.dtors` lists, then `.CRT$XC*`, `$XI*`, `$XL*`, `$XP*`, `$XT*`, `$XD*` with their `___crt_x*_start__`/`_end__` symbols |
| `.pdata`, `.xdata` | unwind data, data directory 3 |
| `.bss` | `.bss` and commons |
| `.idata` | `$2`, five zero words, `$4`, `$5` (the IAT, data directory 12), `$6`, `$7` |
| `.tls` | `.tls$AAA`, `.tls` (mingw's `_tls_start`), `.tls$` (clang's thread-locals), other `.tls$*`, `.tls$ZZZ`; `_tls_used` becomes data directory 9 |
| `.reloc` | base relocations for every `ADDR64` (dynamic base) |

Relocations: `ADDR64`, `ADDR32`, `ADDR32NB` (image-relative), `REL32` and
`REL32_1` to `REL32_5`, `SECTION`, `SECREL` (for TLS). Entry point
`mainCRTStartup`; console subsystem; `DYNAMIC_BASE`, `NX_COMPAT`,
`HIGH_ENTROPY_VA`; stack reserve as the backend requests (Windows reserves
only 1 MB by default, see `NATIVE_BACKEND.md`).

### Testing the PE linker

`tools/bash/native-windows-check.sh` already cross-builds 111 programs and
runs them under wine. It runs once with each linker; outputs must match, and
`x86_64-w64-mingw32-objdump -p` must show the same imports.

## Later phases

1. **Dynamic ELF against glibc**: see phase 2 above.
2. **PE/COFF for Windows**: done, including short import members and ARM64
   (see "ARM64 PE and short import members" below).
3. **Mach-O** for macOS: done, see phase 4 below.
4. **`.eh_frame_hdr`** is done: before layout the linker counts the FDEs in
   the live `.eh_frame` inputs and reserves the header and one program
   header; after relocation it walks the output `.eh_frame`, reads each
   FDE's start through its CIE's `R` augmentation, and writes the sorted
   search table (`pcrel|sdata4` frame pointer, `udata4` count,
   `datarel|sdata4` pairs). An encoding it cannot read makes it write the
   header with an omitted table, which unwinders search linearly. glibc's
   `backtrace()` walks native frames through it.

## Phase 4: every target the compiler builds for

The linker is no longer tied to the native backend's x86-64 objects. It
reads whatever the LLVM backend produces, so `--linker=builtin` works with
`--backend=llvm` for these targets:

| Target | Output | C runtime |
|---|---|---|
| `x86_64-linux-musl` | static ELF64 | bundled musl |
| `x86_64-linux-gnu` | dynamic ELF64 PIE | system glibc |
| `aarch64-linux-gnu` | dynamic ELF64 PIE | glibc (host, `/usr/aarch64-linux-gnu`, or `SALAM_GLIBC_SYSROOT`) |
| `i686-linux-gnu`, `arm-linux-gnueabihf` | dynamic ELF32 at a fixed address | glibc, as above, plus `libgcc.a` |
| `riscv64-linux-gnu` | dynamic ELF64 at a fixed address | glibc, as above |
| `i686-linux-musl` | static ELF32 | bundled musl |
| `aarch64-linux-musl` | static ELF64 | bundled musl |
| `arm-linux-musleabihf` | static ELF32 | bundled musl |
| `x86_64-w64-windows-gnu` | PE32+ | mingw-w64 |
| `i686-w64-windows-gnu` | PE32 | mingw-w64 |
| `aarch64-w64-windows-gnu` | PE32+ (ARM64) | llvm-mingw (UCRT) |
| `x86_64-apple-macos`, `arm64-apple-macos` | Mach-O PIE, ad-hoc signed | `libSystem` |

Dynamic links work for every Linux target. x86-64 and AArch64 code is
position-independent, so those are PIEs; the native backend's i686, ARM and
RISC-V code uses absolute addresses, so those executables load at a fixed
address (`0x08048000` on i686, `0x10000` elsewhere, `ET_EXEC`) and need no
`RELATIVE` relocations. Each relocation is sorted into a call, a GOT load, a
pointer-sized data word or an address formed in code:

- A call to a shared function goes through a 16-byte PLT stub that jumps
  through the function's GOT slot: `jmp *slot` on x86, `adrp`/`ldr`/`add`/`br`
  on AArch64, `auipc`/`ld`/`jalr` on RISC-V, and on ARM `ldr ip`/`ldr pc`
  behind a `bx pc` so Thumb code (glibc's start files are Thumb) can branch
  to it too.
- A data word pointing at a shared symbol gets a symbolic dynamic
  relocation when it sits in writable data; in a PIE, sections holding
  pointers are moved to the RELRO area so they can be relocated at load time.
- An address formed in code, or a pointer in read-only data, cannot be
  relocated at runtime, so a shared variable gets a copy relocation (space
  in `.bss`, sized from the library's symbol table, defined in `.dynsym` so
  the library binds to the copy) and a shared function gets a canonical PLT
  address (the undefined `.dynsym` entry carries the stub's address, and its
  GOT slot uses `JUMP_SLOT` so it still resolves to the real function).
  Symbols defined this way are the only ones in `.gnu.hash`.
- GOT slots are bound eagerly (`DF_BIND_NOW`) with `GLOB_DAT` (`R_RISCV_64` on
  RISC-V, which has no `GLOB_DAT`).
- Thread-locals of the executable keep their static offsets (it is module
  1, laid out exactly as in a static link); thread-locals from a shared
  library get `TPOFF`/`DTPMOD`/`DTPOFF` relocations on their GOT slots, and
  AArch64 TLS descriptors for them are relaxed to initial-exec.
- i686 and ARM write `.rel.dyn` (`DT_REL`, addends in place) with 32-bit
  symbol and dynamic entries; the others write `.rela.dyn`. `.preinit_array`
  (which RISC-V glibc's start file has) gets `DT_PREINIT_ARRAY`.

The driver looks for `Scrt1.o` and `libc.so` in a bundled sysroot or
`SALAM_GLIBC_SYSROOT` first, then in the target's own directories (the host's
multiarch directory, or `/usr/<tuple>/lib` where Debian's cross packages put
them), and on i686 and ARM adds the newest `libgcc.a` it finds for 64-bit
helpers like `__ashldi3`. Shared libraries are read in both ELF classes.
`tools/bash/native-glibc-check.sh` runs test programs, a Salam call into a C
library, and a C program that exercises copy relocations, canonical PLT
addresses and TLS from a library, on every target.

musl targets link statically unless a library named with `link dynamic` has
no static archive but a `.so` exists and the musl sysroot has `libc.so` and
`Scrt1.o` (the fetch script keeps both). The link is then dynamic in the same
way, against musl's `libc.so` with `/lib/ld-musl-<arch>.so.1` as the
interpreter (`i386` and `armhf` for i686 and ARM).
`tools/bash/native-musl-dyn-check.sh` checks it.

With the LLVM backend the driver asks LLVM for an object instead of an
executable and hands it to the same dispatcher the native backend uses.
`--linker=auto` keeps the LLVM backend's own linking for now.

### ELF32 and other machines

Objects carry their class and machine (`EM_X86_64`, `EM_386`,
`EM_AARCH64`, `EM_ARM`, `EM_RISCV`); the first object decides the link and a mismatch
is an error. The reader handles both section header and symbol layouts and
both `RELA` and `REL`; for `REL` the addend is read out of the section,
decoding ARM's branch, `MOVW`/`MOVT` and Thumb encodings. Relocation
application is one function per machine:

- **i386**: absolute, PC-relative, PLT, `GOTOFF`, `GOTPC`, `GOT32`/`GOT32X`
  (always through a GOT entry) and the 16- and 8-bit forms.
- **AArch64**: data, `ADRP` pages, `ADD` and load/store low 12 bits scaled
  by access size, branches, conditional and test branches, `MOVW` groups,
  GOT page and offset pairs.
- **ARM**: `ABS32`, `REL32`, `TARGET1`/`TARGET2`, `BL`/`B` (a `BL` to a
  Thumb function becomes `BLX`; a `B` to one goes through a thunk, see
  "Branch thunks"), `MOVW`/`MOVT` in ARM and Thumb, Thumb branches, `PREL31`
  and the GOT forms.
- **RISC-V (RV64)**: data words, `HI20`/`LO12` absolute pairs, PC-relative
  `PCREL_HI20` with its `PCREL_LO12_I`/`_S` partner (found by the address of
  the `auipc` the low half names), `CALL`/`CALL_PLT` pairs, `JAL`, branches,
  the compressed `RVC_BRANCH`/`RVC_JUMP` forms, `GOT_HI20`, and the
  `ADD`/`SUB`/`SET` and ULEB128 arithmetic of `.eh_frame`. The linker does
  not relax, so `RELAX` and `ALIGN` are left as they are, and
  `__global_pointer$` is defined so musl's start code can load `gp`.
  Relocations inside `.debug_*` sections go through the same code.

`SHF_TLS` sections form `.tdata` (in the data segment) and `.tbss` (no file
or memory space of its own: it is only the template's zeroed tail), and a
`PT_TLS` program header describes them, aligned to the largest member's
alignment, so musl's `__init_tls` sets up every thread's block from it.
`nl_tls.salam` resolves each access model in a static link:

- Local-exec relocations get the variable's offset from the thread pointer:
  below it on x86 (`offset - align(size)`), above the 16-byte TCB on
  AArch64, the 8-byte TCB on ARM, and at the pointer itself on RISC-V.
- Initial-exec relocations (`GOTTPOFF`, `TLS_IE`/`TLS_GOTIE`,
  `TLSIE_*_GOTTPREL_*`, `TLS_IE32`, `TLS_GOT_HI20`) get a GOT slot that holds
  that offset, so no instruction is rewritten.
- General- and local-dynamic relocations (`TLSGD`/`TLSLD`, `TLS_GD`/`LDM`,
  `TLSGD_*`, `TLS_GD32`/`LDM32`, `TLS_GD_HI20`) get a GOT pair of module 1
  and the offset in the block (0 for local-dynamic), which musl's static
  `__tls_get_addr` turns into an address; `DTPOFF`/`LDO` give the offset in
  the block (minus `0x800` on RISC-V).
- AArch64 TLS descriptors are relaxed to local-exec the way lld does it:
  `adrp` and `ldr` become `movz`/`movk` of the offset into `x0`, and the
  `add` and `blr` become `nop`.

Dynamic (glibc) links that contain thread-local sections fail with a message
that names `--linker=system`. `tools/bash/native-tls-check.sh` runs both
models on every Linux target. The ELF writer emits 32- or 64-bit headers, program
headers, section headers and symbols, keeps the first object's `e_flags`
(the ARM EABI and float ABI), uses 64 KiB pages on AArch64 and the classic
`0x08048000` base on i386. ELF common symbols are allocated in `.bss`.

### PE32

i386 COFF uses its own relocation numbers (`DIR32`, `DIR32NB`, `REL32`,
`SECREL`, `SECTION`), which are mapped onto the x64 handling. Linker
symbols get the extra leading underscore of the i386 C ABI, the entry
point is `_mainCRTStartup`, the image base is `0x400000`, absolute
addresses get `HIGHLOW` base relocations, constructor lists use 4-byte
entries, and the optional header is the 224-byte PE32 form with a
`BaseOfData` field and a 24-byte TLS directory.

### ARM64 PE and short import members

ARM64 COFF objects (machine `0xAA64`) use their own relocations. `ADDR64`,
`ADDR32`, `ADDR32NB`, `REL32`, `SECREL` and `SECTION` map onto the x64
handling; `BRANCH26`, `BRANCH19`, `BRANCH14`, `PAGEBASE_REL21`, `REL21`,
`PAGEOFFSET_12A` and `PAGEOFFSET_12L` patch the instruction fields, reading
the addend from the same fields first, as `lld-link` does. A `BRANCH26`
that does not reach gets a thunk (see "Branch thunks"); a conditional or
test branch that does not reach is an error. The optional
header says Windows 6.0 for ARM64, the oldest version `lld-link` writes.

llvm-mingw's import libraries hold short import members (the 20-byte
`0x0000 0xFFFF` header, then the symbol and DLL names) instead of the
complete `.idata` objects GNU `dlltool` writes. Their archives are also
ARM64X hybrids: the regular symbol index lists the native ARM64 members and
the ARM64EC ones sit behind `/<ECSYMBOLS>/`, so reading only the regular
index picks the right ones. For every short import that resolves a symbol
the reader builds a small COFF object in memory and reads it like any other:

- `.idata$5` and `.idata$4` hold the IAT and lookup-table entries, an
  `ADDR32NB` to the hint/name entry or the ordinal with the top bit set,
- `.idata$6` holds the hint and the import name, derived from the name type
  (as is, without the `?`/`@`/`_` prefix, undecorated, or the explicit
  export name),
- `__imp_<symbol>` names the IAT entry and, for code, `<symbol>` names a
  thunk: `adrp x16`/`ldr x16`/`br x16` on ARM64 and `jmp [__imp_<symbol>]`
  on x64 and i386.

The first import from each DLL also adds a head object (the `.idata$2`
descriptor, empty `.idata$4`/`.idata$5` sections that mark where its tables
start, and the DLL name in `.idata$7`) and a tail of null entries. Their
labels end in `#0`, `#1<symbol>` and `#2`, so the same label sort that orders
GNU `dlltool` members keeps each DLL's tables contiguous. Linking x64 and
i686 programs against llvm-mingw's UCRT libraries this way and running them
under wine passes the whole `llvm` test group.

### Branch thunks

A direct branch reaches ±128 MB on AArch64, ±32 MB in ARM mode and ±16 MB
in Thumb mode, and an ARM `B` cannot switch to Thumb (or a Thumb `B.W` to
ARM). After layout the linker checks every `CALL26`/`JUMP26` (AArch64 ELF),
`BRANCH26` (ARM64 PE and arm64 Mach-O), `CALL`/`JUMP24`/`PC24`/`PLT32` (ARM) and
Thumb `CALL`/`JUMP24` relocation. One that misses its target gets
a 16-byte thunk at the end of the input section that holds the branch, and
the branch goes to the thunk instead. The section grows, so layout runs
again until no new thunk is needed; a link that needs none is laid out
once, as before. Thunks are shared by every branch in a section to the same
target.

- AArch64 ELF: `ldr x16, #8; br x16` and the absolute target.
- ARM64 PE and arm64 Mach-O: `adrp x16; add x16; br x16`, PC-relative, so
  the image stays relocatable without a base relocation or a rebase entry.
  On Mach-O a call to an imported function goes to its `__stubs` entry, so
  a thunk for one targets the stub.
- ARM: `ldr pc, [pc, #-4]` and the target, Thumb bit included, so the load
  switches mode when it has to.
- Thumb: `ldr.w pc, [pc]` and the target.

`x16` is the AAPCS64 register reserved for this, and an ARM thunk clobbers
nothing. A conditional or test branch on AArch64 that does not reach is
reported as an error rather than patched with a wrong offset.

### ARM unwind tables

`.ARM.exidx` input sections (`SHT_ARM_EXIDX`) form their own output section
in the read-only segment. One is kept exactly when the code section its
`sh_link` names is live, together with what it references (`.ARM.extab`).
After relocation the 8-byte entries are sorted by function address and
their 31-bit offsets rebased, which is what `SHF_LINK_ORDER` asks for. Each
thunk area (see "Branch thunks") gets an entry of its own, `0x80B0B0B0`
("finish"): a thunk changes neither `sp` nor `lr`, so an unwind that starts
in one returns through `lr` instead of using the previous function's
entry. The linker defines `__exidx_start`/`__exidx_end` and adds a `PT_ARM_EXIDX`
program header, which libgcc finds through `dl_iterate_phdr`. musl links
add `libgcc_eh.a` when the sysroot has it, so programs that call the
unwinder link too.

### Mach-O

The reader takes `MH_OBJECT` files for x86_64 and arm64: sections become
input sections (with their object addresses kept, since Mach-O symbols and
section-relative relocations are absolute in the object), `nlist_64`
symbols become section-relative, and section-relative relocations get a
synthetic symbol per section. It pairs `SUBTRACTOR` with the following
`UNSIGNED`, applies arm64 `ADDEND` to the next relocation, and reads
x86_64's embedded addends including the `SIGNED_1/2/4` bias.
Thread-locals keep their three sections (`__thread_vars`,
`__thread_data`, `__thread_bss`, in that order ahead of `__bss`, with their
section types, and `MH_HAS_TLV_DESCRIPTORS` in the header). A descriptor's
pointer to its `$tlv$init` symbol becomes that symbol's offset in the
template, its `__tlv_bootstrap` word is bound like any import, and the code
that loads a descriptor is relaxed to address it directly (`movq` to `leaq`
on x86-64, `ldr` to `add` on arm64), as ld64 does for local variables. A
thread-local from a dylib is an error.
`__compact_unwind` is dropped; `__eh_frame` is kept and relocated. The
`__DWARF` sections of every object that has `__debug_info`, archive members
included, are concatenated into a `__DWARF` segment between `__DATA` and
`__LINKEDIT`, with their address relocations applied. Mach-O objects carry
no relocations for the offsets between their debug sections, so
`nl_dwmo.salam` walks each object's units (DWARF 2 to 5, decoding every
DIE through its abbreviations) and adds that object's place in each output
section to every offset into another debug section: the abbreviation offset,
`DW_FORM_strp`, `line_strp`, `ref_addr` and `sec_offset` (by attribute:
line table, ranges or range lists, location or location lists,
`str_offsets_base`, `addr_base`), the `.debug_str_offsets` entries, the
string forms in DWARF 5 line table headers, and `.debug_aranges`. An object
whose units cannot be walked (64-bit DWARF, an unknown form) is left out
rather than linked with wrong offsets, and `__debug_names` is dropped when
more than one object contributes, since its tables cannot be concatenated
(lldb builds its own index of the units). The executable gets an
`__unwind_info` section (`nl_unwind.salam`) in which every function with an
FDE uses the DWARF encoding (`UNWIND_X86_64_MODE_DWARF` or
`UNWIND_ARM64_MODE_DWARF` plus the FDE's offset in `__eh_frame`), and the
code between FDEs gets encoding 0, so the system unwinder finds each frame
through `__eh_frame` without guessing. Entries sit in regular second-level
pages of up to 511, behind an index whose last entry marks the end of the
code.

The output is a PIE `MH_EXECUTE`:

- `__PAGEZERO`, `__TEXT` (`__text`, `__stubs`, `__cstring`, `__const`,
  `__eh_frame`, `__unwind_info`), `__DATA_CONST` (`__got`, `__mod_init_func`, `__const`),
  `__DATA` (`__data`, `__bss`) and `__LINKEDIT`.
- Calls to imported functions go through a stub that jumps through a
  non-lazy `__got` slot; there is no lazy binding. GOT loads of local
  symbols point at a rebased slot.
- `LC_DYLD_INFO_ONLY` carries the rebase and bind opcodes. `libSystem` is
  always ordinal 1; every other library the program links becomes its own
  `LC_LOAD_DYLIB`, read from a text stub (`.tbd`, including multi-document
  stubs with reexported libraries) or a real dylib (its install name from
  `LC_ID_DYLIB` and its exports from the exports trie). Each import binds
  to the library that exports it, and anything no input lists binds to
  `libSystem`. The driver finds `lib<name>.tbd` or `.dylib` on the library
  paths and in the SDK (`SDKROOT`, or the Xcode command-line tools and Xcode
  locations), and frameworks as `<name>.framework/<name>.tbd` there.
- Static archives may use the BSD format macOS tools write (`#1/N` member
  names stored after the header and a `__.SYMDEF`, `__.SYMDEF SORTED` or
  `__.SYMDEF_64` index) as well as the GNU one; only the members that
  resolve a symbol are loaded.
- `LC_SYMTAB`, `LC_DYSYMTAB` with the indirect symbol table for the stubs
  and GOT, `LC_LOAD_DYLINKER`, `LC_UUID`, `LC_BUILD_VERSION` (macOS 11),
  `LC_MAIN`, `LC_LOAD_DYLIB`, `LC_DATA_IN_CODE` (jump tables inside
  `__text`) and `LC_CODE_SIGNATURE`.
- An ad-hoc, linker-signed code signature: a SuperBlob with one
  CodeDirectory (version `0x20400`, SHA-256 of every 4 KiB page up to the
  signature, the executable segment marked as the main binary), which
  arm64 macOS requires before it runs anything.

### Testing phase 4

There is no macOS, AArch64 or ARM hardware here, so each target is checked
differently:

- i686 Linux runs natively and AArch64 and ARM run under `qemu-user`; each
  test is built once as an object and linked by both the built-in linker
  and `ld.lld`, and both outputs must agree.
- i686 Windows runs under wine against a mingw `ld` reference.
- Mach-O is compared structurally: the disassembly of every test linked by
  the built-in linker and by `ld64.lld` must match once addresses, page
  numbers, local label names and lld's GOT relaxations are taken out, and
  every page hash of the signature must verify.

On the test sections, 2429 programs for i686 Linux, AArch64, ARM and i686
Windows behaved the same with the built-in linker as with the reference
linker. The few that differed (network servers and thread timing) changed
from run to run with either linker. All 1238 Mach-O links matched
`ld64.lld`. `tools/linker/cross-check.sh` runs the same comparison for one
target at a time, and `tools/linker/macho_check.py` holds the Mach-O
signature and disassembly checks.
