# Compile-time value checks

Status: implemented in `compiler/semantic/sema_range.salam`, on by default
(`--range-checks=on`).

## The problem

A Salam program could build cleanly and then stop, or worse, keep running
with a wrong answer:

| Bug                                   | Before this pass                                      |
| ------------------------------------- | ----------------------------------------------------- |
| `repeat 0 to 3 in i: xs[i]` on 3 items | aborts at run time on the fourth pass                 |
| `i := 5` then `xs[i]`                 | aborts at run time                                    |
| `v.push(1)` twice, then `v[5]`        | panics at run time                                    |
| `a := 2147483647 as i32; a + 1`       | prints `-2147483648`; undefined in a `--release` build |
| 13! in an `int` (`i32`)                | prints a wrong number                                 |
| `d := 0` then `10 / d`                | stops at run time                                     |
| `p := null as Node*; p.key`           | prints `0` (the C compiler folds the undefined read)  |
| `mem.Allocate(32) as i64*`, then `p[4]` | writes past the block, silently                     |

The only compile-time checks were for literal indices (`xs[3]`), constant
division by zero and shift widths. Everything that went through a variable,
a loop counter or a length waited for the run-time checks, which a
`--release` build removes.

## The goal

Report these bugs when the program is compiled, with a message that says
which value goes wrong, what it is, and on which pass of which loop:

```
error[E146]: index out of bounds: 'i' is 3, but 'xs' has 3 elements (on pass 4 of the loop on line 3)
  = help: valid indices of 'xs' are 0 to 2; 'repeat a to b' includes b itself, so count with
          'repeat xs.len() in i:' or 'each i, x in xs:'
```

with no annotations, no new syntax, and no run-time cost.

## How it works

The pass runs once per function, right after definite initialization
(`sema_check_fn.salam`). It walks the function's AST and keeps, for every
local and parameter, an abstract value (`RgV`):

- an integer interval `[lo, hi]`, each end optional;
- a symbolic bound against a container's length: `v >= len(c) + k` and
  `v <= len(c) + k`. This is what catches `repeat 0 to xs.len() in i` on a
  slice whose length nobody knows;
- for a `Vector`, `str`, slice or pointer, its length (or, for a pointer from
  `mem.Allocate`, how many elements fit in the block);
- for a pointer, whether it is null, not null, or null on some paths.

`x.f` of a struct value held in a local is tracked too, so
`n := Node { next = null }` followed by `n.next.key` is caught.

### Control flow

- `if`, `and`, `or`, `?:` and `until` conditions refine the values on each
  side: inside `if i < xs.len():` the analysis knows `i <= len(xs) - 1`, and
  after `if p == null: ret end` it knows `p` is not null. A branch the
  refinement proves impossible is not analyzed.
- A path that ends in `ret`, `throw`, `break`, `continue` or a call to a
  `noret` function (`panic`, `os.Exit`) does not reach the code after it, so
  `if i >= n: panic("bad index") end` protects what follows.
- `switch` arms fall through to the next arm, as at run time; `break` leaves
  the switch.
- Loops are solved to a fixpoint with widening, then analyzed once more with
  reporting on.

### Loops are run, not just summarized

A `repeat` loop whose bounds are known and that runs at most 64 times is
unrolled: each pass is analyzed with the exact counter value, so values that
grow inside the loop are exact too. That is how `f = f * i` over
`repeat 1 to 20 in i` is reported on pass 13, where `f` no longer fits in an
`i32`.

Other counted loops (`repeat`, `each i, x in xs`) get two extra passes, one
with the counter at its first value and one at its last. On those passes a
problem is an error only if the statement certainly runs on that pass: it is
not inside an `if` or `match`, and nothing before it in the body can leave
the pass early. Otherwise it is a warning.

### What is an error and what is a warning

An error (E146, E148, E150, E152) means: whenever this line runs, it goes
wrong. A warning (W147, W149, W151, W153, W154) means some value the analysis
can see makes it go wrong, but other values do not; a check before the line
silences it. A value the analysis knows nothing about (a parameter, a call
result) is never reported, so the pass does not guess.

W147 is held to a stricter standard than the others, because index code is
full of invariants the analysis cannot see. The first sweep over `compiler/`
and `std/` produced 160 of them and every one was a false alarm, in three
shapes, each now excluded:

- a sentinel such as `mut x := -1` that a later loop always replaces, or
  `top - 1` after a loop that always runs: "can be -1" is only reported on a
  pinned or unrolled loop pass, where the counter's value is exact;
- a container whose length is only bounded (`as few as 0 elements`): the
  numeric check needs an exact length, and an index bound that came straight
  from comparing with an exact value (`until i <= 3:`);
- pairs walked two at a time (`until i < n: a[i], a[i + 1]` with an even
  `n`): a bound shifted by arithmetic after the comparison does not count.
  The plain off-by-one shapes (`until i <= xs.len():`,
  `repeat 0 to xs.len() in i:`, `each i, x in xs: xs[i + 1]`) are still
  reported, the last two as errors on the loop's last pass.

| Code | When                                                                         |
| ---- | ---------------------------------------------------------------------------- |
| E146 | index or slice bound out of range on every value it can take                  |
| W147 | index or slice bound can be out of range (see below for what counts)        |
| E148 | signed `+`, `-`, `*` or unary `-` overflows its type on every value          |
| W149 | signed arithmetic can overflow its type                                      |
| E150 | integer division or remainder by a divisor that is 0                         |
| W151 | the divisor's range includes 0 (both ends known)                             |
| E152 | a member or index read through a pointer that is null                        |
| W153 | a pointer that is null on some paths reaching the read                       |
| W154 | an explicit cast of a known value that does not fit and loses bits          |
| W158 | memory from `mem.Allocate` that nothing frees, returns, stores or passes on |

Unsigned arithmetic is not reported: it wraps by definition, and hashes rely
on that. Signed overflow is reported because a `--release` build lets the
backend assume it never happens. The constant folder (`sema_fold.salam`)
reports E148 too, so `const B := A * 4096` no longer folds to `0` in silence
when `A` is `1 << 20`; this is the rule Go, Swift and Rust apply to
constants. Tests that check how each backend wraps at run time now pass
their operands through a function, which is what code that wants run-time
wrapping looks like.

### `repeat a to b` counts down

`repeat a to b` runs from `a` to `b` inclusive, and counts down when `b` is
below `a`. So `repeat 0 to xs.len() - 1 in i:` on an empty `xs` runs twice,
with `i = 0` and `i = -1`, and both reads are out of bounds. The pass models
the loop counter as the hull of both directions, reports W147, and the help
text explains the count-down. The first sweep found this pattern in three
tests (`tests/en/basics/slices.salam`, `tests/en/exec/slice_func.salam`,
`tests/en/llvm/ll_slice.salam`).

`--range-checks=off|on|error` picks the mode; `error` turns the warnings into
errors. Messages exist in English and Persian (`compiler/i18n.salam`).

## Limits

- Only locals and parameters are tracked. Globals, fields of structs reached
  through pointers, and `this` are unknown, because any call could change
  them.
- A variable captured by a lambda, written inside a `defer` or a `try:`
  block, or bound by a `match` pattern is not tracked.
- Calls are crossed in one way only: a function whose every `ret` returns
  `null` makes its result null at the call, so `p := find(3)` followed by
  `p.key` is E152. A function that indexes its parameter is not checked
  against its callers' arguments.
- W158 is a syntactic check on top of the ownership pass: a local bound to
  `mem.Allocate` or `mem.AllocateZeroed` whose name never reaches `mem.Free`,
  a `ret`, an assignment, a field, a collection or another call. Any of those
  counts as handing the block on, so it never reports memory that some other
  code frees. Vectors, maps and strings need no such check: phases 2 and 4 of
  OWNERSHIP.md free the ones that do not escape.
- Each function has a work budget (60,000 node visits) and a cap on tracked
  names (600); a function past either is skipped rather than slowed down.
