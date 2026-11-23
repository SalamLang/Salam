# Overlay components

Behaviour works with `design = none`; only the look is switched off.
Events: `ds-change` bubbles from the root (`detail.value`, `detail.open`).

## Tabs

| element | attributes |
| --- | --- |
| `tabs` | `default value`, `orientation` = horizontal, vertical |
| `tab list` | `variant` = default, line |
| `tab trigger` | `value`, `disabled`, `content` |
| `tab content` | `value` |

Arrows (RTL aware), Home and End move and select. Roving tabindex, ARIA tablist, tab, tabpanel.

## Accordion

| element | attributes |
| --- | --- |
| `accordion` | `type` = single, multiple; `collapsible`; `default value` |
| `accordion item` | `value`, `open` |
| `accordion trigger` | `disabled`, `content` |
| `accordion content` | `content` |

Arrows, Home and End move between triggers. Open and close animate in 200ms, off under reduced motion.

## Collapsible

| element | attributes |
| --- | --- |
| `collapsible` | `open` |
| `collapsible trigger` | `disabled`, `content` |
| `collapsible content` | `content` |

## Dialog, alert dialog, sheet, drawer

All four are native `dialog` elements opened with showModal: focus trap, inert page, Escape and focus return come from the browser plus a small script (`salamDsDialog.open(el)`, `.close(el)`; events `ds-open`, `ds-close`).

Open one with `opens = "<dialog id>"` on a `button` or on `dialog trigger`. Close with `closes = true` on a `button`, a `dialog close` element, or the corner X.

| element | attributes |
| --- | --- |
| `dialog` | `show close` = true, false; `width` for size; existing `open` |
| `dialog header`, `dialog title`, `dialog description`, `dialog footer` | `content` |
| `dialog close` | `variant`, `size` (button values, default outline) |
| `alert dialog` | `size` = default, sm; no outside click close |
| `alert dialog header`, `title`, `description`, `footer`, `media` | `content` |
| `alert dialog action`, `alert dialog cancel` | `variant`, `size` (cancel defaults to outline) |
| `sheet` | `side` = top, right, bottom, left (default right); `show close` |
| `sheet header`, `title`, `description`, `footer` | `content` |
| `drawer` | `side` (default bottom, handle on bottom); drag to dismiss |
| `drawer header`, `title`, `description`, `footer` | `content` |
