# Design extras

## toast

| Element | Attributes | Notes |
| --- | --- | --- |
| `toaster` | `placement` top left, top center, top right, bottom left, bottom center, bottom right (default bottom right); `duration` ms (4000); `limit` visible toasts (3); `close button` | container, one per page |
| `button` | `toast` message; `toast description`; `toast variant` default, success, error, warning, info | click shows a toast |

JS: `salamToast(title, {description, variant, duration, action: {label, onClick}, closeButton, id})`, `salamToast.success/error/warning/info(title, opts)`, `salamToast.dismiss(id)`. Auto-dismiss pauses on hover and focus, swipe sideways or the close button dismisses. A page needs a `toaster` element for the styles and script.

## command

| Element | Attributes | Notes |
| --- | --- | --- |
| `command` | | palette root, filters as you type |
| `command input` | `placeholder` | search field |
| `command list` | | listbox, scrolls |
| `command group` | `heading` | |
| `command item` | `value`, `keywords`, `disabled` | Enter or click activates, fires `ds-select` with `detail.value` |
| `command empty` | | shown when nothing matches |
| `command shortcut` | | right aligned hint |
| `command separator` | | |
| `command dialog` | `hotkey` (for example "k" opens on Ctrl/Cmd+K) | wraps a command in a modal; open it with `opens = "id"` on a button |

Arrow keys move, Home/End jump, Enter activates.

## combobox

| Element | Attributes | Notes |
| --- | --- | --- |
| `combobox` | `name`, `value`, `placeholder`, `search placeholder`, `empty text`, `clearable`, `disabled` | trigger plus popup with search; hidden input when `name` is set; `ds-change` event |
| `combobox item` | `value`, `disabled` | selected item shows a check; picking it again clears |

## select box

| Element | Attributes | Notes |
| --- | --- | --- |
| `select box` | `name`, `value`, `disabled` | custom listbox select with a hidden input; `ds-change` event |
| `select trigger` | `size` default, sm | content is the placeholder |
| `select content` | | popup, holds items and groups |
| `select item` | `value`, `disabled` | |
| `select group` | `heading` | |
| `select separator` | | |

Keyboard: arrows, Home/End, Enter/Space, Escape, typeahead.

## calendar and date picker

| Element | Attributes | Notes |
| --- | --- | --- |
| `calendar` | `calendar` gregorian, jalali; `mode` single, range; `value` ISO date or `from/to`; `min`; `max`; `week start` 0 to 6 (Sunday is 0; default 6 for jalali); `disabled days` weekday numbers like "0,6"; `disabled dates` ISO list | month grid, prev and next, today, keyboard (arrows, Home, End, PageUp, PageDown, Shift for year); `ds-change` with `detail.value/from/to` |
| `date picker` | same as calendar plus `name`, `placeholder`, `disabled` | trigger plus popover calendar, hidden input holds the ISO value |

Jalali mode uses Persian month names and digits and a right to left grid. Values are always ISO Gregorian dates.

## carousel

| Element | Attributes | Notes |
| --- | --- | --- |
| `carousel` | `orientation` horizontal, vertical; `loop`; `autoplay` ms (pauses on hover and focus); `dots` | scroll snap slides, arrow, Home and End keys, mouse drag, `salamDsCarousel.go(el, index)` |
| `carousel content` | | the scroller, holds the items |
| `carousel item` | | one slide |
| `carousel previous`, `carousel next` | | round buttons placed outside the slides, disabled at the ends unless `loop` |

## resizable

| Element | Attributes | Notes |
| --- | --- | --- |
| `resizable` | `orientation` horizontal, vertical | panel group, fires `ds-resize` with `detail.sizes` |
| `resizable panel` | `size` percent (unset panels share the rest), `min size`, `max size` | |
| `resizable handle` | `grip` | between two panels, drag or Arrow keys (5 percent), Home and End |

## sidebar

`sidebar` is already an alias of `aside`, so the shadcn sidebar root is `sidebar panel`.

| Element | Attributes | Notes |
| --- | --- | --- |
| `sidebar provider` | `collapsed`, `hotkey` ("false" turns off Ctrl/Cmd+B) | wrapper, state in `data-state` expanded or collapsed, `ds-change` event |
| `sidebar panel` | `side` left, right; `variant` sidebar, floating, inset; `collapsible` offcanvas, icon, none | below 768px it becomes a sheet with a backdrop |
| `sidebar trigger` | | toggle button |
| `sidebar header`, `sidebar content`, `sidebar footer` | | |
| `sidebar group`, `sidebar group label`, `sidebar group content` | | |
| `sidebar menu`, `sidebar menu item` | | list |
| `sidebar menu button` | `active`, `variant` default, outline; `size` default, sm, lg; `url` | a link when `url` is set, put an icon as a child before the text |
| `sidebar menu badge`, `sidebar separator` | | |
| `sidebar inset` | | main area next to the panel |
