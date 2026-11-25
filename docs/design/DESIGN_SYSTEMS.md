# Design systems for the page DSL

No design system is applied unless you ask for one. Today the only one is
shadcn/ui (the new-york style). The active design is, in this order: the
page's `design` setting, then the project default from `design.toml` or the
`[design]` table of `salam.toml`, otherwise none. With none, components still
work (events, ARIA, open and close) with a minimal look, the same as
`design = "none"`.

The order, from the strongest: a `design` on the element, a `design` on an
ancestor element, the `design` of the `page:`, the project file, none.

## Page settings

```salam
page:
    design = "shadcn"
    theme = "auto"
    palette = "zinc"
    design tokens = "primary: #2563eb; dark primary: #60a5fa; radius: 0.5rem"
    ...
end
```

| setting         | values                                  | default  |
| --------------- | --------------------------------------- | -------- |
| `design`        | `shadcn`, `none`                        | `none`   |
| `theme`         | `auto`, `light`, `dark`                 | `auto`   |
| `palette`       | `neutral`, `zinc`, `stone`, `slate`, `gray`, `mauve`, `olive`, `mist`, `taupe`, or a palette defined in the project file | `neutral` |
| `design tokens` | `name: value; dark name: value; ...`    | none     |

`auto` follows the system setting. `theme` works on any element (box, card,
...): `theme = "dark"` themes that subtree, as does the class `dark`. A page
with a `theme toggle` and `theme = "auto"` also gets a small head script that
applies the saved choice (localStorage key `salam-theme`) before first paint. In `design tokens`, a bare name sets the
token for light and dark, `light name` and `dark name` set one side only.
Tokens are the shadcn variables: `background foreground card popover primary
secondary muted accent destructive border input ring radius chart-1 ... chart-5
sidebar ...` plus `font-sans` and `font-mono`.

## Scoping a design to part of a page

`design` and `theme` work on the `page:` element (the default for the whole
page) and on any single element. The nearest setting wins: an element, then its
ancestors, then the page, then the project file, then none.

```salam
page:
    design = "shadcn"
    card:
        title = "Styled"
    end
    box:
        design = "none"
        button:
            content = "Bare button"
        end
        card:
            design = "shadcn"
            title = "Styled again"
        end
    end
end
```

`design = "none"` on an element removes the design system's default look from
that element and everything under it, until a descendant sets its own `design`.
The element then looks like the bare element: only what the page itself sets
(`padding`, `color`, ...) applies. Behaviour (open and close, ARIA, events) is
kept, as it is for a whole page with `design = "none"`. `design = "shadcn"` on
an element inside such a subtree turns the design back on for it and its
descendants. Persian: `طراحی = "هیچ"` and `طراحی = "شد سی ان"`.

The other way round works too. With `design = "none"` on the page (or no design
at all), one element with `design = "shadcn"` is styled and the rest of the
page stays bare.

`theme = "light"`, `"dark"` or `"auto"` themes one element and its subtree, and
on the page it is the default for the whole page. An element with its own
`theme` inside a themed subtree overrides it.

How it is built: an element that sets `design` gets the attribute
`data-design`, and every element in a scope that was set by an element gets the
marker `data-ds-off` (inside `none`) or `data-ds-on` (inside a design). The
design rules keep zero specificity and are generated as
`:where(:not([data-ds-off])):where(button...)`, or `:where([data-ds-on])` when
the page default is none, so a style written on the element still wins and
nothing is overridden with `!important` or `all: revert`. Pages that never set
`design` on an element get no markers and unchanged CSS. The functional rules a
component needs even with no design (a closed dialog stays hidden, a switch
keeps its input) are not scoped.

`palette` and `design tokens` stay page and project settings: the CSS variables
are written once for the page, so they cannot differ per element. To change one
subtree, set a custom property on the element with the `style` attribute
(`style = "--primary: #c00"`).

Limits: the button group joins (`div[data-slot="button-group"]` rules) are not
scoped, and a selector with two `:where()` parts is gated on its first part
only. The native UI (`--ui=native`, Android, iOS) has no design system, so
`design` and `theme` are accepted there and change nothing.

## Fonts

Tokens `font-sans` and `font-mono` set the families. `font-url` is a space
separated list of stylesheet URLs (Google Fonts style) and becomes `<link>`
tags, with a preconnect for fonts.googleapis.com. `font-face` lists own files
as `Family url [weight] [italic]`, comma separated (`+` in the family stands
for a space) and becomes `@font-face` rules. They apply even with
`design = none`. In the `design tokens` attribute `;` ends a token and `&` is
unusable, so a URL there needs `%3B` and `%26`; the project file has no such
limit.

## Project file

Project defaults for every page (including which `design` to use) come from `design.toml`, or from the `[design]` tables
of `salam.toml`. The generator looks in the current directory (where you run
the build) and then in every parent, and uses the nearest folder that has a
`design.toml` or a `salam.toml` with a design section. `SALAM_DESIGN=<file>`
names a file explicitly. Page settings win over the file; page tokens win over
file tokens, which win over the palette. Without a `design` in the page or the
file, no design is applied.

```toml
[design]
design = "shadcn"
theme = "auto"
palette = "brand"
font-sans = "Inter, sans-serif"
font-url = "https://fonts.googleapis.com/css2?family=Inter:wght@400;700"
font-face = "Brand+Sans /fonts/brand.woff2 400 700"
tokens = "radius: 0.5rem"

[design.tokens]
dark-ring = "oklch(0.7 0.15 260)"

[design.palette.brand]
base = "zinc"
primary = "oklch(0.6 0.2 260)"
dark-primary = "oklch(0.7 0.15 260)"
```

In `design.toml` the prefix is dropped: `[tokens]`, `[palette.brand]`, root keys.
A palette table starts from the built in palette named by `base` (default
`neutral`); every other key is a token, `dark-` or `light-` in front for one
side. A page then says `palette = "brand"`. A config `theme` of `light` or
`dark` sets the page color scheme when the page has no `theme`.

Known limit: a page token of one side (`primary`) does not override a file
token of a specific side (`dark-primary`) for that side.

## Mistakes

An unknown token name or an undefined palette is reported: a
`<meta name="salam-design-warning" content="...">` in the head and a
`console.error`. A token that starts with `--` is a custom property and is
passed through, `ds-` names are reserved.

## How it works

The page asks the std package `layoutgen` for its design (`design_page`).
Each restyled element has a generator that adds its CSS once per page, only
when the element is used. All rules have zero specificity (`:where()`), so a
style written on the element always wins.

Colors are CSS variables written once with `light-dark()`, so one block serves
light, dark and auto. Put `theme = "dark"` (or the class `dark`, or `data-theme`)
on any element to theme its subtree. Components never hard code a color: they
read the variables, so `design tokens` restyles everything at once.

Variants are one word attributes: `variant = "outline"`, `size = "sm"`. Write
enum values in quotes.

Behaviour (open and close, keyboard, ARIA, events) does not depend on the
design. With `design = "none"` every component still works and keeps a minimal
look; only the shadcn styling is dropped.

## Components

Button: `variant` default, destructive, outline, secondary, ghost, link; `size`
default, xs, sm, lg, icon, icon xs, icon sm, icon lg. `theme toggle` is a button
that flips light and dark and remembers the choice.

Other groups, with their elements, attributes and events:

- `design-display.md`: card, badge, alert, separator, progress, meter, skeleton,
  spinner, avatar, aspect ratio, table, typography, empty, entry, breadcrumb,
  pagination, scroll area
- `design-overlay.md`: tabs, accordion, collapsible, dialog, alert dialog, sheet,
  drawer, popover, tooltip, hover card, dropdown menu, context menu, menubar,
  navigation menu
- `design-forms.md`, `design-extras.md`: forms and the rest, when merged

## Adding another design

`design` takes the names registered in the schema value group `design_name`
(`std/layout/values/design-values.salam`): today `shadcn` and `none`. A new
design is one more name there, with `@en` and `@fa`, plus its generators.

The generator contract:

- A design provides its CSS in files named `std/layoutgen/ds_<design>_*.salam`
  (for example `ds_material_button.salam`), next to the shadcn `ds_*.salam`
  files. Each constant holds the zero specificity CSS of one component.
- The element, its HTML, ARIA and events never change between designs. Only
  the CSS differs, so a generator keeps building the same markup and asks for
  the CSS by design name.
- Tokens are the shared CSS variables. A design supplies its own palette text
  in `std/layoutgen/design_palettes.salam` style and reads the same token
  names, so `design tokens` keeps working.
- Fallback rule: a component that the active design does not style uses the
  shadcn CSS. `DsPick(design, shadcn_css, design_css)` returns `design_css`
  when `DsName()` equals `design` and `design_css` is not empty, and
  `shadcn_css` in every other case. A design can therefore ship only some
  components and a page never loses a component.
- `design = "none"` is not a design but the switch that turns styling off:
  `DsOn()` is false and generators add no CSS.

Page code in use:

```salam
if DsOn():
    o.shared_css = DsPick("material", DS_CARD_CSS, DS_MATERIAL_CARD_CSS)
end
```

The checklist for a real design is in `ADDING_A_DESIGN.md`. The behaviour is
tested by `tests/en/stdlib/layoutgen_design_fallback.salam` (and the fa twin),
which registers a made up design name through the page's `data-design`
attribute.
