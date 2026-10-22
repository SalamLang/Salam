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
recursive. A function with exactly one call site whose address is never taken
may be up to 300 instructions, since inlining it adds no code. Each function
then goes through:

1. SROA on struct locals, then mem2reg.
2. Inlining, then SROA and mem2reg again for the locals the inlined code used
   to reach only through a pointer. A struct local that is stored whole exactly
   once and otherwise only loaded whole (a by-value parameter's spill slot, or
   the copy an inlined setter makes) is replaced by the stored value, so
   `Put(a, id, node)` copies the node once instead of three times.
3. Tail recursion elimination. `ret f(x)` becomes a jump back to the top of
   the function, and so does `ret f(x) + y` or `ret f(x) * y` by carrying an
   accumulator, which removes one of the two calls in a recursive Fibonacci.
   A small recursive function is then inlined into itself once, which halves
   the calls that are left.
4. Folding to a fixed point: arithmetic and compares on constants, algebraic
   identities, casts, selects, constant branch conditions, `extractvalue` of a
   known `insertvalue`, a struct rebuilt field by field from itself, address
   arithmetic, and `x % 2^k == 0` as a mask test. Signed division and
   remainder become unsigned when the dividend is provably non-negative, and
   `x / 2^k` becomes a shift under an `x % 2^k == 0` guard.
5. Block merging and jump threading, which turns short-circuit `and`/`or`
   chains into direct branches. A block that only compares a phi with a
   constant and branches is bypassed by every predecessor that feeds the phi
   a constant. A range check `x >= 0 and x < n` with `n`
   known non-negative becomes one unsigned compare.
6. If-conversion of small branches (up to four instructions per arm) into
   `cmov`, except where the result flows into a later address, since a `cmov`
   there would serialize the following load (a binary search keeps its branch
   for this reason). A division may move ahead of the branch when the same
   division already ran unconditionally, as in `if n % j == 0: s += n / j`.
7. Full unrolling of loops with a constant trip count and a small body. The
   counter may step by adding, multiplying, shifting or dividing, so a square
   and multiply loop over a constant exponent unrolls too. A loop of any depth
   that provably runs exactly once loses its back edge, and unrolling runs
   again after each value numbering pass that found new constants, so the
   blocked stdlib loops over a `matrix.Zeros(4, 4)` collapse into straight
   code once the sizes are forwarded from the struct.
8. Loop-invariant code motion of pure instructions into loop preheaders, and
   of loads that no store in the loop can reach (by TBAA class) and that are
   safe to run early: a field of a `&:` parameter or of a stack slot. When
   the only writers in the loop are calls, such a load is still hoisted and
   reloaded right after each call, so a vector's data pointer stays in a
   register on the paths that do not call (the recursive `solve` in N-queens).
   Then comes
   unswitching: an innermost loop that branches on a loop-invariant condition
   is versioned into one copy per outcome. Compares, remainders and
   divisions are folded when the value range of their operand decides them:
   inside `until i < 4000000:` the remainder `(i + 1) % 1000000007` is just
   `i + 1`. A counter bounded by a sign-extended 32-bit length is known to fit
   in 32 bits, so `sext(trunc(i))` on the widened counter folds back to `i`.
   Integer sum loops over an array (`total += v[k]`) get an SSE2 `paddq` or
   `paddd` loop with two accumulators followed by the scalar loop for the
   leftover elements, and a
   loop that only copies elements (`a[k] = b[i]`) becomes `memmove` behind a
   runtime check that a forward copy and `memmove` agree.
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
9. Strength reduction: an array address that is affine in a loop counter, such
   as `b[k * n + j]`, becomes a pointer bumped by a constant each iteration.
   Addresses that differ only by a constant share one pointer.
10. SLP vectorization: runs of stores to adjacent `f32` or `f64` slots are
    traced back through arithmetic, broadcasts, adjacent loads and loop phis,
    and the whole tree becomes SSE `ps`/`pd` code. The tensor and matrix
    multiply kernels rely on this.
11. Global value numbering over the dominator tree, including redundant load
    elimination and store-to-load forwarding. A store only invalidates loads
    of its own TBAA class (llvmgen tags pointer fields and plain data
    separately), the same guarantee the LLVM backend gets from those tags.
    Pointers that come from different `malloc`/`calloc`/`realloc` calls never
    alias, so writing one vector's buffer keeps loads from another one. A
    block-local pass then forwards stores to loads at the same constant
    offset, dropping only the entries a store can overlap, so filling a fresh
    buffer field by field and reading it back costs no loads.
    A field load also sees an earlier whole-struct store. Calls to functions
    declared `memory(read)` or `memory(argmem: read)` (`strcmp`, `strlen`, the
    math library), and `malloc`/`calloc`, do not invalidate loads, so a
    vector's data pointer stays in a register across a `strcmp` loop. A load
    after a merge whose value every predecessor already has (stored or
    loaded) becomes a phi, which keeps the print buffer's length in a
    register across its flush branch.
12. Min/max reassociation: in `min(min(a, b), c)` where `a` comes from the
    previous iteration (a phi, or a phi plus a constant) and `b` and `c` do
    not, the tree becomes `min(a, min(b, c))`, so the loop-carried chain goes
    through one compare and one `cmov` instead of two. The edit distance
    inner loop is the usual case.
13. Mark-and-sweep dead code elimination, which also removes dead phi cycles.
14. Sinking: a pure instruction whose only uses sit in one block it dominates,
    in the same loop, moves into that block, so `dp[j] + 1` is computed only
    on the path that stores it.

An integer parameter of an internal function becomes a constant when every
call passes the same value: a literal, or a local that is only ever assigned
that literal, and recursive calls pass the parameter through unchanged. The
`n` of a recursive `solve(row, n, ...)` is the usual case.

Loads of internal globals that the module never writes (how llvmgen emits a
`const`) fold to their initializer, and so do small copies out of constant
strings, so `println "hello"` stores the bytes as immediates. Instruction selection folds address
arithmetic into memory operands, folds a
single-use load into the ALU instruction or compare that consumes it, loads a
32-bit value with `movslq` when it is also sign-extended (the low half serves
the 32-bit uses), builds an `insertvalue` chain that is returned directly in
the caller's result buffer, fuses a
compare with the branch or select that uses it, and turns `x * k + y` into one
or two `lea`s. Division by a constant uses a shift or a multiply by the
reciprocal, sized to the dividend's known width, so `(i / 50) % 7` costs two
multiplies and a few shifts. The width comes from value ranges too: a counter
below five million, or a digit loop `t = t / 10` started from such a counter,
gets a reciprocal that fits a 32-bit `imul` immediate. A division by a variable shares one `div` between
the quotient and the remainder and uses the 32-bit `div` when both 64-bit
operands fit. Register allocation is linear scan over exact live
ranges with use counts weighted by loop depth. In a loop with more live values
than registers, the values it uses get a copy at its preheader, so the
allocator can spill the long outer range and keep the loop copy in a register. After rewriting, a reload of a spill slot into the scratch register that
already holds it (with no write to the slot or the register in between) is
dropped.
Copy-related values that both spill share one stack slot. When vector
registers run out, the allocator evicts the value whose next use is
farthest away, so a kernel with sixteen accumulators keeps most of them in
registers instead of spilling each one as a temporary arrives. The encoder rotates small loop headers into the latch so
loops are bottom-tested, copies a short loop test into a branch arm that
would otherwise jump to it, shares one epilogue between a function's returns
on Linux, zeroes registers with `xor` where the flags are dead, and aligns loop heads to 16 bytes. An innermost loop
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
- `SALAM_NG_OFF=gvn,ifc,slp,thread,inline,sroa,tailrec,selfinl,unroll,licm,unswitch,lle,dbits,idiom,rvec,iss,runroll,widen,sink,lhoist,mlp,ipcp,belady,aload,lsr,lcse,range,split,mmchain,inl1,afwd,lpro,lsf,scse`
  turns optimizer passes off, which is the quickest way to find the pass
  behind a wrong result. `lcse` is the load part of value numbering and
  `split` the loop live-range splitting in the allocator.
- `SALAM_NG_OFF_RANGE=lo:hi` limits `SALAM_NG_OFF` to the functions compiled
  in that range (1-based, in compile order), and `SALAM_NG_OFF_BASE` names
  passes to turn off everywhere. Halving the range finds the one function a
  pass miscompiles; turning `inline` off in the base keeps a callee's
  optimized body from hiding inside its callers.
- `SALAM_NG_VERIFY=1` checks after optimization that every instruction's
  operands are defined in a block that dominates it, and fails the build
  naming the function and block when one is not. Run it whenever a pass
  moves or creates instructions.
- `SALAM_NG_GVN_MAX=n` stops value numbering after `n` merges and prints the
  function the last one happened in, so a miscompile can be bisected down to
  a single merge. `SALAM_NG_GVN_SKIP=addr,extv,icmp,cast,arith` leaves whole
  instruction kinds alone.

A good end-to-end check is the compiler itself: emit its IR with
`salam llvm compiler/main.salam`, build that IR with the native backend, and
have the result emit the same IR again. The two files must be identical
apart from the build timestamp.

## Where it stands

Measured by CPU time against the LLVM backend at `-O3` on the benchmark suite,
the native backend comes out at about 1.05x in geometric mean and is faster on
17 of the 42 programs (among them `fib_recursive`, `lcg_random`, `dot_product`,
`sum_mod`, `caesar_cipher`, `monte_carlo_pi` and `trapezoidal_integration`).
Against `gcc -O3` on the C backend it is even or faster on about a third of
them. The largest remaining gaps are `game_of_life` and
`longest_increasing_subsequence`, where LLVM also splits the index range of the
inner loop and unrolls it.

Not there yet:

- No DWARF line tables (`-g`), so there is no source-level stepping, although
  backtraces work through `.eh_frame`.
- Loop vectorization covers integer sum reductions only. Other vector code
  comes from the straight-line (SLP) vectorizer.
- x86-64 only. aarch64 would reuse everything up to instruction selection.
- Linking still uses lld or the system linker. A built-in static linker over
  the bundled musl `libc.a` would make Linux builds fully self-contained.
