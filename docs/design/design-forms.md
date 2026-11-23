# Forms (shadcn)

Styled only when the page design is on (`design = shadcn`, the default). Markup and behaviour do not change with `design = none`.

## Text inputs

| Element | Attributes | Notes |
| --- | --- | --- |
| `input` | `type`, `name`, `value`, `placeholder`, `disabled`, `aria invalid = "true"` | every type except checkbox, radio, range, submit, button, reset, hidden, image, color; `jalali date` keeps its own generator |
| `text area` / `input type = "textarea"` | same, `rows`, `content` | grows with its content |
| `label` | `for` | dims next to a disabled control and inside `data-disabled="true"` |

States: hover none, `:focus-visible` ring, `aria invalid = "true"` red ring, `disabled` dimmed.
