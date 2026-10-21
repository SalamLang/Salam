# Proposal: `func drop()`, an opt-in destructor

Status: **proposal, not implemented.** Written 2026-10-19 alongside the
iterator-protocol and interface work, which deliberately left this out.
The escape analysis this proposal was waiting on is now designed in
`OWNERSHIP.md`; automatic drop is its phase 4.

## The problem

Salam has no ownership model. Heap memory is released by hand, and the idiom is

```salam
mut v := Vector {} as Vector<int>
defer v.free()
```

That works, and `std/` holds itself to it: the program leak ratchet in
`tools/program-leak-budget.txt` keeps almost every demo at exactly 0 leaked
allocations. But it only works where a human remembers, and three shapes defeat
it today:

- **Capturing lambdas.** A closure's captured environment is allocated and
  never released; there is no `defer` site for it.
- **String temporaries.** Every discarded `a + b` in a chain is a live block
  until the process exits. The self-host budget in
  `compiler/tools/selfhost-leak-budget.txt` is ~197,000 allocations for one
  `salam build` of `hello.salam`, and its own header says the reason is "no
  arena behind its string temporaries".
- **Structs that own heap.** Every type with a `free()` needs each of its users
  to know that, and to write `defer x.free()`. Nothing enforces it; forgetting
  is silent.

A destructor would close the third, and is the only one of the three that fits
in the type system rather than in the allocator.

## The proposal

A struct may declare

```salam
struct Buffer:
    pub data: Vector<u8> = Vector {}
    pub func drop():
        this.data.free()
    end
end
```

`drop()` is an ordinary method with no parameters and no result. The compiler
calls it automatically at the end of the scope that owns the value, in reverse
declaration order, immediately before any `defer` registered in that scope runs.

Rules worth fixing up front:

- **Opt-in.** A struct with no `drop()` behaves exactly as today. Nothing in
  `std/` changes until its types declare one.
- **Only non-escaping locals.** A value that is returned, stored into a
  longer-lived structure, or passed to a `&:` parameter that keeps it, is not
  dropped at the end of the scope.
- **Not on copies.** Salam copies structs by value, but a `Vector` field inside
  a struct shares its storage (`project_interp_vector_int_share_bug`). Dropping
  every copy would double-free that storage. So either copies must be tracked,
  or `drop()` must be restricted to types that cannot be copied - a notion
  Salam does not have.
- **`drop()` runs once.** Manual `x.free()` followed by an automatic `drop()`
  must not double-free, so either `free()` becomes idempotent by convention or
  an explicit drop cancels the automatic one.

## Why it is not being built

**It needs escape analysis to be sound.** The three rules above are all the same
question: does this value outlive this scope? Answering it conservatively (drop
nothing that is ever passed anywhere) makes the feature useless; answering it
optimistically introduces double-frees and use-after-frees in a language that
currently has neither by construction. Salam has no borrow checker and no
ownership annotations to lean on, and adding one of those is a much larger
change than adding a destructor.

**It sits against the declared design.** `SKILL.md` is explicit that memory is
freed deliberately, and PR #1752 is titled "struct and enum features without
OOP". Automatic cleanup at scope exit is the one piece of RAII that pulls
hardest toward the ownership model the language has chosen not to have.

**It is the only type-system feature here that touches all four backends.** The
C, LLVM, JS and interpreter backends would each need scope-exit emission, and
the interpreter's bytecode VM has its own GC.

## What to do instead, for now

- Keep `defer x.free()`, and keep the two ratchets honest.
- The two leaks that `drop()` would _not_ fix - closure environments and string
  temporaries - are allocator-shaped, not type-shaped. An arena for compiler
  string temporaries would retire the larger of the two numbers above without
  touching the language at all, and the self-host budget file already names
  that as the fix.

## If it is revisited

The smallest useful version is narrower than the above: call `drop()` only on a
local that is **declared and never mentioned again except through its own
methods** - no assignment to another name, no passing by value, no return, no
`&:`. That is a syntactic check, not a dataflow one, so it is sound without
escape analysis, and it covers the common `mut b := Buffer {}` ... use ... end.
Everything else keeps `defer`.
