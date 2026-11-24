# Design systems for the page DSL

Every page gets a design system by default. Today that is shadcn/ui (the
new-york style). `design = none` turns the look off: components still work
(events, ARIA, open and close) with a minimal look.

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
| `design`        | `shadcn`, `none`                        | `shadcn` |
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

Defaults for every page come from `design.toml`, or from the `[design]` tables
of `salam.toml`. The generator looks in the current directory (where you run
the build) and then in every parent, and uses the nearest folder that has a
`design.toml` or a `salam.toml` with a design section. `SALAM_DESIGN=<file>`
names a file explicitly. Page settings win over the file; page tokens win over
file tokens, which win over the palette.

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
