---
description: Build and run a Salam program, reporting output and exit code
---

Build and run the Salam program at `$ARGUMENTS` with the `salam_run` MCP tool.

Report the program's output and exit code. If compilation fails, report the
diagnostics instead and stop.

Use `salam_run` rather than `salam_exec` whenever the program does unsigned
(`u32`/`u64`) arithmetic, bit manipulation, hashing or crypto: the interpreter
behind `salam_exec` miscomputes those.
