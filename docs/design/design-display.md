# Display components

All elements restyle only when `design = shadcn` (the default). With `design = none` they still emit the same structure and ARIA. Every selector is wrapped in `:where()`, so your own styles always win.

## Card

| Element | Part | Notes |
| --- | --- | --- |
| `card` | root | border, radius-xl, shadow-sm |
| `card header` | header | grid; two columns when it holds a `card action`; add class `border-b` for a divider |
| `card title` | title | semibold |
| `card description` | description | muted |
| `card action` | action | top end of the header |
| `card content` | content | |
| `card footer` | footer | add class `border-t` for a divider |

## Badge

| Attribute | Values |
| --- | --- |
| `variant` | `default`, `secondary`, `destructive`, `outline`, `ghost`, `link` |
| `url` | makes the badge an `<a>` (hover styles apply to links only) |

## Alert

| Element | Notes |
| --- | --- |
| `alert` | `role="alert"`; `variant = default \| destructive`; a direct `svg` child becomes the icon |
| `alert title` | one line |
| `alert description` | muted (or destructive tinted) |
