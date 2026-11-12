# Compile-time memory safety without lifetime annotations

Status: **phases 1 to 5 implemented** (`compiler/semantic/sema_own.salam`,
`sema_strfree.salam`, `sema_summary.salam`). Phase 6 waits for a release. This document supersedes `DROP.md`, which stalled on
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

The check is a flow-sensitive dataflow pass that runs once per function. Each
function is registered right after definite initialization
(`sema_check_fn.salam`) and checked once the summaries are ready
(`SemaOwnPost`). It only runs on functions that free something, call a
function whose summary says it always frees an argument (see "Function
summaries"), free in a `defer` and return a value, or (in warn mode) take a
`.ref()` or `.iter()`, so most functions cost nothing.

### Control-flow graph

`sema_own.salam` builds a small CFG over statement node IDs. It models what the
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

| Token        | Made by                                                            |
| ------------ | ------------------------------------------------------------------ |
| init         | a parameter, global or `this` read before any write                |
| fresh        | an allocation site: a literal, a call result, a declaration        |
| field        | the buffer held in field `f` of a token                            |
| element `[]` | what `.ref(i)` points at, inside a container's buffer              |
| old          | earlier allocations from a site that ran again (see recency below) |
| null         | a `Vector`/`HashMap` after its own `free()` nulled the header      |

`b := a` makes `b` point at `a`'s tokens. That is how a free through one name
is seen through every other name.

### Events

- `mem.Free(p)`, and `free()` on a `dyn`, free `p`'s tokens and leave `p`
  dangling.
- `free()` on a `Vector`/`HashMap` frees the buffer and nulls that header, so
  reusing the same variable is fine but any _copy_ now dangles. Reading it
  again (`v.len()` is 0) or freeing it again (a `defer v.Free()` after an
  explicit `v.Free()` frees a null pointer) is defined on every backend, so
  neither is reported; the same through a copy is E139 or E140.
- `.copy()` and `.deep_copy()` build a new buffer, so `b := a.copy()` neither
  shares nor moves `a`'s buffer.
- `push`, `insert`, `reserve`, `resize_set`, `zeros`, `alloc` and `put` _may_
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

| Code | When                                                                                  | Mode           |
| ---- | ------------------------------------------------------------------------------------- | -------------- |
| E139 | a path is used after its storage was freed on every path                              | on, warn       |
| E140 | storage is freed twice on every path                                                  | on, warn       |
| E141 | a returned value is freed by the function's own `defer`                               | on, warn       |
| W142 | used after free on some paths                                                         | on, warn       |
| W143 | freed twice on some paths                                                             | on, warn       |
| W144 | a copy, `.ref()` pointer or iterator used after a call that may have moved the buffer | warn (default) |
| W163 | an owned `Vector`/`HashMap` used after `b := a`, `x.f = a` or handing it to a keeper  | warn (default) |

`--ownership=off|on|warn|error` picks the mode; `warn` is the default.
`on` reports the errors plus W142 and W143; `warn` adds W144, which needs a
wider relevance scan. Measured on 2026-11-03: `salam check` on typical test
programs that import std took 0 to 9% longer (0.77s to 0.84s for
`image_demo`), and on `compiler/main.salam`, which is unusually heavy on
containers, 10.2s to 15.1s. `error` turns the warnings into errors.

W144 became the default once `--ownership=warn` reported nothing on
`compiler/`, `std/` and the test programs. Two fixes in the stale-bit
collection got it there (28 false W144 in `compiler/nativegen` and
`compiler/nativelink`):

- the first state to reach a join was copied without collection, so a
  moved bit from a branch that had rewritten the struct leaked into the
  other branch;
- a field token counted as live whenever its root token was, so the copy
  that `each x in g.field` makes kept every other field of `g`'s old buffer
  alive. A token is now live only if some path holds it or holds a token it
  is a field of. Messages exist in English and Persian (`compiler/i18n.salam`).

On the day phase 1 landed, `on` reported nothing across `compiler/`, every
`std/` package reachable from the test suite, and all 3,129 test programs.

## String temporaries (phase 2)

Two changes stop the string garbage that `+` used to leave behind, with no
change to what a program means:

- **Fused chains.** `a + b + c + ...` on strings is flattened and built by one
  call, `salam_strcat_v` in C or inline IR in LLVM. It measures every part,
  allocates once, copies, and frees the parts it created itself: the
  number, `char` and `float` to string conversions. Before, every `+` allocated
  its own result and the intermediates were never freed.
- **Auto-free of fresh locals.** `x := a + b` (immutable, declared directly in
  a block) gets a synthesized `defer` that frees `x` at the end of its scope,
  but only when every use of `x` is one that cannot keep the pointer:
  - an operand of `+` or a comparison;
  - an index read, or a `switch`/`match` subject;
  - an argument to `print`/`println`;
  - a `str` method returning a non-string, `substr` or `concat`;
  - a `str` package function returning a non-string.

  Returning, storing, assigning, capturing or passing `x` anywhere else keeps
  today's behavior. JS and the interpreter see the deferred statement as a
  bare read, so their garbage collectors stay in charge.

`--ownership=off` turns the auto-free off along with the checks.

Measured on a one-million-iteration loop that builds a five-part string:
0.40s to 0.16s (C), 0.35s to 0.12s (LLVM); 157 MB leaked to 0. The compiler
itself, built with these changes, leaks about 9% fewer blocks for `salam build`
(193,185 to 175,468).

Temporaries that are never named get the same treatment. A fresh `a + b`, or
a call whose summary says it returns a fresh string, written directly as an
argument of a call, is hoisted into a synthetic local right before its
statement and then freed by the rules above. That happens only when:

- the statement is an expression statement, a declaration, an assignment, a
  `ret`, or an `if` (its first condition only, never a loop condition or an
  `else if`);
- the callee's summary says it only reads that parameter, or the callee is a
  `pure` function that returns a number or a `bool` (so it cannot keep or
  return the argument); `print` and `println` already split chains and are
  skipped; externs and unknown callees keep today's behavior;
- the temporary is evaluated unconditionally: not behind `and`/`or`, not in a
  ternary arm, not inside a lambda or `match`;
- hoisting does not reorder side effects: everything evaluated before it in
  the statement is a pure expression; if the temporary itself calls an impure
  function, only literals and local variables may come before it, and nothing
  in it may pass a variable by reference or build a lambda.

Results of `str.FromInt` and the other `numfmt` conversions are still not
freed: `numfmt` allocates with libc `malloc` (it sits below `mem`), so its
results are not `mem.Free` blocks.

An `else if` condition gets the same hoisting. The `else if` is wrapped in an
`else` block that holds the temporary, which is freed at the end of that
block. Loop conditions, ternary arms and the right operand of `and`/`or` still
keep today's behavior: their temporaries would need a free on every iteration
or on a conditional path, which the AST rewrite cannot express without
rebuilding the loop or the expression.

### Reassigned string locals

`out += chunk` and `s = f(s)` drop the old value of a
`mut` string local on every step. Freeing it needs proof that the old value is
heap owned by that local, and a literal initial value (`mut out := ""`) is not.
No runtime check can tell a literal from a heap block, so the proof is a flag
the compiler keeps next to the variable:

```
mut out := ""
mut __own12 := false
...
__ac40 := out + chunk
if __own12: free(out) end
out = __ac40
__own12 = true
```

The flag is `true` exactly when `out` currently holds a block that this local
allocated. A declaration sets it from its initializer (a fused chain or a call
whose summary says it returns a fresh string, else `false`). An assignment
sets it to whether the right side is fresh (again a chain or a fresh call).
The new value is computed into a temporary first, because the right side
reads the old one. Nothing is freed unless the flag is set, so a literal, a
borrowed parameter value (`mut t := s`) and any other non-fresh value are
never freed.

A local qualifies when:

- it is declared directly in a block with a `str` initializer and is `mut`;
- it is declared once in the function and is not captured by a lambda;
- every use only reads it (the read-only rules of the immutable case), or is
  `ret` of it, or is an assignment `x = e` / `x += e` that is a statement of a
  block (not a match arm). `x += e` needs a `str` right side;
- an assignment whose right side reads `x` must have a fresh right side, so
  the new value can never alias the one being freed;
- it is never passed by reference (`&:`), stored (`v.push(x)`, `y := x`,
  `y = x`, a struct field), freed by hand, or handed to a callee that keeps
  or returns its argument. Those make the old value reachable from somewhere
  the compiler cannot see, so the local keeps today's behavior.

When nothing but `ret x` escapes the local, only the reassignments free (the
returned value is the caller's). When it never escapes, a `defer` also frees
the final value at the end of the scope if the flag is set. Reassignments
inside loops, `break`, `continue` and `switch` arms need nothing special: the
free sits right before the assignment it belongs to, and the scope-end `defer`
already runs on every exit.

Measured on 2026-11-04, compiling `compiler/main.salam`: the compiler rewrites
547 locals at 934 assignment sites. Peak memory of `build --emit-c` fell from
1037 MB to 854 MB, `--backend=native --emit-asm` from 2032 MB to 1892 MB and
`--backend=llvm --emit-llvm` from 1459 MB to 1378 MB. The ASan leak counts for
`salam build hello.salam` fell from 175,730 to 172,030: most of what is left is
not an overwritten string but a finished value that nobody frees.

What this does not cover: parameters (`mut s: str` reassigned in the callee),
fields and elements (`this.buf = this.buf + x`), a name declared twice in one
function, and the final value of a local that is returned (it leaks only if
the caller drops it).

The remaining string leaks come from call results (`f() + "x"`, or a
`str.FromInt` result stored in a local). Whether a function returns a fresh
string is an interprocedural fact, so they wait for phase 3 summaries.

## Function summaries (phase 3)

Lifetime annotations exist so a compiler can see what a call does to its
arguments without looking inside the callee. Salam infers that instead.
After type checking, `sema_summary.salam` computes these facts for every
function, std and generic instances included, solved to a fixpoint over the
whole call graph:

- **fresh result**: every `ret` returns a newly allocated string, so the
  caller owns it;
- **keeps** parameter _i_: the argument may be stored, returned or captured;
- **frees** parameter _i_: some path frees it; **must free** when an
  unconditional top-level statement does;
- **grows** parameter _i_: some path pushes into a container reachable from
  it, which may move that buffer;
- **returns an alias** of parameter _i_: for example
  `At(a, id): Node* ... ret a.nodes.ref(id)`.

Sema records the resolved target of every call, so overloads, methods, package
functions and generic instances all map to the right summary. Interface (`dyn`)
calls and externs are unknown. A short list of libc readers (`memcpy`,
`strlen`, ...) is known not to keep its arguments.

What the summaries buy:

- **Leaks.** `"x" + f()` frees `f()`'s result when `f` returns fresh, and
  `x := f()` is auto-freed under the phase 2 rules. A local whose every use
  passes it to a parameter that is only read counts as non-escaping.
- **Errors across calls.** A use after a call that must free the argument is
  E139, even several calls deep (`forget(s)` calls `release(s)`, which frees
  it).
- **Warnings across calls**: a may-free callee gives W142. In warn mode, a callee
  that grows a container gives W144 on every copy and `.ref()` pointer taken
  before the call, including results of alias-returning functions such as
  `at.At`.

The first sweep found real bugs in std:

- `std/telegram/bot`: `_put_chat_id(params: HashMap<str, str>, ...)` took the
  map by value. Its `put` resized the callee's copy, so every request built
  through it silently lost `chat_id`. It now takes `params &:`.
- `std/net/socketio`: `_resolve_ack` kept scanning after the matching waiter.
  A second waiter with the same id would reuse `bins` after they were freed or
  queued. It now stops at the match.
- `std/archive/rar`: the free and the later use of an entry name tested the
  same condition twice; it is now computed once.

## Automatic drop of owned containers (phase 4)

A `Vector` or `HashMap` local is freed at the end of its scope, with nothing
written by hand, when all of the following hold:

- it starts as an empty literal (`Vector {} as Vector<int>`, `HashMap {}`) or
  as the result of a call whose summary says it returns a fresh container;
- it is declared directly in a block, not in a `match` arm;
- every use only reads it or grows it: method calls, `each`, passing it to a
  parameter that is only read or grown;
- it is never reassigned, returned, stored, captured, freed by hand, or passed
  to something that keeps or frees it.

So the usual shape below needs no `defer v.Free()`:

```salam
mut v := Vector {} as Vector<int>
fill(v, 10)
println total(v)
```

Code that already frees by hand keeps working unchanged: a hand-written
`free()` or `defer v.Free()` disqualifies the variable, so nothing is freed
twice. The drop frees what `free()` frees (the buffer, not the elements). C
frees the header fields directly and LLVM loads them through the local's
address. JS and the interpreter see a bare read.

`b := a` on an owned container is not a move yet. Phase 1 already reports the
dangerous consequences of the shared buffer (E139, W144). Making it a hard
move would break existing code, so it waits for phase 6.

## What is not checked yet

Each item below is a phase on the roadmap, or a deliberate limit:

- **Globals and indirect calls.** Effects through global variables, function
  values and `dyn` calls are not summarized.
- **Element ownership.** Freeing a `Vector<str>` frees the buffer only, as
  today.
- **Raw pointers from `mem.Allocate`/`extern`.** Tracked like any other token,
  but no ownership rules apply to them.
- **Closures.** Captures are checked as uses where the lambda is created;
  lambda bodies are not analyzed (phase 5).

## Roadmap

1. **Phase 1 (done).** Local use-after-free, double free, returned value freed
   by `defer`; warnings for may-paths and buffer moves.
2. **Phase 2 (done).** Fused concatenation chains and auto-free of
   non-escaping fresh string locals; see above.
3. **Phase 3 (done).** Inferred function summaries in place of lifetime
   annotations; see above.
4. **Phase 4 (done).** Automatic drop of owned `Vector`/`HashMap` locals;
   see above.
5. **Phase 5 (done).** A capturing lambda that never escapes its scope costs
   no heap allocation: its environment lives on the stack (C backend) or is
   freed at scope exit (LLVM). This covers a local bound to the lambda and a
   lambda written straight into a call whose parameter does not keep it;
   sema hoists the latter into a synthetic local first, unless something in
   that statement writes a captured variable. Escaping lambdas keep a heap
   environment, as before.
6. **Phase 6 (needs a release first).** Each step is gated on the seed, the
   previous release that std/ and compiler/ must keep building with:
   - Release N ships with phases 1 to 5 (`--ownership=on` by default).
   - After N is the seed: report `b := a` of an owned `Vector`/`HashMap`
     followed by a use of `a` as a warn-mode move warning, and migrate std/
     and compiler/ (`.copy()` or reorder) until `--ownership=warn` is clean.
   - Release N+1 makes `--ownership=error` the default and turns the move
     warning into an error with a `.copy()` fix-it.
   - Each flip lands with the `tests/*/errors/own_*` expectations updated in
     both languages.
