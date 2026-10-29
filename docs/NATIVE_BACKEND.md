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
salam build app.salam --backend=native --emit-asm     # also writes app.s
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
   load. `(x + c1) * c2` becomes `x * c2 + c1 * c2` when that continues a
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
   is versioned into one copy per outcome. Compares, remainders and
   divisions are folded when the value range of their operand decides them:
   inside `until i < 4000000:` the remainder `(i + 1) % 1000000007` is just
   `i + 1`. Signed division and remainder by a power of two that stay signed
   become a biased shift and mask (`x < 0 ? x + 2^k - 1 : x`), so the
   quotient and remainder of the same value share the bias; the SHA-256
   rotations in `std/crypto` are built from such pairs. A counter bounded by a sign-extended 32-bit length is known to fit
   in 32 bits, so `sext(trunc(i))` on the widened counter folds back to `i`.
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
   `sign >> count`), the four float operations, compares of any sign
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
   filter application and the tensor element-wise kernels run four values
   per instruction.
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
    and the whole tree becomes SSE `ps`/`pd` code. The tensor and matrix
    multiply kernels rely on this.
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
register.
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
`sub ebx, 1` and `jne`.
Before instruction selection on x86, when a two-address operation (`sub`,
`divsd` and the like overwrite their first operand) reads a value that has
one or two later pure uses in the same block, and those uses only need
values that are ready already, they move ahead of it so the value dies at
the operation and needs no copy. `sum + sign / den` followed by
`0.0 - sign` negates first and divides `sign` in place (`pi_leibniz` runs
11% fewer instructions). A later use that is itself tied to the same value
stays put unless it is commutative with an operand that dies there, or
takes a constant (`lea` and `imul` with an immediate need no tie), so DCT
butterflies keep their order.
When the hints of a value are taken, the allocator tries the register of
any value it is copied to or from that already has one, so phi webs and the
copies around them tend to land in one register. A `select` whose result
replaces a loop value (the value dies at the `select` and the result flows
back into its phi) is emitted with that loop value as the tied operand of the
`cmov`, so the loop carries it in one register with no copy at the latch. A
32-bit division fast path leaves its quotient and remainder in `rax` and
`rdx` on both paths and copies them once after the join.
Copy-related values that both spill share one stack slot. When vector
registers run out, the allocator evicts the value whose next use is
farthest away, so a kernel with sixteen accumulators keeps most of them in
registers instead of spilling each one as a temporary arrives. The encoder rotates small loop headers into the latch so
loops are bottom-tested, copies a short loop test into a branch arm that
would otherwise jump to it, shares one epilogue between a function's returns
on Linux, zeroes registers with `xor` where the flags are dead (a following `div` or `imul` overwrites them anyway), drops a `test r, r` that repeats the one before a conditional jump, and aligns loop heads to 16 bytes. An innermost loop
that fits in 64 bytes is placed so it does not straddle a cache line;
otherwise it gets 32-byte alignment.

### Conventions worth knowing

- Scalars follow the C ABI of the target, so calls to and from C code just
  work. Struct values, which only Salam code passes, travel by pointer to a
  copy, and struct returns use a hidden result pointer.
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
- `r10`, `r11`, `xmm14` and `xmm15` are kept free as scratch registers for
  spill code.
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
- `python3 -I tools/fuzz/salamfuzz.py <salam>` generates random programs
  (integer code with loops and vectors, vector kernels over `int`, `u8`,
  `i16` and `i64`, clamped `f64` code, calls with eight arguments and a
  struct passed by reference, string building, slicing and UTF-8 walks,
  generic functions and a generic struct over several element types, and
  lambdas with captures called directly, from a Vector and through `dyn`)
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
  Programs that only compute and print run unchanged; anything that passes a
  Darwin struct to libc (sockets, `stat`, signals) does not. Compiling the
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
- `tools/bash/native-encoder-check.sh <salam> <files...>` checks the encoder:
  it assembles the `--emit-asm` listing with GNU `as` and compares the result
  with the object nativegen wrote, instruction by instruction.
- `SALAM_BACKEND=native sh tools/bash/run-tests.sh <groups>` runs the regular
  build tests through the native backend.
- `SALAM_NG_DUMP=main salam build app.salam --backend=native` prints the
  optimized IR of `main`. Function names are the mangled ones you see in the
  listing, and a comma-separated list dumps several functions.
- `SALAM_NG_OFF=gvn,ifc,slp,thread,inline,sroa,tailrec,selfinl,unroll,licm,unswitch,lle,dbits,idiom,rvec,iss,runroll,widen,sink,lhoist,mlp,ipcp,belady,aload,lsr,lcse,range,split,mmchain,inl1,afwd,lpro,lsf,scse,lpre,dtest,fch,p2div,aggbonus,rot,bitsimp,xsplit,lreas,swtab,allocep,dse,sxadd,rmw,mtie,usim,inlunr,chain,selhoist,mpre,dloop,lvec,shoist,asplit,pst,sfld,isite,chst,fastx,sscale,ccse,kstr,nowr,crng,jload,mxun,gbase,bcmp,mreas,dcse,aggres,tiny,kbr,paddr,izero,istore,stfwd,seltie,cpref,llphi,memaddr,vshv,lvhoist,vcount,viv,loopinl,cpelim,tord,vrev,cdown,tfold,zins,ndiv,niv,cmp64,adc64,rvc`
  turns optimizer passes off, which is the quickest way to find the pass
  behind a wrong result. `lcse` is the load part of value numbering and
  `split` the loop live-range splitting in the allocator.
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
  budget (`grown`), or a call-site bonus too small (`need`).
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

`salam build app.salam --backend=native -g` writes DWARF 4
into the object: `.debug_info`, `.debug_abbrev`, `.debug_line`,
`.debug_ranges` and `.debug_loc`. With it gdb shows file and line for every
frame, steps through loops, stops on inlined functions by name, and prints
locals and arguments. Debug info never changes the generated code: the
`.text` of a `-g` build is byte-identical to one without it, which
`tools/bash/native-debug-check.sh` checks on every program.

How the pieces fit:

- The frontend emits `!dbg` locations, a `DIFile` per source file,
  `DILocalVariable` plus `llvm.dbg.declare` for every local and parameter,
  and DWARF types (basic types, pointers, `str` as a `char` pointer, structs
  with their members, fixed arrays).
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
  get their own subprogram. Both backends emit DWARF 4.
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
The built-in linker turns all of it into a PDB (see `NATIVE_LINKER.md`);
with the system linker, GNU ld discards it.

`tools/bash/native-gdb-check.sh <salam>` debugs a small program under gdb
and checks the printed locals, a struct split into pieces, folded
constants and an inlined frame.

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
  There is no AVX code path.
- The built-in linker (see `NATIVE_LINKER.md`) links every supported
  target itself (static musl, glibc PIE, Windows PE), so a native build needs
  no outside tool beyond the C runtime files. It keeps `.debug_*` sections
  and writes `.eh_frame_hdr`.
