# Benchmark and Profiling Features

This file covers three things: a monotonic clock, compiler self-profiling,
and two features that are designed but not built yet (program profiling and a
benchmark harness).

## Status

| Part                                                         | State                                                                                                                                |
| ------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------ |
| §1 Monotonic clock (`std/time`, the compiler's own clock)    | **Implemented**                                                                                                                      |
| §2 Compiler self-profiling (`--time-report`, `--time-trace`) | **Implemented** in `compiler/prof_self.salam`                                                                                        |
| §3 Program profiling (`--profile`, `salam prof`, `std/prof`) | Designed, not implemented: `salam build --profile` is an unknown option, `salam prof` an unknown command, and there is no `std/prof` |
| §4 Benchmark harness (`std/bench`, `salam bench`)            | Designed, not implemented: there is no `std/bench` and no `salam bench`                                                              |

An earlier version of this document specified every item twice, once for the
C compiler in `c/src/` and once for the self-hosted compiler in `compiler/`,
because the two had to stay in parity. The C compiler was deleted in #1546.
Everything below is about the self-hosted compiler only.

Until §3 exists, profile a program with the usual native tools on a
`salam build -g` binary: `perf`, `valgrind --tool=callgrind`, Instruments or
Visual Studio. `salam debug` builds with `-g` and starts GDB or LLDB, and
`salam memcheck` runs a program under Valgrind or AddressSanitizer.

---

## 1. Monotonic clock (implemented)

A wall clock is the wrong tool for timing. On Windows,
`GetSystemTimeAsFileTime` has a unit of 100 ns but only updates once per
system timer tick, typically 15.6 ms, so anything shorter reads as 0 or 15600000. On POSIX, `CLOCK_REALTIME` steps when NTP adjusts it and can go
backwards. So the timing code uses a monotonic clock.

### 1.1 Standard library: `std/time`

`std/time/time.salam` has `_mono_ns()` next to the wall-clock `_now_ns()`:

- Windows: `QueryPerformanceCounter` / `QueryPerformanceFrequency`, scaled as
  `(counter / freq) * 1e9 + ((counter % freq) * 1e9) / freq` so the product
  does not overflow.
- POSIX: `clock_gettime` with the platform's monotonic clock ID (1 on Linux,
  8 on macOS, 4 on FreeBSD, 3 on OpenBSD and NetBSD).

`std/time/hires.salam` exposes it:

| Function                                       | Clock                                    |
| ---------------------------------------------- | ---------------------------------------- |
| `MonoNanos`, `MonoMicros`, `MonoMillis`        | monotonic                                |
| `SinceNanos`, `SinceMs`, `SinceUs`, `SinceSec` | monotonic, elapsed since a `MonoNanos()` |
| `NowNanos`, `NowMicros`, `NowMillis`           | wall clock, for timestamps               |
| `ElapsedMs`, `ElapsedUs`, `ElapsedSec`         | difference of two readings you pass in   |

`std/debug`'s `StartTimer`/`StopTimer` use `time.MonoNanos()`.
`tests/en/stdlib/mono_clock_demo.salam` checks the clock.

`benchmark/bench/timing.salam`, the cross-language suite's timer, still
reads `time.NowNanos()`; moving it to `time.MonoNanos()` is open (see §4.6).

### 1.2 The compiler's own clock

`compiler/sal_core.salam` adds what `std/time` does not have:

- `CpuNanos()`: process CPU time, from `GetProcessTimes` on Windows and
  `clock()` elsewhere.
- `PeakRss()`: always returns 0 today (see §2.5).

Wall time in the compiler comes from `time.MonoNanos()`.

---

## 2. Compiler self-profiling (implemented)

### 2.1 Usage

```sh
salam build app.salam --time-report          # table on stderr
salam build app.salam --time-report=json     # one JSON object on stderr
salam build app.salam --time-trace           # writes salam-trace.json
salam build app.salam --time-trace=out.json  # trace to a chosen file
```

This is the Salam analogue of `clang -ftime-report` / `rustc -Ztime-passes`.
The report goes to stderr so it never mixes with what a command writes to
stdout. The trace is a Chrome Trace Event array, readable by Perfetto,
`chrome://tracing` and speedscope.

Only work that actually runs is measured. A build whose modules all come
from the object cache in `.salam-build` reports almost nothing (no phases,
zero counters, an empty trace). Pass `-B` (`--force`) to rebuild everything
when you want the full picture.

### 2.2 Phases

| Phase      | Measured around                              | Where (`compiler/driver/`)                                             |
| ---------- | -------------------------------------------- | ---------------------------------------------------------------------- |
| `source`   | reading a source file                        | `driver_front.salam`, `driver_api_llvm.salam`                          |
| `lexer`    | lexing                                       | `driver_front.salam`, `driver_api_llvm.salam`                          |
| `parser`   | parsing                                      | `driver_front.salam`, `driver_api_llvm.salam`                          |
| `semantic` | semantic analysis                            | `driver_front.salam`, `driver_api_llvm.salam`                          |
| `codegen`  | C, LLVM IR or native code generation         | `driver_codegen.salam`, `driver_api_llvm.salam`, `driver_native.salam` |
| `write`    | writing generated files                      | `driver_codegen.salam`, `driver_api_llvm.salam`                        |
| `cc`       | the host C compiler, for all modules at once | `driver_build.salam`                                                   |
| `link`     | the final link                               | `driver_build.salam`, `driver_native.salam`                            |

`prof_self.salam` also defines `driver`, `llvm` and `jsgen` phases. No scope
records them today, so they never appear; LLVM work is reported under
`codegen`. Rows with zero calls are left out of the table.

### 2.3 Counters

| Counter                                    | Recorded in                        |
| ------------------------------------------ | ---------------------------------- |
| `tokens`, `symbols`                        | `driver_front.salam`               |
| `funcs_emitted`                            | `driver_codegen.salam` (C backend) |
| `modules_built`, `modules_cached`          | `driver_build.salam`               |
| `ast_nodes`, `arena_bytes`, `arena_blocks` | nothing yet: always 0              |

### 2.4 Report format

The table, from a fresh build of a one-line program on the C backend:

```
Salam compile time report
  wall 4.256 s   cpu 1.067 s   peak rss 0 B

  phase            self       total    %self    calls
  ----------------------------------------------------
  cc            1.770 s     1.770 s    41.6%        1
  semantic      1.123 s     1.123 s    26.4%       10
  codegen     424.08 ms   424.08 ms    10.0%        1
  link        153.00 ms   153.00 ms     3.6%        1
  parser        3.99 ms     3.99 ms     0.1%       10
  lexer         2.16 ms     2.16 ms     0.1%       10
  source      497.06 us   497.06 us     0.0%       10

  slowest inputs
    p.salam                                       851.49 ms   (semantic 851.32 ms)
    pp                                            153.00 ms   (link 153.00 ms)
    .../std/unicode/unicode.salam                  83.58 ms   (semantic 82.78 ms)

  counters
    tokens                 14,701   ast nodes                   0
    symbols                   837   funcs emitted             236
    modules built              10   modules cached              0
    arena                     0 B   arena blocks                0
```

"slowest inputs" lists at most eight labels. A label is the source file a
scope worked on, or the output binary for `link`. Labels are copied when a
scope begins, because the strings they came from may be freed before the
report prints, and a long path keeps its tail, which is the part that tells
files apart.

`--time-report=json` writes a single object with a versioned schema, so CI
can compare runs:

```json
{"schema":"salam.timereport.v1","wall_ns":6693212269,"cpu_ns":1068327000,"peak_rss":0,"truncated":false,
 "phases":{"source":{"self_ns":378948,"total_ns":378948,"cpu_ns":389000,"calls":10}, "...": {}},
 "files":[...],"counters":{"tokens":14701,"ast_nodes":0,"symbols":837,"funcs_emitted":236,
 "modules_built":10,"modules_cached":0,"arena_bytes":0,"arena_blocks":0}}
```

`self_ns` excludes time spent in nested scopes; `total_ns` includes it.
Scopes nest up to 64 deep and the table tracks up to 512 files;
`truncated` says whether either limit was hit.

### 2.5 Known gaps

- `peak_rss` is always 0: `PeakRss()` in `compiler/sal_core.salam` has no
  implementation. `getrusage` on POSIX and `GetProcessMemoryInfo` on Windows
  are the obvious sources.
- `ast_nodes`, `arena_bytes` and `arena_blocks` are always 0. The AST is an
  arena of `Node` records with no single allocation point wired to the
  counter, and the compiler has no block arena to report on.
- `cc` is one scope for all modules, because the driver compiles them in
  parallel; its time is not split per input.

### 2.6 Tests

`tools/bash/run-tests.sh timereport` runs three checks against a small
program built from an empty directory:

| Check               | Asserts                                                                                                                             |
| ------------------- | ----------------------------------------------------------------------------------------------------------------------------------- |
| `timereport/json`   | exit 0, a `salam.timereport.v1` object on stderr, `wall_ns > 0`, the expected phase keys, and the sum of `self_ns` within `wall_ns` |
| `timereport/trace`  | `--time-trace` writes trace events                                                                                                  |
| `timereport/parity` | the same phase keys from `$SALAM` and from `$SALAM_SELFHOST`; skipped when `SALAM_SELFHOST` is unset                                |

The parity check was written to compare the C and the self-hosted
compilers. It still works for comparing any two builds, for example a
released compiler and one built from a branch.

---

## 3. Program profiling (design, not implemented)

Nothing in this section exists yet. The function names below are the ones
in `compiler/` today and are the places the design would change.

### 3.1 Modes

```
salam build --profile          # same as --profile=fn
salam build --profile=fn       # call counts + inclusive/exclusive wall time per function
salam build --profile=count    # call counts only (no clock reads)
salam build --profile=graph    # fn, plus caller->callee edge attribution
salam build --profile=off      # explicit off, for overriding a config file
```

`--profile=sample` is reserved and rejected with "not implemented yet" so the
flag space is stable. `salam run --profile=...` and `salam exec --profile=...`
work too; `exec` uses the interpreter path in §3.5.

Companion flags:

```
--profile-out=FILE       # default: salam.prof in cwd
--profile-min=N          # drop functions with fewer than N calls from the report
--profile-exclude=PKG    # do not instrument this package; repeatable
```

`std/prof` itself and `std/core`, `std/mem`, `std/time` are always excluded;
instrumenting the profiler's own clock reads is an infinite regress.

### 3.2 The runtime lives in Salam

The profiling runtime is **`std/prof/prof.salam`**, a normal Salam package.
The driver adds it to the build when `--profile` is on, the same way it
already adds `core` (`sm.ResolveImport("", "core")` in
`compiler/driver/driver_build.salam`):

```salam
if opt.profile_mode != PROF_MODE_OFF:
    profp := sm.ResolveImport("", "prof")
    if str.Len(profp) > 0 and os.Exists(profp):
        work.Push(profp)
    else:
        lg.Log(lgr, lg.PH_DRIVER, lg.LOG_ERROR,
               i18.Tr("--profile needs the 'prof' package in the stdlib"))
        all_ok = false
    end
end
```

Writing the runtime in Salam means the C backend, the LLVM backend, the
native backend and the interpreter all run the same code, and it can be
tested as an ordinary package before any code generator changes. The JS
backend is the exception (§3.5).

### 3.3 `std/prof` API

Two layers. The lower layer is what the code generators call; the upper layer
is for profiling a region by hand.

```salam
@en "prof"  @fa "پروفایل"
package prof

// lower layer: called by generated code
pub func Register(name: str, file: str, line: int): int: ... end
pub func Enter(id: int): ... end
pub func Exit(id: int): ... end

// upper layer: hand-written region profiling
pub func BeginRegion(name: str): int: ... end
pub func EndRegion(handle: int): ... end
pub func Enabled(): bool: ... end
pub func SetEnabled(on: bool): ... end

// output
pub func Dump(path: str): bool: ... end
pub func Report(): str: ... end
pub func Reset(): ... end
```

Every `pub` symbol needs `@en` and `@fa` names.

**Internal representation.** One flat array of records, indexed by slot ID:

```salam
struct Slot:
    name: str
    file: str
    line: int
    calls: i64
    self_ns: i64      // excludes time in callees
    total_ns: i64     // includes callees
    depth: int        // current recursion depth, for correct total_ns
end
```

`Enter(id)` pushes `{id, t0 = time.MonoNanos(), child = 0}` and increments
the slot's `depth`. `Exit(id)` pops, adds `dt - child` to `self_ns`, adds
`dt` to `total_ns` only when `depth == 1` (the outermost frame of a
recursion), decrements `depth` and adds `dt` to the parent frame's `child`.
Without the `depth == 1` guard, `fib(30)` would report a total many times
the program's runtime.

For `--profile=graph`, `Enter` also counts the `(caller_slot, callee_slot)`
edge, in a flat open-addressed table keyed by `caller * 65536 + callee`.

The stack is fixed at 4096 frames. On overflow, `Enter` sets an
`_overflowed` flag and skips the push, `Exit` for that call does nothing,
and the report prints a warning line.

**Dump at exit.** Generated code calls `prof.Dump(<path>)` at the end of the
entry function. Programs that call `os.Exit()` need an at-exit hook in
`std/os` that `prof` registers into.

### 3.4 Code generator instrumentation

**Where.** `cg_function` in `compiler/codegen/codegen_decl.salam` emits the
`Enter` call after the prologue. Every exit from a function body already
goes through `cg_emit_defers` (`compiler/codegen/codegen_stmt.salam`), which
runs at each `ret` and at the natural end of the function, so the `Exit`
call is flushed there.

Do not push a fake entry onto `Cg.fn_defers`. It holds AST node IDs, not
text, and a non-empty defer list changes how a return value is emitted.
Instead add a text hook, `fn_prof_exit: str = ""`, next to `fn_defers`,
flush it at the end of `cg_emit_defers`, and save and restore it wherever
`fn_defers` is saved and restored for nested lambdas.

Check during implementation that a `ret` inside a `match` that produces a
value also reaches a flush point.

Reference `prof.Enter` through the same helper the code generator uses for
any cross-package function, so the mangled name comes from one place.

**Slot registration.** Each module registers its slots from a generated
initializer that runs before `main`, through the module initializer list the
code generator already emits.

**Dead code elimination.** DCE marks roots by source-level use, so calls
added by the code generator would be stripped. The driver must call
`sm.DceMarkRoot("prof", name)` (`compiler/semantic/sema_api.salam`) for
`Register`, `Enter`, `Exit` and `Dump` right after `sm.DceEnable()`.

**Which functions get instrumented:**

- skip anything whose home package is `prof`, `core`, `mem` or `time`;
- skip packages listed in `--profile-exclude`;
- skip `extern` declarations;
- skip lambdas under `--profile=count`, include them under `fn` and `graph`
  as `<parent>.lambda@<line>`;
- instrument the entry function itself, so the report has a 100% root.

### 3.5 Other backends

**LLVM and native.** `emit_function` / `emit_function_body` in
`compiler/llvmgen/llvm_decl.salam` emit `call void @<prof.Enter>(i32 <id>)`
in the entry block and rely on the same defer flush points for `Exit`. The
native backend reads the same IR, so it needs nothing of its own. With
`-O2`/`-O3` LLVM can inline an instrumented function into its caller;
counts stay right but attribution of small functions gets noisy, so the
driver should log a note when `--profile` is combined with `-O2` or higher.

**JS.** `jsg_function` in `compiler/jsgen/jsgen_stmt.salam` (called from
`jsg_emit_functions` in `jsgen_core.salam`). The JS backend does not compile
`std/prof`, so it emits a small preamble with the same API on top of
`performance.now()` and prints the same JSON. Name minification must leave
the preamble alone.

**Interpreter.** Every user-function call goes through `call_func`
(`compiler/interp/interp_glue.salam`), so that is the one place to
instrument, not the call sites. The interpreter keeps a private copy of the
`Slot` layout and writes the same file format, so `salam prof report` does
not care which backend produced a profile. `salam exec --profile` is the
cheapest piece to ship first: no code generation changes at all.

### 3.6 Profile file format

`salam.prof` is text: at most a few thousand lines, and people will grep and
diff it.

```
salam-profile 1
mode fn
program app
wall_ns 1204338211
cpu_ns 1198220004
started 2026-08-07T10:33:02Z
backend llvm
#
# id  calls        self_ns      total_ns  name                 file:line
F 0   1            120400       1204338211  main               app.salam:12
F 1   1664079      840221004    840221004   fib                app.salam:3
F 2   3            9004         2200410     load_table         app/data.salam:41
#
# caller callee count      (only present in mode=graph)
E 0 1 1
E 1 1 1664078
```

Lines starting with `#` are comments, readers skip unknown record types, and
the header is order-independent key-value pairs up to the first `#`. Names
are printed in the source language of the definition, so a Persian program's
profile shows Persian names.

### 3.7 `salam prof` command

A new `CMD_PROF` constant in `compiler/driver/driver.salam` (next to
`CMD_DOC` and the others), dispatched in `dispatch_command`, with options in
`parse_options` (both `compiler/cli/cli_parse.salam`), a handler in
`RunCommand` and help text in `UsageText` (both `compiler/cli/cli_api.salam`).
The rendering is a pure function of the `.prof` file, so it belongs in
`std/prof` (`Report`, `Diff`, `Folded`) rather than in the compiler.

```
salam prof report [FILE]           # flat table, default sort by self time
salam prof report --sort=total|self|calls|name --top=N
salam prof graph  [FILE]           # caller/callee tree (needs mode=graph)
salam prof folded [FILE]           # collapsed stacks for flamegraph.pl / speedscope
salam prof diff   OLD NEW          # per-function delta, for regression hunting
salam prof json   [FILE]           # machine-readable
```

`FILE` defaults to `salam.prof`. `salam prof report`:

```
Profile of app (mode fn, llvm backend), wall 1.204 s

   self%    self      total%     total     calls    function
   ------------------------------------------------------------------
   69.8%    0.840 s    69.8%    0.840 s   1,664,079  fib             app.salam:3
    0.2%    0.002 s   100.0%    1.204 s           1  main            app.salam:12
    0.0%    0.000 s     0.2%    0.002 s           3  load_table      app/data.salam:41

  29.9% of wall time is unattributed (uninstrumented code, runtime, syscalls).
```

The last line matters: an instrumenting profiler only sees instrumented
frames, and a report that sums to 70% without saying why is not trusted.

`salam prof diff` is the CI-facing view:

```
   delta      old        new      calls old/new   function
   -----------------------------------------------------------------
   +31.2%    0.640 s    0.840 s   1.66M / 1.66M   fib
    -8.1%    0.021 s    0.019 s      3 / 3        load_table
```

It exits 1 when a function regresses by more than `--threshold=P` (no
gating unless asked).

### 3.8 Overhead

To be measured on `benchmark/programs/01_fib_recursive`, the call-heavy
worst case. Expected: `count` 1.5x to 2x, `fn` 4x to 8x (two clock reads per
call), `graph` 5x to 10x. Realistic code pays far less. The report header
prints the mode so nobody compares an instrumented run's absolute time with
an uninstrumented one. `--profile-min=N` filters the report, not the
collection.

---

## 4. Benchmark harness (design, not implemented)

### 4.1 `std/bench`

A new package modeled on Go's `testing.B`:

```salam
@en "bench"  @fa "محک"
package bench

pub struct B:
    pub n: i64            // iterations the harness wants this run
    pub name: str
    start_ns: i64
    elapsed_ns: i64
    bytes: i64
    timer_on: bool
end

pub func ResetTimer(b &: B): ... end
pub func StopTimer(b &: B): ... end
pub func StartTimer(b &: B): ... end
pub func SetBytes(b &: B, n: i64): ... end   // enables the MB/s column

// Keeps the optimizer from deleting a loop whose result is unused.
pub func Keep(v: i64): ... end
pub func KeepFloat(v: f64): ... end
pub func KeepStr(v: str): ... end

// Runs `body` adaptively. `body` takes the iteration count and loops itself.
pub func Run(name: str, body: func(i64)): Result: ... end

pub struct Result:
    pub name: str
    pub iters: i64
    pub ns_per_op: f64
    pub min_ns_per_op: f64
    pub median_ns_per_op: f64
    pub stddev_ns: f64
    pub mb_per_sec: f64
    pub samples: int
end
```

A benchmark:

```salam
package main

import bench

func BenchFibRecursive(b &: bench.B):
    repeat b.n:
        bench.Keep(fib(30))
    end
end
```

### 4.2 `Keep`

At `-O2`/`-O3` a loop whose result is discarded is deleted, and the
benchmark measures an empty loop. `Keep` writes to a `pub mut _sink` in
another module, which the optimizer cannot prove unread, and the report path
reads `_sink` once at the end. Under `--lto` this is best effort;
checksum-style benchmarks with `SetBytes` are the safer choice there.

### 4.3 Adaptive iteration and statistics

1. **Warmup.** Run with `n = 1`. If it takes less than `min_time` (default
   100 ms), scale `n` by `min_time / elapsed`, rounded up to a "nice" number
   (1, 2, 5 times a power of ten), and repeat until a run reaches `min_time`
   or `n` hits `max_iters` (default 1e10).
2. **Sampling.** With `n` fixed, run `samples` times (default 8, `--count=N`).
   Discard the first sample as cache warmup unless `samples < 3`.
3. **Statistics.** Report `min`, `median`, `mean` and `stddev` of ns/op, led
   by the median: benchmark noise is one-sided, so the mean is dragged by
   outliers. `min` is the closest thing to the true cost of deterministic
   CPU-bound code, which is also what `benchmark/results/README.md` reports
   for the cross-language suite.
4. **Stability.** Mark a row `noisy` when `stddev / median > 0.05`.

### 4.4 `salam bench` command

A new `CMD_BENCH`, wired like `CMD_PROF` (§3.7).

```
salam bench                       # discover, build with -O2, run all
salam bench ./pkg                 # only that directory
salam bench --run=REGEX           # filter by benchmark name
salam bench --count=N             # samples per benchmark (default 8)
salam bench --min-time=MS         # per-sample target (default 100)
salam bench --json=FILE           # machine-readable results
salam bench --baseline=FILE       # compare against a previous --json
salam bench --fail-under=PCT      # exit 1 if any benchmark regresses by > PCT
salam bench --backend=c -O3 ...   # build flags pass through
```

**Discovery.** Files named `bench_*.salam` or `*_bench.salam`, and in them
top-level functions whose name starts with `Bench` or the language pack's
translation of it. That needs a `BenchPrefix` next to `Entry` and `EntryFor`
in `compiler/langpack.salam`, so a Persian source can name its benchmarks in
Persian. `file_has_entry` in `compiler/driver/driver_entry.salam` is the
model for the "does this file define X" scan; `salam test` already discovers
`*_test.salam` files the same way (`compiler/driver/driver_test.salam`).

**Generated driver.** `salam bench` writes a `main` that imports the
discovered files and calls `bench.Run` for each function, into its own
subdirectory of `.salam-build` so a concurrent build is not disturbed, then
builds and runs it.

**Output:**

```
salam bench (llvm backend, -O2), 8 samples, min-time 100ms

  benchmark                    iters      median      min     stddev
  ------------------------------------------------------------------
  BenchFibRecursive              200     5.12 ms   5.08 ms     1.1%
  BenchJsonParse              20,000    48.2 us   47.9 us     0.8%
  BenchSha256_1MiB             2,000   412.0 us  409.1 us     4.9%  noisy
  BenchStrConcat             500,000   198.0 ns  196.2 ns     0.6%   241.2 MB/s

  3 benchmarks, 1 noisy, 4.81 s total
```

With `--baseline`:

```
  benchmark                     base      new      delta
  ---------------------------------------------------------
  BenchFibRecursive          5.02 ms   5.12 ms    +2.0%
  BenchJsonParse             61.4 us   48.2 us   -21.5%  faster
  BenchSha256_1MiB          409.0 us  412.0 us    +0.7%  (within noise)
```

A delta smaller than the combined stddev of the two runs is marked "(within
noise)" and never trips `--fail-under`.

### 4.5 JSON schema

```json
{
  "schema": "salam.bench.v1",
  "salam_version": "0.6.4",
  "backend": "llvm",
  "flags": "-O2",
  "host": { "os": "windows", "arch": "x86_64", "cpu": "...", "cores": 16 },
  "started": "2026-08-07T10:33:02Z",
  "benchmarks": [
    {
      "name": "BenchFibRecursive",
      "iters": 200,
      "samples": 8,
      "ns_per_op": {
        "median": 5120000.0,
        "min": 5080000.0,
        "mean": 5140000.0,
        "stddev": 56000.0
      },
      "mb_per_sec": null,
      "noisy": false
    }
  ]
}
```

`salam bench --baseline` warns when the baseline's `host` block differs from
the current one, because timings from different machines do not compare.

### 4.6 The `benchmark/` suite

`benchmark/` compares Salam with C, Rust, Go and Python, a different
question from `salam bench`, and stays as it is. Two links between them:

1. `benchmark/bench/timing.salam` should read `time.MonoNanos()` instead of
   `time.NowNanos()`, so the suite stops depending on the 15.6 ms Windows
   wall clock.
2. `benchmark/bench/report.salam` and `salam bench --json` should use the
   same statistics fields.

`benchmark/bench/` already has a `bench.salam`; check that importing a
`std/bench` package from there still resolves the way the suite expects
before choosing the package's filenames.

---

## 5. Requirements for §3 and §4

- **Localization.** Every `pub` symbol in `std/prof` and `std/bench` gets
  `@en` and `@fa` names. New user-visible compiler strings go through
  `i18.Tr` with a Persian entry in `compiler/i18n.salam`. Table column
  headers are translated; numbers go through the compiler's digit shaping.
- **Help.** `prof` and `bench` in the command list of `UsageText`, and
  `--profile`, `--profile-out`, `--profile-min`, `--profile-exclude` under
  the build options.
- **Tests:**

| Test                                                       | Asserts                                                                         |
| ---------------------------------------------------------- | ------------------------------------------------------------------------------- |
| `tests/en/stdlib/prof_api.salam` and its `tests/fa` twin   | hand-called `BeginRegion`/`EndRegion`; `Report()` names the region; elapsed > 0 |
| `tests/en/stdlib/bench_stats.salam`                        | the statistics functions on known arrays, to a fixed epsilon                    |
| shell: `salam exec --profile=count prog.salam`             | `salam.prof` has an `F` record for every function, with exact call counts       |
| shell: `salam build --profile=fn` then `salam prof report` | exit 0, the hot function is named                                               |
| shell: `salam prof diff a.prof b.prof`                     | sign and size of each delta on a hand-written pair of files                     |
| shell: `--profile=count` on every backend                  | the same `F` records and call counts from C, LLVM, native and the interpreter   |

The deterministic call-count tests carry the weight: they check `Enter` and
`Exit` pairing on every control-flow path (early `ret`, `defer`, recursion,
lambdas) without depending on timing, so the test program should exercise
each of those shapes on purpose.

- **Docs.** This file becomes the user guide once §3 and §4 exist,
  `docs/GETTING_STARTED.md` gets a short "measuring performance" pointer,
  and `benchmark/results/README.md` explains how the suite differs from
  `salam bench`.

## 6. Suggested order

1. `std/prof` with hand-called `BeginRegion`/`EndRegion` and `Dump`:
   testable with no code generator changes.
2. Interpreter instrumentation (`salam exec --profile`) plus `salam prof
report`, `folded` and `diff`: a working profiler end to end, and a check
   of the file format and the report tooling.
3. C backend instrumentation through the defer flush points.
4. LLVM (and with it native) and JS.
5. `std/bench` and `salam bench`, with `--json` and `--baseline`.
6. `--profile=graph`, `salam prof graph`, and CI regression gates.

Steps 1 and 2 already give users a usable profiler.
