# Switch lint (W145)

A long run of `if x == 1:`, `if x == 2:`, `if x == 3:` tests reads better as one
`switch x:`. This check finds those runs. It is a style check, so it is off by
default and never changes what a program does.

```
warning[W145]: 4 consecutive comparisons of 'ty' against constants could be a switch
 --> linker.salam:42:8
   |
42 |     if ty == 7:
   |        ^
   |
   = help: group them as 'switch ty:' with one label per constant
   = fix: Rewrite as a 'switch' statement
```

## Using it

| Flag                  | Meaning                                                       |
| --------------------- | ------------------------------------------------------------- |
| `--switch-lint=off`   | the default: nothing is reported                              |
| `--switch-lint=warn`  | report W145 as a warning                                      |
| `--switch-lint=error` | report it as an error (E145); the exit code is 1              |
| `--switch-lint=fix`   | like `warn`, and `salam check --fix` rewrites the runs it can |
| `--switch-lint-min=N` | the shortest run to report, 2 or more (default 4)             |

The language server takes `switchLint` (`off` or `warn`) and `switchLintMin` as
settings, and offers the rewrite as a quick fix. See `docs/LSP.md`.

Files from the standard library and from installed packages are never checked.
Your own `include`d files are.

## What counts as a run

A _link_ is one test of an `if` chain: either an `else cond:` branch, or a
sibling `if` statement whose body always leaves the block (`ret`, `continue`,
`break`, or a call that never returns). Siblings that fall through are not
exclusive, so they never form a run.

An _atom_ is `subject == constant`, in either operand order, and `or` joins
atoms into one link with several labels. A run is a maximal sequence of
**contiguous** links in which every atom compares the same subject, so
`ty == 7`, `(ty) == (8)` and `9 == ty or ty == K` all belong together
(parentheses are dropped by the parser, casts are not).

The subject must be free of side effects: identifiers, `this`, member and
index chains, casts, and arithmetic or bit operators over those. A call, an
assignment, `++`, `?`, a ternary or a lambda disqualifies it, because a
`switch` evaluates its subject once and the chain evaluates it once per test.
Its type must be an integer, `char` or `str` (floats, booleans and enums are
left alone). A constant is a literal or a non-`mut` `const`, also through a
package (`pkg.NAME`).

## Why the run must be contiguous

Inside a run every test is an equality on one subject with its own constant, so
the tests exclude one another and their order does not matter. A test that is
not a plain equality (`name_is(...)`, `(fl & 4) != 0`, `ty == 14 or
starts(...)`, a different subject) may be true at the same time as a later
test, so moving a later test above it could change which branch wins. The
compiler cannot prove otherwise, so such a test ends the run. That is also how
intentionally ordered chains stay silent: there is no suppression comment,
because the cases that depend on order are exactly the ones that never match.

For example, in `classify` (`compiler/nativelink/nl_layout.salam`) only
`ty == 7` and `ty == 8` form a run, and it has 2 links, so nothing is reported
at the default of 4. `ty == 0x70000001` further down cannot move above the
`name_is(l, nm, ".eh_frame")` test: a section can have both, and the function
returns `OC_EHFRAME` for it.

## The rewrite

The quick fix replaces the run with one `switch` and is offered only when all of
these hold. The labels are distinct after folding. No arm has a `break` that
would now leave the `switch` instead of an enclosing loop (`continue` is fine,
it passes through). Every arm's body starts on its own line. No comment or other
text sits between the tests. The subject and labels can be copied from the
source with balanced brackets. A chain that is rewritten must be rewritten in
full: a chain with another kind of test after the run is reported but not
rewritten. Arms that do not already leave the block get a `break`, except the
last arm when there is no `else`. The keywords (`switch`, `else`, `end`, `break`)
and the label separator follow the file's language.

It is marked unsafe on purpose, so a plain `salam check --fix` never applies it.
`--switch-lint=fix` is the explicit opt-in.

## Cost and tests

The pass runs while each block is checked, only when the lint is on, and adds
about 10 percent to a check of the compiler's own sources. A rewrite is not a
speed change: only the C backend turns a `switch` into a C `switch`, the others
compare one label after another, and the native backend already builds tables
from `x == K` chains.

Tests are in `tests/{en,fa}/lint` (diagnostics, counted by the `lint` section of
`tools/bash/run-tests.sh`) and `tests/{en,fa}/lint/fix` (the rewrite: the result
must equal the `.fixed` file, have no W145 left, and print the same output as
before). Run them with `sh tools/bash/run-tests.sh lint`.
