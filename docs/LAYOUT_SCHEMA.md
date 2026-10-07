# Layout schema

The layout DSL (`layout: ... end`) is driven by data in `std/layout/`. The
compiler reads those files at startup, so adding an HTML element or attribute
usually means adding a schema entry and tests, with no compiler change. The
full list of elements is in `docs/LAYOUT_ELEMENTS.md`.

```
std/layout/
  base.salam        the schema structs below
  categories/       content categories (flow, phrasing, interactive, ...)
  elements/         one LayoutElement per element or context variant
  attributes/       LayoutAttribute entries (global and element-scoped)
  style/            CSS properties (LayoutAttribute with destination = "css")
  values/           LayoutValue entries grouped into enums
std/layoutgen/      Salam generator functions (Node in, Output out)
```

Any schema file may hold any mix of `LayoutCategory`, `LayoutElement`,
`LayoutAttribute` and `LayoutValue` consts. Names come from the `@en` and
`@fa` lines. The first `@en` spelling is the canonical name unless `name` is
set. Every spelling in either language is accepted in both English and
Persian files, and spaces, `_`, `-` and ZWNJ are ignored when names are
compared.

## Elements

```salam
@en "link" "a"
@fa "پیوند"
pub const link := LayoutElement {
    generated_name = "a"
    categories = ["flow", "phrasing", "interactive", "palpable"]
    children = ["#transparent"]
    forbidden_descendants = ["#interactive"]
    required = ["url"]
}
```

| field                   | meaning                                                                                                                                         |
| ----------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------- |
| `name`                  | canonical name; needed when several variants share a spelling                                                                                   |
| `generated_name`        | HTML tag                                                                                                                                        |
| `kind`                  | `Normal` (`<x>...</x>`), `Void` (`<x>`, no children), `RawText` (unescaped text, for `script`/`style`), `Virtual` (no tag, a generator decides) |
| `categories`            | WHATWG content categories this element belongs to                                                                                               |
| `parents`               | allowed direct parents (element names or `#category`)                                                                                           |
| `path`                  | exact parent chain, nearest first, e.g. `["row", "table header"]`                                                                               |
| `ancestors`             | at least one ancestor must match                                                                                                                |
| `children`              | allow-list for direct children; `"text"` permits text content, `"#transparent"` inherits the parent's rules, `"none"` allows nothing            |
| `forbidden`             | deny-list for direct children (checked before `children`)                                                                                       |
| `forbidden_descendants` | deny-list for the whole subtree                                                                                                                 |
| `required`              | attributes that must be present (emitted first)                                                                                                 |
| `when`                  | `"attr"` or `"attr=value"`: this variant applies only then                                                                                      |
| `unique`                | `"parent"` or `"document"`: at most one there                                                                                                   |
| `generator`             | `heading`, `media`, `style`, `global`, `script`, `font_face` (built-in) or a function in `std/layoutgen`                                        |
| `placement`             | `"head"` sends the tag to `<head>`                                                                                                              |
| `position`              | `"first"`: must be the first element inside its parent (E125)                                                                                   |
| `styled`                | `false` rejects CSS attributes on this element                                                                                                  |

### Context variants

Several consts can share a spelling. The compiler picks the variant whose
`parents`, `path`, `ancestors` and `when` rules all hold, preferring the one
with the most rules, then falls back to the variant with no rules. A schema
where two rule-less variants share a spelling is rejected. Example: `header`
is `<header>` by default and `<thead>` inside `table`:

```salam
@en "header" "thead"
@fa "سرصفحه"
pub const table_header := LayoutElement {
    name = "table header"
    generated_name = "thead"
    parents = ["table"]
    children = ["row", "#script-supporting"]
    unique = "parent"
}
```

## Attributes

```salam
@en "url" "source" "href" "src"
@fa "نشانی" "منبع" "منبع رسانه"
pub const url := LayoutAttribute {
    generated_name = "href"
    value_type = "url"
    elements = ["link", "head link"]
}
```

| field            | meaning                                                                                                                                                                                                                 |
| ---------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `generated_name` | HTML attribute name; empty means the attribute only feeds the generator. A `*` (`data-*`) makes a pattern whose names are prefixes: `data user id` becomes `data-user-id`                                               |
| `destination`    | `html`, `content`, `class`, `style`, `css`, `dir`, `lang`, `meta`, or `html_or_css` (a bare number becomes the HTML attribute, anything with a unit becomes CSS; used for `width`/`height` on images, media and canvas) |
| `value_type`     | see below                                                                                                                                                                                                               |
| `allowed`        | for `enum`: a value group name or a comma list                                                                                                                                                                          |
| `elements`       | scope; empty means every element                                                                                                                                                                                        |
| `multiple`       | the value is a space-separated list, each item validated                                                                                                                                                                |
| `default`        | emitted when the attribute is absent (scoped attributes only)                                                                                                                                                           |

The same name may be declared once per scope. Lookup tries the element-scoped
entry first, then the global one. Unknown attributes are errors, and so are
CSS attributes on unstyled elements.

Value types: `string`, `token` (one word, no spaces), `int`, `uint`, `float`,
`ufloat`, `number`, `bool`, `url`, `absolute_url`, `relative_url`, `email`,
`color`, `length`, `enum`, `lang`, `dir`, `mime`, `date`, `time`, `datetime`,
`duration`, `idref` (must name an `id` in the same layout), `charset`,
`srcset`, `media_query`, `code` (event handler JavaScript), `ulength`
(non-negative size: a bare number, a CSS length or a CSS function), `bool_enum`
(`true`/`false` like a boolean, or one of the `allowed` values, as for
`hidden = "until-found"`).

`css` is the value type of CSS properties. `allowed` lists what a value may be: the type words `length`, `percentage`, `number`, `integer`, `time`, `angle`, `color`, `image`, `string`, `ident`, `ratio`, `easing`, `flex` and `resolution`, plus `@group` for keyword values defined in `values/`. `maxN` allows up to N space-separated values (as in `margin`) and `list` a comma list (as in `transition-property`). CSS-wide keywords and `var()`/`calc()`-style functions are always accepted. The full table is in `docs/CSS_PROPERTIES.md`.

## Values

```salam
@en "jalali date"
@fa "تاریخ شمسی"
pub const it_jalali_date := LayoutValue { group = "input_type" generated_value = "jalali date" generator = "jalali_date" }
```

`group` ties the value to an `enum` attribute's `allowed`. A `generator` on a
value replaces the element's generator when that value is chosen, and the
attribute itself is not printed.

## Generators

A std generator is a function in `std/layoutgen`:

```salam
pub func jalali_date(n: Node): Output:
    mut o := Output {}
    o.html = "<input type=\"text\"" + n.attrs + ClassAttr(n, n.uid) + ">"
    o.js = "salamJalaliDate(document.querySelector(\"." + n.uid + "\"));"
    o.shared_js = JALALI_JS
    ret o
end
```

`Node` carries `name`, `tag`, `uid` (stable per page: `elm_1`, `elm_2`, ...),
`class_name`, `attrs` (the HTML attributes the default generator would print),
`content`, `inner` (rendered children), `parents` (nearest first) and every
raw attribute via `Get`/`Has`. `Output` has `html`, `css`, `js`, `head`, plus
`shared_css`/`shared_js`, which are emitted once per page no matter how many
elements use them. All generator calls on a page run in one batch in the
compiler's interpreter. If one fails, the build reports E123.

## Tests

Every layout test lives under `tests/{en,fa}/layout/`, split by subject:

| directory     | holds                                                       |
| ------------- | ----------------------------------------------------------- |
| `elements/`   | one element or attribute family per file                    |
| `style/`      | CSS emission: properties, lengths, selectors, global styles |
| `components/` | components and `include`                                    |
| `pages/`      | whole-page output: root content, direction, titles          |
| `errors/`     | everything that must be rejected                            |

A success test is a layout build plus `// EXPECT:` substrings, with
`// EXPECT-NOT:` for text that must not appear. A failure test is
`// EXPECT: E0xx` plus `// EXPECT-MSG:`. A `.salam` file left directly in
`tests/{en,fa}/layout/` belongs to no subdirectory and so would never run;
the runner reports that as a failure rather than skipping it. Run the lot, or
one directory:

```sh
sh tools/bash/run-tests.sh layout
sh tools/bash/run-tests.sh layout/errors
```

`compiler/tests_port/layout_test.salam` fails if the schema has problems
(unknown categories or elements in lists, duplicate entries, ambiguous
variants). Those problems also show up as warning W124 when a layout is
compiled.
