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
- Debug information. Line tables arrive with DWARF support in the backend;
  the linker will then merge and relocate `.debug_*` sections like any other.

## What the inputs contain

A census of the bundled musl `libc.a` (1342 members) and the CRT objects
shows a small, well-behaved surface:

| Feature | Seen |
|---|---|
| Relocations | `R_X86_64_PC32`, `PLT32`, `64`, `32`, `REX_GOTPCRELX` (about 10,000 in all) |
| Sections | `.text`, `.text.*`, `.rodata`, `.rodata.*` including `.rodata.cst*`, `.data*`, `.bss*`, `.init`/`.fini` (only in `crti.o`/`crtn.o`), notes |
| Symbols | global, local and weak (about 290 weak), no COMMON, no IFUNC, no TLS |
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

Plus `PT_GNU_STACK` (non-executable stack) and `PT_GNU_EH_FRAME` once
`.eh_frame_hdr` exists. Segments are aligned to the page size and file
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

## Driver integration

A new option `--linker=auto|builtin|system`:

- `builtin` always uses the built-in linker and fails clearly on inputs it
  does not support (for example a shared library).
- `system` keeps today's behaviour (`cc`, mingw `gcc`, or embedded lld).
- `auto` (the eventual default) picks `builtin` when the target is
  `x86_64-linux-musl`, every `link` directive names a static archive or
  object, and no framework is requested; otherwise it picks `system`.

`auto` now picks `builtin` for both x86-64 Linux flavours whenever every
input is found (static for musl, a PIE for glibc), and `system` otherwise,
for example for Windows targets. `SALAM_LINKER` sets the default mode, which
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
`.eh_frame_hdr` is not written yet; nothing in the runtime unwinds.

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
  an error naming the script.
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

## Later phases

1. **Dynamic ELF against glibc**: see phase 2 above.
2. **PE/COFF for Windows**: reading mingw import libraries (short import
   members), `.idata` import tables, base relocations, `.pdata`/`.xdata`
   kept as produced, subsystem and stack reserve in the optional header.
3. **Mach-O** together with the aarch64 backend, for macOS.
4. **`.eh_frame_hdr`** and a search table, so unwinders and profilers find
   frames in large static binaries quickly.
