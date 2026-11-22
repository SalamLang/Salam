# Animation, navigation, forms, testing and touch

Status: proposed design (2026-11-16), extends `UI_DSL.md`. Uses `when`
conditions and tokens from `UI_STATES_THEMES.md` and the kit from
`UI_COMPONENTS.md`. Codes continue from there: E200 to E229 and W200 to W229
are allocated here (renumber in the landing PR if one is taken).

## 6. Animation

### What React and QML do

React has no animation: CSS transitions, or libraries (Framer Motion,
react-spring) with their own tree of values; exit animations need a wrapper
that delays unmounting. QML has `Behavior`, `NumberAnimation`, `Transition`,
`SpringAnimation`, `ParallelAnimation`: typed and good, but imperative objects
that must be started and stopped. We keep `transition` (state to state) and
add declarative keyframes, enter and exit, and shared elements, all
functions of state, so there is nothing to start or stop.

### Keyframes

```salam
animation Pulse:
    duration = 1200ms
    easing = ease in out
    repeat = forever
    at 0%: scale = 1 end
    at 50%: scale = 1.1, opacity = 0.7 end
    at 100%: scale = 1 end
end

page:
    box: id = "dot"
        play = none
        when busy: play = Pulse end
    end
end
```

- `animation Name:` is a top level declaration; settings `duration`, `delay`,
  `easing`, `repeat` (a number or `forever`), `alternate` (`true` goes back and
  forth), `fill` (`none`, `end`: keep the last frame), `at P%:` frames of
  style attributes (`from` and `to` are `at 0%` and `at 100%`).
- `play = Name` (also `play = Name, 2s` to override the duration, or a list to
  play several) is a normal attribute: its value is part of the element's
  style, so `when`, state prefixes and expressions work. The animation runs
  while the value is set and stops when it becomes `none`; changing to the
  same value does not restart it. Not an imperative handle: there is no
  `start()` or `stop()`.
- Only animatable properties (`style.Animatable`) may appear in frames.
  Non animatable ones are E200. A frame property that changes layout
  (`width`, `padding`) is allowed with warning W201, because it costs a layout
  per frame; `translate`, `rotate`, `scale`, `opacity`, `blur` and colours
  are cheap (paint only) on both targets.
- `animated = (): ... end` runs when one cycle ends (not for `forever`).

### Enter, exit, shared elements and springs

```salam
list:
    each it in items:
        item: key = it.id enter = Fade, 200ms exit = Fade, 150ms content = it.name end
    end
end
```

- `enter = Name` and `exit = Name` take an animation (built ins: `Fade`,
  `Slide up`, `Slide down`, `Slide start`, `Slide end`, `Scale`, `Grow`) and
  apply when an element enters the tree through `if` or `each`, or leaves it.
  An exiting element stays in layout and hit testing off until its animation
  ends. Web keeps the node until `animationend`; native keeps the node as a
  ghost in the scope. A new element with the same `key` while the old one
  exits reuses it (R8).
- `shared = "photo 4"`: when an element with this name disappears in the same
  tick as another with the same name appears, the new one animates from the
  old one's rectangle and style to its own (the old one is not drawn).
  Native reads the rectangles from the last layout; web uses the View
  Transitions API when present and measures with a script otherwise. The
  motion is the new element's `transition` (default `ease out`, 250ms).
- Easing `spring(stiffness, damping)` already exists; the standard adds an
  optional third value `mass`. A spring keeps its velocity when its target
  changes mid flight (on both targets the web output computes the spring with
  the same equations, as a `linear()` easing sampled at build time for
  transitions, and as keyframes for animations).
- Staggering is an expression: `delay = i * 40ms` inside `each i, it in items`.
- `reduced motion` (see `UI_COMPONENTS.md`, accessibility) turns durations to
  zero for everything except an animation with `essential = true`.

### Targets and scheduling

Web: `@keyframes` and `animation` in the page sheet, `transition` as today;
enter and exit by the small patch step. Native: `gui.ui` already has a
transition driver (`std/gui/ui/anim.salam`); keyframes become a track of
`style.Lerp` values per element. The frame loop runs only while at least one
animation is live, repaints only the damaged rectangle, and re-lays out only
for layout properties. An animation never writes state, so it re-runs no
scopes (R1): `SALAM_UI_TRACE` shows `anim Pulse dot` lines instead.

### Diagnostics

| code | kind | message |
| --- | --- | --- |
| E200 | error | `property 'flex' is not animatable (in animation 'Pulse')` |
| W201 | warning | `animating 'width' lays out the page every frame; prefer scale or translate` |
| E202 | error | `no animation named 'Pulze'` |
| E203 | error | `frame at 120% is outside 0% to 100%` |
| E204 | error | `'shared' name used twice in the same scope` (static names only) |

### Decisions taken

| # | decision |
| --- | --- |
| N1 | `animation Name:` keyframes, played by the `play` attribute, a function of state |
| N2 | no imperative start or stop; conditions turn animations on and off |
| N3 | `enter` and `exit` on elements under `if` and `each`; exit delays removal |
| N4 | `shared = name` for shared element motion (FLIP on native, View Transitions on web) |
| N5 | `spring` gets `mass`; velocity is kept when the target changes |
| N6 | paint only properties are the cheap path, layout ones warn |
| N7 | reduced motion is automatic; `essential = true` opts one out |

### First implementation steps

1. Parse `animation` declarations and `play`; web `@keyframes`; E200 to E203.
2. Native keyframe track in `std/gui/ui/anim.salam`; test with script steps
   `advance 100ms` and `dump` on both targets.
3. `enter` and `exit` with delayed removal in the patch step (web and
   native), then `shared`.

## 7. Navigation, windows, dialogs and menus

### What React and QML do

React has no router: React Router or Next supply route tables, links and
history, only on the web. QML has `StackView` and `Window`/`Dialog` types for
native, and nothing for URLs. Our pages must be one route table that is a URL
tree on the web and a screen stack in a native window.

### Routes

The `routes:` block is gone; the `pages/` folder next to `main.salam` is the
route table, see `ROUTING.md`. Server rendering and server functions are in
`SERVER.md`.

```
main.salam
pages/
  index.salam          /
  user/[id].salam      /user/:id
  settings.salam       /settings
  _404.salam           any other path
```

```salam
pub component User(id: int):
    title = "User " + id
    heading: content = "User " + id end
    Link: to = "/" content = "Back" end
end
```

```salam
page:
    title = "Shop"
    nav:
        Link: to = "/" content = "Home" end
        Link: to = "/settings" content = "Settings" end
    end
    Outlet: end
end
```

- One file per route holds one `pub component`. `[id]` segments become the
  component's parameters, converted to the parameter's type (`id: int`); a
  value that does not convert goes to `_404.salam`. `Outlet` shows the matched
  component; a `_layout.salam` with its own `Outlet` nests routes below a folder.
- `Link: to = "/path"` is the only navigation element. `page.GoTo("/user/3")`,
  `page.Back()`, `page.Forward()`, `page.Redirect("/login")` are functions, and
  the state `page.route.path`, `page.route.query` and `page.route.Param("id")` can be
  read like any state (R5), so a highlighted menu item is
  `when page.route.path == "/"`.
- A route may carry a guard: `guard = logged_in` and `redirect = "/login"` at
  the top of the component body. The guard is a bool expression, re-checked on
  every change of its state.
- A route change is a state write: it re-runs the `Outlet` scope and nothing
  else (R1). A `page:` with `enter`/`exit` on the outlet's component animates
  the change (section 6). Scroll and focus are restored on back.

### Targets

- **web**: the History API for `go`, `Link` is `<a href>` (middle click,
  open in new tab and crawlers work). `salam page build` writes one HTML file
  per route that has no parameters (static prerender, with the matching title
  and meta) and a fallback `index.html` for the rest; route changes after the
  first load do not reload.
- **native**: the window keeps a stack of routes; `go` pushes, `go_back` pops,
  Alt+Left, the mouse back button and the OS back gesture call `go_back`;
  `replace` swaps the top. The stack is saved with the window state so a
  restart returns to the same route. `--route=/user/3` on the command line
  (and `--route` in a custom URL scheme once registered) opens a deep link.

### Windows, dialogs and menus (native, with a web fallback)

```salam
window Inspector(node: str):
    title = "Inspector"
    width = 420
    height = 600
    min width = 300
    resizable = true
    box: heading: content = node end end
end

page:
    menubar:
        menu "File":
            item "Open" shortcut = "ctrl o" click = (): open_file() end end
            item "Quit" shortcut = "ctrl q" click = (): quit() end end
        end
    end
    button: content = "Inspect" click = (): open_window(Inspector, "root") end end
    box: context menu = Actions end
end
```

- `window Name(params):` is a second top level window with its own scopes; it
  shares globals and page variables with the page (R5), so one write updates
  both. `open_window(Name, args)` returns a handle that can `close()`,
  `focus()` and `set_title(s)`; the window attributes are `title`, `width`,
  `height`, `min width`, `max width`, `min height`, `max height`, `resizable`,
  `frameless`, `always on top`, `modal` (blocks its parent), `icon`, and a
  handler `close = (): ... end` that may keep it open by returning false (`ask
  to save`). The page itself takes the same attributes.
- On the web a `window` opens as a browser popup when it has `external =
  true`, else as a `Dialog` overlay in the page (`modal` honoured), so a
  program never needs `if SALAM_UI_WEB`.
- **Dialogs**: `Dialog` (kit) for in page content, with a focus trap and
  `Escape`. `confirm("Delete?")`, `alert(text)`, `pick_file(filters)`,
  `save_file(name)`, `pick_folder()` are functions in `gui.dialog` that use
  the OS dialog on native and the browser primitive on web (`confirm`,
  `<input type=file>`, the File System Access API when present); they return
  through `load` style values (R9) so they do not block the event loop.
- **Menus**: `menubar:` at the top of a window is the native menu bar on macOS
  (global bar), drawn inside the window on Windows and Linux (the same widget
  as the web, with `Alt` mnemonics `"&File"` and arrow key navigation), a
  `nav` with role `menu bar` on the web. `context menu = Name` attaches a
  `Menu` component to right click, long press and the menu key. `shortcut`
  items register the key for the window on every target and show it right
  aligned. Tray icons and notifications are `std/gui/notify`, native only
  (calls are no ops on web and the compiler says so with W205).

### Diagnostics

| code | kind | message |
| --- | --- | --- |
| E206 | error | `route '/user/:id' parameter 'id' is not a parameter of component 'User'` |
| E207 | error | `two routes match the same path: '/a' and '/a'` |
| E208 | error | `route 'Home' is not a component` |
| E209 | error | `'Outlet' used with no pages folder` |
| W205 | warning | `'notify' has no effect on the web target` |
| E210 | error | `no window named 'Inspectr'` |

### Decisions taken

| # | decision |
| --- | --- |
| NV1 | one route table (the `pages/` folder), path parameters typed by the component parameters |
| NV2 | `Link`, `go`, `go_back`, `replace`, readable `route` state; no other navigation API |
| NV3 | web is real URLs with static prerender; native is a stack saved with window state |
| NV4 | `window Name(params):` for multi window on native; popup or overlay on web |
| NV5 | `menubar:`, `menu`, `context menu`, `shortcut`: native bar on macOS, drawn elsewhere |
| NV6 | OS dialogs as non blocking functions in `gui.dialog` |

### First implementation steps

1. the route table, `Outlet`, `Link`, `go` on the web (history, prerender); E206 to E209.
2. Same table in `gui.ui` (route stack in the scope tree, Alt+Left).
3. `window` declaration with `open_window` on native (needs `gui.window`
   multi window; Linux and Windows first), then `menubar` and `context menu`.
4. `gui.dialog` functions.

## 8. Forms, validation and data binding

### What React and QML do

React has controlled inputs (`value` plus `onChange`, by hand for every
field) and libraries for validation (React Hook Form, Formik, Zod). QML
binds property to property but has no validation beyond `validator` on
`TextField`. We have R4, which already makes the callback form correct; this
section is the short way to write it and the validation that goes with it.

### `bind`

`bind = name` on `input`, `textarea`, `select`, `checkbox`, `radio`, `slider`
and kit widgets is the open decision 1 of `UI_DSL.md`, taken: it expands at
compile time to the value attribute (`value`, `checked`, `selected`) plus a
handler that writes `name`, exactly the R4 shape, so scopes re-run the same
way. `name` may be a state variable, a field of a struct state (`form.email`)
or an element of a vector (`rows[i].qty`); anything else is E211. For a
number or other non text target, the input converts; text that does not
convert keeps what the user typed, sets `invalid`, and does not write.

Implemented so far: `bind` on an `input` with no type or a text type (`text`,
`password`, `email`, `search`, `url`, `tel`), which needs a `str` variable, and on
`type = "checkbox"`, which needs a `bool`. Number, select, textarea, radio and
slider targets, and the conversion above, are not done; they report an error.
On the web, a focused field is rewritten when the bound value moved away from
what the field shows (a clear after a button), and left alone while the user
types.

```salam
form := Signup()

page:
    form: id = "signup" submit = (): save(form) end
        input: name = "email" bind = form.email rules = required, email end
        input: name = "age" type = number bind = form.age rules = range(13, 120) end
        input: name = "pass" type = password bind = form.pass rules = min length(8) end
        input: name = "pass2" type = password bind = form.pass2
            rules = same as(form.pass) message = "Passwords differ"
        end
        button: type = submit content = "Create" disabled = !signup.valid end
        paragraph: content = signup.email.error visible = signup.email.touched end
    end
end
```

### Rules and state

- `rules = a, b, c` is a comma list of rule calls: `required`, `min length(n)`,
  `max length(n)`, `pattern("...")` (regex of `std/regex`), `email`, `url`,
  `range(a, b)`, `same as(x)`, `one of(list)`, and a lambda
  `(v: str): str` that returns the error text or `""` (also `load` based
  for server checks, R9: the field is `pending` meanwhile). Rules run in order;
  the first failure is the field's `error`. Messages come from the kit's
  string table (English and Persian, selected by `lang`); `message = "..."`
  overrides.
- A `form` element with an `id` exposes derived state (R3) per named field:
  `signup.email.value`, `.error` (`""` when fine), `.valid`, `.touched`
  (left once or submit tried), `.dirty` (differs from start), `.pending`;
  and for the form `valid`, `dirty`, `submitting`, `errors` (a list). The
  names are checked at compile time against the `name` attributes (E212).
  Reading them re-runs only the scopes that read them (R1).
- `submit` runs on Enter in a field, on a `type = submit` button, and never when
  `valid` is false: it instead marks all fields `touched`, focuses the first
  invalid one and announces the error count (`live = assertive`). The `Field`
  widget shows the error when `touched`, so a page does not repeat that logic.
- `invalid` (a style state prefix and `when`) is true when the field has an
  error and is touched: `invalid border = 1, solid, theme.color.danger`.
- Keyboard and mobile: `input` takes `keyboard = text, number, email, phone,
  url, search, password`, `enter key = go, next, search, send, done`,
  `autofill = "email"` (the standard tokens: `name`, `email`, `current
  password`, `new password`, `one time code`, `street address`), and
  `autofocus`. They map to `type`, `inputmode`, `enterkeyhint` and
  `autocomplete` on the web and to the soft keyboard and autofill service on
  mobile; on desktop they only set the validation of the typed characters.
- **Stored state**: `mode := stored("theme", "auto")` is state whose value is
  kept between runs (web `localStorage`, native a file in the user's config
  directory), so a theme choice or a form draft survives a restart.

### Targets

Web: `<form novalidate>` (our rules decide, the browser does not show its own
bubbles), `name` and `autocomplete` attributes, `aria-invalid` and
`aria-describedby` to the error text. Native: the same rules in `gui.ui`
(text editing, caret and selection keep their state through the rules, R4
guarantees that typing re-runs only readers), IME composition does not
validate until committed.

### Diagnostics

| code | kind | message |
| --- | --- | --- |
| E211 | error | `'bind' needs a variable, a field or a vector element, found <expression>` |
| E212 | error | `form 'signup' has no field named 'emial'` |
| E213 | error | `rule 'range' needs numbers; field 'name' is text` |
| E214 | error | `two fields named 'email' in form 'signup'` |
| W215 | warning | `input without a 'name' in a form; its state cannot be read` |

### Decisions taken

| # | decision |
| --- | --- |
| F1 | `bind = target` is accepted: sugar for value plus handler (R4), not a second reactive path |
| F2 | rules are a comma list on the field; first failure is the error; lambdas and `load` are allowed |
| F3 | `form` exposes derived per field state (`error`, `valid`, `touched`, `dirty`, `pending`) |
| F4 | submit never runs when invalid; it focuses the first error |
| F5 | `invalid` is a style state and condition |
| F6 | `stored(key, default)` for persistence |
| F7 | `keyboard`, `enter key`, `autofill` for mobile and web, ignored on desktop |

### First implementation steps

1. `bind` in the page expansion (both generators), E211; tests in
   `tests/*/page` and `tests/*/ui`.
2. `rules` library in `std/gui/kit` (pure functions, plus Persian
   messages) and the derived form state; E212 to E215.
3. `Field` shows errors; `stored`.

## 9. Testing API and hot reload

### What React and QML do

React: Jest or Vitest with Testing Library (`getByRole`, `userEvent`) in a
fake DOM, plus Playwright for a browser. QML: `QtTest` with `TestCase`,
`mouseClick`. We have `SALAM_UI_SCRIPT` steps. The goal is one test source,
run on both targets, that finds elements as a user does (by role and name).

### Test API

A package `gui.test` (Persian name `آزمون رابط`) on top of `std/testing`:

```salam
import gui.test

func TestCounter(t: test.T):
    app := test.Mount(Counter, "name", "a")
    app.Role("button", "a").Click()
    test.Equal(app.Role("button", "a").Text(), "a 1")
    test.Equal(app.Runs(), 1)
    app.Resize(500, 800)
    test.Equal(app.Role("button", "a").Style("padding"), "8")
    app.Scheme(true)
    app.Key("Tab")
    test.Equal(app.Focused().Name(), "a")
    test.Snapshot(app, "counter")
end
```

- Queries by what users see: `Role(role, name)`, `Label(text)`, `Text(text)`,
  `Id(id)`; the tree they search is the accessibility tree of section 4, so a
  test that cannot find a control by role shows a real accessibility gap. No
  CSS selectors exist. A query matches exactly one element (E216 at run time
  when none or more than one; `All(...)` returns a list).
- Actions: `Click`, `DoubleClick`, `Type(text)`, `Key("ctrl a")`, `Hover`,
  `Focus`, `Drag(x, y)`, `Scroll(dy)`, `Press`/`Move`/`Release` for touch;
  `Resize`, `Scheme(dark)`, `Lang`, `Advance(ms)` (time is virtual: timers and
  animations run only when advanced), `Reduced(true)`.
- Reads: `Text`, `Value`, `Style(name)` (a canonical value text), `Rect`,
  `Visible`, `Focused`, `Runs` (scope run counts, the R11 counter), `Tree()`
  as text (`a11y dump`).
- `Snapshot(app, name)` compares the node tree dump (and, native only, a
  rendered image with a tolerance) with a stored file; the same snapshot file
  must match on both targets for the tree (that is the same look check).
- Runs on native headless (`SALAM_WINDOW_BACKEND=headless`) in process, and
  on the web through the JS test host used by the page tests (a headless
  browser is only needed for rendering snapshots). The existing steps
  (`click #id`, `key`, `dump`) stay as the line oriented form of the same
  driver.

### Hot reload and live preview

- `salam page dev app.salam` (web): a local server, rebuild on save in
  milliseconds (only the changed file's pages), reload by server sent
  event. A change that only touches style values or tokens patches the sheet
  without a reload; a change in code reloads the script and restores state
  from a snapshot: instance variables keyed by site and name, scroll and focus
  positions, the route, the typed text. A variable whose type changed resets
  and the console says which.
- `salam run app.salam --ui=native --watch`: uses the interpreter backend
  (`--backend=interp`, no C compile) so that a save re-evaluates the page view
  and handlers in the same process; instance variables, focus, scroll and
  the route stay (same key as the web). A change to a struct's layout or a
  global's type restarts the process and restores the snapshot file written
  just before. The release build is unaffected (it is always compiled).
- Both print the same trace lines as `SALAM_UI_TRACE` for the reload ("kept 14
  variables, reset 1: Counter.name").
- The editor and the LSP show the preview: `salam page dev` serves
  `/__preview/<component>` rendering one component with props from a `preview`
  block next to it (`preview Seg: items = ["a", "b"] value = "a" end`), which
  also gives each widget a gallery page for free.

### Diagnostics

| code | kind | message |
| --- | --- | --- |
| E216 | run time | `test query Role("button", "Save") matched 0 elements; the tree has: ...` |
| E217 | error | `preview 'Seg' names a parameter that does not exist` |
| W218 | warning | `snapshot 'counter' not found; written, review it` |

### Decisions taken

| # | decision |
| --- | --- |
| H1 | one test API (`gui.test`) on both targets, queries by role and name, virtual time |
| H2 | snapshots compare the tree dump on both targets; images are native only with tolerance |
| H3 | hot reload keeps state keyed by site and name; type changes reset with a message |
| H4 | native dev mode uses the interpreter backend; the release path is untouched |
| H5 | `preview Name:` blocks give a component gallery |
| H6 | keep the existing `SALAM_UI_SCRIPT` steps as the line form of the driver |

### First implementation steps

1. `gui.test` over the existing driver in `std/gui/ui/script.salam`: queries
   by tree, `Advance`, `Runs`; port two existing `tests/*/ui` tests.
2. `salam page dev` with sheet patching and state snapshot (web).
3. `--watch` on the interpreter backend; `preview` blocks.

## 10. Gestures, drag and mobile

### What React and QML do

React gives pointer events (`onPointerDown`) and leaves gestures to
libraries (use-gesture, Hammer); mobile means React Native, a second
vocabulary. QML has `PointerHandler`s (`TapHandler`, `DragHandler`,
`PinchHandler`) that are typed and good, with no web. We copy QML's handler
idea as attributes, and the same page runs on Android and iOS through the
engine of `project_mobile_apps` (emitted C, `gui.window` mobile backends).

### Pointer and gesture events

All input is pointers (mouse, touch, pen): `press`, `move`, `release`,
`cancel` receive a `Pointer` (`x`, `y` relative to the element, `id`, `kind`,
`pressure`, `shift`, `ctrl`, ...). Gestures are recognised from them once, in
`gui.ui`, and exposed as attributes:

| attribute | handler argument | fires |
| --- | --- | --- |
| `tap`, `double tap`, `long press` | `Pointer` | after a short press, two, or 500 ms held without moving |
| `pan` | `Pan` (`dx`, `dy`, `vx`, `vy`, `phase`: start, move, end) | finger or mouse drag past a 8 px slop |
| `swipe` | `Swipe` (`dir`: start, end, up, down, `speed`) | fast pan that ends with speed |
| `pinch` | `Pinch` (`scale`, `angle`, `cx`, `cy`, `phase`) | two pointers; also Ctrl wheel and trackpad pinch on desktop |
| `hover` handlers | `Pointer` | mouse and pen only |

`click` keeps working for mouse and touch (it is `tap` plus the keyboard
activation). Which recogniser wins is decided by a gesture arena: the
innermost element that handles a gesture takes it, unless `touch action`
(`auto`, `none`, `pan x`, `pan y`, `pinch`) says the ancestor keeps it:
`touch action = pan y` on a vertical list lets a horizontal `swipe` of a row
through. A scroll area uses the same arena, so a `pan` inside it works when
`touch action = none` is on the inner element. Ambiguity (two handlers for the
same gesture on one element) is W219.

```salam
offset := 0

page:
    box: id = "sheet" touch action = pan x
        translate = offset, 0
        transition = if dragging: 0ms else 200ms end
        pan = (p: Pan): offset = p.dx; dragging = p.phase != end end
        swipe = (s: Swipe): page.Back() end
    end
end
```

### Drag and drop

`draggable = true`, `drag data = value` (typed: `str`, a file list, or a
struct), `drop = (d: Drop): ... end` and the states `drag over` and
`dragging` for style. Web: HTML drag and drop (touch uses the same arena with
a long press); native: `std/gui/drag` and `std/gui/drop` for files and text
from other applications. Reordering a list is `reorderable = (from, to):
... end` on the container, with the animation of section 6.

### Mobile reuse of the DSL

The page is the same; what changes is conditions and a few attributes:

- Conditions `touch`, `compact`, `portrait` (section 1) replace separate
  mobile pages. The kit sizes controls to 44 px on `touch`, and W220 warns
  about a tappable element under 44 by 44 px on a `touch` page.
- **Safe areas**: `safe area = true` on the page or on a bar pads to the
  system insets (notch, status bar, home indicator, rounded corners);
  `keyboard avoid = true` shrinks the page above the soft keyboard and
  scrolls the focused input into view. The tokens `theme.inset.top`,
  `bottom`, `start`, `end` hold the numbers (zero on desktop).
- **Platform parts**: `page: status bar = light`, `orientation = portrait`,
  `back` button maps to `page.Back()` (section 7), pull to refresh is
  `ScrollArea: refresh = (): ... end` (a `load` value, R9), `haptic("tap")`,
  `share(text)`, and `open_url(u)` are functions in `gui.device`; on desktop
  and web they do the nearest thing or nothing.
- **Units**: lengths are density independent on every target (one unit is a
  logical pixel; the window scale maps it), so a page does not change.
- **Packaging** is `salam build app.salam --ui=native --target=android` and
  `--target=ios`; the page, assets and fonts are embedded as on desktop.
  The emulator recipe and signing are tooling, not language.

### Diagnostics

| code | kind | message |
| --- | --- | --- |
| W219 | warning | `two 'pan' handlers on the same element; the second is ignored` |
| W220 | warning | `tap target is 30 by 28; touch targets should be at least 44 by 44` |
| E221 | error | `'drop' handler argument must be 'Drop'` |

### Decisions taken

| # | decision |
| --- | --- |
| G1 | pointers unify mouse, touch and pen; gestures are attributes (`tap`, `pan`, `swipe`, `pinch`, `long press`) |
| G2 | a gesture arena with `touch action`; innermost handler wins |
| G3 | drag and drop through `draggable`, `drag data`, `drop`, `reorderable` |
| G4 | mobile reuses the same page: conditions, `safe area`, `keyboard avoid`, no separate pages |
| G5 | platform calls live in `gui.device`, no ops where meaningless |
| G6 | logical pixels everywhere; 44 px touch target warning |

### First implementation steps

1. Pointer events and the `tap`, `pan` recognisers in `gui.ui` (desktop mouse
   first, testable by `gui.test` `Press`/`Move`/`Release`).
2. Web mapping (pointer events, `touch-action`), `pinch` and `swipe`.
3. `safe area`, `keyboard avoid`, `gui.device`, and the Android build of a
   page.
