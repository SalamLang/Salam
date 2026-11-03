# Tuples and multiple return values

Status: implemented. Parsing is in `compiler/parser/` (`parser_type.salam`,
`parser_decl_var.salam`, `parser_expr_primary.salam`), the lowering in
`compiler/semantic/sema_tuple.salam`, the types in `std/core/core.salam`.

## The problem

A Salam function returned exactly one value. Code that needed two answers had
four ways out, and each one costs something:

| Workaround                      | Example in the tree               | Cost                                    |
| ------------------------------- | --------------------------------- | --------------------------------------- |
| out parameter plus a status     | `session.Get(id, ok &): Session`  | the result is split over two places     |
| out parameter for the error     | `gql.Parse(src, err &): Document` | a throwaway variable on every call      |
| a one-off result struct         | `CoerceResult`, `PathParts`       | a declaration for every function        |
| `Pair<A, B>` from `collections` | `std/collections/pair.salam`      | no syntax, and an import for two values |

## What other languages do

| Language                    | Model                                                      | What it teaches                                                                                    |
| --------------------------- | ---------------------------------------------------------- | -------------------------------------------------------------------------------------------------- |
| Go, Lua                     | several values exist only at `return` and at the call site | cheap, but they cannot be stored, put in a generic or a `Result`; Go needs a special `f(g())` rule |
| Python, Rust, Swift, Kotlin | a real tuple type with destructuring                       | composes with everything, no special cases                                                         |
| Swift, C#                   | labeled elements                                           | self-documenting results                                                                           |
| Odin                        | true multi-value returns                                   | fast, but every backend needs an ABI special case                                                  |
| Zig                         | an anonymous struct                                        | the same lowering Salam uses, without the syntax                                                   |

Salam takes the second row: a tuple is an ordinary value.

## The design

```salam
func divmod(a: int, b: int): (int, int):
    ret a / b, a % b
end

q, r := divmod(17, 5)         // declare two names
q, _ := divmod(17, 5)         // '_' drops an element
q, mut r := divmod(17, 5)     // 'mut' is per name
t := divmod(17, 5)            // keep the tuple whole
println t.0, t.1              // element access, also t.0.1 for nested tuples
x, y = y, x                   // assign several mut targets at once
```

- The type is `(A, B)` with 2 to 8 elements. It works wherever a type does:
  parameters, struct fields, `Vector<(int, str)>`, `HashMap<str, (int, int)>`,
  nested `((int, int), str)`, function types, and `throws`
  (`func f(): (i64, bool) throws`, then `a, b := try f()`).
- `ret a, b` and `(a, b)` build a tuple. With an expected type (a return type,
  a parameter, a field) the elements take the declared element types, so
  `ret 1, 2` fits `(i64, u8)`.
- `a, b := e` needs exactly as many names as elements (`E158`). The unused-name
  rule applies to every name, so a name that is not needed is written `_`.
- There is no implicit spreading: `f(g())` passes one tuple argument, never
  several. Destructure first.
- `match` takes tuple patterns, `(0, y)`: a literal tests the element, a name
  binds it, `_` ignores it, and a guard may follow. The parser turns the pattern
  into the existing struct pattern on `Tuple2` (`Tuple2{f0 = 0, f1 = y}`), so
  tests, bindings, guards and exhaustiveness work as they do for structs.
  Nested tuple patterns are not supported.
- A tuple prints as `(1, "a")`, compares with `==` and `!=` element by element
  (it is a struct), and copies like a struct.

## How it works

A tuple is not a new kind of type. `std/core/core.salam` holds the generic
structs `Tuple2<A, B>` to `Tuple8<A, ..., H>` with public fields `f0`, `f1`, ...
Everything else is frontend work:

| Source                    | Becomes                                                           |
| ------------------------- | ----------------------------------------------------------------- |
| `(A, B)` as a type        | `Tuple2<A, B>`, instantiated through the normal generic machinery |
| `(a, b)` and `ret a, b`   | a struct literal of that instance with fields `f0`, `f1`          |
| `a, b := e`               | `__tupN := e`, then `a := __tupN.f0` and `b := __tupN.f1`         |
| `a, b = x, y`             | `__tupN := (x, y)`, then `a = __tupN.f0` and `b = __tupN.f1`      |
| `t.0`, `t.1`              | the member access `t.f0`, `t.f1`                                  |
| `(0, y)` in a `match` arm | the struct pattern `Tuple2{f0 = 0, f1 = y}`                       |

Because the instances live in `core` they are the same type in every package,
so a tuple can cross a package boundary. And because the backends only ever see
a struct returned by value, the C, LLVM, JavaScript, interpreter and native
backends needed no change at all; the struct-return paths they already had are
the ones tuples use.

The statements around a destructuring are inserted the same way `try` inserts
its temporaries (`s.pre_stmts`), so a `try` in the value runs first and the
names are ordinary locals afterwards.

## Not done

- Labeled elements, `(quot: int, rem: int)`.
- Nested tuple patterns, and tuple patterns in `switch`.
- Using tuples inside `compiler/` and `std/`: both are compiled by the previous
  release, which does not parse them. The standard library gets tuple-returning
  functions one release after this one.
