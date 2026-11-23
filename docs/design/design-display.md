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

## Separator, progress, meter

| Element | Attributes | Notes |
| --- | --- | --- |
| `separator` | `orientation = horizontal \| vertical` | `role="none"`; 1px line |
| `line` (hr) | `orientation` | same look; keeps its margins |
| `progress` | `value`, `max` | no `value` is the indeterminate animation |
| `meter` | `value`, `min`, `max`, `low`, `high`, `optimum` | optimum, suboptimum and even-less-good colours |

## Skeleton, spinner, aspect ratio

| Element | Attributes | Notes |
| --- | --- | --- |
| `skeleton` | | pulse animation, set size with `width` and `height` |
| `spinner` | `content` is the accessible label (default "Loading") | 16px rotating svg, `role="status"`, inherits text colour, fits inside a `button` |
| `aspect ratio` | `ratio = "16/9"` | `aspect-ratio` is inline, so it works with `design = none`; img and video children cover |

## Avatar

| Element | Attributes | Notes |
| --- | --- | --- |
| `avatar` | `size = default \| sm \| lg` | 32, 24, 40 px |
| `avatar image` | `url`, `alt` | hidden by a small script when it fails to load or is empty |
| `avatar fallback` | | shown under the image, hidden once the image loads |
| `avatar badge` | | status dot at the bottom end |
| `avatar group` | | overlapped avatars with a ring |
| `avatar group count` | | the "+3" bubble |
