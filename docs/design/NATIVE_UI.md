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
interpolation space (not done when the gradient has interpolation hints or
`calc()` stop positions). Colour transitions mix premultiplied sRGB
(`css.MixColors`).

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

Today a handler changes the page through `std/dom`
(`dom.SetText(dom.ById("count"), ...)`), which exists only in a browser.
Phase 5 adds state that both targets understand:

```salam
component Counter(start: int):
    state count := start
    box:
        heading: content = "Count: " + count end
        button: content = "+1" click = (): count++ end end
    end
end

page:
    title = "Counter"
    Counter: start = 5 end
end
```

A handler only changes state, and every attribute that reads that state
updates. Natively the component's subtree is rebuilt and reconciled (focus,
scroll position and caret are kept). On the web the generated JavaScript
re-runs the component's view and patches the DOM. `std/dom` stays as a
web-only escape hatch.

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
   block, inline and flex layout, absolute positioning, scrolling, painting,
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
   CSS grid, an SVG subset, Android and iOS.

## Diagnostics

| code | kind    | message                                                                                            |
| ---- | ------- | -------------------------------------------------------------------------------------------------- |
| E165 | error   | `'script' works only on the web; wrap it in 'if SALAM_UI_WEB:' or remove it for the native target` |
| W166 | warning | `'meta' is web page metadata and is ignored by the native target`                                  |
