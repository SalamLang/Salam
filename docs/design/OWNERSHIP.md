# Compile-time memory safety without lifetime annotations

Status: **phase 1 implemented** (`compiler/semantic/sema_own.salam`). Later
phases below are planned. This document supersedes `DROP.md`, which stalled on
exactly the question this design answers.

## The problem

Salam frees memory by hand. `Vector`, `HashMap` and `dyn` are C structs that
hold raw heap pointers, and `str` is a bare `char*`. A copy (`b := a`, or a
by-value argument) shares the heap buffer. After that:

- `free()` on one copy leaves every other copy dangling;
- `push` on one copy can `realloc` the buffer, so every other copy, every
  `.ref(i)` pointer and every live iterator can dangle;
- `mem.Free(s as void*)` on a string leaves every other name for it dangling.

Nothing caught these. `SKILL.md` said it plainly: "Salam does not
borrow-check".

## The goal

Catch use-after-free, double free and dangling interior pointers **at compile
time**, with:

- **no annotations**: no lifetimes, no ownership keywords;
- **no new syntax**, so `std/` and `compiler/` keep building with the previous
  release (see "new syntax lags the seed");
- **no runtime cost**: the analysis only reads the program.

## How it works

The check is a flow-sensitive dataflow pass that runs once per function, right
after definite initialization (`sema_check_fn.salam`). It only runs on
functions that free something, take a `.ref()` or `.iter()`, or use `defer`,
so most functions cost nothing.

### Control-flow graph

`sema_own.salam` builds a small CFG over statement node ids. It models what the
AST walk in `sema_definit.salam` cannot:

- `switch` fallthrough (an arm without `break` runs the next arm);
- `break` and `continue`, including `break` out of a `switch` inside a loop;
- `defer`: block scoped, run in reverse at block exit and on every `ret`,
  `break` and `continue` that leaves the block;
- loops, solved to a fixpoint.

Expression-level `match`, ternaries and short-circuit `and`/`or` are joined
locally.

### Access paths and buffer tokens

Every value is reached through an **access path**: `x`, `x.f`, `this.items`,
`p.*`. Each path maps to a set of **tokens**, abstract heap buffers:

| Token | Made by |
|---|---|
| init | a parameter, global or `this` read before any write |
| fresh | an allocation site: a literal, a call result, a declaration |
| field | the buffer held in field `f` of a token |
| element `[]` | what `.ref(i)` points at, inside a container's buffer |
| old | earlier allocations from a site that ran again (see recency below) |
| null | a `Vector`/`HashMap` after its own `free()` nulled the header |

`b := a` makes `b` point at `a`'s tokens. That is how a free through one name
is seen through every other name.

### Events

- `mem.Free(p)`, and `free()` on a `dyn`, free `p`'s tokens and leave `p`
  dangling.
- `free()` on a `Vector`/`HashMap` frees the buffer and nulls that header, so
  reusing the same variable is fine but any *copy* now dangles.
- `push`, `insert`, `reserve`, `resize_set`, `zeros`, `alloc` and `put` *may*
  move the buffer, which invalidates copies, `.ref()` pointers and iterators.
- Passing a path to a `&:` parameter, or calling a method on it, may give it
  new contents, so its freed state is forgotten. This keeps phase 1 free of
  false positives at the cost of missing bugs that cross calls (phase 3).

### Precision

Each (path, token) pair carries its own freed flag, so a join only reports a
pairing that really existed on some path. Four techniques keep the false
positive count at zero on `std/` and `compiler/`:

- **Recency.** When an allocation site runs again, older pointers from that
  site are renamed to an `old` token. A swap such as
  `t := ...; mem.Free(q); q = t` inside a loop then sees two different buffers,
  not one.
- **Strong and weak updates.** A free is definite only when the path has
  exactly one possible buffer and that buffer is a single object. Frees through
  `old`, `[]` or a collapsed recursive field (`e.next.next...`) are weak and are
  never reported.
- **Stale-bit collection.** A freed bit on a buffer that nothing on that path
  can reach any more is dropped before a join.
- **Relational copies.** `b := a` copies `a`'s per-path flags, not the
  per-buffer summary.

## Diagnostics

| Code | When | Mode |
|---|---|---|
| E139 | a path is used after its storage was freed on every path | on (default) |
| E140 | storage is freed twice on every path | on |
| E141 | a returned value is freed by the function's own `defer` | on |
| W142 | used after free on some paths | warn |
| W143 | freed twice on some paths | warn |
| W144 | a copy, `.ref()` pointer or iterator used after a call that may have moved the buffer | warn |

`--ownership=off|on|warn|error` picks the mode. `error` turns the warnings into
errors. Messages exist in English and Persian (`compiler/i18n.salam`).

On the day phase 1 landed, `on` reported nothing across `compiler/`, every
`std/` package reachable from the test suite, and all 3,129 test programs.

## What is not checked yet

Each item below is a phase on the roadmap, or a deliberate limit:

- **Interprocedural effects.** A helper that frees its argument, or pushes into
  a `&:` Vector while the caller holds a `.ref()`, is invisible (phase 3).
- **Element ownership.** Freeing a `Vector<str>` frees the buffer only, as
  today.
- **Raw pointers from `mem.Allocate`/`extern`.** Tracked like any other token,
  but no ownership rules apply to them.
- **Closures.** Captures are checked as uses where the lambda is created;
  lambda bodies are not analyzed (phase 5).

## Roadmap

1. **Phase 1 (done).** Local use-after-free, double free, returned value freed
   by `defer`; warnings for may-paths and buffer moves.
2. **Phase 2.** Free string temporaries. Sema marks each `str` expression that
   is a fresh allocation; codegen frees intermediates and fuses `a + b + c`
   into one allocation. This retires most of the self-host leak budget and
   makes concatenation faster.
3. **Phase 3.** Function summaries, inferred bottom-up over the call graph:
   which parameters are only read, mutated, freed, stored, or invalidated (for
   example `a.nodes`), and whether the result is fresh or an alias of a
   parameter path. This replaces lifetime annotations. Enables the
   `n := at.At(a, id)` across `check_*(a, ...)` check and overlapping `&:`
   arguments.
4. **Phase 4.** Moves and automatic drop for owned locals: `b := a` of an
   owned `Vector`/`HashMap` moves; using `a` after that is an error with a
   `.copy()` fix-it. Owned locals that are not moved get a synthesized `defer
   x.free()`, lowered in sema so all four backends get it for free. Explicit
   `defer x.free()` becomes a warning.
5. **Phase 5.** Closures: a non-escaping lambda borrows its captures and gets
   a stack-allocated environment (faster than today's heap env); an escaping
   lambda moves its captures.
6. **Phase 6.** Flip `--ownership=error` to the default one release after
   `std/` and `compiler/` are clean under it.
