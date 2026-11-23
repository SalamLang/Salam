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
