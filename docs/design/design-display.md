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

## Table

`table` is restyled and wrapped in `<div data-slot="table-container">` (horizontal scroll). The existing `header`, `main`, `footer`, `row`, `column` and `caption` elements are styled through the container, so nothing changes in how you write tables.

| Element | Attributes | Notes |
| --- | --- | --- |
| `row` | `selected` | hover tint; `selected` gives the muted background |
| `caption` | | below the table, muted |
| `footer` | | muted background, medium weight |

## Typography

`typography` is an opt-in container (`<div data-slot="typography">`). Inside it `heading` levels 1 to 4, `paragraph`, `block quote`, `list`, `code` and `class = "lead" | "large" | "small" | "muted"` get the shadcn typography styles. Pages that do not use it keep the plain defaults.

## Empty

| Element | Attributes | Notes |
| --- | --- | --- |
| `empty` | | centred empty state |
| `empty header`, `empty title`, `empty description`, `empty content` | | |
| `empty media` | `variant = default \| icon` | `icon` is a 40px muted square |

## Entry (shadcn Item)

`item` is already the list item (`li`), so shadcn's Item is called `entry`.

| Element | Attributes | Notes |
| --- | --- | --- |
| `entry` | `variant = default \| outline \| muted`, `size = default \| sm`, `url` | `url` makes it a link with a hover tint |
| `entry group` | | `role="list"` |
| `entry separator` | | |
| `entry media` | `variant = default \| icon \| image` | |
| `entry content`, `entry title`, `entry description`, `entry actions`, `entry header`, `entry footer` | | description clamps to two lines |

## Breadcrumb

| Element | Notes |
| --- | --- |
| `breadcrumb` | `<nav aria-label="breadcrumb"><ol>`; children are the items below |
| `breadcrumb item` | put a `link` inside for a link |
| `breadcrumb page` | current page, `aria-current="page"` |
| `breadcrumb separator` | chevron by default (mirrored in rtl), or your own content |
| `breadcrumb ellipsis` | three dots with a screen reader label ("More", or the `content`) |

## Default texts

Every built-in label (pagination Previous/Next, close buttons, calendar and carousel aria labels, "Toggle theme", combobox "Select..." and "Search...", spinner "Loading", and so on) is English on an English page and Persian on a `صفحه:` page. The Persian words live in one table, `std/layoutgen/design_text.salam` (`DsT` for generated HTML, `@@key@@` markers for shared scripts). To use other words, write `content`, `placeholder`, `empty` or an `aria-label`: what you write always wins over the default.

## Pagination

| Element | Attributes | Notes |
| --- | --- | --- |
| `pagination` | | `<nav aria-label="pagination"><ul>` |
| `pagination link` | `url`, `active` | `active` sets `aria-current="page"` and the outline look |
| `pagination previous`, `pagination next` | `url`, `content` | default labels "Previous" and "Next" (Persian pages: "قبلی" and "بعدی"); chevrons mirror in rtl |
| `pagination ellipsis` | `content` | |

## Scroll area

| Element | Attributes | Notes |
| --- | --- | --- |
| `scroll area` | `orientation = vertical \| horizontal \| both` | give it a `height`; thin rounded scrollbars; scrolls even with `design = none` |
