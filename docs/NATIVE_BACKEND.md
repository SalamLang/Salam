# The native backend

`--backend=native` turns a Salam program into x86-64 machine code without
going through LLVM or a C compiler. It writes the object file itself (ELF on
Linux, COFF on Windows, Mach-O on macOS), so the only outside tool left is the linker, and when
the compiler has LLVM embedded even that runs in-process through lld.

```sh
salam build app.salam --backend=native
salam run app.salam --backend=native
salam build app.salam --backend=native --target=x86_64-w64-windows-gnu
salam build app.salam --backend=native --target=x86_64-apple-macos
salam build app.salam --backend=native --emit-asm     # also writes app.s
salam obj app.salam --backend=native                  # object file only
```

Targets: x86-64 Linux (System V ABI, ELF), x86-64 Windows (Win64 ABI,
COFF with `.pdata`/`.xdata` unwind tables) and x86-64 macOS (System V ABI,
Mach-O). Any other `--target` is rejected with a message that points at
`--backend=llvm`.

On macOS the code is the same as on Linux; only the object differs. Symbols
get the Darwin `_` prefix, calls use `X86_64_RELOC_BRANCH` (the linker adds
stubs), data that lives in another image is loaded through
`X86_64_RELOC_GOT_LOAD`, other RIP-relative references are
`X86_64_RELOC_SIGNED` with the addend stored in the instruction, and every
relocation names its symbol (local ones included), as x86-64 Mach-O expects.
Read-only data that holds pointers goes to `__DATA,__const`, constructors to
`__mod_init_func`. The object carries no unwind tables or DWARF yet, so `-g`
adds nothing on macOS.

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
   Innermost loops whose body is one branch-free block are vectorized with
   SSE2: 32-bit integer and `f32` code four lanes at a time, 64-bit and `f64`
   two at a time. Loads and stores must be contiguous in the counter; bytes
   are widened to 32-bit lanes on load (`movd` and two unpacks) and narrowed
   with `packssdw`/`packuswb` on store. A compare that only feeds selects is
   emitted in its cheaper sense with the select arms swapped, and a
   commutative vector operation writes into an operand that dies there
   (but not into a reduction's accumulator). Adds, subtracts, bitwise operations,
   constant shifts, the four float operations, signed or equality compares
   and selects (on a per-lane or loop-invariant condition) are supported,
   and any number of integer add reductions. A loop that stores gets a
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
the caller's result buffer, copies and clears structs of up to 256 bytes
with 16-byte SSE moves (larger ones call `memcpy`/`memset`), fuses a
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
- `python3 -I tools/linker/machorun.py <exe>` runs a linked x86-64 macOS
  executable on x86-64 Linux: it maps the segments into this process, applies
  the rebase and bind opcodes against glibc (Darwin names such as `___error`
  and `___stdoutp` map to their glibc counterparts) and calls `_main`.
  Programs that only compute and print run unchanged; anything that passes a
  Darwin struct to libc (sockets, `stat`, signals) does not. Compiling the
  `basics`, `features`, `types`, `data`, `games`, `match`, `switch`, `llvm`
  and `general` tests for `x86_64-apple-macos` and running them this way
  matches the expected output for every test outside that set.
- `tools/bash/native-encoder-check.sh <salam> <files...>` checks the encoder:
  it assembles the `--emit-asm` listing with GNU `as` and compares the result
  with the object nativegen wrote, instruction by instruction.
- `SALAM_BACKEND=native sh tools/bash/run-tests.sh <groups>` runs the regular
  build tests through the native backend.
- `SALAM_NG_DUMP=main salam build app.salam --backend=native` prints the
  optimized IR of `main`. Function names are the mangled ones you see in the
  listing, and a comma-separated list dumps several functions.
- `SALAM_NG_OFF=gvn,ifc,slp,thread,inline,sroa,tailrec,selfinl,unroll,licm,unswitch,lle,dbits,idiom,rvec,iss,runroll,widen,sink,lhoist,mlp,ipcp,belady,aload,lsr,lcse,range,split,mmchain,inl1,afwd,lpro,lsf,scse,lpre,dtest,fch,p2div,aggbonus,rot,bitsimp,xsplit,lreas,swtab,allocep,dse,sxadd,rmw,mtie,usim,inlunr,chain,selhoist,mpre,dloop,lvec,shoist,asplit,pst,sfld,isite,chst,fastx,sscale,ccse,kstr,nowr,crng,jload,mxun,gbase,bcmp,mreas,dcse,aggres,tiny,kbr,paddr,izero,zins`
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

Enums are written as enumeration types, so gdb prints `Blue`, not `2`.
CodeView/PDB for MSVC debuggers is not written.

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

- `-g` writes DWARF only; there is no CodeView/PDB for MSVC tools.
- The loop vectorizer needs a single-block loop body with unit-stride
  accesses, and SSE2 limits it: no 64-bit integer compares, no integer
  multiply, no 8- or 16-bit lanes, and float reductions are not reordered.
  There is no AVX code path.
- x86-64 only. aarch64 would reuse everything up to instruction selection.
- The built-in linker (see `NATIVE_LINKER.md`) links every supported
  target itself (static musl, glibc PIE, Windows PE), so a native build needs
  no outside tool beyond the C runtime files. It keeps `.debug_*` sections
  and writes `.eh_frame_hdr`.
