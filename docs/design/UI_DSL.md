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

Keyframe animations, enter and exit, shared elements and springs are
designed in `UI_BEHAVIOUR.md` (section 6).

### States and conditions

Any style attribute may carry one prefix: `hover`, `focus`, `focus within`,
`active`, `disabled`, `checked`, `invalid`, `placeholder`, `selection`
(`hover background = #ffeecc`). Screen size, dark mode, container size and
state of attributes are written as style `if` blocks (`UI_STATES_THEMES.md`,
section 1); design tokens and themes are in section 2 of that file;
`light dark(a, b)` colours work from the start. The standard components,
accessibility and assets are in `UI_COMPONENTS.md`; navigation, forms and
`bind`, testing, hot reload and gestures are in `UI_BEHAVIOUR.md`.

## Reactivity

The page writes plain variables and handlers; the compiler works out who
depends on what, and the runtime re-runs exactly that. There is no virtual
tree and no diff of a whole view: a re-run builds only the scope that read the
changed value, and the existing patch step copies the differences into the
live nodes. Nothing here adds a keyword: `name := value` stays the only way to
declare state (see `NATIVE_UI.md`, "Component and page variables"). Each rule
below is checkable: it names an example and the exact set of scopes that must
re-run, and each has one test for the web target and one for the native
target, written once and run on both (`tests/{en,fa}/page` and
`tests/{en,fa}/ui`). `SALAM_UI_TRACE=1` prints every scope that re-ran and
the value that made it run, on both targets, so a rule can be checked by eye.

A scope is a component instance, the page, or an element, `if` or `each` of
the page block that reads state (`NATIVE_UI.md`, "Scopes"). "Re-run set"
below means the scopes that execute their view code again.

Status: R1, R2, R4 and R8 exist today and get tests; R1 now holds inside
components on the native target (the web target still re-runs a whole
instance). R5, R7, R9 and R10 exist in part. R3, R6, R11 and R12 are new:
R12 has `SALAM_UI_TRACE`. Syntax that inference cannot express is
listed in "Open decisions" and is not implemented until the owner decides.

### R1. Fine grained, decided at compile time

A write marks the state it changes; only scopes whose dependency set contains
that state re-run. The dependency sets are fixed when the page is compiled, so
the runtime never records reads and never compares trees.

```salam
component Counter(name: str):
    count := 0
    button: id = name click = (): count++ end content = name + " " + count end
end

page:
    heading: content = "Counter demo" end
    Counter: name = "a" end
    Counter: name = "b" end
end
```

Pressing `a`: re-run set is the `a` instance. The heading, `b` and the page
do not run. Pressing it ten times in a row re-runs it ten times and nothing
else.

### R2. Writes are batched, one render per frame

Any number of writes inside one handler, timer callback, event delivery or
animation step mark bits; the runtime renders once after the handler returns
(web: one microtask after the last write, applied in the next frame; native:
once per frame). The dirty set is a union, so the same scope never runs twice
in a tick.

```salam
component Stepper:
    a := 0
    b := 0
    button: id = "go" click = ():
        a++
        a++
        b = a
    end content = a + " " + b end
end
```

Pressing `go`: three writes, re-run set is `Stepper` once. A write made while
a render is running (a `mount` hook, a derived callback) joins the next tick,
and a tick that finds nothing touched does no work at all: no view run, no
restyle, no layout, no paint.

### R3. Derived values are memoised

A name that is read but never written (`double := count * 2`) is derived. It
is evaluated once per tick, when something it reads was touched, and every
scope that reads it shares the result. A scope re-runs for a derived value
only when the result changed, not when its inputs changed.

```salam
component Parity:
    n := 0
    odd := n % 2
    label := "parity " + odd
    button: id = "inc" click = (): n += 2 end content = n end
    paragraph: content = label end
end
```

Pressing `inc` (n goes 0, 2, 4): `n` changes, `odd` is evaluated and is
still 0, so `label` is not evaluated again and the paragraph does not
run; only the button's scope runs. A chain of derived values costs one
evaluation per link per tick at most, in dependency order, and nothing for
links whose inputs did not move.

### R4. Writing a parent's value from a child re-runs only its readers

A child changes a parent's state through a callback parameter (as today:
`NATIVE_UI.md`, "Callbacks"). The write inside the callback is a write to the
parent's variable, so the re-run set is the scopes that read it, nothing
above or below them.

```salam
component Field(label: str, set: func (str)):
    box:
        input: id = label input = (v: str): set(v) end end
    end
end

component Form:
    name := ""
    greeting := "Hello " + name
    box:
        Field: label = "name" set = (v: str): name = v end end
        Field: label = "city" set = (v: str): end end
        paragraph: content = greeting end
    end
end
```

Typing in `name`: re-run set is the paragraph's scope only, reached through
the derived `greeting` (R3). The `city` field and both `Field` instances do
not re-run, so the input keeps focus, selection and typed text without any
special case. Two way binding (`bind`, see Open decisions) is the same rule
with the callback written by the compiler.

### R5. Shared state: page variables and stores, no keyword

State that several components use is declared once and read by name, as
today with a page block variable or a program global. Inference handles it:
a global that some scope reads and some code writes is state, wherever the
component is placed.

```salam
cart := Vector<str>()

component Badge:
    box: content = "Cart: " + cart.Len() end
end

component Buy(item: str):
    button: id = item click = (): cart.Push(item) end content = item end
end

page:
    Badge: end
    Buy: item = "tea" end
    Buy: item = "milk" end
end
```

Pressing `tea`: re-run set is `Badge`. The `Buy` instances do not run, because
they only write `cart`. A struct of state in a package (`mut Store`, or a
struct global) is the same: a scope depends on the fields it reads, a write to
one field does not wake readers of another (field level marks for struct
globals are new; today the whole global is one mark). No `provide`/`inject`
pair is needed because every instance is expanded inside its parent and names
resolve at compile time.

### R6. Props flow down, events flow up

A parent value passed as a component parameter makes the child depend on it;
when the parent value changes the child re-runs, and only when the value
changed (compared by value, the same cut-off as R3). A child never writes a
parameter back; it calls a callback or `emit`s (`NATIVE_UI.md`, "Events that
bubble"). A parent that re-runs for its own reasons does not re-run a child
whose parameters hold the same values.

```salam
component Price(amount: int):
    paragraph: content = amount end
end

component Shop:
    qty := 1
    note := ""
    box:
        button: id = "more" click = (): qty++ end content = "more" end
        input: id = "note" input = (v: str): note = v end end
        Price: amount = qty * 10 end
    end
end
```

Pressing `more`: re-run set is the `Price` instance and the `Shop` scope
that reads `qty`; typing in `note` re-runs nothing that reads `qty`, so
`Price` stays. Today a parent re-run re-runs every child natively; the rule
makes it skip equal parameters.

### R7. Effects, with cleanup, and mount and unmount

Code that must run when something appears, disappears or changes, and
undo itself afterwards, is a hook of the component (syntax in Open
decisions). The re-run set of a hook is itself: it never re-runs a view
unless it writes state.

- `mount` runs once after the instance first appears in the tree, after the
  tick's patch, so ids and focus exist.
- `unmount` runs once when the instance leaves the tree (an `if` turned
  false, a row removed, a page left). Cleanups of the instance run in
  reverse order of registration.
- An effect that names values (`watch`) runs after mount and again after a
  tick in which one of those values changed, calling its previous cleanup
  first.
- A timer or subscription started in `mount` and stopped in the cleanup is
  stopped exactly once, on both targets.

```salam
component Clock:
    now := 0
    on mount:
        id := every(1000, (): now++ end)
        ret (): stop_timer(id) end
    end
    paragraph: content = "tick " + now end
end
```

`Clock` placed under an `if`: when the `if` turns false the cleanup runs and
no tick ever writes `now` again; when it turns true, `now` restarts from 0
(R8's rule for an instance that reappears). Re-run set per tick of the timer:
the `Clock` instance.

### R8. Keyed lists keep instance state across moves

A row is matched by its `key` (or `id`) and its site, so inserting, removing
or reordering rows moves each instance's variables, focus and typed text with
its row, and only new keys run `mount`, only removed keys run `unmount`. A
duplicate key among siblings is a compile error when it is a literal and a
run time warning otherwise (E-number to be assigned).

```salam
component Row(name: str):
    hits := 0
    item: key = name content = name + " " + hits click = (): hits++ end end
end

page:
    list:
        each it in items:
            Row: name = it end
        end
    end
end
```

Click `b` twice, then move `b` before `a`: `b` shows 2 and `a` shows 0.
Re-run set of the move: the `each` scope (to reorder), no `Row` instance
runs; of a click: that `Row` only. Toggling one of 1000 rows re-runs one
scope, not 1000.

### R9. Async values: loading, error, value

A value that arrives later (a fetch, a read, a computation on a worker) is
state with three readable parts, so a view can show each without extra
bookkeeping. Starting one is a call; its completion is one write, so it is
batched and tracked like any other (R1, R2).

```salam
component Weather(city: str):
    data := load(() : Report: fetch_report(city) end)
    if data.loading: paragraph: content = "..." end end
    if data.error != "": paragraph: content = data.error end end
    if data.ready: paragraph: content = data.value.temp end end
end
```

When the result arrives: re-run set is the scopes reading `data`. A new
`city` starts a new load; an answer from the previous `city` that arrives
late is dropped (no stale result), and `unmount` drops a pending result.
Timers (`every`, `after`) follow the same rule: a callback runs as a handler,
its writes batch into one tick, and a timer owned by an instance dies with it.

### R10. No stale closures

A handler, callback or effect reads the current value of the variables it
names at the time it runs, never a copy taken when it was created. State
lives in per instance storage, not in the closure, so there is nothing to
go stale.

```salam
component Delay:
    n := 0
    seen := 0
    button: id = "inc" click = (): n++ end content = n end
    button: id = "snap" click = (): after(500, (): seen = n end) end content = seen end
end
```

Pressing `inc` three times within the half second after pressing `snap`
shows `seen` as 3, not 0. A callback inside `each` reads the row's loop
variable as of the row it belongs to (web handlers inside `each` use the
row component's variables, `NATIVE_UI.md`, "The escape hatch and the
limits"). The test fails if a handler observes an earlier value than the
state at call time.

### R11. Same semantics, same tests, both targets

Every rule above has one test written once in the page DSL, with the same
expected output on the web (`salam page build` run in a headless browser or
the JS test host) and natively (`ui.RenderPage`, `ui.DumpPage`, then the
event driver). A test counts scope re-runs (`ui.ScopeRuns()` natively, the
same counter in the page script on the web) and checks the counts in the rule's
"re-run set". When the two targets differ, the rule is not met.

### R12. Errors are visible, not silent

- A variable that is written but never read by a view is an instance
  variable (no marks, no re-runs) and says so in the trace.
- A write the compiler cannot see is covered by `ui.Invalidate()`; the trace
  names it as "full re-run" so it is easy to find and remove.
- `SALAM_UI_TRACE=1` prints one line per scope run: the site, the instance
  or element id, and the first touched value that matched
  (`run page:2 Counter#1 because count`), one line per skipped derived
  value (`skip even: unchanged`), and one per hook (`mount Clock#3`).

### Open decisions for the owner

Everything above keeps the "no state keyword" rule. These need a decision
because inference cannot express them; none is implemented until decided.

1. **Two way input binding.** `bind = name` as a plain attribute on `input`,
   `select`, `checkbox`: it expands to `value = name` plus an input handler
   that writes `name`. Without it, R4's callback form works and is the
   fallback. Decided: `bind` is accepted (`UI_BEHAVIOUR.md`, section 8).
2. **Lifecycle and effects.** `on mount:` and `on unmount:` blocks in a
   component, and `on change name, other:` for an effect that names its
   inputs, written like the existing `on saved` event handlers. Cleanup is
   the returned lambda of the block. The alternative is attributes
   (`mount = ...`) on the first element.
3. **Async values.** `load(func)` returning a value with `.loading`, `.error`,
   `.ready`, `.value`, and `every(ms, func)` / `after(ms, func)` timers owned
   by the instance. All are ordinary std functions; only the owner link
   (cleanup with the instance) is compiler work.
4. **Duplicate key error number and field level marks for struct globals**
   (R5, R8): behaviour changes that older pages could feel, so they are
   flagged.

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
- `gui.style`: 139 properties (`tools/salam/style-properties.tsv`, generated
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
