# One language per page

A page is written in one language. A Persian page (`صفحه:`, or any page in a
Persian file) accepts only Persian names, and an English page (`page:`) only
English names. Before 0.7.0 both languages were accepted everywhere; the
mixed spelling made pages hard to read, hard to translate and hard to search.

## What is checked

| code | scope                                                   | example                              | replacement          |
| ---- | ------------------------------------------------------- | ------------------------------------ | -------------------- |
| E301 | element names, including HTML names such as `div`       | `button:` in a Persian page          | `دکمه`               |
| E302 | attribute and style property names, with state prefixes | `hover background` in a Persian page | `اشاره پس‌زمینه`     |
| E302 | event handlers, prefix and event                        | `on click` in a Persian page         | `هنگام کلیک`         |
| E303 | style keyword values                                    | `layout = "row"` in a Persian page   | `"ردیف"`             |

The language of a page is the language of its own file: the `page:` block of an
English file is English, a component in a Persian file is Persian. A
`layout:` block or component embedded in a Persian general program follows
that program.

Every diagnostic has the message `... write 'X' in a Persian page` and the
help line `write 'X'`, where `X` is the exact replacement. `salam check --fix`
applies the replacement as a safe fix. The std design attributes are already
strict about their enum values (E018); that is unchanged.

## What is never checked

- ids, content, urls, class names, user component names and parameters
- `data x` and `aria x` attributes (the wildcard forms)
- colours (names and hex), numbers, units, durations
- any word that is spelled the same in both languages
- an element, attribute or keyword that has no spelling in the page language
  (`h1` to `h6` have only an English name)
- the words `true` and `false` of a style value, which are Salam keywords of the
  file and follow the file language already
- keyword words inside a longer quoted string are checked word by word, a
  multi word keyword inside a longer string is not
- transition property names (`transition = background, 200ms`) are values of
  another kind and are not checked

## How it works

The schema in `std/layout` lists `@en` and `@fa` spellings. The compiler keeps
the language of each spelling next to the alias table, so a name that matches
only spellings of the other language is reported with the first spelling of the
page language. Style properties come from `std/gui/style` (`PropInfo` name and
`fa`), keywords from `style.KeywordLang`. The check runs before names are
resolved to their canonical form, on the spelling the author wrote, and only
where the node still matches the source text.
