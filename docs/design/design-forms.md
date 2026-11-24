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
| `input type = "checkbox"` | `checked`, `disabled`, `aria invalid`, `indeterminate` | check and dash marks; `indeterminate` (`data-indeterminate`, fa `نامعین`) sets the property and `aria-checked="mixed"`, the first change clears it, a MutationObserver follows later changes |
| `input type = "radio"` | `name`, `checked` | |
| `radio group` | | `role="radiogroup"`, grid with 12px gap |
| `input type = "range"` | `min`, `max`, `value`, `orientation = horizontal / vertical` | track fill follows the value (JS `salamDsSlider`); vertical has the minimum at the bottom |
| `range slider` | `min`, `max`, `step`, `low`, `high`, `name`, `disabled`, `orientation` | two native range inputs over one track; thumbs cannot cross; root keeps `data-low`, `data-high`, `data-value="lo,hi"`; `ds-change` event with `detail.low/high`; inputs are named `name-min` and `name-max` |
| `switch` | `checked`, `name`, `value`, `disabled`, `size = default / sm` | button `role="switch"`; `aria-checked` and `data-state` toggle on click, a hidden checkbox carries the form value, `change` event |

## Select, field, groups

| Element | Variants and attributes |
| --- | --- |
| `select` | native, chevron background; `control size = default / sm` (`data-size`, 32px high; `size` stays the row count). The custom `select trigger` has `size = default / sm` too |
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

## Invalid state

`aria invalid = "true"` (fa `نامعتبر دسترسی‌پذیری`, an explicit attribute in `std/layout/attributes/aria.salam`; other `aria x` names still pass through the `aria` wildcard) prints `aria-invalid="true"` on input, text area, select, checkbox, radio and `select trigger`. All of them get the destructive border and the `--ds-ring-invalid` ring. The same attribute is what `field` `error` styles around.

## Page scripts

Every generator `shared_js` is emitted with a leading `;` line (`layoutgen.Emit`), so a page `script` that does not end in `;` no longer fuses with the next shared script (for example `var x = f()` followed by `(function(){...})()` used to become a call). The compiler appends page scripts with only a newline in `compiler/sal_layout/layout_gen_element.salam` (`gen == "script"`); ending that append with `"\n;\n"` would fix it at the source.
