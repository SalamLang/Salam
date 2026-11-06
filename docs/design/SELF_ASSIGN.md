# Updating a variable in place (E162)

A statement that assigns a target to an expression made of the same target is
a compile error. The compiler names the form to write instead and attaches the
rewrite as a fix, so `salam check --fix` applies it.

```
error[E162]: 'x = x + 1' is not allowed: write 'x++'
error[E162]: 'x = x * 3' is not allowed: write 'x *= 3'
error[E162]: 'x = x * 1' does nothing: remove this statement
```

Persian files get the same code with Persian text:

```
error[E162]: 'ع = ع + ۱' مجاز نیست: بنویسید 'ع++'
error[E162]: 'ع = ع * ۱' کاری انجام نمی‌دهد: این دستور را بردارید
```

There is one code. The rule lives in `compiler/semantic/sema_selfassign.salam`
and runs from the statement check for an assignment, after the target and the
right side are type checked, so it sees folded constants and decorated types.

## What is flagged

`T` is the target, `n` and `e` are any expression, `1` and `0` are the numbers
(`1.0`, `0.0`, `۱`, `۰`, a `const` that is 1 or 0 and a constant expression
such as `3 - 2` count too).

| Pattern                                                    | Error text                       | Fix           |
| ---------------------------------------------------------- | -------------------------------- | ------------- |
| `T = T + 1`, `T = 1 + T`, `T += 1`, `T -= -1`, `T = T - -1` | write `T++`                      | `T++`         |
| `T = T - 1`, `T -= 1`, `T += -1`, `T = -1 + T`             | write `T--`                      | `T--`         |
| `T = T + 0`, `T = 0 + T`, `T = T - 0`, `T += 0`, `T -= 0`  | does nothing, remove it          | delete        |
| `T = T * 1`, `T = 1 * T`, `T = T / 1`, `T *= 1`, `T /= 1`  | does nothing, remove it          | delete        |
| `T = T ** 1`, `T **= 1`                                    | does nothing, remove it          | delete        |
| `T = T \| 0`, `T = T ^ 0`, `T \|= 0`, `T ^= 0` (integers)  | does nothing, remove it          | delete        |
| `T = T << 0`, `T = T >> 0`, `T <<= 0`, `T >>= 0` (integers) | does nothing, remove it          | delete        |
| `T = T & -1`, `T &= -1` (signed integers)                  | does nothing, remove it          | delete        |
| `T = T`                                                    | does nothing, remove it          | delete        |
| `T = T op e` for `+ - * / % ** & \| ^ << >>`               | write `T op= e`                  | `T op= e`     |
| `T = e op T` for `+ * & \| ^`                              | write `T op= e`                  | `T op= e`     |
| `s = s + e` where `s` is a `str`                           | write `s += e`                   | `s += e`      |

Parentheses never hide a pattern: `T = (T) + 1`, `T = (T + 1)` and
`T = T + (e)` are the same statements. The text of a no-op deletion removes the
whole line when the statement is alone on it.

## What is not flagged

| Statement                                    | Why                                                         |
| -------------------------------------------- | ----------------------------------------------------------- |
| `T = T + 2`, `T += 2`, `T *= 2`               | `+=` and friends stay the way to change by any other amount |
| `T = e - T`, `T = e / T`, `T = e % T`, `T = e << T` | not commutative, so not the same as `T op= e`              |
| `s = e + s` for a `str`                       | a prepend, `s += e` appends                                 |
| `T = T * a + b`, `T = T + a + b`, `T = T + a - b` | the top operator's left side is not `T` (float addition is not associative) |
| `T = a + 1`, `T = a + b`                      | a different variable                                        |
| `a[f()] = a[f()] + 1`, `f().x = f().x + 1`    | the target calls a function, the two sides are not one location |
| `T = T + f()` when `T` is a global, a field, an element or a reference parameter | `f()` may change `T`, and the C backend evaluates `T += f()` with the call first while `T = T + f()` reads `T` first |
| `p = p + 1` on a pointer                      | pointer arithmetic, not a number                            |
| `m = m + n` on a `type Meters: int` and on user structs with `operator +` | the operator may mean something else |
| `v[i] = v[i] + 1` on a `Vector`               | `v[i]++` does not compile on the C backend for a Vector element, so the +1 and -1 forms are left alone there; the `+ n` form is flagged |
| `T++`, `T--`, `T += 2`                        | already the form to write                                   |

`T = T + f()` is flagged when `T` is a plain local variable that the call
cannot reach: not a global, not a reference parameter, and not mentioned in the
call. A local is never changed by a callee, so both forms mean the same on
every backend.

## Targets

A target counts when it has no side effects: a variable, a field chain
(`a.b.c`, `this.n`, Persian `این.مقدار`), an element of an array, a slice or a
pointer (`a[i]`, `p[0]`) whose index is made of variables, numbers, field reads
and arithmetic. Elements of a `Vector` count for the compound form only.

## Types

Integers and floats that are not distinct types, and `str` for `s = s + e`.
`T = T` is flagged for every type. Floats: `x + 0.0` is flagged like the
integer case although it turns `-0.0` into `0.0`; nothing in the language
relies on that.

## Migration

`tools/salam/fix-selfassign.salam` rewrites a tree with the fixes the compiler
attaches (`SALAM=./salam salam run tools/salam/fix-selfassign.salam std
compiler tests`). It runs `salam check --error-format=json` on each file,
collects the E162 edits, applies them from the end of each file to the start and
repeats until nothing is left. `--dry-run` only counts.

## Not done

`x = x op e` is the only general rule. Other self-referential updates such as
`x = x + y + 1` or `x = max(x, y)` are left alone on purpose; a separate lint
could cover them.
