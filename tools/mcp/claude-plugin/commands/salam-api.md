---
description: Look up the real public API of a Salam stdlib package
---

Look up the public API of the Salam standard library package named in
`$ARGUMENTS`.

Call `salam_stdlib_symbols` with that package (and a `query` filter if the user
named a specific symbol). If the package does not exist, call
`salam_stdlib_packages` and suggest the closest matches.

Report the declarations verbatim (signatures matter) and add a short usage
example only if the user asked for one. Do not describe functions that were not
in the tool output.
