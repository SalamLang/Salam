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
| `palette`       | `neutral`, `zinc`, `stone`, `slate`, `gray` | `neutral` |
| `design tokens` | `name: value; dark name: value; ...`    | none     |

`auto` follows the system setting. A `data-theme` on any element (or the
class `dark`) themes that subtree. In `design tokens`, a bare name sets the
token for light and dark, `light name` and `dark name` set one side only.
Tokens are the shadcn variables: `background foreground card popover primary
secondary muted accent destructive border input ring radius chart-1 ... chart-5
sidebar ...` plus `font-sans` and `font-mono`.

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
