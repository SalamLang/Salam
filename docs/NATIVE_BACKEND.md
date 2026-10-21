# The native backend

`--backend=native` turns a Salam program into x86-64 machine code without
going through LLVM or a C compiler. It writes the object file itself (ELF on
Linux, COFF on Windows), so the only outside tool left is the linker, and when
the compiler has LLVM embedded even that runs in-process through lld.

```sh
salam build app.salam --backend=native
salam run app.salam --backend=native
salam build app.salam --backend=native --target=x86_64-w64-windows-gnu
salam build app.salam --backend=native --emit-asm     # also writes app.s
salam obj app.salam --backend=native                  # object file only
```

Targets: x86-64 Linux (System V ABI, ELF) and x86-64 Windows (Win64 ABI,
COFF with `.pdata`/`.xdata` unwind tables). Any other `--target` is rejected
with a message that points at `--backend=llvm`.

## How it works

The frontend is shared with the LLVM backend. `llvmgen` lowers the checked
AST to textual LLVM IR exactly as it does for `--backend=llvm`, and nativegen
reads that text instead of handing it to LLVM. Every language feature, std
package and platform quirk llvmgen already handles therefore works here too,
and the two backends cannot drift apart on semantics.

All of it lives in `compiler/nativegen/`:

| File                                                               | Job                                                                                                                                                                         |
| ------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `ng_lex.salam`, `ng_type.salam`, `ng_parse.salam`, `ng_func.salam` | Read the IR text into a compact SSA form (struct-of-arrays). GEPs become `base + index*scale + disp` address nodes. Globals and constants go straight into section buffers. |
| `ng_cfg.salam`                                                     | CFG, reverse postorder, dominators, dominance frontiers, mem2reg                                                                                                            |
| `ng_sroa.salam`                                                    | Splits struct locals into one scalar per field                                                                                                                              |
| `ng_inline.salam`                                                  | Inlines small functions from a cache of already optimized bodies                                                                                                            |
| `ng_opt.salam`                                                     | Constant folding, block merging, jump threading, dead code elimination                                                                                                      |
| `ng_isel.salam`                                                    | Instruction selection into machine instructions on virtual registers                                                                                                        |
| `ng_ra.salam`                                                      | Linear-scan register allocation with lifetime holes, spilling, frame layout                                                                                                 |
| `ng_enc.salam`                                                     | x86-64 encoder, prologues and epilogues, branch relaxation, Windows unwind info                                                                                             |
| `ng_elf.salam`, `ng_coff.salam`                                    | Object file writers                                                                                                                                                         |
| `ng_gdce.salam`                                                    | Removes functions and data nothing references                                                                                                                               |
| `ng_asm.salam`                                                     | The `--emit-asm` listing                                                                                                                                                    |
| `ng_dump.salam`                                                    | `SALAM_NG_DUMP=<function>` prints a function's optimized IR to stderr                                                                                                       |

The driver side is `compiler/driver/driver_native.salam`. After llvmgen has
produced the IR it builds the object, then links it with in-process lld when
`lb.LinkObject` can handle the triple and with `cc` (or
`x86_64-w64-mingw32-gcc` for the Windows target) otherwise.

### Optimizations

Functions are compiled callees first, so a call can be inlined whenever the
callee is small (about 40 instructions, more for `alwaysinline`) and not
recursive. Each function then goes through:

1. SROA on struct locals, then mem2reg.
2. Inlining, then SROA and mem2reg again for the locals the inlined code used
   to reach only through a pointer.
3. Folding to a fixed point: arithmetic and compares on constants, algebraic
   identities, casts, selects, constant branch conditions, `extractvalue` of a
   known `insertvalue`, address arithmetic, and `x % 2^k == 0` as a mask test.
4. Block merging and jump threading, which turns short-circuit `and`/`or`
   chains into direct branches.
5. If-conversion of small branches into `cmov`, except where the result feeds
   an address, since a `cmov` there would serialize the following load.
6. Global value numbering over the dominator tree.
7. Mark-and-sweep dead code elimination, which also removes dead phi cycles.

Instruction selection folds address arithmetic into memory operands, fuses a
compare with the branch that follows, divides by constants with shifts or a
multiply by the reciprocal, and reads struct values straight from their
source when nothing writes memory before the last use. Register allocation
is linear scan over exact live ranges, so a value only competes for a register
where it is actually live.

### Conventions worth knowing

- Scalars follow the C ABI of the target, so calls to and from C code just
  work. Struct values, which only Salam code passes, travel by pointer to a
  copy, and struct returns use a hidden result pointer.
- `rbp` is always the frame pointer. On Linux it is set right after
  `push rbp`; on Windows the prologue has the shape the Windows unwinder
  expects (pushes, allocation, `lea rbp, [rsp+N]`) and every function gets an
  unwind entry.
- `r10`, `r11`, `xmm14` and `xmm15` are kept free as scratch registers for
  spill code.
- An `i1` value lives in the low byte of a register as 0 or 1. Narrow
  integers are extended where an operation needs the full width.

## Testing and debugging

- `tools/bash/backend-sweep.sh <salam> native` builds every test with the
  native backend and compares the output with the C backend's `.out` files.
  With the default sections it agrees on everything except
  `interop/redis_demo`, which needs a live redis server, the same as the LLVM
  backend.
- `tools/bash/native-encoder-check.sh <salam> <files...>` checks the encoder:
  it assembles the `--emit-asm` listing with GNU `as` and compares the result
  with the object nativegen wrote, instruction by instruction.
- `SALAM_BACKEND=native sh tools/bash/run-tests.sh <groups>` runs the regular
  build tests through the native backend.
- `SALAM_NG_DUMP=main salam build app.salam --backend=native` prints the
  optimized IR of `main`. Function names are the mangled ones you see in the
  listing.
- `SALAM_NG_OFF=gvn,ifc,thread,inline,sroa` turns optimizer passes off, which
  is the quickest way to find the pass behind a wrong result.
- `SALAM_NG_GVN_MAX=n` stops value numbering after `n` merges and prints the
  function the last one happened in, so a miscompile can be bisected down to
  a single merge. `SALAM_NG_GVN_SKIP=addr,extv,icmp,cast,arith` leaves whole
  instruction kinds alone.

A good end-to-end check is the compiler itself: emit its IR with
`salam llvm compiler/main.salam`, build that IR with the native backend, and
have the result emit the same IR again. The two files must be identical
apart from the build timestamp.

## Where it stands

The compiler built by the native backend regenerates its own IR about 1.25x
slower than the LLVM -O2 build. On the benchmark suite most integer and
pointer-chasing programs run within 10-30% of `gcc -O3`. Some are even with it
or faster (`quicksort`, `caesar_cipher`). The biggest gaps are loops that gcc
and LLVM unroll or vectorize, such as `game_of_life` and `tensor_matmul`.

Not there yet:

- No DWARF line tables (`-g`) and no `.eh_frame` on Linux. Debuggers walk the
  stack through `rbp`, but there is no source-level stepping.
- No loop unrolling, vectorization or loop-invariant code motion.
- x86-64 only. aarch64 would reuse everything up to instruction selection.
- Linking still uses lld or the system linker. A built-in static linker over
  the bundled musl `libc.a` would make Linux builds fully self-contained.
