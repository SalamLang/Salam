# Design systems for the page DSL

Every page gets a design system by default. Today that is shadcn/ui (the
new-york style). `design = none` turns the look off: components still work
(events, ARIA, open and close) with a minimal look.

## Page settings

```salam
page:
    design = shadcn
    theme = auto
    palette = zinc
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

Variants are one word attributes: `variant = outline`, `size = sm`.

## Elements

| element  | variants                                                    | sizes                                          |
| -------- | ----------------------------------------------------------- | ---------------------------------------------- |
| `button` | default, destructive, outline, secondary, ghost, link       | default, xs, sm, lg, icon, icon xs, icon sm, icon lg |
