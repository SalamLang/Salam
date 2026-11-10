# Native UI from the page language

Status: a page runs in a native window with `salam run page.salam --ui=native`.
The runtime is `std/gui/ui` on top of `gui.window`, `gui.paint`, `gui.typeset`,
`gui.cssbox`, `gui.css` and `gui.reactive`. Later phases are planned below.

## Using it

```salam
mut count := 0

page:
    title = "Counter"
    box:
        heading: content = "Count: " + count end
        button: content = "+1" click = (): count++ end end
    end
end
```

```
salam page build app.salam            web page, as before
salam run app.salam --ui=native       the same page in a native window
salam build app.salam --ui=native     a native executable
```

For a file with a page and no entry function, the compiler does all style work
at compile time. Every declaration list, global rule and media rule is parsed,
validated and shorthand-expanded by `gui.css` while the compiler runs, and the
resulting typed style program is emitted as Salam code that rebuilds it from
literals (`__salam_styles`). The page itself becomes `__salam_page(b)`, a
function of `gui.ui` builder calls (`b.Open("div")`, `b.Decls(3, 4, -1, -1)`,
`b.Text(...)`, `b.On("click", handler)`, `b.Close()`); runtime values, `if` and
`each` stay ordinary Salam code. No HTML or CSS text is parsed when the program
runs, apart from a style attribute whose value is itself a runtime expression.
After every handler the runtime runs the view again and reconciles the new tree
with the old one, so a handler only changes the program's state; hover, focus,
scroll positions and typed text survive. The page view reads the program's
mutable globals. `SALAM_NATIVE_DUMP=1` prints the generated program.

A program that has its own entry function keeps it, and can call
`ui.RunCompiled((): ret __salam_styles() end, (b: ui.Builder): __salam_page(b) end)`,
or build HTML documents with runtime `page Name(params):` views and `ui.Run`.

Tests run headless: `SALAM_WINDOW_BACKEND=headless` with `SALAM_UI_SCRIPT`
naming a file of steps (`click #id`, `type text`, `key Tab`, `dump`,
`shot out.png`, `quit`). The `ui` section of `tools/bash/run-tests.sh` builds
each `tests/<lang>/ui/X.salam` with `--ui=native` and runs it with `X.script`.

## The problem

A `page:` block (`صفحه:` in Persian) compiles to HTML, CSS and JavaScript.
A desktop app can show that page today, but only inside a web view
(`std/gui/webview`), which means shipping or depending on a browser engine
and accepting its look, start time and memory use. There is no way to write
a page once and get a real native window that Salam draws itself.

Most of what a page says is not specific to the web. A box with a border, a
heading, a paragraph of Persian text, a button with a hover colour, a flex
row, a list built with `each`, a form: all of these describe a user
interface, not a document format. The parts that only make sense in a
browser are few: search engine metadata, `<head>` links, scripts, frames,
and raw markup islands such as inline SVG.

## The goal

One page source, two targets:

```
salam page build app.salam              -> app.html (web, unchanged)
salam page run app.salam --ui=native    -> a native window, drawn by Salam
```

- The same syntax, elements, attributes and CSS properties on both targets.
- The same look on both targets and on every OS: the native target draws
  everything itself, like Slint, instead of mapping to OS widgets.
- No browser, no external UI library. Rendering and layout are written in
  Salam; only the window and the pixel blit use the OS.
- Persian and right to left text work from the start (shaping, bidi,
  line breaking).
- Web-only features stay available for web. A native build reports them
  clearly instead of silently dropping them, and `if SALAM_UI_WEB:` lets a
  page keep web-only parts next to portable ones.

## Targets and the portable core

The compiler knows which UI target it is building for:

| target   | how it is chosen                                          | `SALAM_UI_WEB` | `SALAM_UI_NATIVE` |
| -------- | --------------------------------------------------------- | -------------- | ----------------- |
| `web`    | default for every command                                 | true           | false             |
| `native` | `--ui=native` (`salam check`, later `page run`, `bundle`) | false          | true              |

Both names are compile-time constants, folded like `SALAM_OS_LINUX`. In
Persian they are `سلام رابط وب` and `سلام رابط بومی`. Inside a page:

```salam
page:
    title = "Notes"
    if SALAM_UI_WEB:
        meta: name = "description" content = "A small notes app" end
    end
    box:
        heading: content = "Notes" end
    end
end
```

A native build drops the `if SALAM_UI_WEB:` branch before checking, so the
`meta` inside it is never seen.

### What the schema says about each target

Every `LayoutElement` and `LayoutAttribute` in `std/layout` has an optional
`native` field:

| `native` | meaning on the native target                                     | diagnostic |
| -------- | ---------------------------------------------------------------- | ---------- |
| `""`     | portable: works the same way on both targets (the default)       |            |
| `"skip"` | web metadata with no native meaning; the native build ignores it | W166       |
| `"no"`   | needs a browser; the native build refuses it                     | E165       |

Nothing changes for the web target. Both diagnostics appear only with
`--ui=native`.

**Ignored on native (W166)**, because they only tell a browser or a search
engine something:

- elements `meta`, `head link`, `base`, `noscript`, `track`
- attributes for loading and fetching (`srcset`, `sizes`, `loading`,
  `decoding`, `fetch priority`, `crossorigin`, `integrity`, `referrer policy`,
  `ping`, `preload`, `imagesrcset`, `imagesizes`, `blocking`, `nonce`,
  `hreflang`, `download`, `target` and `rel` on links)
- microdata (`itemscope`, `itemtype`, `itemprop`, `itemid`, `itemref`),
  `data *` attributes, `translatable`
- form submission to a server (`action`, `method`, `enctype`, `novalidate`,
  `accept charset`, `form action`, `form method`, `form enctype`,
  `form novalidate`, `form target`); a native form still fires `submit`
- browser text-entry hints (`autocomplete`, `autocapitalize`, `autocorrect`,
  `spellcheck`, `writingsuggestions`, `dirname`)

**Refused on native (E165)**, because they embed another document or a
script, or rely on browser-only machinery:

- elements `script`, `style` (raw CSS), `iframe`, `embed`, `object`,
  `template`, `slot`, `svg` and `math` (raw markup), `image map`, `area`
- attributes `inline style` (raw CSS), `contenteditable`, `popover`,
  `popover target`, `popover target action`, `command`, `commandfor`,
  `shadowrootmode` and the other shadow root attributes, `slot`, `part`,
  `exportparts`, `is`, `srcdoc`, `xmlns`, `use map`, `ismap`
- events that only a browser fires: `before match`, `context lost`,
  `context restored`, `cue change`, `form data`,
  `security policy violation`, `slot change`

**Everything else is portable**, including the semantic elements (`article`,
`section`, `nav`, `header`, `footer`, ...), which render as boxes with the
same default styles a browser gives them. `class` and `global` stay
portable: the native cascade supports element, class and id selectors.

### Native meaning of page settings

| page setting / element  | native target                                                                                 |
| ----------------------- | --------------------------------------------------------------------------------------------- |
| `title`                 | window title                                                                                  |
| `favicon`               | window and taskbar icon                                                                       |
| `lang`, `dir`           | text direction and the default font fallback order                                            |
| `link url = ...`        | another page of the app opens in the window; an `http(s)` address opens in the user's browser |
| `font`                  | font loaded from the app's bundled assets                                                     |
| `media condition = ...` | window size, orientation and OS dark mode                                                     |
| `image url = ...`       | file decoded with the codecs in `std/ai/image`                                                |

## Architecture

```
page source -> parse -> condcomp (SALAM_UI_*) -> expand -> check (target aware)
                                                             |
                               +-----------------------------+------------------------------+
                               |                                                            |
                    web generator (today)                                   native generator (phase 4)
                    HTML + CSS + JS                                Salam code that builds a gui.ui tree
                                                                                            |
                                         gui.ui:     style cascade, layout (block, inline, flex), paint, events, state
                                         gui.text:   fonts, fallback, shaping, bidi, line breaking, caret
                                         gui.paint:  RGBA canvas, anti-aliased paths, borders, gradients, shadows, clip
                                         gui.window: OS window, event loop, blit (X11, Win32, Cocoa, headless PNG)
```

Everything before code generation is shared: the parser, components,
`include`, `if`/`each`, constant folding, schema checks and the runtime
lowering of `page Name(params):` into functions. The native generator
reuses the runtime lowering pattern in `compiler/sal_layout/layout_lower.salam`:
it writes Salam source that builds a node tree and splices it into the
program.

### Why draw instead of using OS widgets

OS widgets (Win32 controls, Cocoa views, GTK widgets) each support a
different, small subset of styling. A design with rounded corners,
gradients, custom fonts and Persian text would look different on every OS
or not work at all. Drawing every pixel, as Slint and Flutter do, gives the
same result everywhere and lets the CSS properties a page already uses keep
their meaning.

### Default look

Pages rely on browser defaults: heading sizes, paragraph margins, list
indent, link colour, the look of buttons and inputs. The native default
theme reproduces those defaults, so a page that looks right in a browser
looks the same natively without changing any web output.

## Typed style values

`css.ProgramSource` writes a program whose values were decided by the compiler.
Loading it compares no strings and parses nothing.

```
p.AddTyped(14, 0, [css.DimId(1.5, 205)])                       margin-top: 1.5em
p.AddTyped(59, 2, [css.ColorValue(0x111111ff, 0xeeeeeeff, 1)])  color: light-dark(#111, #eee)
p.AddTyped(61, 0, [css.FnId(334), css.KwId(595), css.KwId(486), css.Comma(),
    css.ColorValue(0xff0000ff, 0xff0000ff, 0), ...])            background-image: linear-gradient(...)
```

- Property: the integer id from `css.PropertyId`. Longhands are 0 to 121,
  shorthands 200 to 244, logical properties 300 to 333 (`std/gui/css/props.salam`).
  Shorthands are already expanded, so a program only holds longhands.
- Flags: 1 is `!important`, 2 says the value depends on the colour scheme.
- Keywords, units and function names: indices into one name table,
  `std/gui/css/names.salam` (673 names, `css.KeywordId`/`css.KeywordName`). A
  loaded keyword points at the table's static string, so nothing is copied or
  freed. A name that is not in the table (a custom ident, a font family) stays
  text: `Kw`, `KwAs` (keeps the spelling of a font family), `Dim`, `Fn`.
- Lengths and angles: number and unit. Absolute lengths (in, cm, mm, q, pt,
  pc) are already pixels and angles already degrees; em, rem, viewport units and
  percentages stay as they are because they depend on the element and the
  window. `calc()`, `min()`, `max()` and `clamp()` that need nothing from the
  page are folded to one value; a negative folded length stays a `calc()` so the
  property can still clamp it.
- Colours: `ColorValue(light, dark, flags)`, straight RGBA `0xRRGGBBAA` for each
  scheme. Flag 1 marks a value that differs between the schemes
  (`light-dark()`, system colours), flag 2 a colour written in a syntax newer
  than hex, names, `rgb()`, `hsl()` and `hwb()`. `currentColor`, and
  expressions that use it such as `color-mix(in srgb, currentColor, red)`, stay
  as written and resolve in the cascade.
- `var()` and custom properties stay text (`AddVarDecl`), substituted per element.

The IDs are stable: new properties and names are only appended, so an
executable built by one compiler keeps its meaning with a newer `gui.css`.
`AddDecl` (property by name, colours as hex pieces) still loads older programs.
The same resolution runs for HTML-path style sheets when a declaration is
parsed, so both paths reach the cascade with resolved colours.

Measured on a 202-element page shaped like the one in `tools/bench/gui_ui.salam` (40
cards in a grid) with a style sheet that uses hex, `rgba()`, `hsl()`, named, system and
`light-dark()` colours, the minimum of 500 runs on one core of a loaded desktop
(load average 16 on 6 cores), C backend:

| step                                         | before |  typed |
| -------------------------------------------- | -----: | -----: |
| build the program (`__salam_styles()`)       |  30 us |   6 us |
| program plus sheet with the user agent rules | 130 us |  62 us |
| full style computation, 202 elements, cold   | 287 us | 281 us |

Style computation hardly changed with typed values: it was dominated by copying
large structures and by selector matching, not by reading values. Since then
tokens carry their keyword id, values are applied with integer switches on those
IDs, and the cascade passes the sheet, the element tree, the engine and the parent
style by reference instead of copying them on every call. On the same page
(minimum of 500 runs, one core, C backend) the full style computation went from
282 us to 113 us; the keyword IDs alone did not move it, passing by reference did.

### Colours

Every CSS Color 4 and 5 form: hex, 148 named colours, `transparent`,
`currentColor`, `rgb()`/`hsl()`/`hwb()` in both syntaxes, `lab()`, `lch()`,
`oklab()`, `oklch()`, `color()` in srgb, srgb-linear, display-p3, a98-rgb,
prophoto-rgb, rec2020, xyz, xyz-d50 and xyz-d65, `color-mix()` in any of those
spaces with shorter, longer, increasing and decreasing hue, relative colours
(`rgb(from red r g calc(b * 2) / alpha)`), `light-dark()` and the system
colours (Canvas, CanvasText, ButtonFace, ButtonText, ButtonBorder, Field,
FieldText, Highlight, HighlightText, LinkText, VisitedText, ActiveText,
GrayText, AccentColor, AccentColorText, Mark, MarkText, SelectedItem,
SelectedItemText) with Chromium's light and dark values. The conversions use
the CSS Color 4 matrices in f64. Every colour inside sRGB matches Chromium 156
to the 8-bit value (`tests/*/stdlib/css/colors`). Out of gamut colours are
mapped with the CSS Color 4 gamut mapping algorithm (OKLCH chroma reduction,
deltaEOK 0.02); Chromium clips them instead, so those values differ.

Gradients interpolate in the space named with `in <space> [<hue> hue]`;
without it they use sRGB when every stop is a legacy colour and Oklab
otherwise, as Chromium does. `gui.paint` interpolates premultiplied sRGB
between stops, so the cascade splits each segment into 8 stops computed in the
interpolation space. Stops are resolved when the gradient is painted
(`css.GradientStops`), when the length of the gradient line is known: `calc()`
positions, missing and out of order positions are fixed up as CSS Images says,
colour hints are replaced by nine stops placed as Blink places them, and each
segment in a space other than sRGB is split into 35 pieces mixed in that space
(denser near both ends, where the sRGB transfer curve bends most), with
intermediate colours clipped to sRGB as Chromium does. Against Chromium on a
300 pixel line the hint and `calc()` cases match within 2 levels and the Oklab and
OKLCH cases within 4, except `longer hue` around the gamut edge (21).

Colours carry a `legacy` flag (hex, names, `rgb()`, `hsl()`, `hwb()`).
Transitions and animations mix premultiplied sRGB when both colours are legacy and
Oklab otherwise (`css.MixColors`), which matches Chromium's interpolated values to
the 8-bit level except for two half-level roundings in 24 samples.

`border-image` (source, slice with `fill`, width, outset, repeat and the
shorthand, which `border` resets) is parsed, compiled to typed values and painted
by `gui.paint`'s `DrawNineSlice`. A gradient source is drawn at the size of the
border image area. Against Chromium the stretch, repeat and round cases match
within 3 levels away from antialiased stripe edges; `space` differs because
Chromium scales the gap between tiles by the tile scale. Backgrounds are clipped to
their `background-clip` box and positioned, sized and tiled from their
`background-origin` box, so the `padding-box, border-box` double background gives
a gradient border with round corners.

`backdrop-filter` follows Chromium (Skia's save layer with a backdrop): the
backdrop is the content of the backdrop root (the nearest layer: opacity, filter or
another backdrop filter) under the border box, read with mirrored edges, filtered,
clipped to the border box with its radius and drawn source-over with the element's
opacity; the element's content is drawn over it in its own layer, and is the
backdrop root of its descendants. Translucent, faded, filtered and nested cases
match Chromium within 1 level away from the rounded corners, whose antialiasing
differs.

### Dark mode

`Sheet.SetColorScheme(elements, dark)` re-evaluates the media conditions and
computes again only elements whose matched rules contain a scheme-dependent
declaration (`light-dark()`, a system colour, `color-scheme`, `var()`), elements
that a rule with a changed `prefers-color-scheme` condition matches, and the
descendants of elements whose style changed. A page with
`:root { color-scheme: light dark }` therefore restyles everything, since every
element inherits the used scheme; a page without it restyles only the elements
that name a system colour. `gui.ui` calls it on `ColorSchemeChanged` without
running the view again. The script steps `scheme dark`, `scheme light` and
`style <selector> <property>` test it headless.

## Lessons taken from Slint

Slint (`slint-ui/slint`) solves the same problem for its own language, so its
source was read before designing the runtime. What we adopt and what we do
differently:

- **Properties and trackers.** Every value that can change is a property
  cell with a lazy binding and a list of dependents. `set` only marks
  dependents dirty; `get` recomputes. A tracker is a binding without a value,
  used for the window redraw, one paint tracker per element and text shaping
  caches. Literal attributes never allocate a binding, which matters because
  most attributes in a page are literals. Binding loops are reported by the
  compiler and again at runtime with the property name.
- **Layout wiring.** Each container's child geometry is one lazily computed
  property (a flat array of x, y, width, height). Resizing invalidates only
  what depends on the size, with no separate layout pass. Width flows down
  and height is computed from it. Unlike Slint's own layouts, the algorithm
  is CSS: block flow, inline formatting for text, and the flexbox spec (Slint
  itself added a CSS flexbox layout because users asked for it).
- **Software painter.** Premultiplied colour and integer blending. Rounded
  rectangles and borders are drawn line by line with analytic coverage in
  fixed point, one square root per line and corner. Box shadows use an
  analytic separable Gaussian, and gradients use precomputed per-pixel steps.
  Pixels are stored as 32-bit BGRA so X11, Win32, Cocoa and Wayland can take
  the buffer without a copy. We go further than Slint where a CSS page needs
  it: per-side border widths, bilinear and area image scaling with a cache of
  scaled images, real group opacity, and transforms.
- **Partial rendering.** Each element keeps its last screen rectangle and
  paint tracker. A frame repaints a dirty region of at most three
  rectangles, using the buffer age the OS reports. Slint's changelog shows
  many partial-render bugs, so a test that a partial render equals a full
  render runs from the first day.
- **Text.** Shaped paragraphs are cached apart from line breaking, so a
  resize re-breaks lines without shaping again. Glyphs are alpha maps in a
  size-bounded cache keyed by font, size, glyph and a quarter-pixel x
  position. Persian needs real OpenType shaping (GSUB for joining forms and
  ligatures, GPOS for kerning and marks); the presentation forms used by
  `std/ai/image` today stay only as a fallback.
- **Styles.** Slint ships widget libraries per style. A page already styles
  itself with CSS properties, so we ship one default style sheet and expose
  theme tokens as reactive state that follows the OS colour scheme.
- **Platform layer.** One small interface per OS: create a window, report
  size and scale, request a redraw, present a buffer with damage rectangles,
  cursor, IME position, clipboard, colour scheme, timers. The event loop
  waits until the next timer or animation frame, runs deferred change
  handlers, advances the animation clock and paints once if the redraw
  tracker is dirty.

## Interactivity: reactive state

A handler only changes the program's state: mutable globals, and variables
declared inside a component or the page block. Every element that reads that
state updates. Natively this works like Svelte or Solid: the compiler finds
what each part of the page reads, marks every write, and the runtime re-runs
only the parts whose state changed.

```salam
mut total := 0

component Counter(name: str, start: int):
    count := start
    box:
        button: id = name click = (): count++ total++ end content = name + " " + count end
    end
end

page:
    title = "Counter"
    heading: content = "Total: " + total end
    Counter: name = "a" start = 0 end
    Counter: name = "b" start = 10 end
end
```

### Component and page variables

A line `name := value` at the top of a component body or of the page block
declares a variable. There is no keyword: the compiler classifies every such
variable while it expands the page (`compiler/sal_layout/layout_infer.salam`,
shared by the native and the web targets), from two facts.

- **Written:** an assignment, `++`/`--`, a method call other than the read
  only ones, passing it to a `&:` parameter of a program function, or passing
  it to a child component whose parameter is written, anywhere in the body:
  in handlers, in callbacks passed to children, in values. Passing it to a
  function of another package, or calling a method on it, counts too unless
  its value is plainly a number, string or bool, which nothing can change
  through a call. Lambda parameters, locals and loop variables of the same
  name hide it.
- **Read by the view:** an attribute value or text, an `if` condition, an
  `each` collection, a key, a value passed to a child component (a lambda
  passed to a child counts when the child calls it in its view), and anything
  a derived value it feeds is read by.

| Written | Read by the view | Kind | What the compiler generates |
|---|---|---|---|
| yes | yes | state | per instance storage, write marks, the instance depends on it |
| yes | no | instance variable | per instance storage, no marks, never re-runs anything |
| no | either | derived value | its expression, worked out each time the instance runs |

An instance variable starts from its value once, when the instance is made,
and keeps it across re-runs; a derived value follows the parameters and the
state it is made from. A variable neither read nor written gets a warning.
`mut name := value` is accepted too and means the same. A page block variable
that is written becomes a mutable global, so it gets the same reactivity as
one, and a derived one is substituted where it is used. A value passed to a
child component makes the child depend on it, so a child re-runs (natively,
with its parent; on the web, on its own) when only its parameters change.
When the analysis cannot tell, it picks state: a wrong guess costs one extra
re-run, never a stale screen.

`SALAM_NATIVE_DUMP=1` shows the result: only state variables get
`ui.TouchInstance` marks.

### Scopes

The compiler splits the generated view into scopes. Each scope is a closure
that builds a run of sibling elements, `b.Scope(site, (b: ui.Builder): ... end)`,
so the runtime can call it again on its own later.

- The page itself is site 0.
- Every component instance is one scope. Components are still expanded
  inline, but the expanded body is wrapped in its own closure, and the whole
  instance re-runs when anything it reads changes, as a React component does.
  Inside a component there are no smaller scopes.
- Outside components, an element whose own attributes or text read state is a
  scope, an `if` whose condition reads state is a scope, and an `each` whose
  collection reads state is a scope (the list scope). An element that reads
  only loop variables is not a scope.

Each site gets a dependency set at compile time: the mutable globals read by
its own attributes, text, conditions and collections, not by nested scopes.
Calls to functions of the same program count as reads of whatever those
functions read. Component parameters are substituted into the body, so a
parameter bound to `total` makes the instance depend on `total`.

### Change marks

Every state global that some scope reads gets a small ID and a bit in a
generated `i64` word (`__salam_touched_0`, 31 IDs per word). The compiler
inserts `__salam_touched_0 |= 4` before every statement anywhere in the
program that may write it: assignments, compound assignments, `++` and `--`,
method calls other than the read only ones (`len`, `get`, `has`, ...), passing
it to a `&:` parameter of a program function, and passing it to a function of
another package (which cannot be checked, so it is assumed to write). A write
to a component's state variable becomes `ui.TouchInstance(component, instance)`
instead, so only that instance re-runs; a write to an instance variable gets no
mark at all. A mark costs one OR into a global;
handlers that change nothing run no extra code.

The page view starts with `b.Reactive(words, take, sites)`: `take(w)` reads and
clears word `w`, and `sites()` registers each site's dependency mask with
`ui.ScopeSite` once. `SALAM_NATIVE_DUMP=1` shows all of it.

### Runtime

After every handler, timer or animation step the runtime reads the words.
When nothing was touched it does nothing at all: no view run, no matching, no
restyle, no layout and no paint. Otherwise it re-runs the scopes whose masks
meet the touched bits, outermost first, and skips scopes inside one that is
already re-running.

A re-run builds the scope's elements again. When the new run of elements has
the same shape (tags and child counts) as the old one, the runtime copies the
changed text and attributes into the existing nodes and drops the new ones, so
nothing else is rebuilt: changed elements are restyled one by one, and changed
text is reshaped in place. If the paragraph keeps its size (or its block
cannot change size) no layout runs and only the damage is repainted. When the
shape changed (an `if` flipped, a row was added) the new elements are spliced
into the tree, matched with the old ones by tag, `id` and `key` so focus and
typed text survive, and the element, style and layout passes run once.

Component state and instance variables live in generated vectors, one per
declaration, indexed by an instance slot. An instance keeps its slot when its parent
re-runs: it is matched by site and by its position among siblings of the same
site. A new instance initialises its state from the declaration.

Inside `each`, an instance follows its row's key instead of its position.
The key is the `key` attribute (Persian `کلید`, written as `data-key` on the
web) of the row's first element, or its `id` when it has no key; when the row
starts with a component, the component's first element counts. The scope is
then opened with `b.KeyedScope(site, key, ...)` and matched by site and key,
so inserting, removing or reordering rows moves each instance's state with its
row:

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

A key that reads the component's own variables is ignored, because the key is
needed before they exist.

### On the web

`salam page build` gives the same variables to the page block. Every
component instance that keeps a written variable gets one global per such
declaration
(`__salam_ws_N`), a render function `__salam_view_I()` built from the
instance's elements with the runtime view lowering (dynamic values, `if` and
`each` become Salam code that returns HTML, compiled to JavaScript with the
event handlers), and an init function. In the HTML the instance is a pair of
comments, `<!--salam:I--><!--/salam:I-->`, which the page script fills when it
loads. Every write to a state variable gets a `salamWsTouch(I)` call before it,
as the native target gets `TouchInstance`, and so does every nested instance
that reads it (a value passed to a child), so a child re-renders when only its
parameters change. The write analysis skips names hidden by a lambda
parameter, a local or a loop variable. The page script then re-renders the
touched instances once, after the handler returns, and patches the DOM
between the two comments in place: elements are matched by tag and by
`data-key` or `id`, attributes and text are updated where they changed, an
input keeps what was typed unless the page changes its `value`, and the
focused element and its selection are kept. An instance nested in another one
keeps its DOM and state when the outer one re-renders; when it appears again
(an `if` turned true) its state starts from the declaration, as natively.
Styles of the instance's elements are compiled once into the page's style
sheet with an `sI-` class prefix.

A component with written variables inside `each` has no comments of its own:
its elements are drawn inline by the instance (or page part, below) that
holds the `each`, and each such variable is one Vector (`__salam_ws_N`) indexed by a slot.
At the instance's place the render function calls `salamWsSlot(I, key)`, and
the page script returns the slot kept for that key, or a new one, in which
case the state is set from its declarations. The key is the row's key, by
the same rule as natively (the `key` of the row's first element, or its
`id`, or else the row's index), joined with the slots of the instances
around it. A slot not asked for during a render is freed, so a row that goes
away and comes back starts from its declarations. Reads and writes go
through `salamWsCur(I)`, which a handler attribute in the row sets first
(`onclick="salamWsAt(I,slot);__sl_ev_7()"`), and a write re-renders the
instance that holds the `each`, which patches the rows by key.

The page block may read mutable globals. Every element, `if` or `each` of
the page block whose own attributes, condition or collection read one (also
through a function of the program) becomes a page part: a pair of comments
and a render function, like an instance without state. The compiler records
which globals every instance and page part reads, and before every statement
of the program that may write one of them (the write analysis of the native
target) it adds `salamWsTouch` for each of them. Static IDs inside instances
and page parts still count for `dom.ById` checks (E138).

`ui.ScopeRuns()` counts scope re-runs (a full re-run counts one), for tests.

### Components talking to each other

A child talks to the components around it in two ways, and both work the
same natively and on the web.

**Callbacks.** A parent passes a function to a child as a parameter, and the
child calls it from a handler:

```salam
component Counter(name: str, change):
    count := 0
    button: id = name click = (): count++ change(count) end content = name + " " + count end
end

component Pair(report: func (int)):
    sum := 0
    box:
        paragraph: content = "sum " + sum end
        Counter:
            name = "a"
            change = (v: int):
                sum += v
                report(sum)
            end
        end
    end
end

page:
    Pair: report = (v: int): total = v end end
end
```

The parameter may be untyped (`change`) or typed (`report: func (int)`), and
the value may be a lambda or the name of a function of the program. Since
components are expanded by the compiler, the call `change(count)` becomes a
call of the lambda the parent wrote, with the parent's names: `sum` is the
`Pair` instance's state, `report` is the page's lambda. A write there marks
the instance that owns the state (`ui.TouchInstance` natively, `salamWsTouch`
on the web) like any other write, so only that instance and the scopes that
read the changed globals run again. A child that calls a callback its parent
did not pass does nothing (`Counter: name = "solo" end`).

**Events that bubble.** A child sends a named event with an optional value,
`emit("saved", value)` (Persian `برانگیز("ذخیره"، مقدار)`). Where a component
is used, `on saved = (v: T): ... end` (Persian `هنگام ذخیره = ...`) handles
it. The event goes to the nearest use site that has a handler for the name,
then on to every level above it up to the page, where an `on saved` line at
the top of the page block is the last handler. A handler that calls `stop()`
(Persian `بایست()`) ends the trip. Handlers take the value as their one typed
parameter, or no parameter when they do not need it, and they change the
state of the component they are written in:

```salam
component Button(name: str):
    button: id = name click = (): emit("pressed", name) end content = name end
end

component Toolbar:
    picked := "none"
    box:
        Button: name = "save" on pressed = (n: str): picked = n end end
        Button:
            name = "quit"
            on pressed = (n: str):
                picked = n
                stop()
            end
        end
    end
end

component Panel:
    count := 0
    Toolbar: on pressed = (): count++ end end
end

page:
    on pressed = (n: str): println "page got " + n end
    Panel: end
end
```

Pressing `save` sets the toolbar's `picked`, adds one to the panel's `count`
and reaches the page; pressing `quit` stops at the toolbar. The route is known
at compile time, because every instance is expanded inside its parent: the
compiler replaces `emit` with the value kept in a temporary and one call per
handler on the way up, nearest first, and a handler that calls `stop()`
becomes a lambda returning whether to go on, which guards the rest. There is
no event object and nothing to look up while the page runs, and the writes
in the handlers are marked like any other, so an event from a deep child
re-runs only the instances whose state changed. In `events_bubble`, where the
page also counts the presses in a paragraph, `save` re-runs two scopes (the
panel instance, with the toolbar and button inside it, and that paragraph)
and `quit` re-runs one, the toolbar; an unrelated sibling never runs. On the web the page script
re-renders the instances whose state changed, here the panel, the toolbar,
the button and the page part.

Inside `each`, an event from a row goes to the instance that holds that row:
two `Group` instances in an `each`, each with rows in an `each` of their own,
each get only their own rows' events, because the handler is written with the
group's state, which is reached through its own key (natively the captured
instance slot, on the web `salamWsCur` of the group, set by the row's handler
attribute). The tests are `tests/{en,fa}/ui/events_*.salam` and
`tests/{en,fa}/page/components/events_*.salam`.

A handler written at a use site sends its own `emit` from the level where it
is written, so a parent can turn a child's event into one of its own:
`on pressed = (n: str): emit("changed", n) end`. `emit` only works in the page
block and in components; a function of the program that calls it is an error
(E012), because there is no instance to start from.

### The escape hatch and the limits

- Writes the compiler cannot see do not update the page: state changed inside
  another package, through a pointer, or through a second name for the same
  Vector. Call `ui.Invalidate()` after such a change; it re-runs the whole
  view. `ui.Refresh()` does the same.
- Instances inside `each` without a `key` or `id` on the row are matched by
  position: removing the first row moves every later row's component state
  up by one. Give the row a key.
- A component variable that is written needs a type the compiler can see
  without type checking: a literal, a typed component parameter or
  `value as Type`.
- Variables are declared only at the top of a component body or of the page
  block, not inside an element, `if` or `each`.
- On the web, component variables work in the page block built by
  `salam page build`,
  not in runtime pages with parameters. Instances and page parts that read
  mutable globals are drawn by the page script, so they show nothing without
  JavaScript. A component inside `each` takes its parameters from the row
  every time the instance holding the `each` renders; giving a parameter a
  new value inside the component does not change the caller's value.
- Selecting a row in a list re-runs one scope per row that reads the selection.
- A handler or callback body written on one line cannot put a call right
  after a value (`picked = n stop()` reads `n stop` as one name); put the
  statements on separate lines. Lambda parameters need a type, as everywhere
  in Salam. A `ret` in a callback or handler ends that callback, not the
  handler that sent the event.
- On the web, a handler inside `each` cannot read the row's loop variable,
  or a component parameter bound to it, because handlers are functions of
  the page script; declare a variable in the row's component
  (`label := name`) and read that: a variable a handler reads gets storage
  that is set again every time the instance renders.
- The fallback for a hand written view without `b.Reactive` is the old one:
  the whole view runs after every handler.

Numbers for the 200 element page in `tools/bench/gui_ui.salam` are in the
`gui.ui` site page.

## Phases

Each phase is one pull request.

0. **Target gating.** `native` schema field, E165 and W166, `--ui=native`
   for `salam check`, `SALAM_UI_WEB` and `SALAM_UI_NATIVE`, this document.
1. **Paint and text.** `std/gui/paint` (canvas, paths, anti-aliased fill and
   stroke, rounded rects, borders, gradients, shadows, clipping, images) and
   `std/gui/text` (the TrueType, shaping and bidi code moved out of
   `std/ai/image`, plus font fallback, line breaking and caret hit testing).
   Tests compare PNG output against checked-in images.
2. **Windows.** `std/gui/window`: X11, Win32 and Cocoa windows that show a
   pixel buffer, an event loop with pointer, keyboard, text input, resize,
   DPI, clipboard and timers, and a headless backend for tests.
3. **UI runtime.** `std/gui/ui`: node tree, computed styles, the cascade
   (default theme, element styles, `hover`/`focus`/`active`, `media`),
   block, inline, flex, grid and table layout, absolute positioning, scrolling, painting,
   hit testing, event dispatch, focus order.
4. **Native generator.** `compiler/sal_layout/layout_native.salam`,
   `salam page run --ui=native`, `salam page shot` (render to PNG),
   `ui.Run` for programs, `engine = "native"` in `salam bundle`.
5. **Reactive state** on both targets, with two-way binding for inputs.
6. **Widgets and input.** Every input type, text area, select with a popup,
   forms, details, dialog, progress, meter, tables, lists, text editing with
   selection, IME and clipboard.
7. **Responsive design, motion and themes.** `media` on native, CSS custom
   properties, transitions and animations, HiDPI, custom fonts.
8. **Tooling.** Live preview with reload, LSP target diagnostics, site
   learn pages in English and Persian, CI on Linux, Windows and macOS.
9. **Later.** Accessibility from `aria` and roles, a GPU renderer, Wayland,
   subgrid, an SVG subset, Android and iOS.

## Diagnostics

| code | kind    | message                                                                                            |
| ---- | ------- | -------------------------------------------------------------------------------------------------- |
| E165 | error   | `'script' works only on the web; wrap it in 'if SALAM_UI_WEB:' or remove it for the native target` |
| W166 | warning | `'meta' is web page metadata and is ignored by the native target`                                  |
