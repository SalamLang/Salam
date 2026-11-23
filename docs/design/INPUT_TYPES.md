# One input element, many types

The page language has one text control: `input`. The separate `text area`
element (also `textarea`, Persian متن بلند) goes away; a multi-line box is an
`input` whose `type` is `textarea`.

```salam
input:
    type = "textarea"
    rows = 4
    placeholder = "Your note"
    bind = note
end
```

## What `type` means

`type` is an open enum. The schema lists the known values (`std/layout/values/input-types.salam`),
and `open = true` on the attribute lets any other single word through
(letters, digits, `-`, `_`, `.`).

| value                                  | web                     | native control                          |
| -------------------------------------- | ----------------------- | --------------------------------------- |
| text, search, email, tel, url, number  | `<input type=...>`      | single-line text field                  |
| password                               | `<input>`               | text field showing bullets              |
| textarea                               | `<textarea></textarea>` | multi-line editor                       |
| checkbox, radio                        | `<input>`               | checkbox, radio button                 |
| range                                  | `<input>`               | slider                                  |
| color                                  | `<input>`               | text field with a color swatch          |
| submit, button, reset                  | `<input>`               | push button                             |
| file, image                            | `<input>`               | drawn, not editable                     |
| hidden                                 | `<input>`               | not drawn, not focusable                |
| date, time, jalali date                | `<input>`               | text field                              |
| anything else (`month`, `my-widget`)   | `<input type="x">`      | text field, type kept in the attribute  |

On the web an unknown type is written as it is, so the browser decides (an
unknown type behaves as `text`). Natively an unknown type is a text field and
the word stays in the `type` attribute, so `ui.ById("x").Attr("type")` and
handlers can read it.

A word that is not a known type but is within edit distance 1 (2 for words of
six letters or more) of one gives warning W298 with the likely spelling:

```
warning[W298]: 'chekbox' is not a known input type
  help: did you mean type = "checkbox"?
```

The word is still written as it is. Only a constant type is checked, so a type
that is only known at runtime never warns.

## Attributes of the textarea type

`rows`, `cols`, `wrap`, `maxlength`, `minlength`, `placeholder`, `readonly`,
`disabled`, `required`, `autofocus`, `autocomplete`, `dirname`, `form`, `name`,
`value`, `content` and `bind` work the same as on the old element. `resize` is a
style property and is unchanged. `value` and `content` are the starting text
(`value` is new on this type: a `<textarea>` has no value attribute, so it is
written as the element's text). `bind` on this type binds the text.

## How the compiler picks the element

`textarea input` is a second schema element named `input` with
`when = "type=textarea"`. The `type` must be a constant: a literal, or a
component parameter that is given a literal. A type that is only known at run
time (a state variable) cannot pick the element, so `"textarea"` there gives a
plain `<input type="textarea">`. Resolution scores the context, so `input` with
`type = "textarea"` becomes `<textarea>` on the web (generated name
`textarea`) and the native text-area control (`_K_TEXTAREA`); every other
`input` stays `<input>`. This already shipped; the work here is to make it the
only way to write a multi-line box.

## The old element

`text area`, `textarea` and متن بلند were accepted for one release as a
deprecated spelling (warning W297). The release after that removes the element
from the schema. The three spellings are then an error, E297, with the
replacement as the hint:

```
error[E297]: the 'text area' element was removed
  help: write an 'input' with type = "textarea" instead
```

E297 replaces the generic E111 (unknown layout element) for exactly these three
names, so old pages get a pointer instead of a "did you mean footer" guess.

The Persian input type value stays ناحیه متن (`type = "ناحیه متن"`).

## Removal patch plan

The patch below ships in the release after the one that contains W297. It
lands as one pull request, kept as a draft until that release is tagged. The
compiler and std must still build with the installed seed, so it uses no new
syntax.

Delete:

- `std/layout/elements/text_area.salam`, the whole file (the `text_area`
  element with the spellings `text area`, `textarea` and متن بلند).

Remove the `"text area"` entry from the `elements` list of these attributes in
`std/layout/attributes/html.salam`: `name_attr`, `form_attr`, `required_attr`,
`disabled_attr`, `autofocus_attr`, `autocomplete_attr`, `rows_attr`,
`cols_attr`, `maxlength_attr`, `minlength_attr`, `readonly_attr`, `wrap_attr`
and `dirname_attr`. Delete `placeholder_attr` entirely: it listed only
`text area`, and `placeholder` in `std/layout/attributes/forms.salam` already
covers `input` and `textarea input`. `forms.salam` already lists
`textarea input` for `name`, `value`, `placeholder`, `required` and `disabled`.

Compiler:

- `compiler/sal_layout/layout_expand.salam`: `area` in the bind check is now
  `is_input and str.Equals(kind, "textarea")`; the element name tests go.
- `compiler/semantic/sema_layout_check.salam`: the W297 branch on the `text
  area` element is deleted. `lay_report_unknown` reports E297 (through
  `lay_is_removed_textarea`) for the three spellings before it falls back to
  E111.
- `compiler/semantic/sema_layout_style.salam`: `LAYOUT_TEXTAREA_CODE` stays; it
  is now the code of E297.
- `compiler/i18n.salam`: the message "the '%s' element is deprecated and will
  be removed" becomes "the '%s' element was removed" in `k_msg_en`, and its
  Persian pair becomes «عنصر «%s» برداشته شد». The hint string is unchanged.

Tests:

- `tests/en/page/elements/textarea_deprecated.salam` and the fa twin move to
  `tests/*/page/errors/textarea_removed.salam` and expect E297 with both
  messages. `tests/en/page/errors/textarea_removed_alias.salam` covers the
  `textarea` spelling.

Docs: the `text area` row of `docs/LAYOUT_ELEMENTS.md` is deleted.

Kept on purpose, because they name the control and not the removed spelling:

- `std/layout/elements/input.salam` (`textarea_input`, `when = "type=textarea"`)
  and `std/layout/values/input-types.salam` (`it_textarea`).
- `std/gui/ui/node.salam` (`"text area"` and `"textarea input"` mapping to
  `_K_TEXTAREA`) and the style key `"text area"` in `std/gui/style/cascade.salam`.
  They are internal names of the native control; the cascade tests use them.

Checks to run: the `tests/*/page/errors/textarea_removed*` and
`input_textarea*` tests, `tools/bash/check-i18n.sh`, and
`tools/bash/check-fa-names.sh`.

## Custom components

A component that wraps an `input` is expanded before checking, so an `input`
with any `type` inside a component, with the type passed as a parameter, is
checked and generated like a hand-written one on both targets. Parameter values
pass through the same open-word check. `tests/en/page/components/component_input_type.salam`
and `tests/*/ui/input_types.salam` pin this.
