# Variants, conditions, themes and tokens

Status: proposed design (2026-11-16), extends `UI_DSL.md` ("States and
conditions", "Value syntax"). Every rule works on the web target and on the
native target, or it is not added (the rule of `UI_DSL.md`). Diagnostic codes
are allocated here from E172 and W172 on; E167 to E169 stay reserved. If a
code is taken by the time a step lands, renumber it in the table of the PR
that lands it and in this file.

The two sections come from one finding: the editor playground
(`editor/page.salam`, `app.salam`, `anim.salam`) cannot be written in the
standard today. It toggles class names from code (`seg-btn active`,
`toggle on`), uses descendant selectors (`:root[data-theme="light"] .dd-item`),
keeps `@media` rules in a raw string, recolours icons with `mask-image`, sets
CSS custom properties from script, and picks light or dark colours with a
`data-theme` attribute. All of that is one idea: style that depends on
something. This file gives it one syntax.

## 1. Variants and conditions

### What React and QML do

React has no styling of its own: a class name is computed in JavaScript
(`className={active ? "btn on" : "btn"}`) and the stylesheet is somewhere
else, so nothing checks that the class exists. Libraries add variants
(cva, Tailwind `data-[state=on]:`), still as strings. QML has `states:
State { when: cond; PropertyChanges {...} }`, typed and checked, but
verbose, and it has no screen or container conditions.

### What we do

A condition is written once, next to the attributes it changes, and the
compiler knows every condition when the page is compiled. There is no class
name and no selector.

```salam
component Seg(items: Vector<str>, value: str, pick: func (str)):
    row:
        radius = 8
        border = 1, solid, theme.color.border
        each it in items:
            button: key = it content = it click = (): pick(it) end
                padding = 7, 14
                background = transparent
                color = theme.color.muted
                hover color = theme.color.fg
                transition = 150ms
                when it == value:
                    background = theme.color.accent
                    color = theme.color.on accent
                    font weight = semibold
                end
            end
        end
    end
end
```

`when condition:` ... `end` is allowed inside any element and holds style
attributes only (also state prefixes: `hover background = ...`). Several
`when` blocks may follow each other. A condition is a Salam expression of
type `bool`, or one of the environment conditions below, joined with `and`,
`or`, `not`.

| condition | true when |
| --- | --- |
| any `bool` expression (`it == value`, `open`, `count > 3`) | the expression is true; it is reactive: a write to a name it reads re-evaluates it and nothing else (R1 and R3 of `UI_DSL.md`) |
| `dark` | the colour scheme in use is dark (section 2) |
| `screen width < 900`, `screen height >= 600` | the window (web: viewport) size in pixels; `<`, `<=`, `>`, `>=`, and `a <= screen width < b` |
| `compact`, `regular`, `wide` | named width classes: below 600, 600 to 1024, above 1024; one place to change them (`screen classes` in the theme, section 2) |
| `landscape`, `portrait` | window shape |
| `touch`, `mouse` | primary input kind |
| `reduced motion`, `high contrast` | user settings; section 2 and `accessibility section of `UI_COMPONENTS.md` |
| `rtl`, `lang = "fa"` | text direction, page language |
| `container width < 400`, `container height ...` | the nearest ancestor with `container = true` |
| `in card hover`, `in card focus within`, `in card active` | a state of the nearest ancestor whose `group = card` |

The six names `dark`, `compact`, `regular`, `wide`, `touch`, `mouse` are
reserved inside `when`; a local variable of the same name is E172 (rename it).

### Rules

1. **Cascade is flat.** No selectors, no specificity. The order of strength
   is: base attributes, then every `when` block in source order, then state
   prefixes in the fixed order of `UI_DSL.md` (a state prefix inside a `when`
   counts as a state prefix). Later wins at the same level.
2. **No descendant selectors.** A parent never styles a descendant by name.
   Use a parameter (`Seg(compact = true)`), a `group` condition
   (`in card hover`), or a theme token. This is what removes
   `:root[data-theme="light"] .dd-item.sel` (the theme does it) and
   `.controls .seg-btn` (the button reads `when screen width < 1025` itself).
3. **Container conditions.** `container = true` marks an element as a query
   container; `container = "card"` names it and `when card width < 400`
   targets that one. A container condition with no container ancestor is
   E174. A container's own size must not depend on its children's reaction to
   the condition (no loop): a rule that changes the container's size is
   evaluated once per frame, the second evaluation uses the new size, and a
   third change in the same frame is ignored with the trace line
   `container loop`.
4. **Values can be conditional too**, for one attribute: any expression,
   `padding = if wide: 24 else: 12 end`. Prefer `when` for more than one
   attribute.
5. **Structure** stays `if`: a condition that adds or removes elements is an
   `if`, not a `when` (`when` never changes the tree, so state, focus and
   text survive it).
6. **Mobile menu.** `body.menu-open .controls { transform: translateX(0) }`
   becomes state plus a condition:

```salam
menu := false

page:
    header:
        button: id = "menu-toggle" content = "Menu" click = (): menu = !menu end
            visible = false
            when compact: visible = true end
        end
        nav: id = "controls" position = fixed, end = 0, top = 0
            width = 330
            translate = 0
            when compact and not menu: translate = 110%, 0 end
            transition = 300ms
        end
    end
end
```

7. **Run time values replace JS-set custom properties.** `--row-a` set from a
   drag handler is `rows = row_a, 6, 1fr` with `row_a := 1fr` as state; the
   gutter writes `row_a` and nothing else re-runs (R4).
8. **Other editor leftovers**, each one standard property: `tab size = 4`
   (text, number of spaces), `touch action = none | pan x | pan y | pinch |
   auto` (which gestures the element keeps; section 10 of
   `UI_BEHAVIOUR.md`), `will change` is not added: the compiler decides
   layers and the web output emits `will-change` itself for elements that
   have a running transition or an animation.

### Targets

- **web**: a static `when` on environment conditions becomes `@media` and
  `@container` rules in the page sheet. A reactive `when` becomes one boolean
  attribute per distinct condition on the element (`data-w3`) set by the
  existing patch step; the sheet has `[data-w3]{...}`. Names are generated and
  per component, never written by the user and never in the DOM API, so
  nothing can collide. `dark` becomes `@media (prefers-color-scheme: dark)`
  unless the page forces a scheme (section 2), when it becomes
  `[data-scheme=dark]`.
- **native**: `Style` is computed as `Default`, `Inherit`, base patch, then
  the patch of each true `when` block, then states. Each reactive condition is
  a dependency of the element's style scope; environment conditions are
  window signals (resize, scheme, input kind) that restyle only elements that
  name that condition. Container conditions are checked after layout; a
  crossing restyles the subtree and lays out again once.

### State blocks

A state can also be written as a block. It is an alias of the prefixed form:

```salam
button:
    background = blue
    hover:
        background = navy
        padding = 12
    end
    focus within:
        border = 2, solid, red
    end
end
```

means `hover background = navy`, `hover padding = 12` and
`focus within border = 2, solid, red`. Every state of the prefix table
(`hover`, `focus`, `focus within`, `active`, `disabled`, `checked`, `invalid`,
`placeholder`, `selection`, and the Persian names written with spaces) works as
a block name. The parser rewrites the block into prefixed attributes, so the
web sheet and the native state patches are the ones the prefixed form makes.
Inside a conditional block the cascade stays base, conditional blocks in source
order, then states.

A state block holds style attributes only (E230) and does not nest (E231).
`before` and `after` are not state blocks: they are web-only pseudo content, not
part of the state table, and have no native meaning.

### Diagnostics

| code | kind | message |
| --- | --- | --- |
| E172 | error | `'dark' is a reserved condition name; rename the variable` |
| E173 | error | `a 'when' condition must be a bool or an environment condition, found <type>` |
| E174 | error | `'container' condition used outside an element marked container` |
| E175 | error | `'when' holds style attributes only; use 'if' to add or remove elements` |
| W176 | warning | `'when' condition is always true or always false` |
| E177 | error | `no ancestor has group 'card'` |
| E230 | error | `a 'hover' block holds style attributes only; move elements and conditions out of it` |
| E231 | error | `state blocks cannot be nested: 'focus' inside 'hover'; write the state prefix on each attribute instead` |

### Decisions taken

| # | decision |
| --- | --- |
| V1 | `when cond:` blocks inside an element; no class names, no selectors, no descendant rules |
| V2 | flat cascade: base, `when` in source order, then states |
| V3 | environment conditions are words (`dark`, `compact`, `touch`, `screen width < n`), usable on both targets |
| V4 | container queries through `container = true or "name"` |
| V5 | ancestor state through `group = name` and `in name hover` |
| V6 | `when` never changes the tree; structure is `if` |
| V7 | JS custom properties become state read in an attribute value |
| V8 | add `tab size`, `touch action`; do not add `will change` |

### First implementation steps

1. Parser: `when` block in the layout block grammar, `group` and `container`
   attributes; AST node `AST_WHEN` (lowered like state prefixes in
   `sema_layout_fold.salam`). Test: errors E172 to E177 in `tests/*/page/errors`.
2. Web: static environment conditions to `@media`/`@container`; reactive ones
   to the boolean attribute. Test: build, check the sheet text.
3. Native: `ui.Builder.When(id, cond)` patch layer, window signals for
   screen, scheme and input kind; container check after `flow` layout.
   Test: `tests/*/ui` with `SALAM_UI_SCRIPT` steps `resize 500 400`,
   `scheme dark`.
4. Port `seg-btn`, `toggle`, the mobile menu and the light theme of the
   editor; delete `global: selector` uses.

## 2. Theming and design tokens

### What React and QML do

React has no theme: libraries bring one (CSS variables, a context provider,
MUI `createTheme`), checked by TypeScript at best. CSS has custom properties,
untyped and global strings. QML has `palette` and `Material.theme`, typed, but
one fixed set of names. Dark mode is `prefers-color-scheme` or a class set by
JavaScript, and the page needs both.

### What we do

A **theme** is a named, typed set of tokens. A token is read as
`theme.group.name` anywhere a value is accepted. The group fixes the type;
every value is checked at compile time by `gui.style` like any attribute.

```salam
theme Light:
    scheme = light
    color:
        bg = #f4f6fa
        panel = #ffffff
        fg = #1b2030
        muted = #5f6878
        accent = #ff7a18
        on accent = #2a1402
        border = #d8dde7
    end
    space: sm = 4, md = 8, lg = 16 end
    radius: sm = 4, md = 8 end
    font: body = "Vazirmatn", system, sans  code = mono end
    size: body = 15, small = 13, title = 24 end
    time: fast = 150ms, slow = 300ms end
    shadow: card = [0, 4, 12, rgba(20, 30, 60, 0.18)] end
    screen classes: compact = 600, wide = 1024 end
end

theme Dark extends Light:
    scheme = dark
    color: bg = #0f1115, panel = #161922, fg = #e6e9ef, muted = #9aa4b8 end
end
```

- Groups: `color`, `space`, `radius`, `size`, `font`, `time`, `shadow`,
  `easing`, `screen classes`. A group name may carry a user group
  (`color:` entries can be any name). Names may have spaces (`on accent`)
  and Persian names (`رنگ`, `پس زمینه`) like attributes.
- `extends` copies the parent's tokens and overrides some. Every theme that
  is selectable must define, directly or by `extends`, all tokens of the
  first theme in the file (the base). A missing one is E178.
- A theme is a compile-time value, not a runtime object: tokens in static
  attributes are folded when only one theme can reach them.

### Choosing a theme

```salam
page:
    theme = auto
    box: theme = Contrast
        button: theme color accent = #e11d48 content = "Delete" end
    end
end
```

- `theme = auto` (the default when the program declares themes with both
  schemes): use the theme whose `scheme` matches the system, `Light` or
  `Dark`, and follow changes. `theme = Dark` pins one. `theme = expr`
  where `expr` is state (`mode := auto`) switches at run time.
- `theme = Name` on any element themes that subtree. `theme color accent =
  red` overrides one token for the subtree: per-component theming without
  a second theme. A component declares the tokens it reads in its own
  `theme` block of defaults (`component Button: theme color accent =
  theme.color.primary`), so a user can restyle one instance from outside.
- A program with no `theme` gets the built-in `Light`/`Dark` pair of the
  standard component set (`UI_COMPONENTS.md`). `light dark(a, b)` still works
  for a one-off colour.

### Targets

- **web**: each theme becomes a rule of custom properties on a scheme
  attribute, `[data-theme=Dark]{--t-color-bg:#0f1115;...}`; a token read is
  `var(--t-color-bg)` (the compiler writes both; the user never sees them).
  `auto` adds `@media (prefers-color-scheme: dark)` rules; a pinned theme
  sets `color-scheme` and the attribute on the root. Subtree themes set the
  attribute on that element. Tokens fold to literals when only one theme
  exists. The first paint is correct without script (the attribute is set
  inline from the stored choice only when the program stores one, see
  `UI_BEHAVIOUR.md` on storage).
- **native**: `Style` values holding a token keep the token id; switching
  theme marks the elements that read tokens (a bit per token group in the
  scope, same mechanism as state, R1), restyles only them and repaints. The OS
  scheme comes from `gui.window` (`ColorSchemeChanged`, already exists);
  `reduced motion` and `high contrast` likewise.
- Tokens are animatable: with `transition` on, a theme switch cross fades the
  colours that change (oklab, `color.Lerp`), on both targets.

### Typed access from code

`theme.color.accent` is also readable in Salam code of a handler or value as
a `str` of the canonical text (not a CSS string), and `set_theme(Dark)`
writes the page's theme state. Tokens never produce CSS text in user
code.

### Diagnostics

| code | kind | message |
| --- | --- | --- |
| E178 | error | `theme 'Dark' lacks token 'color.on accent' defined by 'Light'` |
| E179 | error | `token 'color.accent' is not a colour: <gui.style error>` (the group's type) |
| E180 | error | `no token 'theme.color.acent'` (with the nearest name as a hint) |
| E181 | error | `'extends' cycle: Dark, Light, Dark` |
| W182 | warning | `token 'space.lg' is never used` (only for themes in the page's own file) |
| W183 | warning | `'fg' on 'bg' has contrast 2.9:1, below 4.5:1 for text` (only when both resolve statically; see accessibility) |

### Decisions taken

| # | decision |
| --- | --- |
| T1 | typed token groups in `theme Name:` blocks; read as `theme.group.name` |
| T2 | `extends` for light/dark and brand variants; all tokens must exist (E178) |
| T3 | `theme = auto` follows the OS; `theme = Name` or state pins it; subtree `theme` on any element |
| T4 | `theme color accent = red` overrides one token for a subtree (per component theming) |
| T5 | web uses custom properties as an output detail, native keeps token ids; the user never writes `var()` |
| T6 | keep `light dark(a, b)` for one-offs |
| T7 | theme switches animate through `transition`; `reduced motion` turns it off |
| T8 | breakpoints are tokens (`screen classes`), so `compact` means one thing everywhere |
| T9 | contrast check W183 at compile time for static pairs |

### First implementation steps

1. Parse `theme` declarations (top level, like `component`), check token
   types with `gui.style.Parse` by group; E178 to E181.
2. Resolve `theme.g.n` in attribute values; web emits custom properties and
   the root attribute; native keeps a token table in `gui.ui`.
3. Built-in `Light` and `Dark` for the standard components; tests for
   switching on both targets (`tests/*/ui` with `scheme dark`).
4. Port the editor's `:root[data-theme="light"]` rules to a `Light` and
   `Dark` pair; delete the raw sheet string.
