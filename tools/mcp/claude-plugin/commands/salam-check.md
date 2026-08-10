---
description: Type-check Salam source and report exact diagnostics
---

Type-check the Salam source at `$ARGUMENTS` (default: every `.salam` file
changed in the working tree) using the `salam_check` MCP tool.

For each file, report the diagnostics with their exact `file:line:column` and
error code. If there are none, say so plainly and stop — do not run a build
just to double-check.

If a check passes but the user reports a build failure, look for a bare `ret`
in `main`, which Salam's analyzer accepts and the C backend rejects.
