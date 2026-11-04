# The native backend

`--backend=native` turns a Salam program into x86-64, 32-bit x86, AArch64, 32-bit ARM or RISC-V machine code without
going through LLVM or a C compiler. It writes the object file itself (ELF on
Linux, COFF on Windows, Mach-O on macOS), so the only outside tool left is the linker, and when
the compiler has LLVM embedded even that runs in-process through lld.

```sh
salam build app.salam --backend=native
salam run app.salam --backend=native
salam build app.salam --backend=native --target=x86_64-w64-windows-gnu
salam build app.salam --backend=native --target=x86_64-apple-macos
salam build app.salam --backend=native --target=aarch64-linux-musl
salam build app.salam --backend=native --target=i686-linux-musl
salam build app.salam --backend=native --target=i686-w64-windows-gnu
salam build app.salam --backend=native --target=arm-linux-musleabihf
salam build app.salam --backend=native --target=riscv64-linux-musl
salam build app.salam --backend=native --target=arm64-apple-macos
salam build app.salam --backend=native --target=universal-apple-macos  # arm64 + x86_64
salam build app.salam --backend=native --emit-asm     # also writes app.s
salam build app.salam --backend=native --cpu=x86-64-v2 # SSSE3 and SSE4.1 vector code
salam build app.salam --backend=native --cpu=x86-64-v3 # 256-bit AVX2 loops
salam obj app.salam --backend=native                  # object file only
```

Targets: x86-64 Linux (System V ABI, ELF), x86-64 Windows (Win64 ABI,
COFF with `.pdata`/`.xdata` unwind tables) and x86-64 macOS (System V ABI,
Mach-O), 32-bit x86 Linux (i386 System V ABI, ELF32) and Windows (cdecl and
stdcall, COFF), 32-bit ARM Linux (armhf: ARMv7-A, VFPv3-D16, AAPCS-VFP,
ELF32), RISC-V 64 Linux (RV64GC, LP64D, ELF), AArch64 Linux
(AAPCS64, ELF) and arm64 macOS (Apple's AAPCS64 variant, Mach-O). Any other
`--target` is rejected
with a message that points at `--backend=llvm`.

On macOS the code is the same as on Linux; only the object differs. Symbols
get the Darwin `_` prefix, calls use `X86_64_RELOC_BRANCH` (the linker adds
stubs), data that lives in another image is loaded through
`X86_64_RELOC_GOT_LOAD`, other RIP-relative references are
`X86_64_RELOC_SIGNED` with the addend stored in the instruction, and every
relocation names its symbol (local ones included), as x86-64 Mach-O expects.
Read-only data that holds pointers goes to `__DATA,__const`, constructors to
`__mod_init_func`. Functions get `__eh_frame` entries, as on Linux, with
each FDE's start written as a `SUBTRACTOR`/`UNSIGNED` pair, and `-g` adds the
DWARF sections under `__DWARF` (see "Debug information").

## AArch64

The AArch64 backend shares everything up to and including register
allocation with x86-64; only the last step differs. `ng_a64.salam` turns each
machine instruction into A64 words, using `x16`/`x17` (the ABI's
intra-procedure scratch registers, which the allocator never sees) for
addresses, large immediates and memory operands. The allocator's sixteen
integer IDs map onto `x0`-`x7` (arguments), `x8` (the indirect-result
register, which carries a struct return), `x9`/`x10` (its spill scratch) and
`x19`-`x22` (callee-saved); its float IDs map onto `v0`-`v7` and `v16`-`v23`,
all caller-saved, so `d8`-`d15` are never touched. Every function keeps a
frame record (`x29`/`x30`), slots are addressed from `sp`, incoming stack
arguments from `x29`.

Flags need care because the machine instructions follow x86 semantics. A
condition is translated by looking at what set the flags: after `cmp`/`sub`
the carry is inverted (ARM's carry means no borrow), after `add` it is not,
after a logical op it is known to be clear, and after a float compare the
x86 unsigned conditions become ARM's ordered/unordered ones. ALU ops only use
the flag-setting form (`adds`, the flag-setting `and`, or a trailing `tst`
for `orr`/`eor`)
when a later instruction reads the flags. Narrow (8/16-bit) compares shift
both operands to the top of a 32-bit register first, which keeps signed and
unsigned order. Division emulates x86's `rdx:rax` pair with `sdiv`/`udiv`
plus an `msub` only when the remainder is used; a 32-bit move to the same register still emits `mov wN, wN`,
since that is what zero-extends. Atomics are `ldaxr`/`stlxr` loops.

On macOS the same code goes into a Mach-O object with
`ARM64_RELOC_BRANCH26`, `PAGE21`/`PAGEOFF12` and their GOT forms, and an
`ARM64_RELOC_ADDEND` entry in front of any reference with an offset. Apple's
ABI passes every variadic argument on the stack in 8-byte slots (Linux uses
registers), so a call to `printf` places its arguments after the fixed ones
in memory. Every test, compiled for `arm64-apple-macos` and linked by both
the built-in linker and `ld64.lld`, disassembles to the same instructions.

Windows on Arm (`aarch64-w64-windows-gnu`) writes the same code into an
ARM64 COFF object. COFF relocations carry no addend, so an offset is written
into the instruction itself (the `adrp` immediate, the `add` immediate, the
branch field), and a global with an offset is addressed as `adrp`/`add` of
the symbol plus the offset in the load or store. Windows has no GOT: the
address of an external comes from `adrp`/`add` as well, which yields the
import thunk for a function from a DLL. A frame of 4 KiB or more is probed
first (`mov x15, #size/16; bl __chkstk; sub sp, sp, x15, uxtx #4`), and a
call to a variadic function passes floating-point arguments in integer
registers, as the Windows ARM64 ABI requires. `x18` belongs to the OS and
the allocator never uses it. The runtime is llvm-mingw (UCRT): the built-in
linker finds it through `aarch64-w64-mingw32-clang` on `PATH` or
`SALAM_LLVM_MINGW`, and `--linker=system` links with that clang instead.
Every function gets a `.pdata` entry whose `.xdata` codes follow the
prologue instruction by instruction (`save_fplr_x`, `set_fp`, the
allocation, `save_reg` for each callee-saved register within 504 bytes of
`sp`, `nop` for anything else), so debuggers and the OS unwinder can walk
the frames. The native backend gate builds the `llvm` tests with the
built-in linker and with lld and runs both sets on a `windows-11-arm`
runner; all of them pass on that hardware.

The compiler builds for Windows on Arm too, and that build reproduces the
reference IR on the runner. `tools/bash/package-winarm.sh` turns it into a
self-contained package (about 30 MB): `salam.exe`, `std`, and the part of
llvm-mingw's aarch64 runtime the built-in linker needs under
`sysroots/aarch64-w64-mingw32/lib`, which the compiler finds next to
itself. On a Windows on Arm host with no `--backend`, the compiler uses the
native backend, since neither LLVM nor tcc ship for that host. The gate unpacks
the package on the runner and builds and runs the `llvm` tests with it
(`tools/bash/native-host-check.sh`), with nothing else installed. Releases
ship it as `salam-<version>-windows-arm64.zip` (and `.tar.gz`), smoke-tested
on a `windows-11-arm` runner before publishing.

On Linux, AArch64 and RISC-V objects carry `.eh_frame` (`ng_cfi.salam`).
The encoders record CFI events as they write each prologue and epilogue: the
CFA moves to the frame pointer plus 16 once the frame record is saved, every
callee-saved register gets its slot, and each epilogue is wrapped in
`remember_state`/`restore_state` with the CFA back on `sp` after the frame
record is reloaded. libgcc's `_Unwind_Backtrace` walks the same number of
frames there as on x86, and gdb uses the tables instead of guessing from the
prologues.

The loop, SLP and sum-reduction vectorizers run on AArch64 too. They emit the
same machine ops as on x86, and `ng_a64` lowers each SSE2-shaped one to its
NEON counterpart: `paddd` to `add .4s`, `pandn` to `bic`, `pcmpgtd` to
`cmgt`, `punpcklbw` to `zip1 .16b`, `packuswb` to `sqxtun` and `sqxtun2`,
`shufps`/`pshufd` to `dup` or four `ins`, and packed float ops to
`fadd`/`fmul` on `.4s`/`.2d`. qemu keeps NEON registers in memory, so
vectorized loops can run slower under emulation than on hardware; the
`linux-arm64-run` gate job times every benchmark with and without the
vectorizers on an AArch64 runner. armhf and RISC-V keep them off: the armhf
baseline (VFPv3-D16) does not promise NEON, and rv64gc has no vector unit.

ARM hardware returns 0 for an integer division by zero instead of trapping,
so on AArch64 (and on RISC-V, which returns -1) release builds keep the
explicit check that debug builds have on every target: one compare and a
branch to the panic per division by a non-constant.

## 32-bit x86

The `i686` targets reuse the x86-64 pipeline with three additions.

- Before instruction selection, `ng_legal.salam` splits every 64-bit integer
  into a pair of 32-bit halves: shifts by a constant become two shifts and an
  `or`, loads and stores touch both words, and a multiply becomes three 32-bit
  multiplies (one when both operands are sign-extended 32-bit values). A
  division or remainder by a constant below 2^32 uses the hardware `div`
  the way libgcc does, without the call: when the high word is already below
  the divisor one `div` of `edx:eax` gives the quotient's low word and the
  remainder, otherwise the high word is divided first. A signed one divides
  the absolute value and fixes the signs with `xor`/`sub`. (32-bit ARM has no
  divide instruction and uses a 64-bit magic multiply built from 32-bit
  pieces instead.) A division whose operands range analysis proves fit in 32
  bits becomes a 32-bit one, and one whose operands are provably
  non-negative becomes unsigned. Other divisions call the libgcc helpers once
  per operand pair: the quotient comes from `__divdi3` (or `__udivdi3`) and a
  remainder of the same operands is the dividend minus quotient times divisor,
  so `n % j` next to `n / j` costs one call. An `i64` converted to a float is
  stored and loaded with x87 `fild`, which is exact and needs no call, unless
  range analysis shows it fits in 32 bits (then `cvtsi2sd` takes the low
  half). Variable shifts and the other conversions call libgcc. A call that
  returns an `i64` gets its high half from `edx`. 64-bit atomics call small `lock cmpxchg8b` routines the backend
  writes into the object itself.
- After the optimizer runs on the split code, an add or subtract whose carry
  the legalizer spelled as a compare becomes `add`/`adc` (or `sub`/`sbb`),
  and a 64-bit `<` or `>=` becomes `cmp` on the low halves plus `sbb` on the
  high halves feeding one branch. Loop counters keep their 32-bit type here:
  the pass that widens them to 64 bits for address arithmetic runs only on
  the 64-bit targets, where it saves a sign extension instead of costing a
  register pair.
- Calls follow cdecl: every argument goes on the stack, floats come back in
  `st0` and are moved to an SSE register right away, so everything after the
  call uses SSE2 as on x86-64. SSE2 is therefore required (any x86 CPU since
  2001).
- The register allocator gets `eax`, `ecx`, `edx`, `ebx`, `ebp` and `esi`,
  with `edi` as its spill scratch and `xmm0`-`xmm5` for floats. An
  instruction whose spilled operands need a second scratch register borrows
  `esi`, `ebx` or `ebp`: one that holds no live value at that point is used
  as is, otherwise it is saved to a frame slot around the instruction. The encoder
  drops REX prefixes, addresses globals with absolute 32-bit relocations, and
  wraps a byte instruction whose operand sits in `ebp`, `esi` or `edi` (no
  8-bit form on i386) in a pair of `xchg`s with a free low register.
  Divisions by a constant use a 32-bit `mul` for the magic number.

On Linux the object is ELF32 with `REL` relocations (addends stored in the
code). On Windows it is an i386 COFF object: every symbol gets the `_`
prefix, and a Win32 function the frontend marks `x86_stdcallcc` is named
`_Name@N`; it pops its own arguments, so the caller subtracts `N` from
`esp` right after the call to keep its fixed argument area in place.
The loop vectorizer is off on these targets for now. i686 Linux objects carry `.eh_frame` (frames are `esp`-based, so a debugger cannot walk them otherwise); armhf objects carry ARM EHABI tables instead (see "32-bit ARM"). On i686 the straight-line (SLP) vectorizer runs, so unrolled `f32`/`f64` kernels use SSE2 pairs and quads, and so does the sum-reduction vectorizer: an `i64` total comes out of `paddq` lanes as two `movd` halves.
Built for `i686-linux-musl` and run natively, the `basics`, `features`,
`types`, `data`, `general`, `match`, `switch`, `llvm`, `stdlib`, `apps`,
`interop` and `webframework` tests match their expected output except those
that need `-D` flags, a database library or a trapping release-mode
division, `types/size_type` (whose output assumes a 64-bit `usize`) and a UDP
test that also fails when clang compiles the same IR. The same groups built
for `i686-w64-windows-gnu` and run under wine match too. Thread start
routines on 32-bit Windows must be `__stdcall`, so the frontend wraps every
`spawn`ed function in a small `x86_stdcallcc` thunk there, and the backend
pops a stdcall definition's arguments with `ret N`.

## 32-bit ARM

The `arm-linux-musleabihf` target (also `armv7`) produces ARM-mode ARMv7-A
code for the hard-float ABI with VFPv3-D16 and no NEON, the Debian armhf
baseline. Like AArch64 it shares the pipeline up to register allocation and
lowers each machine instruction in `ng_arm.salam`, and like i686 it splits
64-bit integers into pairs first.

- Integer IDs map to `r0`-`r3` (arguments) and `r4`-`r11` (callee-saved);
  `r12` and `lr` are the lowering's scratch registers (`lr` is always saved).
  Floats live in `d0`-`d7`; `d13`-`d15` are scratch and saved with `vpush`
  only in functions that touch floats. Globals are addressed with
  `movw`/`movt` pairs, and an offset that does not fit their 16-bit addend
  is added separately.
- Calls follow AAPCS-VFP: a 64-bit value starts in an even register or an
  8-byte aligned stack slot (the legalizer tags the low half), doubles take
  `d` registers, a `float` may back-fill an odd `s` register, and variadic
  doubles travel in core registers. libgcc's 64-bit integer and float
  conversion helpers use the soft-float convention even in a hard-float
  build, so their float arguments and results go through core registers.
- There is no divide instruction in the baseline: division by a value calls
  `__aeabi_idiv`/`__aeabi_uidiv` (and the `mod` forms, whose remainder comes
  back in `r1`); division by a constant still uses a multiply. A 64-bit
  division and remainder of the same operands share one `__aeabi_ldivmod`
  call (quotient in `r0:r1`, remainder in `r2:r3`).
- The 64-bit carry and compare chains use `cmn`/`adc` and `cmp`/`sbcs`, and
  a word load or store with an index and no offset uses the scaled register
  form (`ldr r0, [r1, r2, lsl #3]`).
- Atomics are `ldrex`/`strex` loops between `dmb` barriers, borrowing a
  register with `push`/`pop` for the status word; 64-bit atomics call
  `ldrexd` routines the backend writes into the object.

Built with the built-in linker against a musl sysroot and run under
qemu-arm, the test groups listed for i686 match their expected output with
the same exceptions. Every function gets an ARM EHABI unwind entry
(`ng_exidx.salam`): the opcodes undo the prologue (the stack adjustment,
`vpop d8-d10` when the function saved them, then a pop under mask of the
pushed registers and `lr`), inline in `.ARM.exidx` when they fit in three
bytes and in `.ARM.extab` with personality routine 1 otherwise. libgcc's
unwinder walks the frames with them. `-g` gives line tables, frames and
variables (see "Debug information").

## RISC-V

`riscv64-linux-musl` produces RV64GC code for the LP64D ABI, lowered from the
shared machine instructions in `ng_rv64.salam` with the same register
assignment and frame layout as AArch64 (`a0`-`a7` for arguments, `s0`/`ra`
frame record, slots addressed from `sp`).

RISC-V has no flags register, so a flag-setting instruction leaves its
operands in `t5`/`t6` in "compare form": a compare stores both sides, a
`test` or logic result stores the result and zero, a negate stores zero and
the operand, and narrower sizes are shifted to the top of the register so
both signed and unsigned order survive. Every condition then becomes a
`slt`/`sltu`/`xor` sequence on that pair; the carry of an `add` keeps its
first operand in `t4`, and a float compare keeps both operands in
`ft10`/`ft11` for the float compare instructions. A conditional jump is a short branch
over a `jal`, so distance is never a problem. When an integer compare or
`test` feeds only the jump, `setcc` or `cmov` right after it, and no path from
there reads the flags again before they are redefined, the pair skips the
compare form: the jump becomes one `blt`/`bgeu`/`beq`... on the original
registers (`zero` for a compare with 0, `addiw` sign extensions for 32-bit
operands), and a `setcc` becomes one `slt`/`sltu`.

The machine instructions follow x86, where a 32-bit operation clears the
upper half of the register, so 32-bit results are zero-extended after the
`w` forms. At calls into C, 32-bit arguments are sign-extended as the psABI
requires, and so are 32-bit return values. Single-precision values must stay
NaN-boxed, so a float zero is made at the value's own width. Globals are
addressed with absolute `lui`/`addi` pairs (static executables) and calls with
`auipc`/`jalr`. A division whose quotient is unused is a single `rem`, and a
remainder next to its quotient is a `mul` and `sub` instead of a second
divide. Byte and halfword atomics run `lr.w`/`sc.w` on the containing word
with a mask.

After a function is encoded, every instruction that has an RV64C form
becomes 2 bytes (`c.addi`, `c.li`, `c.mv`, `c.add`, the register-pair ALU
ops, `sp`- and register-based loads and stores, `c.lui`, `c.jr`/`c.jalr`,
`c.addi16sp`, `c.addi4spn`). A jump becomes `c.j` and a branch comparing a
register in `x8`-`x15` with zero becomes `c.beqz`/`c.bnez` when the target
is close enough (2 KiB and 256 bytes): every candidate starts short, any that
does not reach is made long again, and that repeats until nothing changes,
since lengthening one can only push others out of reach. The remaining
branches are re-encoded for the new distances, which only shrink. An instruction with a relocation keeps 4 bytes, as does the `jalr`
of an `auipc`/`jalr` call pair and everything from an `lr` to its `sc`
(the spec keeps compressed code out of constrained LR/SC loops). Debug rows,
variable ranges and CFI move with the code; the CIE's code alignment is 1
and the object's `e_flags` gain `EF_RISCV_RVC`. The compiler built for
riscv64 is about 20% smaller this way. `SALAM_NG_OFF=rvc` turns it off.

Built with the built-in linker against a musl sysroot and run under
qemu-riscv64, the test groups listed for i686 match their expected output
with the same exceptions.

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
| `ng_tailrec.salam`                                                 | Turns self tail calls into loops, with an add or multiply accumulator                                                                                                       |
| `ng_opt.salam`                                                     | Constant folding, block merging, jump threading, if-conversion, dead code elimination                                                                                       |
| `ng_unroll.salam`                                                  | Fully unrolls small loops whose trip count is a constant                                                                                                                    |
| `ng_licm.salam`                                                    | Loop discovery, preheaders, loop-invariant code motion, loop weights and live-range splitting for the allocator                                                             |
| `ng_lpro.salam`                                                    | Promotes loop-invariant loads to registers, reloading after calls                                                                                                           |
| `ng_lsr.salam`                                                     | Strength-reduces affine array addresses into pointer induction variables                                                                                                    |
| `ng_gvn.salam`, `ng_slp.salam`                                     | Value numbering with redundant load elimination, and SLP vectorization                                                                                                      |
| `ng_isel.salam`                                                    | Instruction selection into machine instructions on virtual registers                                                                                                        |
| `ng_ra.salam`                                                      | Linear-scan register allocation with lifetime holes, spilling, frame layout                                                                                                 |
| `ng_rasp.salam`                                                    | Splits a value's range between registers and its stack slot, with the moves at split points and block edges                                                                 |
| `ng_enc.salam`                                                     | x86-64 encoder, prologues and epilogues, branch relaxation, Windows unwind info                                                                                             |
| `ng_a64.salam`                                                     | AArch64 encoder: machine instructions to A64 words, flag translation, frame records, relocations                                                                            |
| `ng_legal.salam`                                                   | Splits 64-bit integers into 32-bit pairs for the 32-bit targets, and turns ARM divisions into library calls                                                                 |
| `ng_arm.salam`                                                     | 32-bit ARM encoder: machine instructions to A32/VFP words, frame, atomics, relocations                                                                                      |
| `ng_rv64.salam`                                                    | RISC-V 64 encoder: machine instructions to RV64GC words, flag emulation, frame, atomics, relocations                                                                        |
| `ng_elf.salam`, `ng_coff.salam`, `ng_macho.salam`                  | Object file writers                                                                                                                                                         |
| `ng_gdce.salam`                                                    | Removes functions and data nothing references                                                                                                                               |
| `ng_fastx.salam`                                                   | The early-return path tested before the prologue                                                                                                                            |
| `ng_scale.salam`                                                   | Shares one multiply between addresses that scale the same index by a size x86 cannot encode                                                                                 |
| `ng_asm.salam`                                                     | The `--emit-asm` listing                                                                                                                                                    |
| `ng_dump.salam`                                                    | `SALAM_NG_DUMP=<function>` prints a function's optimized IR to stderr                                                                                                       |

The driver side is `compiler/driver/driver_native.salam`. After llvmgen has
produced the IR it builds the object, then links it with in-process lld when
`lb.LinkObject` can handle the triple and with `cc` (or
`x86_64-w64-mingw32-gcc` for the Windows target) otherwise.

### Optimizations

Functions are compiled callees first, so a call can be inlined whenever the
callee is small (about 40 instructions, more for `alwaysinline`) and not
recursive. A function with exactly one call site whose address is never taken
may be up to 300 instructions, since inlining it adds no code. Returning a
small struct, or taking one by value, raises the limit too, since inlining
removes the copies through memory (`image.GetPixel` returns a `Color`). Each function
then goes through:

1. SROA on struct locals, then mem2reg.
2. A chain of `if x == K: ret C` tests over dense keys (at least five, with
   holes allowed when the fallback returns a constant) becomes one range
   check and a load from a read-only table, bytes when the results fit
   (`image.jpeg`'s 64-way `_zigzag`).
3. Inlining, then SROA and mem2reg again for the locals the inlined code used
   to reach only through a pointer. A struct local that is stored whole exactly
   once and otherwise only loaded whole (a by-value parameter's spill slot, or
   the copy an inlined setter makes) is replaced by the stored value, so
   `Put(a, id, node)` copies the node once instead of three times. A local
   filled by one whole-struct copy and afterwards only read field by field
   (`n := at.Get(a, id)` followed by `n.a`, `n.b`) loads just those fields
   from the source at the copy, and a copy that is patched field by field and
   copied straight back to where it came from, with no other memory access in
   between, becomes stores into the original. Below the size limit, a call
   may still be inlined when its arguments make it cheaper: each pointer to
   a scalar local of the caller (an `i &: int` parameter), more for every
   load and store the callee makes through it, and each constant
   argument earns a bonus (not for recursive callees, where inlining the
   first call only grows the caller), since inlining turns the first into a register
   and folds the second. `prim_fold(s, i)` in a scanning loop is the usual
   case. A struct result that the caller only reads field by field earns a
   bonus too (`p_peekn` returns a whole `Token`), and a callee of at most ten
   instructions is inlined even after the caller used up its growth budget.
   A call inside a loop to a callee that makes no calls itself earns a larger
   bonus when the caller has at most 40 blocks (`image.BlendPixel` in a fill
   loop); bigger callers, such as an image codec's frame decoder, only grew
   and got slower with it.
   A callee too big to inline whole may still be inlined in part. When a
   function has a cheap, call-free common path that updates state and a
   region that only some calls reach and that calls out (the body of a loop
   that usually runs zero times, such as the flush loop of `audio.MsbWrite`,
   or a buffer's grow path), the region is copied into a separate function
   `<name>.cold` that takes the values live into it as arguments (up to six;
   field addresses are recomputed inside instead of passed). A call in a loop
   then inlines only the common path, which ends in a call to the cold
   function where the region used to start. The original function is left
   whole for its other callers, and a cold function is only compiled when
   some call site uses it. Under `-g` the cold function keeps the region's
   source lines and gets a subprogram entry under the callee's name, so a
   breakpoint on the function also stops there. In a caller of at most 40
   blocks, calls that arrive through an inlined body and sit in a loop get a
   second chance at being inlined, whole or in part, so `MsbUnary` also
   brings in the bit read its loop makes. The FLAC encoder and decoder in
   `audio_flac` run 8.7% fewer instructions with both (206 million instead of
   226 million; the LLVM build runs 181 million).
4. Tail recursion elimination. `ret f(x)` becomes a jump back to the top of
   the function, and so does `ret f(x) + y` or `ret f(x) * y` by carrying an
   accumulator, which removes one of the two calls in a recursive Fibonacci.
   A small recursive function is then inlined into itself once, which halves
   the calls that are left.
5. Folding to a fixed point: arithmetic and compares on constants, algebraic
   identities, casts, selects, constant branch conditions, `extractvalue` of a
   known `insertvalue`, a struct rebuilt field by field from itself (also
   when a nested struct field is rebuilt leaf by leaf, as a by-value `Token`
   is when it is pushed), address
   arithmetic, `x % 2^k == 0` as a mask test, `(x & y) ^ (x & z)` as
   `x & (y ^ z)`, the bit select `(x & y) ^ (~x & z)` as
   `((y ^ z) & x) ^ z`, and `x < 0 ? ~y : y` as `y ^ (x >> 63)` after
   pulling negations out (`(-x << 1) - 1` is `~(x << 1)`), which turns a
   zigzag decode into four instructions. Signed division and
   remainder become unsigned when the dividend is provably non-negative,
   `sext(x + k)` with a no-overflow add becomes `sext(x) + k` so the constant
   lands in the address displacement, and
   `x / 2^k` becomes a shift under an `x % 2^k == 0` guard. Two bounds on
   the same value (`c >= 97 and c <= 122`, as a branch pair or an `and`)
   become one unsigned compare of `c - 97`, and a sign-extended byte that is
   corrected with `+ 256` when negative (`ByteAt`) becomes a zero-extending
   load. On x86-64 a compare of two zero-extended bytes (or 16-bit values),
   at least one of them loaded, compares the narrow values instead, with a
   signed predicate turned unsigned, so one load folds into the compare:
   `cmp r8b, byte [mem]` in `RawDeflate`'s match-length loop
   (`SALAM_NG_OFF=zxcmp`; `image_rtl_card` runs 3.3 million fewer
   instructions, 2.0%, and `image_basics` 2.2 million).
   `(x + c1) * c2` becomes `x * c2 + c1 * c2` when that continues a
   constant chain (an FNV hash of an int folds to one multiply), but not on
   loop indices, where it would hide the pattern strength reduction needs.
   Inserting a zero into a struct that is still zero at that offset is
   dropped, so a struct literal stores only its nonzero fields after one
   wide clear.
6. Block merging and jump threading, which turns short-circuit `and`/`or`
   chains into direct branches. A block that only compares a phi with a
   constant and branches is bypassed by every predecessor that feeds the phi
   a constant. A range check `x >= 0 and x < n` with `n`
   known non-negative becomes one unsigned compare.
7. If-conversion of small branches (up to four instructions per arm) into
   `cmov`, except where the result flows into a later address, since a `cmov`
   there would serialize the following load (a binary search keeps its branch
   for this reason). A division may move ahead of the branch when the same
   division already ran unconditionally, as in `if n % j == 0: s += n / j`.
   A branch whose arm only tests a second condition before reaching the same
   join (`if pa <= pb and pa <= pc`) first becomes one branch on the `and`
   (or `or`) of both conditions, so the PNG Paeth predictor turns into two
   selects. A select between two adds that share an operand becomes one add
   of a select (`c ? s + a : s + b` is `s + (c ? a : b)`). When both arms
   of a branch end in a load that only feeds the join, the load moves below
   the join on a phi of the two addresses (picking the English or Persian
   keyword table, then one load). Two arms that only compute `addr(G1, i)`
   and `addr(G2, i)` become `addr(select(c, G1, G2), i)`; LICM then moves the
   select out of the loop, so the loop loses the branch on `c`.
8. Full unrolling of loops with a constant trip count and a small body. The
   counter may step by adding, multiplying, shifting or dividing, so a square
   and multiply loop over a constant exponent unrolls too. A loop of any depth
   that provably runs exactly once loses its back edge. The size limit is
   checked per iteration: a branch on the counter (`if i < 15:` inside a
   16-step loop) is resolved for each iteration and only the arm that runs
   is counted, so a carry loop whose last step differs still unrolls. The
   growth from full unrolling does not count against the callee's inline
   limit (up to 320 instructions), so a small carry loop is still inlined
   into its caller after it was unrolled, as LLVM does. Unrolling runs
   again after each value numbering pass that found new constants, so the
   blocked stdlib loops over a `matrix.Zeros(4, 4)` collapse into straight
   code once the sizes are forwarded from the struct. A loop may also leave
   early (`ret i` or `break` inside the body): each copy of the exiting block
   becomes one more incoming edge of the exit block's phis, and a value the
   exit uses directly first gets such a phi. Matching a constant needle in
   `str.IndexFrom` then becomes one `cmp byte [p + k], imm` per needle byte.
9. A loop entered from several blocks first gets a single preheader (the
   incoming values meet in new phis there), so the loop passes below also
   see loops that start right after an `if`. Loop-invariant code motion of
   pure instructions into loop preheaders (an
   add of a loop value, a constant and an invariant is regrouped so the
   invariant part moves out, and an address `p[c + i]` with `c` invariant
   becomes `(p + c)[i]` so the base pointer is computed once), and
   of loads that no store in the loop can reach (by TBAA class) and that are
   safe to run early: a field of a `&:` parameter or of a stack slot. When
   the only writers in the loop are calls, such a load is still hoisted and
   reloaded right after each call, so a vector's data pointer stays in a
   register on the paths that do not call (the recursive `solve` in N-queens).
   A field that the loop also writes is promoted too when every other
   memory access in the loop provably touches something else (another field
   at a known offset, another stack slot, a `malloc` result, or a
   parameter's memory when the field lives in a stack slot) and the calls
   in the loop are conditional (at most four): the field lives in a
   register, is written back before each call and read again after it,
   and is stored once on every edge that leaves the loop. The bit writer
   that FLAC encoding keeps on its stack then keeps its accumulator and
   bit count in registers while it writes a Rice code.
   A floating-point constant compared inside a loop is materialized once in
   the preheader, since `ucomisd` needs it in a register
   (`until zx * zx + zy * zy <= 4.0` in Mandelbrot), and so is a 64-bit
   integer constant that does not fit an immediate. A call to a read-only
   function with invariant arguments moves out of a loop that writes no
   memory when it surely runs in the first iteration: every path from the
   header reaches it without leaving the loop, following the header's
   branch with the values the loop starts with. A `strlen(s)` in the
   condition of a character loop then runs once.
   Then comes
   unswitching: an innermost loop that branches on a loop-invariant condition
   is versioned into one copy per outcome. When the condition compares an
   invariant value with a constant (`kind == 2`), the copy where it holds
   also knows the value itself, so every other `kind == k` in that copy
   folds as well. If-conversion has usually turned the arms of such a
   switch into selects before this runs, so a loop that selects on two or
   more such compares of the same value is unswitched on them too, one
   constant per round, and the copy left over is split on the last one: the
   PNG encoder's filter loop becomes one loop per filter type, each of which
   the vectorizer then handles on its own (`SALAM_NG_OFF=uswsel` keeps the
   older branch-only form). Up to eight rounds may run per function, within
   a budget of 880 cloned instructions. Compares, remainders and
   divisions are folded when the value range of their operand decides them:
   inside `until i < 4000000:` the remainder `(i + 1) % 1000000007` is just
   `i + 1`. Signed division and remainder by a power of two that stay signed
   become a biased shift and mask (`x < 0 ? x + 2^k - 1 : x`), so the
   quotient and remainder of the same value share the bias; the SHA-256
   rotations in `std/crypto` are built from such pairs. A counter bounded by a sign-extended 32-bit length is known to fit
   in 32 bits, so `sext(trunc(i))` on the widened counter folds back to `i`.
   Widening rewrites addresses inside the loop as `base + sext(pos)` plus the
   wide counter, so loop-invariant code motion runs once more afterwards and
   moves the new `sext(pos)` and `base + sext(pos)` to the preheader
   (`SALAM_NG_OFF=winv`). `RawDeflate`'s hash-insert loop loses three
   instructions per position: `image_rtl_card` runs 3.2 million fewer
   instructions (1.9%) and `image_basics` 11.2 million (1.9%).
   Beyond counters, a web of 32-bit values joined by phis, selects and `nsw`
   adds, subtracts and multiplies that ends in sign extensions for addressing
   is computed in 64 bits instead (no signed overflow means the wide result
   is the extension of the narrow one): values entering the web are extended
   once where they are defined, constants are re-emitted wide, a compare
   against a 32-bit value outside the web reads the low half, and the
   extensions disappear. The web is rewritten only when the extensions it
   removes outweigh the ones it adds, each weighted by loop depth. `heap_sort`'s
   `largest`, `2 * i + 1` and `2 * i + 2` lose all their `movsxd`.
   Integer sum loops over an array (`total += v[k]`) get an SSE2 `paddq` or
   `paddd` loop with two accumulators followed by the scalar loop for the
   leftover elements, and a
   loop that only copies elements (`a[k] = b[i]`) becomes `memmove` behind a
   runtime check that a forward copy and `memmove` agree (only from 16
   elements up; shorter copies stay a loop).
   The counter itself (or a value derived from it) can be data: a store of
   `i` or `i * 3 + 7` broadcasts the scalar index and adds a constant
   `[0, 1, ...]` lane vector from `.rdata`.
   Before a loop is matched, the loop vectorizer hoists pure instructions
   whose operands all come from outside the loop (an address base like
   `p + from` that a later rewrite left inside the loop) into the preheader.
   Innermost loops whose body is one branch-free block, counting up to a
   limit with `<` or `<=` (before or after the body), are vectorized with
   SSE2: bytes sixteen lanes at a time, 16-bit integers eight, 32-bit integer
   and `f32` code four, 64-bit and `f64` two. Loads and stores must be
   contiguous in the counter; a loop that also does 32-bit work widens its
   bytes or 16-bit values on load (`movd` or `movsd` and unpacks, then
   `psrad` when the extension is signed) and narrows bytes with
   `packssdw`/`packuswb` on store. A load from a loop-invariant address (the
   data pointer of a vector passed by reference) is done once before the
   vector loop, behind a runtime check that no store in the loop reaches it.
   A second counter that steps down by one while the first steps up (the
   `lo < hi` swap of a reversal) is rewritten as `lo0 + hi0 - i`, the loop
   runs while `i < (lo0 + hi0 + 1) / 2`, and its loads and stores read and
   write the lanes just below the address and reverse them with `pshufd`
   (4- and 8-byte elements). `a[lo]` and `a[hi]` on the same buffer need no
   runtime check, since the loop condition keeps the two ranges apart.
   An integer sum whose addend is `y + c` with `c` the same on every
   iteration accumulates `y` and adds `c` times the element count once after
   the vector loop (`flac`'s Rice cost adds `1 + param` per residual).
   The descending index gets its own 64-bit counter in the vector loop, so
   the swap loop is two loads, two shuffles, two stores and the two counters. A 32-bit multiply, which SSE2 has no
   instruction for, is two `pmuludq` (the even lanes, then the odd ones
   shifted down) joined by `pshufd` and `punpckldq`; on AArch64 it is one
   `mul .4s`. A compare that only feeds selects is
   emitted in its cheaper sense with the select arms swapped, and a
   commutative vector operation writes into an operand that dies there
   (but not into a reduction's accumulator). Adds, subtracts, bitwise operations,
   shifts by a constant or a loop-invariant count (the count is masked like
   a scalar shift's and broadcast once before the loop; a 64-bit arithmetic
   shift, which SSE2 lacks, is a logical shift plus `xor` and `sub` with
   `sign >> count`, and a shift by 63 is `pshufd` of the high halves plus
   `psrad 31`), the four float operations, compares of any sign
   (unsigned 32-bit ones flip the sign bit first; 64-bit ones other than
   equality only on AArch64, since SSE2 needs a long sequence for them)
   and selects (on a per-lane or loop-invariant condition) are supported,
   and any number of integer add reductions and min or max reductions
   (`m = x > m ? x : m`, or the same as an `if`, either sign; 64-bit ones
   on AArch64 only), which end in a shuffle-and-select tree, and counts of bytes that match a
   compare (`if b[i] == 0: n += 1`, either polarity), which mask each match
   to 1 and sum sixteen at a time with `psadbw` (`uaddlv` on AArch64) into
   64-bit lanes. A loop that stores gets a
   runtime check that each stored range is disjoint from every range it
   loads from another base. The vector loop runs while a full vector fits
   and the scalar loop finishes the rest, so the PNG filter scoring and
   the tensor element-wise kernels run four values per instruction.
   Byte and 16-bit code written in `int` (`(x - a) & 255`, `x as int + y`
   then `as u8`) is not widened when the narrow lanes give the same answer:
   the vectorizer first tries the loop with every `int` value in byte lanes,
   then in 16-bit lanes, and only then in 32-bit ones. Adds, subtracts,
   multiplies (16-bit lanes only), bitwise operations and left shifts wrap
   the same way in any width, so they only need the low bits of their
   operands. A value that is compared, shifted right or selected for such a
   use needs its full value instead, so a range pass bounds every `int` in
   the loop (`zext` of a byte is 0 to 255, adds and subtracts add the
   bounds, and so on) and the loop takes the narrow width only if each such
   value fits it. A signed compare or arithmetic shift of values known to be
   non-negative is turned into the unsigned form first, which is the same
   result and fits a byte where the signed form would not. The extends and
   truncates between the widths then disappear, constants and loop-invariant
   values are broadcast at the narrow width, a 16-bit loop that loads bytes
   widens them with one `movq` and `punpcklbw` (and `psraw 8` when signed)
   and stores them back with `pand` and `packuswb`, and byte shifts, which
   SSE2 does not have, are 16-bit shifts plus a mask. A floor average of two
   bytes, `(a + b) >> 1` with both operands under 256, is
   `(a & b) + ((a ^ b) >> 1)` in byte lanes. So the PNG encoder's Sub, Up and
   Average filters run sixteen bytes per iteration (the average keeps its
   nine-bit sum out of the lanes) and its Paeth filter eight, in 16-bit
   lanes. On `apps/image_rtl_card` the loop that applies Sub, Up and Average
   went from 176 to 34 instructions per 16 bytes and the Paeth loop from 158
   to 74 per 8 bytes, 16.6 million instructions down to 5.5. A loop
   with a reduction or a down-counting second counter keeps its old width.
   `SALAM_NG_OFF=vnarrow` turns the narrow lanes off. On a 4096-byte row run
   100000 times (minimum CPU time of five runs), the encoder's combined
   Sub/Up/Average loop takes 0.045 s instead of 0.279 s (LLVM 0.014 s, as it
   gives each filter its own loop), a bare Average filter 0.019 s instead of
   0.179 s (LLVM 0.020 s) and Paeth 0.225 s instead of 0.508 s (LLVM
   0.272 s). AArch64 runs the same code with NEON (`zip1` for the widening,
   `sqxtun` for the narrowing).
   On x86-64 a vector select between `x` and `0 - x` on the sign of `x` is
   an absolute value: one `vpabsd`, `vpabsw` or `vpabsb` with AVX2, and a
   shift, `xor` and subtract (`psrad`, `pxor`, `psubd`) on SSE2
   (`SALAM_NG_OFF=vabs`). Two compares that need an inversion (`<=`, `>=`,
   `!=`) joined by `and` or `or` and feeding only selects are computed as
   the opposite compares joined the other way, and the selects swap their
   arms (`not a and not b` is `not (a or b)`), which saves both inversions
   in Paeth's `pa <= pb and pa <= pc` (`SALAM_NG_OFF=vdemor`). Index math
   such as `j - 4` keeps its no-overflow flag in the vector loop, so the
   address folds into `[base + j - 4]` instead of a `lea` and a `movsxd`
   (`SALAM_NG_OFF=lvnsw`). Counted with callgrind on a copy of the PNG
   encoder's loops over a 4096-byte row, the filter scoring loop runs 28.8
   instructions per byte instead of 33.3 on SSE2 (LLVM 29.8) and 8.9
   instead of 33.3 with AVX2 (LLVM 8.1), the Paeth filter 6.8 instead of 9.1
   (LLVM 11.3) and 2.2 instead of 9.1 (LLVM 2.8), and with one loop per
   filter Sub and Up 0.48 instead of 2.10 (LLVM 0.30 to 0.32) and 0.29
   instead of 1.18 (LLVM 0.11 to 0.14), Average 0.94 instead of 2.10 (LLVM
   0.81) and 0.51 instead of 1.18 (LLVM 0.40). In wall time (median of nine
   interleaved runs, 40000 or 200000 rows, range in brackets) the scoring
   loop takes 301 ms [287-349] instead of 346 ms [328-395] (LLVM 327 ms
   [313-356]) and with AVX2 122 ms [118-128] instead of 344 ms [327-393]
   (LLVM 116 ms [114-132]); Paeth with AVX2 takes 38 ms [36-48] instead of
   98 ms [93-130] (LLVM 54 ms [52-56]), and Sub 46 ms [42-63] instead of
   110 ms [98-157] (LLVM 33 ms [31-51]).
   With `--cpu=x86-64-v3` (or `x86-64-v4`) the loop vectorizer uses the
   256-bit AVX2 registers instead: eight 32-bit or `f32` lanes, four 64-bit
   or `f64`, sixteen 16-bit and thirty-two bytes. The vector step, the
   `[0, 1, ...]` lane constant (now 32 bytes in `.rdata`), the runtime
   overlap checks and the scalar remainder all follow the wider vector.
   Broadcasts are one `vpbroadcastb/w/d/q` or `vbroadcastss/sd` from a `movd`,
   a 32-bit multiply is one `vpmulld` instead of the `pmuludq` sequence, and
   a reduction first combines the upper half, taken with `vextracti128`, with
   the lower one (an add, or a compare and select for min and max) before the
   usual 128-bit steps. Loops in narrowed byte lanes take thirty-two bytes
   per step (the combined PNG filter loop 0.026 s instead of 0.045 s, the
   Average filter 0.012 s), and 16-bit loops that do not load bytes sixteen
   values. Loops that widen bytes or 16-bit values into wider lanes load
   them with one `vpmovzxbw`, `vpmovzxbd` or `vpmovzxwd` (the `sx` forms
   when signed) straight from memory, and loops that store their lanes back
   as bytes mask them, fold the upper half onto the lower one with
   `vextracti128` and pack (`vpackuswb`, after `vpackssdw` for 32-bit
   lanes), so the PNG Paeth filter takes sixteen bytes per step and the
   filter scoring loop eight (`SALAM_NG_OFF=yzext` keeps these loops at 128
   bits). Loops that count matching bytes or walk a second counter down
   keep 128-bit vectors, and so does every loop when `SALAM_NG_OFF=avx2` is
   set. The SLP vectorizer (below)
   first looks for runs of eight adjacent `f32` or four `f64` stores and
   builds them as one 256-bit tree, then packs what is left into 128-bit
   ones, so the 4x16 `f32` matrix multiply micro-kernel keeps eight `ymm`
   accumulators instead of sixteen `xmm` ones (`SALAM_NG_OFF=ypack` keeps SLP
   at 128 bits). The integer sum vectorizer's two accumulators become 256-bit
   too, eight `int` or four `i64` lanes each, so it reads sixteen `int` values
   per iteration (`SALAM_NG_OFF=ysum` keeps it at 128 bits). A function that touches a
   256-bit register is VEX-encoded throughout, scalar float code included,
   so it never mixes legacy SSE with AVX, and it runs `vzeroupper` before
   each call and return; functions without 256-bit code are encoded exactly
   as before. In such a function a register copy followed by a vector
   operation that overwrites the copy (`vmovaps a, b` then `vpsubd a, a, c`)
   becomes the one three-operand form `vpsubd a, b, c` once registers are
   assigned (`SALAM_NG_OFF=vex3`). With AVX2 a vector select is one
   `vpblendvb` instead of a `pand`, `pandn` and `por` on the mask: the
   instruction has a fourth register, the mask, which the register
   allocator tracks as a third input, and a spilled second source is read
   straight from its stack slot (`SALAM_NG_OFF=vblend`). A function with a
   blend is VEX-encoded like one with 256-bit code. A vectorized loop
   without a reduction, whose body is at most 48 instructions, is then
   unrolled the way runtime unrolling handles scalar loops: four copies of
   the body when it has at most 16 instructions and two otherwise, behind
   one check that that many vector steps remain, with the single-step
   vector loop after it taking the last ones. The copies fold their offsets
   into the addresses (`[rdi + r8 + 32]`), they do not count against the
   function's inline cost, and a loop that reads or writes a local array too
   small for two vector steps (the four channel values of a PNG pixel) is
   left alone (`SALAM_NG_OFF=lvunr`). After a 256-bit loop without a
   reduction or a down-counting counter, whose body is at most 48
   instructions, one 128-bit copy of the body runs when half a vector or
   more is left, so at most fifteen bytes of a byte loop (seven values of a
   16-bit one) go through the scalar loop instead of 31. The step uses the
   low halves of the loop's 256-bit constants instead of broadcasting new
   ones, and like the unrolled copies it does not count against the
   function's inline cost (`SALAM_NG_OFF=vtail`).
   On the same copy of the PNG encoder's loops (callgrind,
   4096-byte rows, instructions per byte, the numbers before the blends,
   `xmm14` and the unrolling and LLVM's in brackets), the filter scoring loop runs 6.81 with
   AVX2 (8.93, LLVM 8.57) and 28.59 on SSE2 (28.84, LLVM 30.26), where
   `xmm14` keeps one more accumulator out of memory. Sub and Up run 0.34
   on SSE2 (0.48, LLVM 0.30 to 0.33) and 0.22 with AVX2 (0.29, LLVM 0.11 to
   0.14), Average 0.90 and 0.49 (0.99 and 0.53, LLVM 0.81 and 0.40) and
   Paeth 6.65 and 1.88 (6.83 and 2.22, LLVM 10.76 and 2.64). The 128-bit
   tail step takes Sub and Up with AVX2 to 0.20 (0.226 and 0.224 before it),
   Average to 0.46 (0.49) and Paeth to 1.83 (1.89); a 4092-byte Sub row now
   ends in one 16-byte step and twelve scalar bytes instead of 28 scalar
   bytes. LLVM still runs Sub and Up at 0.11 to 0.13: in this copy of the
   loops it knows the row length, ends each row in unrolled 8-byte steps
   and reads one operand of each `vpsubb` straight from memory, which the
   native loop loads first. In wall time on a shared machine (median of
   nine interleaved runs, 200000 rows, load average about five on six cores)
   AVX2 Sub takes 23 ms [18-45] instead of 29 ms [18-42] (LLVM 17 ms
   [16-37]), Up 20 ms [19-42] instead of 22 ms [20-38] (LLVM 14 ms [13-32])
   and Average 39 ms [31-85] instead of 43 ms [31-79] (LLVM 36 ms [25-73]).
   With the constant-pool splats and rematerialized constants (see the
   register allocator below) the filter scoring loop runs 27.59 on SSE2
   (28.59 before, LLVM 30.00) and 6.57 with AVX2 (6.82, LLVM 9.06), and
   Paeth 6.28 and 1.64 (6.65 and 1.83, LLVM 10.69 and 2.63); the 16-bit
   Paeth lanes are masked to bytes with one `vpand` from the constant pool
   per store instead of a `mov`, `vmovd`, `vpbroadcastd` and `vpand`.
   Measured the same way (40000 rows), Paeth takes 74 ms [71-84] on SSE2
   instead of 80 ms [75-100] (LLVM 122 ms [118-127]) and 27 ms [24-36] with
   AVX2 instead of 31 ms [26-37] (LLVM 49 ms [46-56]); the scoring loop
   stays within the noise, 313 ms [290-461] against 316 ms [302-424] on
   SSE2 and 91 ms [85-110] against 90 ms [85-118] with AVX2 (LLVM 329 ms
   [314-492] and 123 ms [120-132]).
   In wall time (median of nine interleaved runs,
   40000 or 200000 rows, range in brackets) the AVX2 scoring loop takes
   92 ms [85-235] instead of 115 ms [110-150] (LLVM 115 ms [112-121]), Sub
   29 ms [24-37] on SSE2 instead of 46 ms [34-69] (LLVM 29 ms [25-39]) and
   19 ms [19-21] with AVX2 instead of 25 ms [21-31] (LLVM 16 ms [15-19]),
   and Paeth with AVX2 141 ms [136-153] instead of 163 ms [156-170] (LLVM
   230 ms [223-249]).
   The register allocator keeps 256-bit values in the same class
   as the other vector registers and gives them 32-byte spill slots, read and
   written with unaligned moves so the frame stays 16-byte aligned; on Windows
   it keeps them out of `xmm6`-`xmm15`, whose upper halves a callee may
   clobber. `SALAM_NATIVE_CPU` sets the level when `--cpu` is not given, so a
   whole test sweep can run with AVX2, and both reach `--lib` builds too. The default stays the x86-64 baseline
   (SSE2); a program built for `x86-64-v3` needs a CPU with AVX2 and stops
   with an illegal instruction on one without it, and a program built for
   `x86-64-v2` needs SSE4.1 the same way. `--cpu` also reaches LLVM (`-march`
   for clang, the target machine's CPU in-process) when the backend is `llvm`.
   `--cpu=x86-64-v2` keeps 128-bit vectors and legacy (non-VEX) encoding and
   replaces SSE2 sequences in the vectorizers' output with the SSSE3 and
   SSE4.1 instruction that does the same job. At this level:
   - A vector select is one `pblendvb`. Its mask has to sit in `xmm0`, so
     instruction selection copies the mask there right before the blend and
     the allocator sees `xmm0` as a fixed use, the way a shift sees `rcx`.
     The mask's own register gets `xmm0` as a hint, so a mask that is only
     used by the blend is computed in place and the copy disappears. In a
     function with such a blend the allocator hands out `xmm0` last, so other
     values do not take it and get moved out again. A select is then at most
     three instructions (the copy of the mask, the copy of the tied first
     source, the blend) against five for `pand`, `pandn` and `por` with their
     copies. A spilled or rematerialized second source is read straight from
     its stack slot or the constant pool: legacy encoding wants 16-byte
     alignment there, which both have.
   - A select that picks one of the two values it compares is one `pminsd`,
     `pmaxud`, `pminsb`, `pmaxuw` or similar, at 8, 16 and 32 bits, and the
     compare goes away when nothing else uses it. Minimum and maximum
     reductions use the same instructions, on 256-bit values too at
     `x86-64-v3`.
   - Loads that widen are one `pmovzx*` or `pmovsx*` from memory, an
     absolute value is one `pabsb`, `pabsw` or `pabsd`, a 32-bit multiply is
     one `pmulld`, an equal compare of 64-bit lanes is one `pcmpeqq`, and
     stores that narrow lanes to bytes are one `pshufb` with a mask from the
     constant pool (`pand` and packs before).

   `SALAM_NG_OFF=sse41` turns all of it off, and `SALAM_NG_OFF=vblend` just
   the blends. With `sse41` off, `--cpu=x86-64-v2` writes the same bytes as
   the default level, and `--cpu=x86-64-v3` the same bytes as before this
   level had any code. `blendvps` and `blendvpd` are not used: `pblendvb` does the job
   for every lane width because the masks come from compares and are all
   ones or all zeros per lane. Measured numbers are under "Where it stands"
   below.
   On the machine these numbers come from (an AMD EPYC of the first Zen
   generation, which splits each 256-bit operation into two 128-bit halves,
   so it shows the low end of the gain) the minimum CPU time of five runs of
   a loop over 4096 elements, called 400000 to 1000000 times, drops by 2.0x
   for an `f32` saxpy, 2.0x for an `int` sum, 2.1x for an `i64` sum, 2.0x for a
   `u8` clamp, 2.0x for an `f32` dot product with `--fast-math`, 1.97x for an
   `int` min and max, 1.54x for `int` add and multiply, 1.34x for an `f64`
   scale and 1.0x for an `i16` multiply. With the 256-bit SLP trees a
   256x256 `f32` `tensor.GemmAcc`, run 40 times, takes 0.033 s instead of
   0.063 s (1.9x; the SSE2 build takes 0.060 s), and the `nn` demos that
   spend their time in it gain: `nn_cnn_train_demo` 0.708 s to 0.619 s,
   `nn_transformer_demo` 0.820 s to 0.726 s, `nn_mlp_train_demo` 0.317 s to
   0.300 s and `nn_layers2_demo` 0.069 s to 0.065 s. An `int` and an `i64`
   sum over 4096 elements, 400000 times, take 0.199 s with 256-bit
   accumulators instead of 0.257 s with 128-bit ones. The `audio` and
   `image` tests do not change, and none of the 42 programs under
   `benchmark/` has a loop the vectorizer takes on, so their binaries do not
   change.
   A field read from a struct loaded whole (`v.get(i).kind`, or a phi of two
   such loads) becomes a load of just that field, and a stack copy of a
   struct that is only ever written is dropped, so `n := at.Get(a, i)` then
   `n.kind` no longer copies the whole node.
   A demanded-bits pass drops masks whose cleared bits no later instruction
   reads, so `x = (x * a + c) % 2^31` keeps the mask off its loop-carried
   chain. Loop load elimination keeps a value stored in one iteration and
   loaded in the next (`cur[j]` then `cur[j - 1]`) in a register, and 32-bit
   counters that are only sign-extended for indexing become 64-bit counters.
   Index expressions built from such a counter (`row - col + n - 1`) are
   rebuilt in 64 bits with their loop-invariant part summed once before the
   loop.
   Range checks on the loop counter (`c - 1 >= 0 and c - 1 < w`) split the
   loop in two: a copy without the checks runs while the counter is inside
   the range where they all hold, and the original loop runs the iterations
   at the edges.
   Small innermost loops whose trip count is only known at runtime
   (`until j < i:` with `j += 1`, or counting down) run two iterations per
   test, four when the body is tiny, with the original loop finishing the
   rest. The test compares the
   counter with a limit adjusted once before the loop, so it never computes a
   counter value that could overflow.

10. Strength reduction: an array address that is affine in a loop counter, such
    as `b[k * n + j]`, becomes a pointer bumped by a constant each iteration.
    Addresses that differ only by a constant share one pointer.
11. SLP vectorization: runs of stores to adjacent `f32` or `f64` slots are
    traced back through arithmetic, broadcasts, adjacent loads and loop phis,
    and the whole tree becomes SSE `ps`/`pd` code (AVX2 `ymm` code for runs of
    eight `f32` or four `f64` under `--cpu=x86-64-v3`). The tensor and matrix
    multiply kernels rely on this. A broadcast of a function parameter is
    placed at the top of the entry block, so straight-line code in that
    block can use it.
12. Global value numbering over the dominator tree, including redundant load
    elimination and store-to-load forwarding. A store only invalidates loads
    of its own TBAA class (llvmgen tags pointer fields and plain data
    separately), the same guarantee the LLVM backend gets from those tags.
    A stack slot whose address never escapes has its own memory version, so
    a store to one local array keeps loads from another one (in
    `x25519`'s `_gf_mul`, `a.v[i]` stays in a register while `t[i + j]` is
    updated). A store to such a slot also leaves loads from every other
    place alone, so a struct copied into a local and patched field by field
    keeps the AST's base pointer in a register.
    Pointers that come from different `malloc`/`calloc`/`realloc` calls never
    alias, so writing one vector's buffer keeps loads from another one. A
    block-local pass then forwards stores to loads at the same constant
    offset, dropping only the entries a store can overlap, so filling a fresh
    buffer field by field and reading it back costs no loads.
    A field load also sees an earlier whole-struct store. Calls to functions
    declared `memory(read)` or `memory(argmem: read)` (`strcmp`, `strlen`, the
    math library), and `malloc`/`calloc`, do not invalidate loads, so a
    vector's data pointer stays in a register across a `strcmp` loop. Two
    such calls with the same arguments and no write between them are one
    call, so `strlen(s)` computed twice runs once. Known values survive a
    join when no block between it and its dominator writes memory and that
    region has no loop (or when the function writes no memory at all), so
    `L.pos` read before an `if` is not read again after it. A division or
    remainder equal to one that dominates it is reused (`hash % cap` in `has`
    then `get`). A load
    after a merge whose value every predecessor already has (stored or
    loaded) becomes a phi, which keeps the print buffer's length in a
    register across its flush branch. When exactly one predecessor lacks the
    value, the load moves into that predecessor instead (it must lead only to
    the merge, or the address must be a global, a stack slot or a non-null
    parameter), so the common path keeps the value in a register and only the
    rare path reloads it.
13. Min/max reassociation: in `min(min(a, b), c)` where `a` comes from the
    previous iteration (a phi, or a phi plus a constant) and `b` and `c` do
    not, the tree becomes `min(a, min(b, c))`, so the loop-carried chain goes
    through one compare and one `cmov` instead of two. The edit distance
    inner loop is the usual case.
14. Dead store elimination on stack slots whose address never escapes: a
    store, or a constant-length `memset`, is dropped when later stores in the
    same block overwrite all of it before anything reads it, or when the
    block returns before any read. Zeroing a struct and then setting every
    field leaves only the field stores.
15. Mark-and-sweep dead code elimination, which also removes dead phi cycles.
    A loop with no side effects whose values nobody reads after it, and whose
    counter provably reaches its bound, is removed (a sum whose result was
    dropped leaves an empty counting loop behind).
16. Sinking: a pure instruction whose only uses sit in one block it dominates,
    in the same loop, moves into that block, so `dp[j] + 1` is computed only
    on the path that stores it.

An integer parameter of an internal function becomes a constant when every
call passes the same value: a literal, or a local that is only ever assigned
that literal, and recursive calls pass the parameter through unchanged. The
`n` of a recursive `solve(row, n, ...)` is the usual case.

Loads of internal globals that the module never writes (how llvmgen emits a
`const`) fold to their initializer, and so do small copies out of constant
strings, so `println "hello"` stores the bytes as immediates. `strlen` of a
constant string folds to its length, and a `memcpy` whose length is then a
small constant becomes inline moves, so `name + "_"` no longer measures or
calls anything for the `"_"`. When a function uses several addresses
`base + i * s` with the same `i` and a scale `s` that x86 cannot encode
(a 280-byte AST node), `i * s` is computed once. Instruction selection folds address
arithmetic into memory operands, folds a
single-use load into the ALU instruction or compare that consumes it (a
byte load only into a compare with a constant), gives a global that a loop
reads by index one base register set up at function entry (x86 cannot add an index
to a `rip`-relative address), turns a
load, an add/sub/and/or/xor and a store back to the same address into one
`add [mem], reg` (or immediate), loads a
32-bit value with `movslq` when it is also sign-extended (the low half serves
the 32-bit uses), ties a commutative floating-point add or multiply to the
operand that dies there (and squares `x * x` in one register), ties an
integer multiply to a freshly loaded operand that dies there, ties a `cmov`
to the operand that dies there by inverting the condition, turns
`x & 0xffffffff` into a 32-bit `mov`, emits `ror` for
`(x >> n) | (x << (bits - n))` (also with `^` or `+`) and `rol %cl` for the
variable form `(x << r) | (x >> (32 - r))` even when the counts are masked
with `& 31`, builds an `insertvalue` chain that is returned directly in
the caller's result buffer (or, when the chain is only stored to a local
or through a pointer argument with no load, store or call in between,
directly in that memory), copies and clears structs of up to 256 bytes
with 16-byte SSE moves (larger ones call `memcpy`/`memset`), fuses a
compare with the branch or select that uses it, and turns `x * k + y` into one
or two `lea`s. Division by a constant uses a shift or a multiply by the
reciprocal, sized to the dividend's known width, so `(i / 50) % 7` costs two
multiplies and a few shifts. The width comes from value ranges too: a counter
below five million, or a digit loop `t = t / 10` started from such a counter,
gets a reciprocal that fits a 32-bit `imul` immediate. A division by a variable shares one `div` between
the quotient and the remainder and, on x86-64 only, uses the 32-bit `div`
when both 64-bit operands fit. Register allocation is linear scan over exact live
ranges with use counts weighted by loop depth. In a loop with more live values
than registers, the values it uses get a copy at its preheader, so the
allocator can spill the long outer range and keep the loop copy in a register.
The reverse case is handled before instruction selection: a value computed in
a loop that contains a call and is still used after the loop's single exit is
copied at that exit, so its loop range is short enough to stay in a register
(a vector's data pointer during a `push` loop). After rewriting, a reload of a spill slot into the scratch register that
already holds it (with no write to the slot or the register in between) is
dropped. Inside an innermost loop without calls, a stack slot that is only
read gets loaded once before the loop into a register the loop does not
touch and that is not live on entry, and the reloads become uses of that
register. When a reload is replaced by a register this way, or a reload
becomes a copy into `r10` or `r11` on x86-64, and the scratch register is
not written again before the next label, jump or return, its uses are renamed
and the copy dropped, since the scratch registers never carry a value
across a block boundary (`SALAM_NG_OFF=scdead`; 1.4 million fewer
instructions for `image_rtl_card`, 1.9 million for `image_basics`).
A load of a stack slot that was just stored from a register (or an
immediate) of the same size, with no write to the slot or the register in
between and no jump target on the way, becomes a register copy (or a move of
the immediate). This runs after allocation and again after the encoder's jump
cleanup, and on x86-64 a copy into a scratch register is folded into the
next instruction that reads it. The same walk on x86-64 tracks which 64-bit
registers hold equal values after a copy and drops a copy whose registers
are already equal, such as the second half of `mov r8, rax` followed by
`mov rax, r8` (one instruction per divisor in `prime_factorization`).
A loop whose counter only feeds its own `+ 1` and its `< N` exit test, with
a constant start and bound, counts down to zero instead, and on x86 a
`test r, r` right after an `add`, `sub` or `lea` of the same register (or a
bitwise operation) is dropped when the branch after it only reads the zero
or sign flag, the `lea` becoming an `add`. `hello_print`'s loop ends in
`sub ebx, 1` and `jne`. A load into a register that is only tested for zero
or sign becomes `cmp [mem], 0` when every path from there writes the
register again before reading it (a bounded walk over both branch targets),
like the print buffer's `is a terminal` flag. A select between `x` and `x + 1` on an
unsigned or `>=`/`<=` float compare is `adc r, 0` or `sbb r, -1` on the carry
flag (`monte_carlo_pi`'s `count += hit`), the way LLVM writes it; NaN leaves
the carry set, so the float form stays correct for unordered inputs. A multiply of a value by itself squares the
destination register (`imul r, r` after the copy), so the source dies at the
copy and the pair coalesces (`pow_mod`'s squaring chain).
Before instruction selection on x86, when a two-address operation (`sub`,
`divsd` and the like overwrite their first operand) reads a value that has
one or two later pure uses in the same block, and those uses only need
values that are ready already, they move ahead of it so the value dies at
the operation and needs no copy. `sum + sign / den` followed by
`0.0 - sign` negates first and divides `sign` in place (`pi_leibniz` runs
11% fewer instructions). A later use that is itself tied to the same value
stays put unless it is commutative with an operand that dies there, or
takes a constant (`lea` and `imul` with an immediate need no tie), so DCT
butterflies keep their order. A loop value that also feeds the phi
leaving the loop still counts as dying there, since that phi reads the
header's next definition, and an add, subtract or multiply by a constant is
never the tied operation (it is three-address already).
When the hints of a value are taken, the allocator tries the register of
any value it is copied to or from that already has one, so phi webs and the
copies around them tend to land in one register. A `select` whose result
replaces a loop value (the value dies at the `select` and the result flows
back into its phi) is emitted with that loop value as the tied operand of the
`cmov`, so the loop carries it in one register with no copy at the latch. A
32-bit division fast path leaves its quotient and remainder in `rax` and
`rdx` on both paths and copies them once after the join.
Copy-related values that both spill share one stack slot. A value that
got a register but is mostly copied to and from values sharing one such
slot is then put in that slot too, when the copies that disappear outweigh
the loads and stores its other uses need, weighted by loop depth, and its
range does not overlap anything else living in the slot
(`SALAM_NG_OFF=cpspill`). A vector accumulator whose sum is spilled no
longer goes through a register and back every iteration: `EncodePNG`'s
filter-scoring loop drops four instructions per four bytes, and
`image_rtl_card` runs 1.6 million fewer instructions (1.0%),
`image_basics` 2.1 million. When vector
registers run out, the allocator evicts the value whose next use is
farthest away, so a kernel with sixteen accumulators keeps most of them in
registers instead of spilling each one as a temporary arrives.
On x86-64 a vector that repeats one constant (a broadcast of a constant,
or the byte mask and sign bias that narrowing stores and unsigned compares
need) is loaded from the constant pool instead of being assembled in a general
register and moved over with `movd` and a shuffle or broadcast, and a vector
operation that uses such a mask as its source reads it from the pool,
`pand xmm0, [rip + c]` (`SALAM_NG_OFF=vkpool`). The allocator
rematerializes a spilled constant vector, one loaded from the pool (or a
copy of one) or a zeroed one: a reload loads the constant again or zeroes
the register instead of reading the stack slot, and the source operand of
a vector operation or blend reads the pool directly. Such a constant whose
every use can read it from memory (those sources and, without the
three-operand VEX forms, the source of a copy, which needs its own move
anyway) costs nothing to spill, so the allocator spills it before it
evicts anything else (`SALAM_NG_OFF=remat`). `EncodePNG`'s filter scoring
loop then keeps all of its accumulators in registers: 113 instructions per
four bytes on SSE2 instead of 117 and 51 per eight with AVX2 instead of 53. Scalar constants are not rematerialized: a general register reload is
already one instruction, and the loop slot hoisting above handles them.
The SSE2 scoring loop still reloads its two row pointers from the stack
for each of its four loads: the general registers are taken by values
that live through the loop without being used in it, and neither
rematerialization nor a larger slot-hoisting budget changes that.

A value that does not get a register for its whole range is split instead
of living in its stack slot from start to end (`SALAM_NG_OFF=rasp` turns
this off). A value evicted by a heavier one keeps its register up to the
eviction, and that point moves back to the cheapest block boundary before
it, so a loop is not entered with the value in a register it loses inside
the loop. The rest stays in the stack slot and gets a register again from
the cheapest boundary before its next use, but only when the uses it
covers there, weighted by loop depth, pay at least three times for the
reload. Uses that cost nothing from memory do not count: the source of a
copy (the reload goes straight into the copy's register) and, on x86, the
second operand of an ALU instruction, which reads the slot directly. A
value that only has a register free for part of its range takes it for
that part when the part covers two uses, and pieces made by splitting
never evict other values. Every definition of a split value is stored to
its slot, so giving up a register needs no store, while taking one is a
load or a move. Where the two ends of a block edge disagree on a value's
location, the moves go at the end of a predecessor with one successor or
at the start of a successor with one predecessor, as one parallel copy
with `r11` or `xmm15` breaking cycles. Splits never fall between an
instruction that sets the flags and the jump, `setcc`, `cmov` or carry
operation that reads them, after a block's first branch, or inside the
short jumps a block's own lowering contains (an atomic retry loop, a
select's branch). A function with a critical edge that would need moves
(no predecessor or successor of its own to hold them) is allocated again
without splitting; that happens in about one function in three hundred.

The encoder rotates small loop headers into the latch so
loops are bottom-tested (a header of up to ten instructions with one early
exit, like `mandelbrot`'s `zx * zx + zy * zy <= 4.0 and iter < max_iter`,
is copied whole), copies a short loop test into a branch arm that
would otherwise jump to it (up to eleven instructions when that test is the
loop's latch, so a division fast path ends in the loop branch instead of a
`jmp` to it), shares one epilogue between a function's returns
on Linux, zeroes registers with `xor` where the flags are dead (a following `div` or `imul` overwrites them anyway), drops a `test r, r` that repeats the one before a conditional jump (and any
compare or test whose flags nothing reads, such as one left behind when its
branch folded to a constant), and aligns loop heads to 16 bytes. An innermost loop
that fits in 64 bytes is placed so it does not straddle a cache line;
otherwise it gets 32-byte alignment. Padding uses NOPs of up to 15 bytes,
and padding longer than that is a short `jmp` over the gap, so falling into
an aligned loop costs one instruction.

### Conventions worth knowing

- Scalars follow the C ABI of the target, so calls to and from C code just
  work. Between Salam functions, struct values travel by pointer to a copy,
  and struct returns use a hidden result pointer.
- A struct passed to or returned from an `extern` C function follows the
  target's C rules (`ng_cabi.salam`). On x86-64 and AArch64 llvmgen already
  splits it into the pieces the ABI wants, and a larger result's hidden
  pointer goes in `x8` on AArch64. On i686 every struct argument is
  copied onto the stack; Linux returns every struct through a hidden pointer
  that the callee pops (`ret 4`), while Windows returns 1, 2, 4 and 8 byte
  structs in `eax`/`edx:eax` and one holding a lone `float` or `double` in
  `st0`. On armhf a homogeneous struct of up to four floats or doubles uses
  consecutive VFP registers (or the stack, after which no float back-fills),
  other structs fill `r0`-`r3` a word at a time and spill onto the stack
  (8-byte aligned ones start at an even register), and a result of up to 4
  bytes comes back in `r0`. On RISC-V a struct of up to 16 bytes goes in
  `a`/`fa` registers (one or two floats, or a float and an integer, use the
  float registers while enough are free), a larger one by reference.
- On Linux frames are addressed from `rsp` and `rbp` is a sixth callee-saved
  register. Each function gets an `.eh_frame` entry, so gdb, perf and other
  unwinders can walk native frames. On Windows `rbp` is the frame pointer and
  the prologue has the shape the Windows unwinder expects (pushes,
  allocation, `lea rbp, [rsp+N]`), with an unwind entry per function.
- On Linux, a function whose entry only compares a parameter with a constant
  and branches to a block returning a constant or a parameter (`if nid == 0:
ret end` at the top of a recursive AST walk) tests that parameter before
  the prologue and returns without saving any register. The `.eh_frame`
  entry starts its pushes after this short exit path.
- `r10`, `r11` and `xmm15` are kept free as scratch registers for spill
  code. On x86-64 Linux and macOS `xmm14` is allocated like the other vector
  registers (`SALAM_NG_OFF=xfree` keeps it free as well); the rare
  instruction whose spilled operands need a second vector scratch register
  borrows one of `xmm8`-`xmm14` that holds nothing live there, or saves one
  to the stack around the instruction when all of them do. On Windows and
  the other targets the vector scratch registers stay as they were.
- Stack temporaries share space when their live ranges do not overlap. A
  slot whose address is taken with `lea` stays live until the next call in
  its block (or the block's end), since the call reads it through the
  register.
- After block layout, a `jcc` whose flags come from a constant just moved
  into a register (`mov r, K; test r, r`, optionally through `setcc` and a
  second `test`) becomes a `jmp` or disappears.
- An `i1` value lives in the low byte of a register as 0 or 1. Narrow
  integers are extended where an operation needs the full width.

## Testing and debugging

- `tools/bash/backend-sweep.sh <salam> native` builds every test with the
  native backend and compares the output with the C backend's `.out` files.
  With the default sections it agrees on everything except
  `interop/redis_demo`, which needs a live redis server, the same as the LLVM
  backend.
  With `SALAM_NATIVE_CPU=x86-64-v3` in the environment every build uses the
  256-bit AVX2 loops, so the same sweep (and the fuzzer below) checks them
  against the same `.out` files.
- `python3 -I tools/fuzz/salamfuzz.py <salam>` generates random programs
  (integer code with loops and vectors, vector kernels over `int`, `u8`,
  `i16` and `i64`, clamped `f64` code, calls with eight arguments and a
  struct passed by reference, string building, slicing and UTF-8 walks,
  generic functions and a generic struct over several element types, and
  lambdas with captures called directly, from a Vector and through `dyn`,
  and workers on spawned threads that add into a shared total under a
  `sync` mutex, so the result does not depend on scheduling)
  that are well defined by construction (values are kept small with a prime
  modulus, divisors and shift counts are offset or masked, indices are
  wrapped into range), builds each with the reference backend and the native
  backend, and compares their output. `--count`, `--seed`, `--target` with
  `--runner`, `--ref` and `--keep` (which saves the programs that differ)
  adjust it. The gate runs 30 on x86-64 and 15 per emulated target, seeded
  with the run number, so each run tries new programs.
- `python3 -I tools/linker/machorun.py <exe>` runs a linked x86-64 macOS
  executable on x86-64 Linux: it maps the segments into this process, applies
  the rebase and bind opcodes against glibc (Darwin names such as `___error`
  and `___stdoutp` map to their glibc counterparts) and calls `_main`.
  `readdir$INODE64` goes through a shim that copies glibc's entry into the
  Darwin `struct dirent` layout, so directory listings work. Programs that
  only compute, print and use files run unchanged; anything else that passes
  a Darwin struct to libc (sockets, `stat`, signals) does not. Compiling the
  `basics`, `features`, `types`, `data`, `games`, `match`, `switch`, `llvm`
  and `general` tests for `x86_64-apple-macos` and running them this way
  matches the expected output for every test outside that set.
- `tools/bash/native-cross-check.sh <salam> aarch64-linux-musl qemu-aarch64`
  builds the tests for another target (also `i686-linux-musl` with `env` as
  the runner, `arm-linux-musleabihf` and `riscv64-linux-musl`), links them
  against a musl sysroot and runs them under qemu. It skips the tests that
  need `-D` flags, a database library or a trapping release-mode division,
  plus on 32-bit targets the two that assume a 64-bit `usize` or an `i64`
  `abs()`; everything else must pass, and the native backend gate runs it for
  all four targets on every change. The native-built compiler for AArch64
  reproduces the x86-64 compiler's IR for the whole compiler.
  With `NATIVE_CROSS_STAGE=<dir>` it only builds the programs, into
  `<dir>/<group>/<name>`, and `tools/bash/native-stage-tests.sh <dir>` runs
  them on the machine they are for. The gate does this for macOS on Apple
  silicon and on Intel, Windows on x86-64 and Linux on AArch64: the programs
  are built on Linux, run on a real machine of each kind, and the compiler
  built by the native backend for that machine reproduces the reference IR
  there and builds and runs programs with `--backend=native` itself.
  `tools/bash/native-hw-stage.sh` adds the C ABI tests (native and LLVM
  backends), the timer test and a few `--icf` programs to those machines and
  to Windows on Arm, with i686 under its x86 emulation. On arm64 macOS and
  Windows on Arm, `tools/bash/native-icf-compiler.sh` has a compiler linked
  with `--icf` reproduce the reference IR too.
- Those native-built compilers are kept for internal use (they are not
  releases). `tools/bash/package-native.sh` lays one out like a release
  archive (`salam`, `std/`, a `BUILD.txt` naming the commit, and for Windows the mingw-w64 or llvm-mingw
  runtime under `sysroots/`) as `salam-native-<platform>.tar.gz`, and
  `tools/bash/native-package-check.sh <tarball>` unpacks it into an empty
  folder and, with `SALAM_STD` unset, builds and runs a few tests with
  `--backend=native`, among them the TCP and HTTP loopback tests and the TLS
  self-test. The gate does this on the real machine for `linux-x64` (a static
  musl build), `linux-arm64`, `macos-arm64`, `macos-x64`, `windows-x64` and
  `windows-arm64`, and on pushes to `main` and manual runs it uploads each
  package that passed as the workflow artifact `salam-native-<platform>`,
  kept for 14 days.
- `tools/bash/native-compile-report.sh <salam>` builds the compiler and a few
  large tests with `--backend=native` and with `--backend=llvm` and prints a
  Markdown table of build times and executable sizes; the gate's `linux` job
  adds it to its run summary. Without in-process LLVM, `--backend=llvm` hands
  the IR to the `clang` on `PATH`. It fails only on gross regressions,
  measured on the totals as native over LLVM ratios, so machine speed cancels
  out: more than `NATIVE_REPORT_MAX_TIME_RATIO` (0.6) of LLVM's build time or
  `NATIVE_REPORT_MAX_SIZE_RATIO` (1.5) of its size. `NATIVE_REPORT_RUNS`
  repeats each build and keeps the fastest.
- `tools/bash/native-encoder-check.sh <salam> <files...>` checks the encoder:
  it assembles the `--emit-asm` listing with GNU `as` and compares the result
  with the object nativegen wrote, instruction by instruction.
  `NATIVE_ENC_FLAGS="--release --cpu=x86-64-v3"` checks the VEX encodings of
  the vectorized loops too.
- `SALAM_BACKEND=native sh tools/bash/run-tests.sh <groups>` runs the regular
  build tests through the native backend.
- `SALAM_NG_DUMP=main salam build app.salam --backend=native` prints the
  optimized IR of `main`. Function names are the mangled ones you see in the
  listing, and a comma-separated list dumps several functions.
- `SALAM_NG_OFF=gvn,ifc,slp,thread,inline,sroa,tailrec,selfinl,unroll,licm,unswitch,lle,dbits,idiom,rvec,iss,runroll,widen,sink,lhoist,mlp,ipcp,belady,aload,lsr,lcse,range,split,mmchain,inl1,afwd,lpro,lsf,scse,lpre,dtest,fch,p2div,aggbonus,rot,bitsimp,xsplit,lreas,swtab,allocep,dse,sxadd,rmw,mtie,usim,inlunr,chain,selhoist,mpre,dloop,lvec,vnarrow,shoist,asplit,pst,sfld,isite,chst,fastx,sscale,ccse,kstr,nowr,crng,jload,mxun,gbase,bcmp,mreas,dcse,aggres,tiny,kbr,paddr,izero,istore,stfwd,seltie,cpref,llphi,memaddr,vshv,lvhoist,vcount,viv,loopinl,cpelim,tord,vrev,cdown,tfold,mexit,sqself,vsign,redinv,ldcmp,phix,selinc,wweb,wwdep,wwtr,tdup,padjmp,dtst,zins,ndiv,niv,cmp64,adc64,rvc,avx2,rasp,hotpath,inlnest,spromo,ypack,ysum,winv,zxcmp,cpspill,scdead,yzext,uswsel,vex3,vabs,vdemor,lvnsw,vblend,xfree,lvunr,vtail,vkpool,remat,sse41`
  turns optimizer passes off, which is the quickest way to find the pass
  behind a wrong result. `lcse` is the load part of value numbering and
  `split` the loop live-range splitting in the allocator and `rasp` the
  splitting of a value's range between registers and its stack slot. `hotpath` is the
  partial inlining through `.cold` functions, `inlnest` the second inlining
  round for calls that came in with an inlined body, and `spromo` the
  promotion of fields the loop writes. `yzext` is the 256-bit form of loops
  that widen or narrow their lanes, `uswsel` the unswitching of selects on
  a switch value, `vex3` the three-operand VEX rewrite, `vabs` the vector
  absolute value, `vdemor` the swapped compares under `and` and `or`,
  `lvnsw` the no-overflow flag on vector index math, `vblend` the AVX2
  `vpblendvb` and `x86-64-v2` `pblendvb` selects, `xfree` the allocation of
  `xmm14`, `lvunr` the unrolling of vectorized loops and `sse41` the SSSE3 and
  SSE4.1 code of `--cpu=x86-64-v2`.
- `SALAM_NG_OFF_RANGE=lo:hi` limits `SALAM_NG_OFF` to the functions compiled
  in that range (1-based, in compile order), and `SALAM_NG_OFF_BASE` names
  passes to turn off everywhere. Halving the range finds the one function a
  pass miscompiles; turning `inline` off in the base keeps a callee's
  optimized body from hiding inside its callers.
- `SALAM_NG_OFF_FNS=f1,f2` limits `SALAM_NG_OFF` to the named functions
  instead. When a pass only breaks a program in combination (a callee and the
  callers it was inlined into), drop names from the full list one at a time;
  `SALAM_NG_LIST=1` prints every function with its compile index.
- `SALAM_NG_INLTRACE=1` prints why the inliner kept a call: callee too big
  (`skip`, with its cost and limit), not cached yet, caller over its growth
  budget (`grown`), or a call-site bonus too small (`need`). Lines starting
  with `hotpath` show where a callee was split for partial inlining, and
  `spromo` the loops whose fields were promoted.
  `SALAM_NG_INL_MAX=n` overrides the base inline limit for experiments.
- `SALAM_NG_VERIFY=1` checks after optimization that every instruction's
  operands are defined in a block that dominates it, and fails the build
  naming the function and block when one is not. Run it whenever a pass
  moves or creates instructions.
- `SALAM_NG_GVN_MAX=n` stops value numbering after `n` merges and prints the
  function the last one happened in, so a miscompile can be bisected down to
  a single merge. `SALAM_NG_GVN_SKIP=addr,extv,icmp,cast,arith` leaves whole
  instruction kinds alone.

### Debug information

`salam build app.salam --backend=native -g` writes DWARF 5
into the object: `.debug_info`, `.debug_abbrev`, `.debug_line`,
`.debug_addr`, `.debug_rnglists` and `.debug_loclists`. `-gdwarf-4` writes
DWARF 4 instead, with `.debug_ranges` and `.debug_loc` in place of the last
three, and `-gdwarf-5` names the default. With it gdb shows file and line for every
frame, steps through loops, stops on inlined functions by name, and prints
locals and arguments. Debug info never changes the generated code: the
`.text` of a `-g` build is byte-identical to one without it, which
`tools/bash/native-debug-check.sh` checks on every program.

How the pieces fit:

- The frontend emits `!dbg` locations, a `DIFile` per source file,
  `DILocalVariable` plus `llvm.dbg.declare` for every local and parameter,
  and DWARF types (basic types, pointers, `str` as a `char` pointer, structs
  with their members, fixed arrays). Building those types never pulls in
  a package's globals or functions, which only the code decides, so `-g`
  keeps the order of globals and the data layout of a build without it.
- `ng_dbg.salam` prescans that metadata. A location is an interned pair of
  `DILocation` and inlined-at location, so the inliner rebases callee
  locations onto the call site. The encoder records a line row whenever the
  line changes and a range whenever the inline context changes; those
  become `DW_TAG_inlined_subroutine` entries with `DW_AT_ranges` when an
  inlined body is split.
- Variables (`ng_dvar.salam`) never enter the IR as instructions, so no pass
  can treat them as real code. mem2reg turns every promoted store and phi
  into an event `(variable, value)` anchored to an instruction or a block
  start. Events follow their anchor when an instruction is deleted, follow
  value replacements, and are copied with the value map when unroll,
  unswitch or runtime unrolling clone code; other cloning passes mark the
  variable unreliable instead. SROA leaves become fragments of the
  original variable and are written with `DW_OP_piece`.
- At instruction selection a reaching-definitions pass gives every block
  its incoming values. Before register rewriting the events become location
  markers on machine instructions, checked against each virtual register's
  live segments; a register location stays valid after the last use until
  something overwrites that register or a call clobbers it. The encoder
  turns the markers into `.debug_loc` lists: a register, a stack slot,
  memory behind a pointer register, or a constant for values that were
  folded away. A value that does not exist at a point shows as
  `<optimized out>`, never as a stale value.
- When a function is inlined, its cached body carries its variable events,
  so the inlined frame shows its own arguments and locals.
- Module globals get a `DW_TAG_variable` with `DW_OP_addr`, unless global
  dead code elimination removed them.
- Blocks that declare variables become `DILexicalBlock`s, written as
  `DW_TAG_lexical_block` with their own ranges, so gdb only shows the
  variables in scope and two `a`s in sibling branches stay apart. Lambdas
  get their own subprogram.
- The version travels in the IR: llvmgen writes it into the `"Dwarf Version"`
  module flag and nativegen reads it from there, so `--backend=llvm` follows
  the same flags and default. The C backend passes `-gdwarf-4` or
  `-gdwarf-5` to the C compiler when one is given.
- In DWARF 5 the units have a version 5 header (`DW_UT_compile`). The line
  table header describes its entries with `DW_LNCT_path` and
  `DW_LNCT_directory_index`; directory 0 is the compilation directory and
  file 0 the unit's source file, which is also file 1 (as gcc does), so the
  file numbers in the line program and in `DW_AT_decl_file` are the same as
  in DWARF 4. There are no MD5 checksums, since the frontend has none to give.
  Every code address in `.debug_info` (the `DW_AT_low_pc` of the unit,
  functions, inlined calls and lexical blocks) is a `DW_FORM_addrx` index into
  `.debug_addr`, and the unit carries `DW_AT_addr_base`. Range and location
  lists are `DW_RLE_offset_pair`/`DW_LLE_offset_pair` entries from the unit's
  base address, referenced with `DW_FORM_sec_offset`, so no list base
  attributes are needed; their ULEB128 offsets make `.debug_loclists` about
  half the size of `.debug_loc`. Names stay inline (`DW_FORM_string`):
  `.debug_str_offsets` and `DW_FORM_strx` pay off with split DWARF, which the
  backend does not write. `DW_AT_language` stays `DW_LANG_C99` in both
  versions, so gdb and lldb read expressions as C.
- `dsymutil` rewrites location lists with `DW_LLE_base_addressx`, which needs
  the unit's `DW_AT_addr_base`; it only writes one when its input has one.
  That is why DWARF 5 units use `.debug_addr` even though the relocation
  count is the same as with `DW_FORM_addr`: without it a dSYM fails
  `llvm-dwarfdump --verify` with an unresolved address index per variable.
- Instructions inlined from a function without debug info (the runtime's
  print helpers) take the location of the call, as LLVM's inliner does, and
  instruction selection starts every function with no location, so the
  argument moves at a function's entry stay on its opening line instead of
  the last line of the function emitted before it, and the code of an
  inlined `println` belongs to the line that prints.
- Windows objects carry the same sections (long names through the COFF
  string table, `SECREL` relocations), and stack slots are addressed from
  `rbp` there. Both the built-in PE linker and mingw `ld` keep them, so gdb
  for mingw reads lines, frames and variables.
- On Linux i686, armhf, AArch64 and RISC-V the encoders record the same line
  rows, function ranges and variable locations, with each target's DWARF
  register numbers and its stack pointer as the frame base; the 32-bit
  targets write 4-byte DWARF addresses. gdb (natively, or `gdb-multiarch`
  against `qemu -g`) stops on a source line, prints arguments and locals,
  and walks the frames; `tools/bash/native-gdb-check.sh <salam> <triple>
<runner>` checks that on each target. Windows x64 and Windows on Arm images
  carry the same DWARF in PE sections (offsets as `SECREL`). 32-bit Windows
  objects carry it too, with code addresses as `DIR32`, plus a `.debug_frame`
  built from the same CFI as i686 Linux's `.eh_frame`, since those frames
  are `esp`-based and Windows has no `.pdata` on x86;
  `tools/bash/native-windows-debug-check.sh` checks both Windows targets.
- Mach-O objects carry the same DWARF under `__DWARF`. Offsets between
  debug sections are written into the object resolved, since Mach-O linkers
  do not relocate them, and code addresses are section-relative `UNSIGNED`
  relocations. The built-in linker keeps the sections in a `__DWARF` segment
  of the executable, where lldb reads them directly, the way Go programs
  carry theirs, together with the DWARF of C objects and archive members
  linked in (see `NATIVE_LINKER.md`). With `--linker=system` they stay in the object and the
  executable's debug map points at it, so the driver runs `dsymutil` (or
  `llvm-dsymutil`) before it deletes the object, as clang does, and lldb
  finds the result in `<program>.dSYM`. Without either tool it keeps the
  object and says where it is.
  `tools/bash/native-lldb-check.sh` builds a program on any host and debugs
  it under lldb on a Mac.
- Each function's first line row after its opening line carries
  `DW_LNS_set_prologue_end`, so `break func` stops after the arguments have
  been loaded into registers. gdb only trusts its own prologue analysis for
  compilers it knows, and on i686, where every argument arrives on the
  stack, that would stop before any of them has a location.
- Arguments also have a location before the prologue, for a debugger that
  stops on a function's first instruction (`break *func`, or a caller's
  `stepi`). Instruction selection records where each argument arrives: a
  register, or on i686 and x86-64 a stack slot, written as
  `DW_OP_call_frame_cfa` plus its offset, since those functions have CFI.
  That location covers the entry block from the function's first byte for as
  long as the variable still holds the argument and nothing has overwritten
  the register (an instruction other than the argument moves, or one that
  writes it, ends a register location). A struct argument passed by pointer
  and split into fields by SROA gets the whole struct behind that pointer
  until the first of its fields is loaded.

Enums are written as enumeration types, so gdb prints `Blue`, not `2`.
On Windows targets, `-g` also writes CodeView into each object: a
`.debug$S` section with `S_OBJNAME`, `S_COMPILE3`, one
`S_GPROC32`/`S_LPROC32` per function, C13 line blocks, file checksums and a
string table (`ng_cv.salam`). Scalar locals and parameters get `S_LOCAL`
with built-in type indices (integers, floats, `bool`, and `str` as a char
pointer) and the same ranges as their DWARF location lists:
`S_DEFRANGE_REGISTER` for a register, `S_DEFRANGE_REGISTER_REL` for a stack
slot. A variable that is one constant everywhere becomes `S_CONSTANT`.
Structs, enums, arrays and pointers get type records in `.debug$T`
(a pointer to a struct goes through a forward reference, so self-referential
structs work), and a struct local split into registers gets one
`S_DEFRANGE_SUBFIELD_REGISTER` per field, or `S_DEFRANGE_REGISTER_REL` with
its field offset when that field is on the stack. Each inlined call gets an
`S_INLINESITE` (with an `LF_FUNC_ID` for the callee and binary annotations
for its code ranges and lines, the call-site line of any call inlined
further in), a `DEBUG_S_INLINEE_LINES` entry gives the callee's first line,
and the function's own line table gives the outermost call site for inlined
code, so a debugger shows the inlined frames and steps over them as calls.
i686 functions never keep a frame pointer, so on that target each function
also gets an `S_FRAMEPROC` (frame size, saved register bytes, both frame
registers `VFRAME`) and a `DEBUG_S_FRAMEDATA` subsection, the FPO data a
Windows debugger walks such stacks with. It has one record per prologue
step, as clang writes them: one at the function start, one after each
register push and one after the stack allocation, each with the local,
parameter and saved register sizes, the remaining prologue length and a
program (`$T0 .raSearch = $eip $T0 ^ = $esp $T0 4 + = $ebx $T0 4 - ^ = ...`)
that finds the return address and the saved registers. The relocation in
front of the records is `DIR32NB`, so the records hold offsets from the
function start.
The built-in linker turns all of it into a PDB (see `NATIVE_LINKER.md`);
with the system linker, GNU ld discards it.

`tools/bash/native-gdb-check.sh <salam>` debugs a small program under gdb
and checks the printed locals, a struct split into pieces, folded
constants, an inlined frame, and the arguments at a function's first
instruction.

A good end-to-end check is the compiler itself: emit its IR with
`salam llvm compiler/main.salam`, build that IR with the native backend, and
have the result emit the same IR again. The two files must be identical
apart from the build timestamp.

## Where it stands

Measured by CPU time against the LLVM backend at `-O3` on the benchmark suite,
the native backend comes out at about 0.96x in geometric mean and is faster on
19 of the 42 programs (among them `fib_recursive`, `fib_iterative`,
`lcg_random`, `dot_product`, `caesar_cipher`, `counting_sort`,
`game_of_life` and `knapsack_01`). Counted in executed instructions, which is
steadier than wall time on a loaded machine, the geometric mean is 0.95x. The
largest remaining gap is `matrix_stdlib_smallops` (about 1.2x), where LLVM
specializes the generic blocked matrix multiply for 4x4 operands. A compiler
built with the native backend runs within a few percent of one built with
LLVM on a full self-compile, while the native build itself is several times
faster than the LLVM path.

Library code is further behind. Counted with callgrind, the image tests
`apps/image_rtl_card` and `apps/image_basics` run 1.08x and 1.18x the
instructions of their LLVM builds (143.5 against 132.4 million, 555.4
against 471.5 million; before the constant-pool splats and
rematerialization they were 144.9 and 557.3 million, before the AVX2 blends, `xmm14` and the vector loop
unrolling they were 145.6 and 558.6 million, before the 256-bit widening, per-filter unswitching
and vector `abs` they were 152.3 and 570.3 million, 1.15x and 1.21x, before
the narrow byte and 16-bit lanes 163.3 and 587.2 million, 1.23x and 1.25x,
and before `winv`, `zxcmp`, `cpspill` and `scdead` 174.6 and 608.0 million,
1.32x and 1.29x). With `--cpu=x86-64-v3` the two take 116.0 and 515.2
million against LLVM's 101.9 and 415.0, 1.14x and 1.24x (116.3 and 515.7
million before the constant-pool splats and rematerialization, 116.4 and
518.1 million before the 128-bit tail step, 119.7 and 523.3
million before the blends, `xmm14` and the unrolling, 151.6 and 569.0
million, 1.49x and 1.37x, before the 256-bit widening). On `image_rtl_card`, `EncodePNG` and
`RawDeflate` now run 90.5 million instructions against 97.4 million for
LLVM's `Encode`, which inlines both, and 62.9 million against 67.5 million
with AVX2 (91.8 and 63.2 million before the rematerialization), where the filter scoring loop
with its five `int` sums runs eight lanes in both builds. What
is left is mostly not spilling: LLVM inlines `DrawLine` into a constant
horizontal span and unswitches the alpha test that keeps native `DrawLine`'s loop at full
register pressure (about 8 million), and in `image_basics` it inlines
`_vp8_quant` with constant arguments, peels the DC coefficient and hoists
the reciprocal division out of the loop.

With `--cpu=x86-64-v2` the two take 130.7 and 534.7 million instructions
against 143.6 and 555.4 at the default level (135.6 and 542.4 with
`SALAM_NG_OFF=vblend`, so without the blends) and LLVM's 118.3 and 441.9 at the
same level. On the copy of the PNG encoder's loops, 4096-byte rows, in
instructions per byte, with callgrind:

| loop           | default | v2 without blends | v2    | LLVM default | LLVM v2 |
| -------------- | ------- | ----------------- | ----- | ------------ | ------- |
| filter scoring | 27.33   | 22.09             | 18.09 | 29.50        | 21.74   |
| Paeth          | 6.28    | 4.03              | 3.66  | 9.76         | 6.39    |
| Sub            | 0.35    | 0.35              | 0.35  | 0.32         | 0.32    |
| Up             | 0.35    | 0.35              | 0.35  | 0.30         | 0.30    |
| Average        | 0.90    | 0.90              | 0.90  | 0.81         | 0.81    |

Sub, Up and Average run in byte lanes with no select, so this level has nothing
for them. Copying the mask into `xmm0` never made a loop worse: all eighteen
`tests/en/general/lvec_*` programs take the same or fewer instructions with the
blends than with `SALAM_NG_OFF=vblend`. Over 40000 rows, the median and range of
nine interleaved runs, with the load around 4 on 6 cores from other jobs,
scoring takes 175 ms [170-184] at `x86-64-v2` and 280 ms [274-293] at the
default level (LLVM 200 ms [191-207] and 309 ms [303-337]), and Paeth 42 ms
[38-44] and 69 ms [65-74] (LLVM 77 ms [73-80] and 103 ms [100-114]).

Small loops over 4096 elements, default to `x86-64-v2` (LLVM at `x86-64-v2`), in
million instructions for 1000 calls: an `int` min and max reduction 18.7 to 9.5
(4.9), a `u8` clamp over 16384 bytes 15.7 to 12.6 (9.6), `int` add and multiply
14.5 to 7.3 (7.2). Median and range of nine interleaved runs, same load: the min and max with 100000 calls 103 ms [96-125] to 46 ms [42-81] (LLVM 36 ms [34-43]), add and multiply with 100000 calls 91 ms [81-102] to 57 ms [53-83] (LLVM 58 ms [56-67]), the clamp with 40000 calls 32 ms [31-57] to 31 ms [28-46] (LLVM 23 ms [20-27]). An `i16` multiply and an `f32` saxpy do not change.
None of the 42 programs in `benchmark/programs` changes: at `x86-64-v2` each
is the same binary as at the default level, so the level brings nothing to the
benchmark table. All of these come from one machine (an AMD EPYC of the first
Zen generation). Published tables put `pmulld` at two micro-ops and 10 cycles of
latency on Intel since Haswell; no Intel CPU has been timed here.

Not there yet:

- CodeView carries no FPO frame data, so on i686 a debugger walks the
  stack by frame pointer only.
- The loop and SLP vectorizers run on x86-64 and AArch64 (SLP also on
  i686); armhf and RISC-V baselines have no vector unit to use.
- The loop vectorizer needs a loop body that if-conversion can turn into
  one block (a load inside one arm of a branch keeps it scalar) and
  unit-stride accesses (counting up, or down by one through a second
  counter). On x86-64 it has no 64-bit multiplies or ordered
  64-bit compares, and reductions run in 32- or 64-bit lanes (narrower
  values are widened first). Float
  reductions are not reordered, since that would change the rounding of the
  result, unless the program is built with `--fast-math`, which lets float
  add reductions run in vector lanes too (and gives LLVM `reassoc nsz`).
  AVX2 is opt-in (`--cpu=x86-64-v3`) and the loop, SLP and sum-reduction
  vectorizers use it; loops that count bytes or reverse stay at 128 bits.
  `x86-64-v2` stays at 128 bits and uses no SSE4.2 (`pcmpgtq`), so ordered
  64-bit compares stay out of its loops too. There is no
  AVX-512 or FMA code,
  no 64-bit ordered compare even with AVX2, and no runtime dispatch, so a
  build is either baseline everywhere or needs AVX2 everywhere.
- The built-in linker (see `NATIVE_LINKER.md`) links every supported
  target itself (static musl, glibc PIE, Windows PE), so a native build needs
  no outside tool beyond the C runtime files. It keeps `.debug_*` sections
  and writes `.eh_frame_hdr`.
