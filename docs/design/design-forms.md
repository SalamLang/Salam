# Forms (shadcn)

Styled only when the page design is on (`design = shadcn`, the default). Markup and behaviour do not change with `design = none`.

## Text inputs

| Element | Attributes | Notes |
| --- | --- | --- |
| `input` | `type`, `name`, `value`, `placeholder`, `disabled`, `aria invalid = "true"` | every type except checkbox, radio, range, submit, button, reset, hidden, image, color; `jalali date` keeps its own generator |
| `text area` / `input type = "textarea"` | same, `rows`, `content` | grows with its content |
| `label` | `for` | dims next to a disabled control and inside `data-disabled="true"` |

States: hover none, `:focus-visible` ring, `aria invalid = "true"` red ring, `disabled` dimmed.

## Checkable and range

| Element | Attributes | Notes |
| --- | --- | --- |
| `input type = "checkbox"` | `checked`, `disabled`, `aria invalid` | check and indeterminate marks |
| `input type = "radio"` | `name`, `checked` | |
| `radio group` | | `role="radiogroup"`, grid with 12px gap |
| `input type = "range"` | `min`, `max`, `value` | track fill follows the value (JS `salamDsSlider`); vertical not supported |
| `switch` | `checked`, `name`, `value`, `disabled`, `size = default / sm` | button `role="switch"`; `aria-checked` and `data-state` toggle on click, a hidden checkbox carries the form value, `change` event |

## Select, field, groups

| Element | Variants and attributes |
| --- | --- |
| `select` | native, chevron background; no sm size (`size` is the row count) |
| `field set`, `legend` | legend `variant = legend / label` |
| `field` | `orientation = vertical / horizontal / responsive`, `error`, `disabled` |
| `field label`, `field content`, `field title`, `field description`, `field error`, `field group`, `field separator` | `field label` takes `for`; separator shows `content` in the middle; `field group` is the container for responsive |
| `input group` | `disabled`; children: `input`, `input group addon`, `input group text`, `button` |
| `input group addon` | `align = inline-start / inline-end / block-start / block-end` |
| `button group` | `orientation = horizontal / vertical`; children `button`, `button group text`, `button group separator` |
| `toggle` | `variant = default / outline`, `size = default / sm / lg`, `pressed`, `disabled`; `aria-pressed`, `data-state`, `change` event |
| `toggle group` | `type = single / multiple` (default single), `variant`, `size`, `spacing = 0..4`; arrow keys, Home, End |
| `keyboard`, `keyboard group` | |
| `input otp` | `length` (6), `group size` (0 = no separator), `name`, `disabled`; digits auto-advance, backspace goes back, paste fills; `change` and `complete` events |
