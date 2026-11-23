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

`text area`, `textarea` and متن بلند stay accepted for one release as a
deprecated spelling. They lower to the same output as `input` with
`type = "textarea"` and report warning W297 with the replacement:

```
warning[W297]: 'text area' is deprecated; write 'input' with type = "textarea"
```

The next release makes it error E297 with the same hint, then the element is
deleted from the schema. Tests, examples, docs and the site move to the new
spelling now; one test per language keeps the old spelling to pin the warning.

The Persian input type value stays ناحیه متن (`type = "ناحیه متن"`).

## Custom components

A component that wraps an `input` is expanded before checking, so an `input`
with any `type` inside a component, with the type passed as a parameter, is
checked and generated like a hand-written one on both targets. Parameter values
pass through the same open-word check. `tests/en/page/components/component_input_type.salam`
and `tests/*/ui/input_types.salam` pin this.
