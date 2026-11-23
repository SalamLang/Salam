# Null and optional attributes

An attribute or style property whose value is exactly `null`, or whose value
is an optional (`T?`) that is empty, is not set. A required attribute that
could be null is a compile error.

## Representation

`T?` is `core.Option<T>`: a struct with `value: T` and `present: bool`, built
with `core.Some(x)` and `core.None()`, or assigned `null`. It has `IsSome()`,
`IsNone()`, `UnwrapOr(d)`, `Unwrap()`.

`HashMap.Get(k)` returns `V` and yields the zero value of `V` for a missing
key; it is not optional. So `works().Get(id).title` is optional only when the
field `title` is declared `str?`. A map of `V?` values is the way to model a
missing entry. Nothing here needs new syntax in `compiler/` or `std/`.

## Semantics

1. Optional attribute, value of type `T?`: set when present (using `.value`),
   not set when empty. Web: no attribute in the HTML, no style declaration,
   element text stays empty. Native: the property is not set.
2. Literal `null` on an optional attribute: accepted, means not set. The
   string `"null"` is just a string.
3. Required attribute (the `required` list of the element schema in
   `std/layout/elements`, for example `url` on `link` and `image`): a literal
   `null` or an expression of type `T?` is error E299 with a fix hint. Make
   it a plain `T` with `.UnwrapOr(default)`, `.Unwrap()`, or `.value` inside
   `if x.IsSome():`.
4. Values of non-optional type are unchanged.
5. Reactive: the web runtime re-renders the HTML and patches attributes, and
   the patch removes attributes missing from the new HTML, so a value going
   empty and back adds and removes the attribute. Native reconciles the
   builder calls the same way.

## Implementation

- `sema_layout_check`: literal `null` on an optional attribute is replaced by
  the synthetic empty value that already means "skip"; on a required one it
  reports E299.
- `layout_lower`: a runtime slot of an attribute kind is written through
  `web.PageAttr*` helpers that take `T` or `T?` (generic overloads), and the
  whole attribute is dropped when empty. Required slots go through
  `web.PageRequired`, which the view type check (E135 path) rejects when the
  argument is optional.
- `std/gui/ui` builder: optional overloads of the property setters.
