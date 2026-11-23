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

## Popover, tooltip, hover card

A root element holds a trigger and a content. A trigger with no children renders an outline button with its `content`; a trigger with children wraps them (put your own `button` inside). Placement is shared: `side`, `align`, `side offset`, flips when there is no room and stays inside the viewport. Every content closes on Escape and pointer outside; popover returns focus to its trigger. Events: `ds-open`, `ds-close`.

| element | attributes |
| --- | --- |
| `popover`, `tooltip`, `hover card` | tooltip and hover card: `delay` in ms (tooltip 0, hover card 700, close 300) |
| `popover trigger`, `tooltip trigger`, `hover card trigger`, `menu trigger` | `variant`, `size` (button values, used when no children) |
| `popover content`, `tooltip content`, `hover card content` | `side` (top, right, bottom, left), `align` (start, center, end), `side offset` |
| `popover header`, `popover title`, `popover description` | `content` |

Tooltip opens on hover and keyboard focus and has an arrow.

## Menus

| element | attributes |
| --- | --- |
| `dropdown menu`, `context menu`, `menubar menu` | roots |
| `menu trigger` | like the other triggers |
| `context menu trigger` | the right click area |
| `menu content`, `context menu content`, `menu sub content` | `side`, `align`, `side offset` |
| `menu item` | `variant` = default, destructive; `inset`; `disabled`; `value` |
| `menu checkbox item` | `checked`, `inset`, `disabled` |
| `menu radio group` | `value` |
| `menu radio item` | `value`, `checked`, `disabled` |
| `menu group`, `menu label` (`inset`), `menu separator`, `menu shortcut` | |
| `menu sub`, `menu sub trigger`, `menu sub content` | submenu |
| `menubar`, `menubar menu`, `menubar trigger` | bar with hover switching and Left and Right |

Keyboard: arrows, Home, End, typeahead, Enter and Space select, Escape closes and returns focus, Right opens a submenu and Left closes it (swapped in RTL). Items fire `ds-select` (`detail.value`, `detail.checked`) and a normal click.

## Navigation menu

`navigation menu` > `navigation menu list` > `navigation menu item` > `navigation menu trigger` + `navigation menu content` (links) or a `navigation menu link` (`url`, `active`). Opens on hover or click, closes on Escape or outside click.
