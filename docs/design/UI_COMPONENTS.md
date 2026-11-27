# Standard components, accessibility and assets

Status: proposed design (2026-11-16), extends `UI_DSL.md`. Conditions
(`when`) and tokens (`theme.group.name`) are the ones of `UI_STATES_THEMES.md`.
Codes continue from that file: this one uses E184 to E199 and W184 to W199, and E232 (private component).

## 3. The standard component set

### What React and QML do

React ships no widgets; every project picks a library (MUI, Radix, shadcn),
each with its own props, theming and accessibility. QML ships Controls
(Button, CheckBox, ComboBox, Slider, TabBar, Dialog, Menu), with Basic,
Material, Fusion and Universal styles, but they are C++ and cannot be edited
as page code.

### What we do

A package `std/gui/kit` (Persian name `گیت ابزار`, the usual naming rules of
`naming-standard.md`) written entirely in the page language: every widget is a
`component` built from the standard elements, theme tokens, `when` blocks and
the accessibility attributes below. Because it is page code it runs on both
targets with the same behaviour and the same tests, a user can copy one widget
and change it, and there is no second implementation to keep in step. A page
imports nothing: the kit's names are in scope in every page file, and a user
component of the same name wins (W184 warns).

Native look: widgets are drawn by `gui.ui` from tokens, never by the OS
toolkit (`NATIVE_UI.md`, "Why draw instead of using OS widgets"). The look
comes from the theme: the kit ships `Light`, `Dark`, and a `Windows` pair
(`Windows Light`, `Windows Dark`: 4 px radius, 32 px control height, 2 px
focus ring, system accent through `theme.color.system accent` read from the
OS) chosen by `theme = system` on Windows. Metrics are tokens
(`theme.size.control`), so a theme can change density. Keyboard, focus and
text editing follow the platform's conventions (section 4).

### The set

| area | components |
| --- | --- |
| actions | `Button` (`kind = primary, secondary, ghost, danger`; `size = sm, md, lg`; `icon`; `loading`), `IconButton`, `Link`, `Menu`, `MenuBar`, `ContextMenu`, `Toolbar` |
| choice | `Checkbox`, `Radio` and `RadioGroup`, `Switch` (the editor's `toggle`), `Segmented` (the editor's `seg`), `Select`, `Combobox`, `Slider`, `Stepper` (number), `Tabs` |
| text input | `Field` (label, hint, error around any input), `TextInput`, `TextArea`, `PasswordInput`, `SearchInput`, `CodeView` (`spans`, line numbers) |
| display | `Text`, `Heading`, `Badge`, `Avatar`, `Icon`, `Image`, `Divider`, `Progress`, `Spinner`, `Skeleton`, `Card`, `Tag` |
| collections | `List` (virtual), `Table` (virtual rows, sort, resize, select), `Tree`, `Accordion`, `Carousel` |
| overlays | `Dialog`, `Drawer`, `Popover`, `Tooltip`, `Toast`, `Dropdown` (the editor's `dd`) |
| layout | `Stack` (`layout` shortcuts), `Split` (draggable gutter, the editor's `gutter`), `ScrollArea`, `Spacer`, `Page` shell (`header`, `nav`, `main`, `footer` regions that collapse by `compact`) |
| date and time | `DatePicker`, `TimePicker`, `Calendar` (later, same API shape) |

### API shape

Every widget follows the same rules, so a user learns one:

1. Data in as parameters, events out as `on name` or callbacks, no write back
   (R6). Two way use is `bind` (section 8 of `UI_BEHAVIOUR.md`).
2. Parameters are named, typed, have defaults, and a variant is a parameter
   of an enum type (`kind: Kind = primary`), never a free string.
3. Styling from outside is tokens (`theme color accent = red`) or the
   `style` parameter below, not class names.
4. Every widget takes the standard attributes `id`, `key`, `visible`, `grow`,
   `align self`, `margin`, `width`, `height` and the accessibility attributes.
5. A widget with a popup (Select, Menu, Popover, Tooltip, Dialog) renders it in
   the page's overlay layer (`layer`), never inside its own clipping box, and
   closes on `Escape` and outside press.

```salam
page:
    title = "Settings"
    Page:
        Field: label = "Name" hint = "Shown on your profile" error = name_error
            TextInput: bind = name end
        end
        Segmented: items = ["Light", "Dark", "System"] bind = mode end
        Select: items = langs, bind = lang placeholder = "Language" end
        Switch: bind = notify label = "Notifications" end
        Button: kind = primary content = "Save" click = (): save() end loading = saving end
    end
end
```

Component children: a `component` may place the elements written inside its
use site with an element `content:` (Persian `محتوا`), once, anywhere in
its body; `Card` is `box: radius = theme.radius.md ... content: end end`.
A named slot is `slot: name = "actions" end` (Persian
`جایگاه: نام = "کنش" پایان`) and the use site fills it with
`slot = "actions"` (`جایگاه = "کنش"`) on an element. It is expanded at compile
time like a component, so no runtime cost and the same on both targets.

`style` parameter: `style = [padding = 4, color = red]` is not added. A
widget exposes the few attributes it allows (`width`, `grow`, `margin`);
everything else is a token, so a widget cannot be broken by a style from
outside. A user who needs more copies the widget (it is page code).

### Visibility: `pub component`

A component belongs to the file that defines it. Without `pub` it is private:
the file's own page and the other components of that file use it, and a page
in another file that `include`s it does not. `pub component Card` (Persian
`همگانی بخش Card`, the same `همگانی` as `pub` everywhere) can be used from any
file that includes its file. The rule is the one of functions and structs, it
is checked while the page is expanded, so it is identical on the web and the
native target.

```salam
pub component Panel(title: str):
    Frame: content = title end
end

component Frame:
    box: padding = 8 content: end end
end
```

- A public component may use the private components of its own file; they
  expand correctly when `Panel` is used from another file, because the lookup
  of a name inside a body starts in the file of that body.
- Two files may each have a private component of the same name; each file
  sees its own. Two public components of one name still clash (the second is
  ignored with the warning that exists today).
- A name is looked up in the using file first, then among public components.
  A private component of another file is never found by accident: it is
  reported as E232.
- `pub page` stays an error (a page is not shared by name), and the standard
  kit components of this chapter are always visible.

### Run time content

Markup injected at run time (`SetHtml` in the editor, for highlighting and for
dropdown items) is not part of the standard: there is no HTML. Its three uses
become typed:

- **Rows and items** are `each`, a `List`, or the `items` parameter of a
  widget. The editor's `dd-item` rows are `Dropdown: items = list` where the
  item type is a struct (`label`, `badge`, `selected`).
- **Highlighted text** is a `Text` or `CodeView` with `spans`: a
  `Vector<Span>`, where `Span` has `text: str` and `style: str`, naming an
  entry of the `text` token group
  (`theme.text.keyword`, set in the theme as `text: keyword = #569cd6,
  comment = #6a9955 italic end`). Patching diffs spans by index, so a
  highlighted editor of 5000 lines re-runs only changed lines when `spans`
  is per line inside `each` with `key`.
- **Line numbers and markers** are generated content: an element may hold
  `before:` and `after:` blocks (`before: content = n + 1 color =
  theme.color.muted end`). They are real nodes on native and `::before` /
  `::after` on web, never selectable, never in the accessibility tree.

### Diagnostics

| code | kind | message |
| --- | --- | --- |
| E232 | error | `component 'Hidden' is private to 'parts.salam'; it cannot be used from another file` (hint: `mark it 'pub' where it is defined: 'pub component Hidden'`) |
| W184 | warning | `component 'Button' hides the standard component of the same name` |
| E111 | error | `unknown layout element 'content'` (`content:` used outside a component) |
| E114 | error | `duplicate slot placeholder 'default' in component` |
| E187 | error | `no value 'giant' for parameter 'size' of Button: expected sm, md or lg` |
| E188 | error | `'Span.style' names no entry of the 'text' token group` (static strings only) |

### Decisions taken

| # | decision |
| --- | --- |
| C1 | one kit in page code (`std/gui/kit`), identical on web and native, no OS widgets |
| C2 | variants are enum parameters; outside styling is tokens only |
| C3 | `content:` and named slots, expanded at compile time |
| C4 | no run time markup: `each`, `items`, typed `Span` lists, `before:` and `after:` |
| C5 | Windows look is a theme pair plus metric tokens, chosen by `theme = system` |
| C6 | overlays live in the page layer and close on `Escape` |
| C7 | the editor's own widgets (`seg`, `toggle`, `dd`, `gutter`) become `Segmented`, `Switch`, `Dropdown`, `Split` |

### First implementation steps

1. `content:` and slots in the compiler (component expansion).
2. `Button`, `Field`, `TextInput`, `Checkbox`, `Switch`, `Segmented`, `Card`,
   `Text` (smallest set that a form page needs), with one `tests/*/page` and
   one `tests/*/ui` each.
3. Overlay layer in `gui.ui`; then `Select`, `Dialog`, `Menu`, `Tooltip`.
4. Virtual `List` and `Table` (needs `gui.flow` scroll windowing).
5. `Windows` theme pair and a same look check against the web output.

## 4. Accessibility

### What React and QML do

React gives you the DOM and you add `aria-*` by hand; nothing warns when you
forget (lint rules help). QML has `Accessible.role`, `name`, `onPressAction`
properties per item, which are easy to forget, and screen readers on desktop
are patchy. We do both targets from the same attributes, with defaults from
the element kind and compile time warnings when something is missing.

### Attributes

| attribute | meaning |
| --- | --- |
| `role` | only for a generic `box` that acts as a widget: `button`, `tab`, `tab list`, `menu`, `menu item`, `dialog`, `alert`, `slider`, `list`, `list item`, `tree`, `tree item`, `switch`, `progress`, `status`, `region`, `image`, `none` |
| `label` | the accessible name, a string or expression (Persian `برچسب`); `labelled by = id`; `description`, `described by = id` |
| `hidden from readers` | `true` removes the element and its children from the accessibility tree; visual `visible = false` already does |
| `live` | `off`, `polite`, `assertive` for a region whose changes are announced |
| `expanded`, `selected`, `checked`, `pressed`, `current`, `busy`, `required` | `bool` facts of the element, filled by the standard widgets, written by hand for a custom `box` |
| `focusable` | `true` makes a `box` focusable (a `click` handler on a non focusable box is W190); `false` removes a control from the tab order |
| `focus group` | children share one tab stop and move with arrow keys (tabs, menu, radio group, toolbar) |
| `focus trap` | `true` keeps Tab inside the element (dialog); focus returns to the opener on close |
| `shortcut` | `"ctrl s"`, `"escape"`, `"alt 1"`; works on both targets, shown in menus, announced to the reader |

Element kinds have implicit roles (`button`, `link`, `heading` with its
`size` as level, `list`, `item`, `input` by `type`, `image`, `form`, `table`),
so most pages need no attribute but `label` on icon only controls and `alt` on
images.

### Focus and keyboard

- **Order** is document order. A positive `focus order` does not exist;
  reorder the tree (or use `order`, which also reorders focus). This keeps
  both targets equal.
- **Ring**: `focus visible` is a state prefix (`focus visible outline = 2,
  solid, theme.color.accent`), true only after keyboard use, so a mouse press
  shows no ring. Every control has a default ring from the theme.
- **Keys** by role, written once in the kit: `Space` and `Enter` press a
  button; arrows move inside a `focus group`; `Home`/`End`; `Escape` closes
  the innermost overlay; type ahead in lists and menus. Text editing keys
  follow the platform (Ctrl or Cmd).
- **Reduced motion**: the condition `reduced motion` (section 1) is true when
  the user asks for it; `transition` and `animation` durations are set to 0 for
  non essential motion (the compiler's default; a `essential` flag on one
  animation keeps it, for a progress spinner).
- **High contrast** and **text scale**: `high contrast` is a condition and the
  `Light`/`Dark` themes have contrast variants; `size` tokens and `em`
  follow the system text scale on both targets.

### Targets

- **web**: roles, `label` and the facts become `role` and `aria-*`
  attributes, `focusable` a `tabindex`, `focus group` a roving tabindex
  script, `live` an `aria-live`. Standard elements already map to semantic
  tags.
- **native**: `gui.ui` builds an accessibility tree from the nodes (one entry
  per node with a role, name, state, bounds, actions) in package
  `std/gui/a11y`, updated by the same patch step (a changed `label` or fact
  marks one entry). Platform bridges: Windows UI Automation (providers over
  COM, `LiveRegionChanged`, focus events), Linux AT-SPI over D-Bus, macOS
  NSAccessibility. The first bridge is Windows UIA, then AT-SPI; the tree
  and its tests (a script step `a11y dump` prints it) exist before any
  bridge, so web and native trees are compared in tests (`tests/*/ui`).
  The IME and caret position for text input are reported on the same path.

### Diagnostics

| code | kind | message |
| --- | --- | --- |
| W189 | warning | `button without text or label; add 'label'` (icon only controls) |
| W190 | warning | `'click' on a 'box' that is not focusable; use a button or set 'focusable = true' and a role` |
| W191 | warning | `image without 'alt'; use alt = "" for a decorative image` |
| E192 | error | `unknown role 'tabs'; did you mean 'tab list'?` |
| W193 | warning | `'focus order' is not supported; reorder the elements` |
| W194 | warning | `two elements have the same 'shortcut' in one window` |

### Decisions taken

| # | decision |
| --- | --- |
| A1 | one attribute set (`role`, `label`, facts, `live`) for both targets; ARIA is an output detail |
| A2 | implicit roles from the element kind; `label` and `alt` required where text is missing (warnings) |
| A3 | document order is focus order; no positive tab index |
| A4 | `focus visible` ring on keyboard only, default in every theme |
| A5 | keyboard behaviour lives in the kit by role, not per page |
| A6 | native tree in `std/gui/a11y`, bridges UIA first, AT-SPI, NSAccessibility next |
| A7 | `reduced motion`, `high contrast`, text scale are conditions |
| A8 | `shortcut` attribute; shown in menus and announced |

### First implementation steps

1. Attributes in the schema and `native` mapping, W189 to W194 in the layout
   check.
2. Web output (`role`, `aria-*`, roving tabindex) and `focus visible`.
3. Native a11y tree and `a11y dump` script step; compare with web in tests.
4. Windows UIA bridge, then AT-SPI.

## 5. Assets: images, fonts and icons

### What React and QML do

React gets assets from the bundler (`import logo from "./logo.png"`), one
URL per file, and `srcset` by hand. QML has `qrc:` resources compiled into the
binary and `@2x` file selectors. We do what both do, with names the compiler
checks.

### Rules

1. **Paths are relative to the file that writes them** and are checked at
   compile time: `image: source = "logo.png"`, `background = image("bg.png")`.
   A missing file is E195. A path from a runtime expression is a URL or a
   file read at run time and is not bundled.
2. **Bundling**. Web: files are copied to `assets/` with a content hash in
   the name (`logo.3fa9c1.png`) and cached for a year; unused files are not
   copied. Native: files are embedded in the executable (one table, no
   duplicates, `assets` flag `--assets=dir` keeps them as files for a fast
   edit loop; hot reload uses it).
3. **Density**: `logo.png`, `logo@2x.png`, `logo@3x.png` next to each other
   are one asset. Web emits `srcset` and `image-set()`; native picks the file
   for the window's scale factor and re-picks when the window moves between
   screens. A page that gives only `logo.png` for a high density screen gets
   W196 once per file at build.
4. **Vector images**: `.svg` is a supported source (subset: paths, shapes,
   fills, strokes, gradients, clip paths, transforms, `viewBox`; no script,
   no external references, no CSS inside). Native rasterises at the size
   needed (cached per size); web uses the file as it is. An unsupported
   feature is E197 with its line.
5. **Icons**: `icon: source = "play.svg" color = theme.color.fg`. An
   icon is an image whose colour comes from `color`: web draws it with
   `mask-image` and `background: currentColor`, native uses the image's
   alpha as a mask and fills it with the text colour. It is `1em` square by
   default (`width`, `height` change it) and mirrors in right to left when it
   has `mirror = true`. A multi colour image is an `image`. The editor's
   `mask-image` icons are `icon` elements.
6. **Icon sets**: a top level `icons Name:` block maps names to files
   (`play = "play.svg"`), used as `Icon: name = "play"`. The kit's own icons
   (arrow, check, close, chevron, menu, search) are a built in set.
7. **Fonts**: a top level `font` block declares a family and its files; it is
   then usable by name in `font` and in `theme.font`.

```salam
font Vazirmatn:
    file = "assets/fonts/Vazirmatn-Variable.woff2"
    weight = 100, 900
    subset = "arabic", "latin"
end

page:
    font = "Vazirmatn", "system"
    image: source = "hero.png" alt = "The editor" width = "fill" image fit = "cover" end
    icon: source = "play.svg" color = theme.color.accent end
end
```

   Web: `@font-face` with `font-display: swap`, `preload` for the first font.
   Native: registered with `gui.typeset` from the bundled bytes; a missing
   glyph falls back along the family list then to the system font. A family
   with `subset` is split by Unicode range on the web.
8. **Loading**: `image` decodes off the main thread on native and loads lazily
   when `loading = lazy` (web `loading="lazy"`, native: only when near the
   viewport). While loading it shows `background` (a placeholder colour or a
   blur); the states `loading` and `error` are conditions of the element
   (`when image error: ...`). Width and height or `aspect ratio` should be
   known so that layout does not jump (W198).

### Diagnostics

| code | kind | message |
| --- | --- | --- |
| E195 | error | `asset 'logo.png' not found (looked in the directory of this file)` |
| W196 | warning | `'logo.png' has no '@2x' file; it will look soft on high density screens` |
| E197 | error | `svg feature not supported on every target: <feature> (line N)` |
| W198 | warning | `image has no width, height or aspect ratio; the page will move while it loads` |
| E199 | error | `font file 'x.ttf' cannot be read as a font` |

### Decisions taken

| # | decision |
| --- | --- |
| S1 | asset paths are checked at compile time; embedded (native) or hashed copies (web) |
| S2 | `@2x`, `@3x` naming for density on both targets |
| S3 | SVG subset, rasterised on native; unsupported features are errors |
| S4 | `icon` tints by `color` (mask on web, alpha mask on native); `icons Name:` sets |
| S5 | `font Name:` block with file, weight range, subset; no raw `@font-face` |
| S6 | `loading = lazy`, `loading` and `error` conditions on images |
| S7 | `--assets=dir` keeps assets as files for live preview |

### First implementation steps

1. Resolve and check asset paths in the compiler; web copy with hashes;
   native embed table (reuse `embed` if present).
2. `icon` element with tint on both targets; SVG subset renderer in
   `gui.paint` (needed anyway for the kit icons).
3. `font` block (web `@font-face`, native `gui.typeset` registration).
4. Density selection by scale factor; W196, W198.
