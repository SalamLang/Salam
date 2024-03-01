---
description: Type-check Salam source and report exact diagnostics
---

Type-check the Salam source at `$ARGUMENTS` (default: every `.salam` file
changed in the working tree) using the `salam_check` MCP tool.

For each file, report the diagnostics with their exact `file:line:column` and
error code. If there are none, say so plainly and stop; do not run a build
just to double-check.

Each diagnostic comes with the offending source line and often a `hint`.
When the first error is a parse error, fix that one and check again before
looking at the rest; later errors are usually knock-on effects. For an error
code you do not understand, call `salam_docs` with the code (e.g. `E085`).

A check skips codegen and linking. If a check passes but the user reports a
build failure, run `salam_build` to see the backend's error.
