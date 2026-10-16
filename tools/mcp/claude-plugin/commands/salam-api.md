---
description: Look up the real public API of a Salam stdlib package
---

Look up the public API of the Salam standard library package named in
`$ARGUMENTS`.

Call `salam_stdlib_symbols` with that package (and a `query` filter if the user
named a specific symbol). A bare leaf such as `json` or `http` resolves to its
dotted name. If the user describes a function rather than naming a package
("read a file", "sha256"), call `salam_stdlib_search` instead. If the package
does not exist, the tool suggests the closest names.

Report the declarations verbatim (signatures matter) and add a short usage
example only if the user asked for one. Do not describe functions that were not
in the tool output.
