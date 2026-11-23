# Design extras

## toast

| Element | Attributes | Notes |
| --- | --- | --- |
| `toaster` | `placement` top left, top center, top right, bottom left, bottom center, bottom right (default bottom right); `duration` ms (4000); `limit` visible toasts (3); `close button` | container, one per page |
| `button` | `toast` message; `toast description`; `toast variant` default, success, error, warning, info | click shows a toast |

JS: `salamToast(title, {description, variant, duration, action: {label, onClick}, closeButton, id})`, `salamToast.success/error/warning/info(title, opts)`, `salamToast.dismiss(id)`. Auto-dismiss pauses on hover and focus, swipe sideways or the close button dismisses. A page needs a `toaster` element for the styles and script.
