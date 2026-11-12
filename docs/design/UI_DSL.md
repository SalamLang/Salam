# One page language, two targets: the Salam UI standard

Status: accepted design (2026-11-11). Supersedes the "mini browser" design in
`NATIVE_UI.md`.

## The rule

A Salam program describes its user interface only in the page language
(`page:`, `component`, elements, attributes, `each`, `if`, events, state).
The compiler turns that one description into:

- **web**: HTML, CSS and JavaScript;
- **native**: Salam code that builds typed UI nodes, laid out and drawn by
  `gui.ui` in a window from `gui.window` (Linux, Windows, macOS now, Android
  later).

HTML and CSS are output formats of the web target only. They never appear in
Salam source, and the native target never parses or interprets them.

Salam has its own style standard, in the spirit of QML: similar to CSS where
CSS is good, different where CSS is accidental. Every property in the
standard works on every target. A property is added only when every target
implements it. The only web-only parts of a page are web metadata (SEO,
`meta`, `head link`, and similar), which the native build ignores.

## What goes away

| removed                                                  | replaced by                                  |
| -------------------------------------------------------- | -------------------------------------------- |
| `std/gui/css` (parser, cascade, selectors, UA sheet)     | `gui.style` (the standard) + `gui.color`     |
| `std/gui/cssbox` (CSS block/flex/grid/table layout)      | `gui.flow` (layout engine for the standard)  |
| `gui.ui` HTML parser, `ui.Run(view: func () str)`, `RunHtml`, `Render(doc)`, `DumpHtml` | typed builder only |
| the native generator's CSS path (`css.CompileRule`, `__salam_styles`) | typed style values from `gui.style` |
| CSS property attributes in `std/layout/style` (about 500) | the standard's vocabulary (below)           |
| `style:` element, `inline style`, `global: selector = ...`  | nothing; raw CSS is not part of Salam      |

Kept: `gui.window`, `gui.paint`, `gui.typeset`, and the non-CSS parts of
`gui.ui` (scopes and state, events and bubbling, focus, text editing,
controls, damage tracking, `SALAM_UI_SCRIPT` headless tests).

## Packages and who owns what

```
compiler (lexer, parser)   literals: 50%  #3366aa  12px  1fr  200ms  45deg
                           value lists: padding = 8, 16
compiler (layout)          checks every value with gui.style at compile time,
                           web: CSS text from gui.style, native: typed values
gui.color                  every colour syntax, conversion, mixing, CSS output
gui.style                  the standard: properties, value kinds, keywords
                           (en + fa), parsing, validation, defaults per
                           element, inheritance, interpolation, web CSS
gui.flow                   layout: column, row, wrap, grid, stack, absolute,
                           scroll; input typed FlowStyle, output rectangles
gui.ui                     node tree, events, state, text through gui.typeset,
                           paint through gui.paint, window through gui.window
```

`gui.color` depends on nothing in gui. `gui.flow` depends on nothing in gui.
`gui.style` depends only on `gui.color`. `gui.ui` depends on all of them.

## Value syntax

The same syntax is used in page source, in a string given to a style
attribute (`color = "red"` equals `color = red`), and in a string computed at
run time (`color = c` where `c` is a `str`). `gui.style` parses all three, so
validation is identical.

| form                         | example                                   | meaning                                    |
| ---------------------------- | ----------------------------------------- | ------------------------------------------ |
| bare number                  | `8`, `1.5`                                | the property's natural unit: pixels for lengths, milliseconds for times, degrees for angles, a plain number for `opacity`, `grow`, `line height`, `scale` |
| percent                      | `50%` (Persian `۵۰٪`)                     | percent of the reference (container size for lengths); outside page attributes `50%` is the number `0.5` |
| unit suffix                  | `12px` `1.5em` `2rem` `1fr` `50vw` `50vh` `200ms` `2s` `45deg` `0.5turn` `1rad` | explicit unit; also as calls: `px(12)`, `em(1.5)`, `fr(1)` (Persian names come from the schema) |
| length arithmetic            | `100% - 20`, `50% + 1em`                  | a length is a sum of units; web emits `calc()` |
| colour                       | `red`, `#fff`, `#ffff`, `#3366aa`, `#3366aa80`, `rgb(51, 102, 170)`, `rgba(0, 0, 0, 0.25)`, `hsl(210, 50%, 40%)`, `hwb(...)`, `lab(...)`, `lch(...)`, `oklab(...)`, `oklch(...)`, `cmyk(0, 40%, 60%, 10%)`, `color(display p3, 0.3, 0.6, 0.4)`, `mix(red, blue, 30%)`, `light dark(white, black)`, `current`, `transparent`, Persian names (`قرمز`) | |
| keyword                      | `bold`, `center`, `row`, `solid`, `auto`  | from the property's keyword set (en or fa) |
| list                         | `padding = 8, 16` (Persian `،`)           | a comma after a value continues the same attribute; a newline after a trailing comma continues it too |
| group                        | `shadow = [0, 4, 8, black], [0, 1, 2, red]` | `[...]` groups one item of a repeated property |
| call                         | `fr(1)`, `repeat(3, 1fr)`, `minmax(100, 1fr)`, `linear(90, red, blue)`, `image("a.png")`, `ease(0.2, 0, 0, 1)` | |
| string                       | `"red"`, `"8, 16"`, `"#fff"`              | parsed with this same syntax; never CSS     |
| any Salam expression         | `width = w`, `gap = n * 2`, `color = c`   | static values are checked at compile time; dynamic ones at run time |

`#` followed by 3, 4, 6 or 8 hex digits is a colour literal anywhere in
Salam (its type outside page attributes is `str`, holding the same text).
`%` directly after a number literal, followed by something that cannot start
an operand (space, end of line, `,`, `)`, `]`, `end`), is a percent; `a % b`
and `7%3` stay the remainder operator.

Composite values take their parts in any order where the kinds differ, like
`border = 2, solid, #3366aa` or `border = red, 1`.

## Vocabulary

Names are English canonical names; every one also gets a Persian name in
`std/layout`. "1-4" means one to four comma values in top, end, bottom,
start order (start and end follow the text direction). Every box uses
border-box sizing; there is no `box sizing` property.

### Layout of children (container)

| property | values | default |
| --- | --- | --- |
| `layout` | `column` `row` `grid` `stack` `flow` | `column` (`flow` for text elements) |
| `gap` | len, or `row gap, column gap` | 0 |
| `align` | cross axis: `start` `center` `end` `stretch` `baseline` | `stretch` |
| `justify` | main axis: `start` `center` `end` `between` `around` `evenly` | `start` |
| `align content` | wrapped lines / grid: `start` `center` `end` `stretch` `between` `around` `evenly` | `start` |
| `wrap` | `true` `false` (row, column) | `false` |
| `reverse` | `true` `false` (row, column) | `false` |
| `columns`, `rows` | track list: len, `fr`, `auto`, `minmax(a, b)`, `repeat(n or fill or fit, tracks)`, `fit(len)` | none |
| `auto columns`, `auto rows` | track | `auto` |
| `auto flow` | `row` `column` `dense` `row dense` `column dense` | `row` |

`column`: children top to bottom (flex column). `row`: children in the text
direction (flex row; mirrored in right to left). `grid`: tracks. `stack`:
every child in the same cell, later children on top (aligned by `align` and
`justify`). `flow`: inline flow, children placed like words in a paragraph
and wrapped onto lines, baseline aligned; text elements use it.

### Child in its parent

| property | values | default |
| --- | --- | --- |
| `grow`, `shrink` | number | 0, 1 |
| `basis` | len, `auto` | `auto` |
| `align self` | `auto` + the `align` keywords | `auto` |
| `justify self` | grid and stack only: `auto` `start` `center` `end` `stretch` | `auto` |
| `column`, `row` | grid line number (1 based, negative from the end) | auto |
| `column span`, `row span` | integer | 1 |
| `order` | integer | 0 |
| `position` | `normal` `absolute` `fixed` `sticky` | `normal` |
| `top` `bottom` `start` `end` `left` `right` | len | `auto` |
| `inset` | 1-4 len | `auto` |
| `layer` | integer (paint order and hit testing) | 0 |

### Box

| property | values | default |
| --- | --- | --- |
| `width` `height` | len, `auto`, `fill` (100% of the parent's content box), `fit` (fit content), `min content`, `max content` | `auto` |
| `min width` `min height` `max width` `max height` | len, `none` (max) | 0 / `none` |
| `aspect ratio` | number (`16 / 9` is an expression) | none |
| `padding`, `margin` | 1-4 len; sides: `top`, `bottom`, `start`, `end`, `left`, `right`, plus `x`, `y` (`padding x = 16`); margin also takes `auto` | 0 |
| `overflow` (+ `overflow x`, `overflow y`) | `visible` `hidden` `scroll` `auto` | `visible` |
| `visible` | `true` `false` (false removes it from layout) | `true` |
| `clip` | `true` `false` (clip children to the rounded border box) | `false` |

### Look

| property | values |
| --- | --- |
| `background` | paint: colour, `linear(angle or to side, stops...)`, `radial(...)`, `conic(...)`, `image("x.png")`; a list paints layers top first |
| `background size` | `cover` `contain` `auto` or 1-2 len |
| `background position` | 1-2 len or `center` `top` `bottom` `start` `end` |
| `background repeat` | `true` `false` `x` `y` |
| `border` | composite: width, style (`solid` `dashed` `dotted` `double` `none`), colour; sides `border top` ... ; parts `border width` (1-4), `border style`, `border color` (1-4) |
| `radius` | 1-4 len (top start, top end, bottom end, bottom start); corners `radius top start` ... |
| `outline` | composite like `border`, plus `outline offset` |
| `shadow` | groups of x, y, blur, spread, colour, `inset` |
| `opacity` | number 0..1 or percent |
| `blur`, `brightness`, `contrast`, `grayscale`, `saturate`, `sepia`, `invert`, `hue rotate` | filters on the element |
| `backdrop blur` (and the same filters with `backdrop`) | filters on what is behind it |
| `blend` | `normal` `multiply` `screen` `overlay` `darken` `lighten` ... |
| `translate` (1-2 len), `rotate` (angle), `scale` (1-2 numbers), `skew` (1-2 angles), `origin` (1-2 len or keywords) | transforms, applied after layout |
| `cursor` | `default` `pointer` `text` `move` `grab` `not allowed` `wait` `crosshair` `resize x` `resize y` ... |
| `interactive` | `true` `false` (false: the element ignores the pointer) |
| `selectable` | `true` `false` (text selection) |
| `image fit` | `cover` `contain` `fill` `none` `scale down` (for `image` and `video`) |
| `accent` | colour for checkboxes, radios, range, progress |
| `caret` | colour of the text caret |
| `scrollbar` | `auto` `thin` `none`, `scrollbar color` |

### Text (inherited)

| property | values |
| --- | --- |
| `color` | colour |
| `font` | family list: strings and `sans` `serif` `mono` `system` |
| `font size` | len |
| `font weight` | `thin` `light` `normal` `medium` `semibold` `bold` `black` or 100..900 |
| `italic` | `true` `false` |
| `line height` | number (times font size) or len |
| `letter spacing`, `word spacing` | len |
| `text align` | `start` `center` `end` `justify` `left` `right` |
| `decoration` | `none` `underline` `overline` `strike`; `decoration color`, `decoration style`, `decoration thickness` |
| `transform text` | `none` `upper` `lower` `capitalize` |
| `wrap text` | `true` `false` |
| `max lines` | integer (with an ellipsis) |
| `ellipsis` | `true` `false` (single line) |
| `direction` | `auto` `rtl` `ltr` |
| `white space` | `normal` `keep` (keep spaces and newlines) |
| `text shadow` | groups of x, y, blur, colour |
| `font features` | strings like `"tnum"` |

### Motion

| property | values |
| --- | --- |
| `transition` | duration, easing, delay; applies to every animatable property |
| `transition properties` | list of property names (default all) |
| easing keywords | `linear` `ease` `ease in` `ease out` `ease in out` `steps(n)` `ease(x1, y1, x2, y2)` `spring(stiffness, damping)` |

Keyframe animations get their own page syntax later (a named `animation:`
block), not part of the first version.

### States and conditions

Any style attribute may carry one prefix: `hover`, `focus`, `focus within`,
`active`, `disabled`, `checked`, `invalid`, `placeholder`, `selection`
(`hover background = #ffeecc`). Screen conditions and dark mode are written
as conditions on attributes in a later version; `light dark(a, b)` colours
work from the start.

## Contracts between packages

These are the boundaries the parallel work builds against. A package may add
more public functions; it must not change these without updating this file.

### gui.color (package `color`)

```salam
pub struct Color:
    pub r: f64 = 0.0     sRGB, 0..1, may be outside 0..1 for wide gamut
    pub g: f64 = 0.0
    pub b: f64 = 0.0
    pub a: f64 = 1.0
    pub current: bool = false     the `current` keyword (inherit text colour)
end
pub func Parse(text: str, out &: Color): bool    every syntax in the table, en + fa names
pub func Error(text: str): str                   "" when Parse accepts text, else a message
pub func Rgb(r: int, g: int, b: int): Color
pub func Rgba(r: int, g: int, b: int, a: f64): Color
pub func FromArgb(v: u32): Color
pub func Argb(c: Color): u32                     straight (not premultiplied), clamped to sRGB
pub func Css(c: Color): str                      shortest CSS that every current browser renders the same
pub func Mix(a: Color, b: Color, amount: f64): Color      in oklab
pub func Lerp(a: Color, b: Color, t: f64): Color          for transitions, in oklab, premultiplied alpha
```

`light dark(a, b)` parses to a colour with both variants kept (the exact
shape is up to `gui.color`, exposed as `Light(c)`, `Dark(c)`,
`HasDark(c)`).

### gui.flow (package `flow`)

```salam
pub struct Len:          resolved except percent: px + pct * reference
    pub px: f64 = 0.0
    pub pct: f64 = 0.0
    pub kind: int = 0    LEN_FIXED, LEN_AUTO, LEN_FILL, LEN_FIT, LEN_MIN_CONTENT, LEN_MAX_CONTENT, LEN_NONE
end
pub struct FlowStyle:   every property from "Layout of children", "Child in its parent", and "Box"
                        except the visual ones, plus border widths (they take space) and
                        direction (rtl mirrors row, grid columns and start/end)
pub struct Tree         nodes by int id; parent, children in order
pub func NewTree(): Tree
tree.Add(parent: int, style: FlowStyle): int
tree.SetStyle(id, style), tree.Remove(id), tree.Move(id, parent, index)
tree.SetMeasure(id, fn: func (int, f64, f64) Size)   leaf measuring (text, images); args: id, available width (-1 = unbounded), available height
tree.Layout(width: f64, height: f64)                 re-lays out only dirty subtrees
tree.X(id), Y(id) (relative to parent border box), W(id), H(id), Baseline(id), ContentW(id), ContentH(id) (for scrolling)
tree.SetScroll(id, x, y)                              scroll offsets shift children
```

em, rem, vw and vh are resolved to pixels by the caller before they reach
`gui.flow`; percent stays percent because only layout knows the reference.

### gui.style (package `style`)

```salam
pub const P_WIDTH := ...         one int id per property; names and fa names in a table
pub func PropByName(name: str): int      en and fa names, -1 when unknown
pub func PropName(id: int): str
pub func Inherited(id: int): bool
pub func Animatable(id: int): bool

pub struct Value                 typed, tagged value of any property
pub func Parse(prop: int, text: str, out &: Value): bool     the value syntax above
pub func Error(prop: int, text: str): str                     "" or a message with the expected form
pub func Source(prop: int, v: Value): str                     Salam expression that rebuilds v without parsing
pub func Css(prop: int, v: Value): str                        CSS declarations for the web target
pub func BaseCss(): str                                       base sheet so web matches native defaults

pub struct Style                 every property as a typed field
pub func Default(element: str): Style     defaults per element kind (heading 1-6, paragraph, button, link, input ...)
pub func Set(s &: Style, prop: int, v: Value)
pub func Inherit(parent: Style, child &: Style)
pub func Lerp(prop: int, a: Value, b: Value, t: f64): Value
```

`gui.style` does not import `gui.flow`. `gui.ui` maps a `Style` to a
`flow.FlowStyle` (resolving em, rem, vw and vh to pixels) in phase 2.

The compiler turns a static page value (literal AST) into canonical text and
calls `Parse`; on failure it reports `Error` at the attribute. For web it
emits `Css`; for native it emits `Source`. A dynamic value is parsed at run
time on native and converted by a small JavaScript helper on web.

### As built in phase 1

The packages are on main: `gui.color` (#3141), `gui.flow` (#3146),
`gui.style` (#3143), and the page literals (#3148). Their PR bodies list
every decision taken where this document was silent; they are part of the
contract. The main ones:

- `gui.flow`: one `inset: Edges` field instead of separate sides,
  `direction` is `DIR_INHERIT` by default, `text_align` and
  `Size.baseline`/`Size.last_baseline` added; min width and min height
  default to `auto` (content minimum); fixed and absolute boxes without
  insets keep their static position; `flow` ignores `gap`; `column = -1`
  is the last column.
- `gui.style`: 137 properties (`tools/salam/style-properties.tsv`, generated
  with `tools/salam/gen-style-table.salam`), `Style` fields per property
  (`layout_kind` for `layout`), extra API `Get`, `Format`, `EaseAt`, `Info`;
  web CSS relies on `BaseCss()` and `--salam-*` custom properties and nested
  `&>*` rules, so it must go into a style sheet rule, never a `style`
  attribute.
- Page literals: a static list reaches the layout code folded by
  `lay_list_text` / `LayoutListText` (`compiler/semantic/sema_layout_fold.salam`)
  as CSS text joined with spaces; before folding it is `AST_TUPLE` (list) and
  `AST_ARRAY_LIT` (group), and number tokens keep their unit
  (`LayoutNumUnit`).

## Phase 2 contracts

### Attribute resolution in the compiler (owned by the web branch)

One function decides, for every style attribute of every element, what the
standard says about it. Both generators call it; neither looks at CSS text.

```salam
StyleAttr:          prop (gui.style id), state (0 base, then hover, focus,
                    focus within, active, disabled, checked, invalid,
                    placeholder, selection), static (bool), value (style.Value
                    when static), expr (AST of the runtime expression when
                    not static), kind of the runtime expression (number, str,
                    other)
```

- The attribute name (en or fa, with an optional state prefix) is looked up
  with `style.PropByName`. A static value is turned into canonical Salam
  value text (list items joined with `", "`, groups in `[ ]`, Persian
  keywords and digits as written) and parsed with `style.Parse`; a failure is
  a compile error carrying `style.Error`.
- Transition period: an attribute that is not in the standard but is a
  known CSS attribute of today's schema, or a string value that the standard
  rejects but today's CSS check accepts, still builds for the web with
  warning W171 ("not part of the Salam style standard"; the message shows
  the standard form when one exists). The native target rejects it (E170).
  After the pages are migrated, W171 becomes an error and the old CSS
  attributes are deleted.

### Native builder (owned by the native branch)

`gui.ui` keeps its scope, state, event and reconcile machinery and its
`SALAM_UI_SCRIPT` test driver. Its node tree, style and layout are rebuilt
on `gui.style`, `gui.flow`, `gui.paint` and `gui.typeset`. The generated
program uses only this API:

```salam
ui.RunPage(patches: func () Vector<ui.Patch>, view: func (ui.Builder))
ui.Patch            ordered (prop, style.Value) pairs; built once at start
patch.Add(prop: int, v: style.Value)
b.Title(s), b.Lang(s), b.Dir(s)
b.Open(kind: str)          canonical English element name from std/layout
                           ("box", "heading", "button", ...), never an HTML tag
b.Patch(state: int, id: int)                 a static patch for this element
b.Set(state: int, prop: int, v: style.Value) one dynamic typed value
b.SetText(state: int, prop: int, text: str)  one dynamic value parsed at run time
b.Id(s), b.Key(s), b.Attr(name, value), b.BoolAttr(name, value)
b.Text(s), b.On(event, fn), b.Close()
b.Scope(...), b.KeyedScope(...), b.Reactive(...), b.Instance(...), b.Fresh()
```

The element's computed style is `style.Default(kind)`, then `Inherit` from
the parent, then its base patch and `Set` values, then the patches of every
active state in a fixed order. Text inside a `flow` parent (text nodes and
inline text elements such as bold, italic, code, link, span) is laid out as
one paragraph through `gui.typeset` and measured as one `gui.flow` leaf, so
it wraps across spans like the web does.

## Order of work

Phase 1 (done): page literals #3148, `gui.color` #3141, `gui.flow` #3146,
`gui.style` #3143.

Phase 2, two branches in parallel:

5. **Web on the standard** (`feat/ui-standard-web`): attribute resolution;
   `std/layout` gains the standard's properties (generated from
   `style-properties.tsv`) with the transition rules above; web CSS from
   `style.Css` with `style.BaseCss()` first; runtime values on web; page tests.
6. **Native on the standard** (`feat/ui-standard-native`): `gui.ui` rebuilt
   on the four packages with the builder above; `layout_native.salam` emits
   it; delete `gui.css`, `gui.cssbox`, the HTML parser and `ui.Run(html)`
   and their tests and website pages; `tests/*/ui` keep passing.

Then:

7. **Migrate pages**: a rewrite tool for common CSS values; tests, examples,
   website; W171 becomes an error and the old CSS attributes go.
8. **Same look check**: render pages on both targets and compare
   (headless browser in Docker or CI against `--ui=native` screenshots).
9. **Docs**: website pages, the skill file, the book.
