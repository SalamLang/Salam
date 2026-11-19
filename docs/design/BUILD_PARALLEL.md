# Build time of a large project, and what is left to parallelise

This note records where the wall time of `salam build compiler/main.salam --backend=c` really goes (the compiler building itself, 88 modules), what was fixed, and why the rest is not simply "run lexing, parsing and checking on threads".

## The question

A user on macOS saw each standard library package take up to ten seconds, one after the other, and asked why lexing, parsing, semantic analysis and object compilation do not run in parallel.

## Measurements

All numbers are from one 6 core Linux box, `--log-level=info` timestamps plus `--time-trace`, same source tree, a compiler built from main as the reference. Wall seconds.

| Stage | Before | After | Serial or parallel |
| --- | --- | --- | --- |
| Source load, lex, parse of all 88 modules | 0.5 | 0.8 | serial, but negligible |
| Semantic analysis (`sm.Run` per module) | 788 | 225 | serial |
| Dead code elimination and reachability (`DceFinish`, between analysis and codegen) | 188 | 1 | serial |
| C generation of the 88 modules | 121 | 121 to 158 (box was loaded) | already parallel, one forked worker per CPU |
| Writing the C files | inside codegen | inside codegen | parallel (same workers) |
| Compiling object files with clang | 228 | 228 to 331 (box was loaded) | already parallel, a job pool sized by CPU count |
| Link | 1.2 | 1.1 | serial, one process |
| Whole build | 1334 | 724 under load, about 580 expected on an idle box | |

Lexing and parsing together cost under a second of a 22 minute build. They are not worth threading. The time was in semantic analysis, and inside it in two accidental quadratics, not in anything that needs more cores.

## Where the time was

Semantic analysis grew with the number of modules already loaded. A five definition module such as `std/rand` took 4 seconds early in the build and 9 seconds near the end. A profile (callgrind on a program importing `tls`, 9.9 billion instructions) showed:

1. `copy_program_deep_copies` walked every node of the shared AST on every `sema_collect`, asking `intrinsic_method_canon` about each member node. That alone was 36 percent of the run. The AST only grows, so the walk is now done once per node, with a watermark and a sticky found flag kept on the `Sema`.
2. `DceFinish` and `DceReachable` used `Vector<str>` membership tests. With tens of thousands of reachable functions and roughly a million call edges, the worklist loop scanned all edges for every popped function and scanned the reached list for every edge. It is now a hash set for roots and reachability and a per caller edge list. This was the 3 minute silent gap between "analysis complete" and the first "generating C".
3. `su_has_ctx_word` ran four substring searches on every identifier comparison that involved a non ASCII name, 13 million times in the sample. It now looks for the lead bytes of the four words first. On the same sample this removes about 4 percent of the remaining instructions (7.15 to 6.87 billion).

After the first two fixes the same profile shows no function with an obviously wrong complexity. The remaining cost is real checking work: `check_function_body`, the ownership summary (`SemaOwnPost`), range checks (`rg_stmt`) and the lexer, all roughly linear.

## Is the front end skipped for unchanged packages

No. The object cache works at the C level only. After the front end and code generation, a module's `.c` and `.h` are compared with the files on disk, and when both are equal and the `.o` exists the C compiler is not run ("reused N cached objects"). A whole build is skipped only when the build manifest says every source file and the flags are unchanged. So a one line change in `main.salam` still lexes, parses, analyses and generates C for all 88 modules, and only recompiles the objects that changed. After the fixes above that front end costs about 4 minutes for the compiler itself instead of about 16.

## Ranked options

1. Hot spot fixes in sema and DCE (done). About 3.4 times less front end wall time, no threading, no risk to output order. This was the biggest single gain by far.
2. Cache the analysed state of unchanged std packages between builds. Large win for incremental builds (the front end is the floor of every rebuild), but high risk: the semantic tables (`types`, `syms`, `scopes`, `sigs`) and the AST arena are all index based and shared, and generic instances, overloads and the ownership summary refer across packages. A serialised form needs a stable numbering and a content hash of every transitive import. Not attempted.
3. Pipeline code generation behind analysis. Code generation of module M needs the whole program reachability (`DceFinish`), because dead functions are dropped per module. That makes the boundary hard: nothing can be emitted before analysis of every module finishes, unless emission is split into a conservative per module pass and a later prune. Gain is at most the overlap of 225 s analysis with 120 s codegen. Medium risk, medium gain.
4. Threaded semantic analysis per package. Package load order matters for diagnostics and for `s.pending` generic instances, the symbol and type tables are one `Vector` each with index handles, and globals such as `g_sm_dce_*`, `g_sm_own*`, the diagnostic engine and the logger are process wide. It needs per package arenas and a merge step. High risk, large rewrite, and the output must stay byte identical. Not worth it now that the serial cost is no longer quadratic.
5. Threaded lex and parse of independent files. Trivially parallel in principle (each file lexes into its own token stream), but the parser allocates nodes in the single shared `Ast`, whose node ids are part of the generated names. Total gain is under one second. Not worth it.

## Code generation and C compile are already parallel

Code generation forks one worker per CPU (not on Windows), assigns modules largest first by source size, each child writes its `.c`, `.h` and a status file, the parent waits and reads the statuses. `SALAM_SERIAL_CODEGEN=1` turns it off. Object compilation goes through `compile_parallel`, a pool of CPU count jobs sorted by source size. The link is one process.

On the machine of the report the C compile is the stage that is bounded by cores and by `-O3`. Lowering the optimisation level of std packages that change rarely is a build policy question, not a compiler one.

## What is left

* The front end is still one thread. After the fixes it is dominated by real checking work in `check_function_body`, `SemaOwnPost` and `rg_stmt`.
* Per package caching of analysis (option 2) is the only change that would make a one line edit fast, and needs a design of its own.
* `dce_note_call` records every call edge including duplicates. De duplicating them would cut memory for very large programs.
