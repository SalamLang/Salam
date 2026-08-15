# Benchmark and Profiling Features - Design

Applies to: `c/src/**` (the C compiler) and `compiler/*.salam` (the self-hosted
compiler). Every item below is specified twice, once per implementation, because
the two must stay in parity (see `CONTRIBUTING.md`; a fix in `c/src/X.c` is
expected to land in `compiler/X.salam` in the same change).

## Status

| Part                                                                                 | State                     |
| ------------------------------------------------------------------------------------ | ------------------------- |
| §1 Monotonic clock (C, self-hosted, stdlib)                                          | **Implemented**           |
| §2 Part A: compiler self-profiling (`--time-report`, `--time-trace`), both compilers | **Implemented**           |
| §3 Part B: program profiling (`--profile`, `salam prof`)                             | Designed, not implemented |
| §4 Part C: benchmark harness (`std/bench`, `salam bench`)                            | Designed, not implemented |

Shipped files, §1 and Part A:

- `c/src/core/timer.{c,h}` - `salam_mono_ns` / `salam_cpu_ns` / `salam_peak_rss`
- `c/src/core/prof_self.{c,h}` - the accumulator, table/JSON report, Chrome trace
- `c/src/core/arena.{c,h}` - `arena_stats()`
- `c/src/ast/ast.{c,h}` - `ast_node_count()`
- `c/src/cli/{options.h,cli.c,cli_options.c,cli_help.c}` - the two flags
- `c/src/driver/{driver.c,build.c}` - `driver_main` wrapper + the scope calls
- `compiler/prof_self.salam` - the self-hosted port
- `compiler/sal_core.salam` - `CpuNanos()` / `PeakRss()`
- `compiler/{cli.salam,driver.salam}` - flags, `Run` wrapper, scope calls
- `std/time/{time.salam,hires.salam}` - `_mono_ns` + the `Mono*`/`Since*` API
- `std/debug/debug.salam` - `StartTimer`/`StopTimer` moved onto that clock
- `tests/en/stdlib/mono_clock_demo.{salam,out}` - 11 assertions
- `tools/bash/run-tests.sh` - new `timereport` section (3 checks)

Sections 3 and 4 below remain the design for the work that has not landed.
Section 8 records what the implementation of §1/Part A changed relative to the
original design, and why.

---

## 0. Scope

Three separate subsystems ship under this umbrella. They are independent and can
land in the order given.

| Part                           | Question it answers                               | User-facing surface                              |
| ------------------------------ | ------------------------------------------------- | ------------------------------------------------ |
| **A. Compiler self-profiling** | "Why is _compiling_ my program slow?"             | `salam build --time-report`, `--time-trace=FILE` |
| **B. Program profiling**       | "Why is _my program_ slow?"                       | `salam build --profile=MODE`, `salam prof <sub>` |
| **C. Benchmark harness**       | "Is this change faster or slower than last week?" | `std/bench`, `salam bench`                       |

### 0.1 What is written twice, and what is not

Both compilers must gain every feature here, but "both compilers" does not mean
"everything twice". The design deliberately pushes as much as possible into
shared Salam packages, so the duplicated surface stays small enough to keep in
sync by hand. Every row below is specified for both implementations in its own
section; use this table to check that nothing is missing before calling a part
done.

| Component                      | Written once (shared)                                                       | Written twice (C + `compiler/*.salam`)                                                                                                     |
| ------------------------------ | --------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------ |
| Monotonic clock (§1)           | `std/time._mono_ns` for user programs                                       | `salam_mono_ns` (`c/src/core/timer.c`) and `co.CpuNanos` (`compiler/sal_core.salam`), because the compilers cannot import their own stdlib |
| Self-profiler (A)              | -                                                                           | the whole accumulator: `c/src/core/prof_self.c` + `compiler/prof_self.salam`                                                               |
| A: instrumentation points      | -                                                                           | ~8 scope calls in each driver                                                                                                              |
| A: CLI flags, help             | -                                                                           | `cli_options.c` / `cli.salam`, `cli_help.c` / `UsageText`                                                                                  |
| Program profiler runtime (B)   | **`std/prof/prof.salam`** - accumulator, recursion handling, `.prof` writer | -                                                                                                                                          |
| B: codegen instrumentation     | -                                                                           | `cg_function` in each backend of each compiler (table in B.4/B.5)                                                                          |
| B: interpreter instrumentation | -                                                                           | `call_func` in each interpreter (table in B.5)                                                                                             |
| B: `.prof` report/diff/folded  | ideally `std/prof`, see B.7                                                 | fallback only                                                                                                                              |
| B: `salam prof` command wiring | -                                                                           | the five rows of B.7's table                                                                                                               |
| Benchmark harness (C)          | **`std/bench`** - adaptive iteration, statistics, `Keep`                    | -                                                                                                                                          |
| C: `salam bench` command       | the synthesized driver is generated Salam, identical from both              | discovery, driver synthesis, rendering                                                                                                     |
| C: langpack `Bench` prefix     | -                                                                           | `langpack_data.c` + `compiler/langpack.salam` tables                                                                                       |

The two `std/` packages are where the hard logic lives (recursion-correct
`total_ns`, Go-style iteration scaling, median/stddev). Getting them into `std/`
rather than into either compiler is what keeps the duplicated code mechanical.

What is explicitly **out of scope** for the first implementation, and why:

- **Sampling/interrupt-driven profilers** (SIGPROF, Windows timer queue thread).
  They need per-platform signal handling and stack unwinding, which is a much
  larger surface than instrumentation. Part B leaves a hook (`PROF_MODE_SAMPLE`)
  so this can be added later without a format change.
- **Hardware counters** (perf_event_open, PDH). Same reason.
- **Heap profiling by allocation site.** `std/mem` already tracks totals under
  `SALAM_MEM_DEBUG`; a full allocation-site profiler is a separate design.
- **Replacing the existing cross-language suite in `benchmark/`.** That suite
  compares Salam against C/Rust/Go/Python. Part C is a _microbenchmark harness
  for Salam code_. Section C.6 covers how they meet.

---

## 1. Shared foundation: a monotonic high-resolution clock

Everything here needs a clock, and the clock we have today is not good enough.

`std/time/time.salam:162-188` defines `_now_ns()` as:

- Windows: `GetSystemTimeAsFileTime`. This is wall-clock time, and although its
  unit is 100 ns its _actual_ update granularity is the system timer tick,
  typically 15.6 ms. Timing anything under ~50 ms with it produces either 0 or 15600000. It is unusable for profiling.
- POSIX: `clock_gettime(0, ...)`, and 0 is `CLOCK_REALTIME`. That is wall clock
  too, so it steps when NTP adjusts and can go backwards.

So step one, shared by parts A, B and C:

### 1.1 C compiler: `c/src/core/timer.c` + `c/src/core/timer.h`

```c
/* c/src/core/timer.h */
#ifndef SALAM_CORE_TIMER_H
#define SALAM_CORE_TIMER_H
#include "core/prelude.h"

/* Monotonic nanoseconds since an unspecified epoch. Never goes backwards,
   never jumps with the wall clock. Resolution is sub-microsecond on every
   supported platform. */
uint64_t salam_mono_ns(void);

/* Best-effort CPU time (user+sys) of the current process, in nanoseconds.
   Returns 0 when the platform cannot report it. */
uint64_t salam_cpu_ns(void);

/* Peak resident set size in bytes, 0 when unavailable. */
uint64_t salam_peak_rss(void);
#endif
```

Implementation matrix:

| Platform | `salam_mono_ns`                                                                                           | `salam_cpu_ns`                                 | `salam_peak_rss`                                    |
| -------- | --------------------------------------------------------------------------------------------------------- | ---------------------------------------------- | --------------------------------------------------- |
| Windows  | `QueryPerformanceCounter` / `QueryPerformanceFrequency` (frequency cached in a file-static, queried once) | `GetProcessTimes`, kernel+user 100 ns ticks    | `GetProcessMemoryInfo().PeakWorkingSetSize` (psapi) |
| Linux    | `clock_gettime(CLOCK_MONOTONIC, ...)`                                                                     | `clock_gettime(CLOCK_PROCESS_CPUTIME_ID, ...)` | `getrusage().ru_maxrss * 1024`                      |
| macOS    | `clock_gettime(CLOCK_MONOTONIC, ...)` (10.12+), else `mach_absolute_time` + `mach_timebase_info`          | `getrusage` user+sys                           | `getrusage().ru_maxrss` (already bytes on macOS)    |
| other    | `clock()` scaled by `CLOCKS_PER_SEC`                                                                      | same                                           | 0                                                   |

Notes that matter for this repository specifically:

- `QueryPerformanceCounter` needs `windows.h`, which the compiler already pulls
  in from `driver.c`. Do **not** add it to `core/prelude.h`; keep it local to
  `timer.c` so `hdr_prelude` in `codegen_header.c` is unaffected.
- `GetProcessMemoryInfo` needs `-lpsapi`. Add it to the compiler's own link line
  in `c/Makefile` only (not to generated-program link lines). If we want to
  avoid the extra library, `K32GetProcessMemoryInfo` is in kernel32 on Windows 7+
  but is not reliably resolvable under tcc; see
  `salam_core_foundations_landmines` in the memory notes for the pattern of
  tcc failing to link some kernel32 symbols. Prefer `-lpsapi` under gcc/clang
  and compile the RSS body out under `SALAM_CC_TCC`, returning 0.
- 64-bit division on the QPC path: compute
  `(counter / freq) * 1000000000 + ((counter % freq) * 1000000000) / freq` so it
  does not overflow after ~5 seconds of uptime the way `counter * 1000000000 / freq`
  does with a 10 MHz frequency.

Register `core/timer.c` in `c/Makefile` and in the quick-build source lists (the
same lists commit `24fd5d43` had to fix for `doc/`, `driver/doc_build.c` and
`llvm/orc_call.c`; there is more than one such list and missing one is the usual
build break here).

### 1.2 Self-hosted compiler: `compiler/sal_core.salam`

`compiler/sal_core.salam` is the counterpart of `c/src/core/*`. Add there:

```salam
@en "MonoNanos"
pub func MonoNanos(): i64: ... end
pub func CpuNanos(): i64: ... end
pub func PeakRss(): i64: ... end
```

Platform selection uses the `if SALAM_OS_WINDOWS:` / `if SALAM_ARCH_X64:` form
already used in `std/time/time.salam:162`. Remember the landmine recorded in
`salam_compiler_landmines_fs_os_build`: platform macros are only valid in `if`,
not in expressions.

### 1.3 Standard library: fix `std/time`

`std/time/hires.salam` gains monotonic variants, and the existing wall-clock ones
keep their meaning (they are used for timestamps, which is correct):

```salam
@en "MonoNanos"   @fa "نانوثانیه یکنواخت"   @ar "نانو ثانیة رتیبة"
pub func MonoNanos(): i64: ret _mono_ns() end

@en "MonoElapsedNs"  ...
pub func MonoElapsedNs(start_ns: i64): i64: ret _mono_ns() - start_ns end
```

`_mono_ns()` goes in `std/time/time.salam` next to `_now_ns()`, with:

- Windows: `QueryPerformanceCounter` + `QueryPerformanceFrequency` externs.
- POSIX: `clock_gettime(1, buf)` (1 is `CLOCK_MONOTONIC`; the existing code
  passes 0, which is `CLOCK_REALTIME`).

Then `std/debug/debug.salam:138-165` (`StartTimer`/`StopTimer`) switches from
`time.NowNanos` to `time.MonoNanos`. Those two functions are currently
unreliable on Windows for exactly the granularity reason above; this is a real
bugfix that falls out of the design.

---

## 2. Part A: compiler self-profiling

### A.1 Goal

`salam build --time-report app.salam` prints where the compiler spent its time,
broken down by phase and by input file, plus memory. This is the Salam analogue
of `clang -ftime-report` / `rustc -Ztime-passes`.

### A.2 What gets measured

The pipeline in `driver_build` (`c/src/driver/build.c:487`) already has clean
phase boundaries. The phases, matching the existing `phase_t` enum in
`c/src/logger/logger.h:31`:

| Phase      | Measured around                                                               |
| ---------- | ----------------------------------------------------------------------------- |
| `driver`   | everything not attributed elsewhere (arg parsing, dir scans, path resolution) |
| `source`   | `source_load`                                                                 |
| `lexer`    | `lexer_run`                                                                   |
| `parser`   | `parser_run` + `cc_prune_program` + `salam_merge_program`                     |
| `semantic` | `sema_run_cached`                                                             |
| `codegen`  | `cg_run` / the C-emitting step                                                |
| `cc`       | the `system()` invocation of the host C compiler, per module                  |
| `link`     | the final link command                                                        |
| `write`    | `write_file` for `.c`/`.h`, plus `file_has_content` comparisons               |

Two extra phases exist only for other backends and are recorded the same way:
`llvm` (in `llvm_build.c`) and `jsgen` (in `js_build.c`).

Counters recorded alongside the timings:

- tokens produced, AST nodes allocated, symbols interned, functions emitted;
- modules compiled vs modules skipped by the incremental cache (`cdirty[]` in
  `build.c:557` already carries this bit, it just is not reported);
- arena bytes allocated and blocks allocated;
- peak RSS at exit.

### A.3 C implementation

**New files:** `c/src/core/prof_self.c`, `c/src/core/prof_self.h`.

```c
/* c/src/core/prof_self.h */
#ifndef SALAM_CORE_PROF_SELF_H
#define SALAM_CORE_PROF_SELF_H
#include "core/prelude.h"
#include "logger/logger.h"

typedef enum {
    TP_DRIVER = 0, TP_SOURCE, TP_LEXER, TP_PARSER, TP_SEMANTIC,
    TP_CODEGEN, TP_LLVM, TP_JSGEN, TP_WRITE, TP_CC, TP_LINK,
    TP__COUNT
} timing_phase_t;

typedef enum {
    TC_TOKENS = 0, TC_AST_NODES, TC_SYMBOLS, TC_FUNCS_EMITTED,
    TC_MODULES_BUILT, TC_MODULES_CACHED, TC_ARENA_BYTES, TC_ARENA_BLOCKS,
    TC__COUNT
} timing_counter_t;

/* No-ops unless prof_self_enable() has been called, so the instrumentation
   is safe to leave in place unconditionally. */
void prof_self_enable(bool trace_events);
bool prof_self_on(void);

/* Scoped timing. Nesting is allowed: an inner region's time is subtracted
   from the enclosing region's "self" time but kept in its "total". */
uint64_t prof_self_begin(timing_phase_t ph, const char *label);
void prof_self_end(timing_phase_t ph, uint64_t token);

void prof_self_count(timing_counter_t c, uint64_t n);

/* Reporting. `fmt` is one of the PROF_FMT_* values below. */
#define PROF_FMT_TABLE 0
#define PROF_FMT_JSON  1
void prof_self_report(FILE *out, int fmt);

/* Chrome Trace Event JSON, for chrome://tracing / Perfetto / speedscope. */
bool prof_self_write_trace(const char *path);

void prof_self_shutdown(void);
#endif
```

And the ergonomic macro pair, in the same header:

```c
#define PROF_SCOPE_BEGIN(ph, label) uint64_t _pt_##ph = prof_self_begin((ph), (label))
#define PROF_SCOPE_END(ph)          prof_self_end((ph), _pt_##ph)
```

**Data structure.** A file-static struct, no allocation on the hot path:

```c
typedef struct {
    uint64_t total_ns;   /* wall, including nested regions */
    uint64_t self_ns;    /* wall, minus directly nested regions */
    uint64_t cpu_ns;
    uint32_t calls;
} phase_acc_t;

static struct {
    bool         on;
    bool         trace;
    phase_acc_t  acc[TP__COUNT];
    uint64_t     counter[TC__COUNT];
    uint64_t     start_ns;      /* process start, for the % column */
    /* nesting stack, so self_ns is correct */
    struct { timing_phase_t ph; uint64_t t0, cpu0, child_ns; } stack[32];
    int          depth;
    /* trace events, only when `trace` is set */
    vec_t        events;        /* of trace_ev_t, arena-free malloc/realloc */
} g_prof;
```

`prof_self_begin` pushes, records `salam_mono_ns()` and `salam_cpu_ns()`.
`prof_self_end` pops, adds `now - t0` to `total_ns`, adds
`now - t0 - child_ns` to `self_ns`, and adds its own `total` into the parent's
`child_ns`. Depth is capped at 32; overflow degrades to counting `total_ns` only
rather than corrupting the stack.

The `trace_events` path appends `{name, cat, ph:'X', ts, dur, pid, tid}` records
which `prof_self_write_trace` dumps as a Chrome Trace Event array. That format is
readable by `chrome://tracing`, Perfetto UI and speedscope with zero extra work
on our side, so we get a flamechart of the build for free.

**Instrumentation points.** Exact call sites, all in `c/src/driver/`:

```c
/* build.c, inside the per-file work loop around line 638 */
PROF_SCOPE_BEGIN(TP_SOURCE, path);
source_file_t *src = source_load(arena, path);
PROF_SCOPE_END(TP_SOURCE);

PROF_SCOPE_BEGIN(TP_LEXER, path);
bool lok = lexer_run(arena, log, modpack, src, &toks);
PROF_SCOPE_END(TP_LEXER);
prof_self_count(TC_TOKENS, toks ? toks->len : 0);

PROF_SCOPE_BEGIN(TP_PARSER, path);
bool pok = parser_run(arena, log, toks, &program);
if (!cc_prune_program(arena, log, path, cc, program)) pok = false;
PROF_SCOPE_END(TP_PARSER);

PROF_SCOPE_BEGIN(TP_SEMANTIC, path);
sema_result_t *sr = sema_run_cached(...);
PROF_SCOPE_END(TP_SEMANTIC);
```

plus `TP_CODEGEN` around the `cg_*` call, `TP_WRITE` around the `write_file`
pair, `TP_CC` around each per-module compile `system()` call, and `TP_LINK`
around the final link `system()`. The label passed is the module path so the
per-file table has meaningful rows.

`driver_main` (`c/src/driver/driver.c:676`) calls `prof_self_enable()` right
after `cli_parse` succeeds, and calls `prof_self_report` / `prof_self_write_trace`
/ `prof_self_shutdown` on **every** exit path. Since `driver_main` has many
`return`s, add a single `driver_main_inner()` and make `driver_main` a wrapper:

```c
int driver_main(int argc, char **argv)
{
    int rc = driver_main_inner(argc, argv);
    if (prof_self_on()) {
        prof_self_report(stderr, g_opt_time_report_fmt);
        if (g_opt_time_trace) prof_self_write_trace(g_opt_time_trace);
        prof_self_shutdown();
    }
    return rc;
}
```

This is a smaller and safer change than threading cleanup through fifteen
`return` statements.

**Arena stats.** `c/src/core/arena.h` currently exposes only
`arena_new/alloc/strndup/strdup/free`. Add:

```c
typedef struct { uint64_t bytes_used, bytes_reserved, blocks; } arena_stats_t;
arena_stats_t arena_stats(const arena_t *a);
```

`driver_build` reports it into `TC_ARENA_BYTES`/`TC_ARENA_BLOCKS` just before
`arena_free(arena)` at `build.c:1208`.

### A.4 CLI wiring (C)

`c/src/cli/options.h`, in `options_t`:

```c
bool  time_report;        /* --time-report */
int   time_report_fmt;    /* PROF_FMT_TABLE | PROF_FMT_JSON */
const char *time_trace;   /* --time-trace=FILE, NULL when off */
```

`c/src/cli/cli_options.c`, in the `cli_parse_options` chain near line 104:

```c
} else if (strcmp(arg, "--time-report") == 0) {
    out->time_report = true;
} else if ((val = cli_opt_value(arg, "--time-report")) != NULL) {
    out->time_report = true;
    if (strcmp(val, "json") == 0)       out->time_report_fmt = PROF_FMT_JSON;
    else if (strcmp(val, "table") == 0) out->time_report_fmt = PROF_FMT_TABLE;
    else { fprintf(stderr, i18n_tr("salam: unknown --time-report format '%s'\n"), val); return false; }
} else if ((val = cli_opt_value(arg, "--time-trace")) != NULL) {
    out->time_trace = val;
} else if (strcmp(arg, "--time-trace") == 0) {
    out->time_trace = "salam-trace.json";
}
```

`c/src/cli/cli_help.c` gains the two flags under the build options block, with
their text routed through `i18n_tr` like the rest.

### A.5 Report format

Default table, written to stderr so it never contaminates stdout (which
`salam js`, `salam doc` and `--emit-*` write to):

```
Salam compile time report
  wall 1.842 s   cpu 2.031 s   peak rss 148.2 MB

  phase        self       total     %self   calls
  ---------------------------------------------------
  cc          0.981 s    0.981 s    53.3%      12
  semantic    0.402 s    0.517 s    21.8%      14
  parser      0.171 s    0.203 s     9.3%      14
  codegen     0.118 s    0.118 s     6.4%      12
  link        0.092 s    0.092 s     5.0%       1
  lexer       0.041 s    0.041 s     2.2%      14
  source      0.021 s    0.021 s     1.1%      14
  write       0.011 s    0.011 s     0.6%      24
  driver      0.005 s    1.842 s     0.3%       1

  slowest files
    std/semantic.salam            0.311 s   (sema 0.240 s, cc 0.048 s)
    app.salam                     0.204 s   (cc 0.180 s)
    std/encoding/json/json.salam  0.150 s   (sema 0.101 s)

  counters
    tokens          412,004      ast nodes     98,221
    symbols          11,904      funcs emitted  2,118
    modules built        12      cached             2
    arena           41.2 MB in 44 blocks
```

Rows with zero calls are omitted. Numbers are formatted through
`c/src/core/sal_format.c` so the digit shaping honours the active language pack;
this matters for `--lang=fa`/`--lang=ar` where the rest of the output is already
localized.

`--time-report=json` emits a single object with `phases`, `files`, `counters`,
`wall_ns`, `cpu_ns`, `peak_rss` so CI can diff it. Schema is versioned:
`{"schema":"salam.timereport.v1", ...}`.

### A.6 Self-hosted implementation

**New file:** `compiler/prof_self.salam`, mirroring `c/src/core/prof_self.c`
one-to-one (same enum values, same field names, same report text).

Wiring differences that follow from how `compiler/` is written:

- The enums become `pub const TP_DRIVER = 0` etc. **Watch out**: per
  `salam_compress_pkg_and_more_compiler_bugs`, `pub enum` values cannot cross a
  package boundary, so int consts are the correct choice here, not an enum.
  Also, per `salam_core_foundations_landmines`, `pub const` names collide at
  link time across packages, so prefix them (`TP_`, `TC_` are already distinct
  enough, but verify against `compiler/logger.salam`'s `PH_*` set).
- The accumulator struct lives in a module-level `mut` state variable, same as
  `compiler/logger.salam` keeps its logger state.
- Instrumentation goes into `compiler/driver.salam` at the same logical points:
  the per-file loop that calls `lx.Run`, `ps.Run`, `sm.Run`, `cg.Run`.
- The nesting stack: use a fixed-size array of a small struct rather than a
  `Vector<Frame>`. Nested generics do not work (`salam_typed_decl_and_cross_pkg_generic_infer`),
  and a plain `Vector<Frame>` of a single-level struct is fine, but a fixed array
  avoids the allocation entirely on the hot path.
- `opt.time_report`, `opt.time_report_fmt` and `opt.time_trace` are added to
  `pub struct Dr` at `compiler/driver.salam:201`, next to `keep_c` (line 220)
  and `asan` (line 230). The flag parse goes into `compiler/cli.salam`'s option
  loop, and the dispatcher at `compiler/cli.salam:337` needs no change.

**Parity test.** `tests/en/features/time_report.salam` is not the right shape
here because the output is timing-dependent. Instead add a shell check to
`tools/bash/run-tests.sh` in a new `selfprof` section that runs both binaries
with `--time-report=json`, parses the JSON, and asserts:

1. exit code 0,
2. the `schema` field is `salam.timereport.v1`,
3. every phase key present in the C output is present in the Salam output,
4. `wall_ns > 0` and `sum(phase.self_ns) <= wall_ns * 1.05`.

That validates parity of _shape_ without depending on machine speed.

---

## 3. Part B: program profiling

### B.1 Modes

```
salam build --profile          # same as --profile=fn
salam build --profile=fn       # call counts + inclusive/exclusive wall time per function
salam build --profile=count    # call counts only (no clock reads; ~2% overhead)
salam build --profile=graph    # fn, plus caller->callee edge attribution
salam build --profile=off      # explicit off, for overriding a config file
```

`--profile=sample` is reserved and rejected with "not implemented yet" so the
flag space is stable.

`salam run --profile=...` and `salam exec --profile=...` work too; `exec` uses
the interpreter path described in B.6.

Companion flags:

```
--profile-out=FILE       # default: salam.prof in cwd
--profile-min=N          # drop functions with fewer than N calls from the report
--profile-exclude=PKG    # do not instrument this package; repeatable
```

`std/prof` itself and `std/core`, `std/mem`, `std/time` are always excluded;
instrumenting the profiler's own clock reads is an infinite regress.

### B.2 The runtime lives in Salam, not in the C backend

This is the central design decision. The profiling runtime is
**`std/prof/prof.salam`**, a normal Salam package, and the driver injects it into
the build work list when `--profile` is active, exactly the way `build.c:594-610`
already force-injects `mem` and `core`:

```c
if (opt->profile_mode != PROF_MODE_OFF) {
    const char *profp = salam_resolve_import(arena, "", "prof");
    FILE *pf = profp ? fopen(profp, "rb") : NULL;
    if (pf) { fclose(pf); if (nwork < SALAM_MAX_INPUTS) work[nwork++] = profp; }
    else { LOG_E(log, PH_DRIVER, i18n_tr("--profile needs the 'prof' package in the stdlib")); all_ok = false; }
}
```

Why this beats emitting a C runtime from `codegen_header.c`:

1. **Backend parity for free.** The C backend, the LLVM backend and the
   interpreter all compile the same Salam source. Writing the runtime in C would
   mean writing it three times (C text for `codegen`, IR for `llvm.salam`,
   built-ins for `interp`).
2. **Compiler parity for free.** `c/src/` and `compiler/` both consume
   `std/prof/prof.salam`. There is no second copy to keep in sync, which is the
   single largest recurring cost in this repository.
3. It is testable as an ordinary stdlib package before any codegen work exists.

The JS backend is the one exception and is handled in B.5.

The self-hosted driver force-injects the same two packages at
`compiler/driver.salam:883-887`, so its counterpart goes in the same place:

```salam
if opt.profile_mode != PROF_MODE_OFF:
    profp := sm.ResolveImport("", "prof")
    if str.Len(profp) > 0 && os.Exists(profp):
        work.push(profp)
    else:
        lg.Log(lgr, lg.PH_DRIVER, lg.LOG_ERROR,
               i18.Tr("--profile needs the 'prof' package in the stdlib"))
        all_ok = false
    end
end
```

Note what is _not_ duplicated: `std/prof/prof.salam` itself. Both compilers
consume the identical package from `std/`, so the profiler's accumulator,
recursion handling and file format exist exactly once. Only the ~10 lines of
injection and the instrumentation call sites are written twice. That asymmetry
is the whole reason for putting the runtime in Salam.

### B.3 `std/prof` API

Two layers. The lower layer is what codegen emits; the upper layer is what a user
calls by hand when they want to profile a region rather than a function.

```salam
@en "prof"  @fa "پروفایل"  @ar "تحلیل"
package prof

// ---- lower layer: emitted by codegen, not meant to be hand-written ----

// Registers a function name once and returns its slot id. Codegen calls this
// from a module-level initializer so the per-call path never touches strings.
@en "Register"
pub func Register(name: str, file: str, line: int): int: ... end

@en "Enter"
pub func Enter(id: int): ... end

@en "Exit"
pub func Exit(id: int): ... end

// ---- upper layer: hand-written region profiling ----

@en "BeginRegion"
pub func BeginRegion(name: str): int: ... end

@en "EndRegion"
pub func EndRegion(handle: int): ... end

@en "Enabled"
pub func Enabled(): bool: ... end

@en "SetEnabled"
pub func SetEnabled(on: bool): ... end

// ---- output ----

@en "Dump"
pub func Dump(path: str): bool: ... end

@en "Report"
pub func Report(): str: ... end

@en "Reset"
pub func Reset(): ... end
```

`@fa` / `@ar` names are required on every `pub` symbol; the repository convention is
that all three are present or the translated import paths break.

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

`Enter(id)`:

```
if !_on: ret end
_stack[_sp].id = id
_stack[_sp].t0 = time.MonoNanos()
_stack[_sp].child = 0
_sp += 1
_slots[id].depth += 1
```

`Exit(id)`:

```
if !_on: ret end
_sp -= 1
dt := time.MonoNanos() - _stack[_sp].t0
s := &_slots[id]
s.calls += 1
s.self_ns += dt - _stack[_sp].child
if s.depth == 1: s.total_ns += dt end    // only the outermost frame of a recursion
s.depth -= 1
if _sp > 0: _stack[_sp-1].child += dt end
```

The `depth == 1` guard is what makes `total_ns` correct for recursive functions;
without it, `fib(30)` reports a total of many times the program's runtime.

For `--profile=graph`, `Enter` additionally bumps an edge counter keyed by
`(caller_slot, callee_slot)`. Store edges in a flat open-addressed table sized to
a power of two, keyed by `caller * 65536 + callee`, growing by rehash. Do **not**
use `HashMap<int, Vector<Edge>>`: nested generics never work
(`salam_typed_decl_and_cross_pkg_generic_infer`), so a hand-rolled flat table is
the only option and is faster anyway.

Stack depth is fixed at 4096 frames. On overflow, `Enter` sets a
`_overflowed` flag, skips the push, and `Exit` for that call is a no-op; the
report prints a warning line. Silently corrupting the stack on deep recursion
would be worse than an incomplete profile.

**Auto-dump at exit.** The user should not have to call `prof.Dump`. Codegen
emits, at the end of the entry function, a call to `prof.Dump(<path>)` before
the implicit `return 0` at `codegen_decl.c:192`. For programs that call
`os.Exit()`, `std/os` gains a small "at exit" hook list that `prof` registers
into; this is a two-function addition to `std/os/process.salam`.

### B.4 Codegen instrumentation, C backend

**Where.** `cg_function` in `c/src/codegen/codegen_decl.c:137`, whose Salam twin
is `cg_function` in `compiler/codegen.salam:1522`.

**What.** After the prologue (`cg->indent++`, line 173) and before the body
(`cg_block(cg, fn->a)`, line 190):

```c
int prof_id = -1;
if (cg->prof_mode != PROF_MODE_OFF && cg_prof_wants(cg, fn, owner)) {
    prof_id = cg_prof_slot(cg, fn, owner);   /* interns name, returns index */
    cg_line(cg, "salam_prof_Enter(%d);", prof_id);
}
```

and the exit is the interesting part, because Salam functions have `ret` in the
middle of blocks and a defer mechanism (`cg_emit_defers`, line 191). The
instrumentation must not leak a missing `Exit` on an early return.

The clean answer is to **reuse the flush points the defer machinery already
has**. `cg_emit_defers` (`codegen_stmt.c:17`) is called from exactly five places:
the four `AST_RETURN` sub-cases at `codegen_stmt.c:184`, `190`, `193` and `196`,
and the natural end of the function at `codegen_decl.c:191`. Those five points
are, by construction, every exit from a Salam function body.

What must **not** be done is pushing a synthetic entry onto `cg->fn_defers`.
That vector holds `ast_node_t *`, not strings (`vec_push(cg->a, &cg->fn_defers,
n->a)` at `codegen_stmt.c:167`, consumed by `cg_stmt(cg, (ast_node_t *)...)` at
line 22), so a pushed string would be reinterpreted as an AST node and crash.
There is also a behavioural trap: the `n->a && cg->fn_defers.len > 0` guard at
line 178 changes how the return value is emitted (via a `__retv` temporary) as
soon as the vector is non-empty, so injecting a fake defer would silently change
codegen for every instrumented function.

Instead add a raw-text exit hook to `cg_t` in `codegen_internal.h`:

```c
const char *fn_prof_exit;   /* NULL when not profiling this function */
```

set it next to the `Enter` emission, and extend `cg_emit_defers` to flush it
after the AST defers:

```c
void cg_emit_defers(cg_t *cg)
{
    size_t i = cg->fn_defers.len;
    for (; i > 0; i--) cg_stmt(cg, (ast_node_t *)cg->fn_defers.data[i - 1]);
    if (cg->fn_prof_exit) cg_line(cg, "%s", cg->fn_prof_exit);
}
```

`fn_prof_exit` is saved/restored around the function body exactly where
`saved_defers` already is (`codegen_decl.c:155` and `199`, and the lambda
equivalents at `codegen_lambda.c:68` and `133`), so nested lambda bodies do not
inherit their parent's exit hook.

One control-flow path needs explicit verification during implementation:
`AST_RETURN` inside a `match` takes the `cg->match_result_tmp` branch at
`codegen_stmt.c:170-177`, which emits a `goto <match_end_label>` and does **not**
call `cg_emit_defers`. That is correct for defers only if the code at the match
end eventually falls into one of the five flush points. Confirm it does; if it
does not, that is a pre-existing `defer` bug and should be fixed there rather
than worked around in the profiler.

The `salam_prof_Enter` symbol name: `std/prof`'s `Enter` mangles to whatever
`SALAM_MOD_PREFIX` + package + name produces. Use the same helper codegen already
uses to reference a cross-package function so the mangling stays in one place;
do not hardcode the string.

**Self-hosted parity.** `compiler/codegen.salam` mirrors this structure exactly,
so the change is mechanical:

| C                                                                | Salam                                                              |
| ---------------------------------------------------------------- | ------------------------------------------------------------------ |
| `cg_function`, `codegen_decl.c:137`                              | `cg_function`, `compiler/codegen.salam:1522`                       |
| `cg_t.fn_defers` (`vec_t` of `ast_node_t *`)                     | `Cg.fn_defers` (`Vector<int>` of `at.NodeId`), `codegen.salam:179` |
| save/restore at `codegen_decl.c:155`, `199`                      | `codegen.salam:1548-1549`, `1594-1596`                             |
| `cg_emit_defers`, `codegen_stmt.c:17`                            | `cg_emit_defers`, `codegen.salam:4713`                             |
| four `AST_RETURN` flush points, `codegen_stmt.c:184/190/193/196` | `codegen.salam:4910/4917/4920/4923`                                |
| lambda save/restore, `codegen_lambda.c:68`, `133`                | `codegen.salam:5147`, `5154`                                       |

Add `pub fn_prof_exit: str = ""` to `struct Cg` next to `fn_defers` and flush it
at the end of `cg_emit_defers`. Use `""` rather than a null check; an empty
string is the idiomatic "unset" in this codebase and avoids a nullable `str`.

**Slot registration.** Each module accumulates its slots into a module-level C
array and a generated `static void salam_prof_register_<module>(void)` that runs
from the module initializer list (`cg->deferred`, emitted into `main` at
`codegen_decl.c:184-188`). That mechanism already exists for other module init
work, so slot registration piggybacks on it.

**DCE.** `dce_enable()` is on for builds (`build.c:624-625`), and it marks roots
by _source-level_ usage. Codegen-injected calls happen after that analysis, so
`prof.Enter`, `prof.Exit`, `prof.Register` and `prof.Dump` would be dead-stripped
and the build would fail to link. `driver_build` must call:

```c
if (opt->profile_mode != PROF_MODE_OFF) {
    dce_mark_root("prof", "Register");
    dce_mark_root("prof", "Enter");
    dce_mark_root("prof", "Exit");
    dce_mark_root("prof", "Dump");
}
```

right after `dce_enable()`. This is the kind of thing that produces a confusing
link error six hours into implementation, so it belongs in the design.

The self-hosted DCE exposes the same three entry points, so the same four calls
go into the equivalent spot in `driver.salam`'s build path:

| C (`c/src/semantic/dce.h`) | Salam (`compiler/semantic.salam`)                                   |
| -------------------------- | ------------------------------------------------------------------- |
| `dce_enable()`             | `sm.DceEnable()`, line 11754                                        |
| `dce_enabled()`            | `sm.DceEnabled()`, line 11764                                       |
| `dce_mark_root(pkg, fn)`   | `sm.DceMarkRoot(pkg, fn)`, line 11769                               |
| (queried during pruning)   | `sm.DceReachable(pkg, fn)`, line 11815, used at `driver.salam:1127` |

**Which functions get instrumented** (`cg_prof_wants`):

- skip anything whose home package is `prof`, `core`, `mem`, `time` (the
  regress problem);
- skip anything in a package listed in `--profile-exclude`;
- skip `extern` declarations (no body to instrument);
- skip lambdas under `--profile=count` (they inflate the slot table without
  telling the user much), include them under `fn`/`graph` with a synthesized
  name `<parent>.lambda@<line>`;
- instrument the entry function itself, so the report has a 100% root.

### B.5 Other backends

**LLVM backend** (`c/src/driver/llvm_build.c`, `c/src/llvm/*`, mirrored in
`compiler/llvm.salam:2073 emit_function`). Same shape: emit a `call void
@salam_prof_Enter(i32 <id>)` at the entry block, and rely on the same
defer-flush points for `Exit`. Because `std/prof` is compiled through the same
LLVM path, the symbol exists naturally.

One caveat worth stating: with `-O2`/`-O3`, LLVM can and will inline the
instrumented function into its caller, and the `Enter`/`Exit` pair travels with
it, so counts stay correct but the _attribution_ of very small functions gets
noisy. Document that `--profile` builds should use `-O1` for stable attribution,
and have the driver emit an informational log line when `--profile` is combined
with `-O2` or higher rather than silently changing the user's optimization level.

**JS backend.** Emission point in each compiler:

| C                                              | Salam                                             |
| ---------------------------------------------- | ------------------------------------------------- |
| `jsg_function`, `c/src/jsgen/jsgen_stmt.c:477` | `jsg_function`, `compiler/jsgen.salam:3202`       |
| called from `jsgen.c:83`                       | `jsg_emit_functions`, `compiler/jsgen.salam:3388` |

The JS backend does not compile `std/prof` as Salam; it emits JavaScript.
Instrument with a small emitted preamble that mirrors the same API:

```js
const __salam_prof = { slots: [], stack: [], on: true,
  Register(n,f,l){...}, Enter(id){...}, Exit(id){...}, Dump(){...} };
```

using `performance.now()` (millisecond float with microsecond resolution) and
dumping to `console.log` as the same JSON the native side writes. `--no-minify`
interaction: the profiler preamble must be excluded from name minification or
the report shows mangled names; `js_build.c` already has a
`no_js_minify_names` concept to hook into.

**Interpreter.** This one is nearly free and worth doing first, because it needs
no codegen at all. Both interpreters funnel _every_ user-function invocation
through one function, so there is exactly one place to instrument on each side:

| C                                                                                          | Salam                                                                       |
| ------------------------------------------------------------------------------------------ | --------------------------------------------------------------------------- |
| `call_func`, defined at `c/src/interp/interp_expr.c:260`, declared `interp_internal.h:258` | `call_func`, `compiler/interp.salam:2838`                                   |
| ~10 call sites across `interp_expr.c` + `interp.c:594` (the `main` entry)                  | `eval_call`, `compiler/interp.salam:3344`; public entry `CallFunc`, `:5137` |

Instrument the _definition_, never the call sites; the call sites are where a
future one gets missed.

```c
/* c/src/interp/interp_expr.c, at the top of call_func */
uint64_t t0 = 0;
int sid = -1;
if (interp_prof_on()) { sid = interp_prof_slot(fn); t0 = interp_prof_enter(sid); }
... existing body ...
if (sid >= 0) interp_prof_exit(sid, t0);   /* every return path */
```

`call_func` has multiple `return`s, so the C wraps its body in an inner
`call_func_inner()` and instruments the wrapper, the same shape `driver_main` /
`driver_main_inner` already uses for `--time-report`. The Salam port does the
same to `call_func` at `interp.salam:2838`, with the E088 caveat from §8.4:
`call_func` is private and sits above the file's public section, so the inner
function must be placed above it rather than appended.

The interpreter's accumulator is a private copy of the same `Slot` layout on
each side (it cannot call into `std/prof`, since the interpreter does not link
the program it is running) and writes the same file format, so
`salam prof report` does not care which backend produced the profile.
`salam exec --profile` therefore gives users a zero-build-step profiler, which
is the fastest thing to ship and the best way to validate the report tooling.

Note the caveat from `salam_exec_unsigned_wrong`: the interpreter gets u32/u64
arithmetic wrong, so interpreter profiles of crypto/bit-heavy code may reflect
different control flow than a compiled build. Say so in the docs.

### B.6 Profile file format

`salam.prof` is **text**, not binary. Rationale: the file is at most a few
thousand lines, users will inevitably want to grep and diff it, and a binary
format buys nothing here while costing an endianness question and a reader in
two languages.

```
salam-profile 1
mode fn
program app
wall_ns 1204338211
cpu_ns 1198220004
started 2026-08-07T10:33:02Z
backend c
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

Rules: lines starting with `#` are comments, unknown record types are skipped by
readers (forward compatibility), the header is order-independent key-value pairs
until the first `#`. Names are printed in the source language of the definition,
not translated, so the profile of a Persian-source program shows Persian names.

### B.7 `salam prof` subcommand

A new command, so it needs the same five-point wiring in each compiler. Both
lists are exact; `CMD_PROF`/`CMD_BENCH` take the next free ordinals after
`CMD_DOC` (C: the `cli_command_t` enum in `options.h:25-44`; Salam:
`pub const CMD_DOC := 18` at `compiler/driver.salam:133`, so `CMD_PROF := 19`
and `CMD_BENCH := 20`).

| Step             | C                                                     | Salam                                                                            |
| ---------------- | ----------------------------------------------------- | -------------------------------------------------------------------------------- |
| command constant | `cli_command_t` enum, `c/src/cli/options.h`           | `pub const CMD_PROF := 19`, `compiler/driver.salam` next to `CMD_DOC` (line 133) |
| argv dispatch    | `cli_dispatch_command`, `c/src/cli/cli_subcmd.c:18`   | `dispatch_command`, `compiler/cli.salam:337`                                     |
| option fields    | `options_t`, `c/src/cli/options.h`                    | `pub struct Dr`, `compiler/driver.salam:202`                                     |
| option parsing   | `cli_parse_options`, `c/src/cli/cli_options.c:97`     | `parse_options`, `compiler/cli.salam:439`                                        |
| command handler  | `driver_main_inner`'s switch, `c/src/driver/driver.c` | `RunCommand`'s `if` chain, `compiler/cli.salam:1279`                             |
| help text        | `cli_print_usage`, `c/src/cli/cli_help.c`             | `UsageText`, `compiler/cli.salam:1125`                                           |

The report/diff/folded rendering itself is a pure function of the `.prof` text
file, so it is the one part worth writing _once_ as a Salam package
(`std/prof`'s `Report`/`Diff`/`Folded` reading a parsed file) and calling from
both drivers, rather than porting a renderer twice. The C driver already links
nothing of the sort today, so if that proves awkward, the fallback is a small
duplicated renderer in `c/src/driver/prof_cmd.c` +
`compiler/prof_cmd.salam` - which is the same shape as every other
already-duplicated driver command.

Subcommands:

```
salam prof report [FILE]           # flat table, default sort by self time
salam prof report --sort=total|self|calls|name --top=N
salam prof graph  [FILE]           # caller/callee tree (needs mode=graph)
salam prof folded [FILE]           # collapsed stacks, feeds flamegraph.pl / speedscope
salam prof diff   OLD NEW          # per-function delta, for regression hunting
salam prof json   [FILE]           # machine-readable
```

`FILE` defaults to `salam.prof`.

`salam prof report` output:

```
Profile of app (mode fn, c backend), wall 1.204 s

   self%    self      total%     total     calls    function
   ------------------------------------------------------------------
   69.8%    0.840 s    69.8%    0.840 s   1,664,079  fib             app.salam:3
    0.2%    0.002 s   100.0%    1.204 s           1  main            app.salam:12
    0.0%    0.000 s     0.2%    0.002 s           3  load_table      app/data.salam:41

  29.9% of wall time is unattributed (uninstrumented code, runtime, syscalls).
```

That last line matters. Instrumented profilers only see instrumented frames, and
a report that silently sums to 70% and does not say why trains users to distrust
it. Print the residual explicitly.

`salam prof diff` output, which is the CI-facing one:

```
   delta      old        new      calls old/new   function
   -----------------------------------------------------------------
   +31.2%    0.640 s    0.840 s   1.66M / 1.66M   fib
    -8.1%    0.021 s    0.019 s      3 / 3        load_table
```

with exit code 1 when any function regresses by more than `--threshold=P`
(default: no gating, gating is opt-in).

`salam prof folded` writes `main;load_table 2200410` style lines, which is the
input format for Brendan Gregg's `flamegraph.pl` and for speedscope. That gives
flamegraphs without us writing an SVG renderer.

### B.8 Overhead

Expected and to be verified against `benchmark/programs/01_fib_recursive`
(the call-heavy one) as the worst case:

| Mode    | Expected overhead on `01_fib_recursive` |
| ------- | --------------------------------------- |
| `count` | 1.5x to 2x (one increment per call)     |
| `fn`    | 4x to 8x (two clock reads per call)     |
| `graph` | 5x to 10x                               |

These are honest numbers for an instrumenting profiler on a workload that is
nothing but function calls; on realistic code the overhead is far lower. The
`salam prof report` header should print the mode so nobody compares an
instrumented run's absolute time against an uninstrumented one.

`--profile-min=N` filters the _report_, not the collection, so filtering does not
change measured times.

---

## 4. Part C: benchmark harness

### C.1 `std/bench`

New package `std/bench/bench.salam`, modeled on Go's `testing.B` because that
design is well proven and users coming from Go will recognize it.

```salam
@en "bench"  @fa "محک"  @ar "قیاس"
package bench

pub struct B:
    pub n: i64            // iterations the harness wants this run
    pub name: str
    // internal
    start_ns: i64
    elapsed_ns: i64
    bytes: i64
    timer_on: bool
end

@en "ResetTimer"   pub func ResetTimer(b &: B): ... end
@en "StopTimer"    pub func StopTimer(b &: B): ... end
@en "StartTimer"   pub func StartTimer(b &: B): ... end
@en "SetBytes"     pub func SetBytes(b &: B, n: i64): ... end   // enables MB/s column

// Prevents the optimizer from deleting the loop body when the result is unused.
@en "Keep"         pub func Keep(v: i64): ... end
@en "KeepFloat"    pub func KeepFloat(v: f64): ... end
@en "KeepStr"      pub func KeepStr(v: str): ... end

// Runs `body` adaptively and returns the result. `body` takes the iteration
// count and must loop internally.
@en "Run"          pub func Run(name: str, body: func(i64)): Result: ... end

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

A benchmark file looks like:

```salam
package main

import bench

func BenchFibRecursive(b &: bench.B):
    mut i := 0 as i64
    until i < b.n:
        bench.Keep(fib(30))
        i += 1
    end
end
```

### C.2 `Keep` and why it is not optional

Under `gcc -O3` or LLVM `-O3`, a loop whose result is discarded is deleted
outright and the benchmark measures an empty loop. `Keep` must be a real barrier.
Implementation: a `pub mut _sink: i64` module variable that `Keep` writes to, and
which the compiler cannot prove is unread because it is `pub` and lives in
another translation unit. That is enough for the C backend with separate module
compilation, and it survives LTO only if the sink is also read somewhere
observable, so `bench`'s report path reads `_sink` once at the end. Under
`-flto`, document that `Keep` is best-effort and recommend `SetBytes` +
checksum-style benchmarks for LTO builds.

### C.3 Adaptive iteration and statistics

`Run(name, body)`:

1. **Warmup.** Run with `n = 1`. If it takes less than `min_time` (default
   100 ms), scale `n` up by `min_time / elapsed`, rounded up to a "nice" number
   (1, 2, 5, 10, 20, 50, ... times a power of ten, so reported `n` values are
   readable), and repeat until the run reaches `min_time` or `n` hits
   `max_iters` (default 1e10). This is Go's algorithm and it is the right one.
2. **Sampling.** Once `n` is fixed, run it `samples` times (default 8, `--count=N`
   to change). Discard the first sample as cache warmup unless `samples < 3`.
3. **Statistics.** Report `min`, `median`, `mean`, `stddev` of ns/op. Lead with
   **median**, not mean: benchmark noise is one-sided (the machine can only be
   slower than ideal, never faster), so the mean is dragged by outliers and the
   median is the honest central estimate. Report `min` too, because for
   deterministic CPU-bound code the minimum is the closest thing to the true
   cost. This matches what `benchmark/RESULTS.md` already does for the
   cross-language suite ("geometric-mean minimum time"), so the two stay
   comparable.
4. **Stability warning.** If `stddev / median > 0.05`, print a `noisy` marker on
   that row. A benchmark harness that reports 3 significant figures on data with
   20% variance is lying to the user.

### C.4 `salam bench` command

New `CMD_BENCH`, wired the same way as `CMD_PROF`.

```
salam bench                       # discover, build with -O2, run all
salam bench ./pkg                 # only that directory
salam bench --run=REGEX           # filter by benchmark name
salam bench --count=N             # samples per benchmark (default 8)
salam bench --min-time=MS         # per-sample target (default 100)
salam bench --json=FILE           # machine-readable results
salam bench --baseline=FILE       # compare against a previous --json
salam bench --fail-under=PCT      # exit 1 if any benchmark regresses by > PCT
salam bench --cc=... -O3 ...      # build flags pass through
```

**Discovery.** Files matching `bench_*.salam` or `*_bench.salam`, and inside
them, top-level functions whose name starts with `Bench` (English) or the
langpack's translated equivalent.

**Per-compiler split.** `std/bench` is shared, like `std/prof`: one Salam
package, consumed identically by both compilers, so the adaptive-iteration
algorithm and the statistics exist once. What is written twice is only the
`bench` _command_: discovery, driver synthesis, and result rendering.

| Piece                                                      | C                                                                                  | Salam                                                                                              |
| ---------------------------------------------------------- | ---------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------- |
| `CMD_BENCH` constant + dispatch + options + handler + help | the five rows of the B.7 table, same files                                         | same, `compiler/{driver,cli}.salam`                                                                |
| new `bench` command body                                   | `c/src/driver/bench_build.c` (new), alongside `doc_build.c` / `serve_build.c`      | `compiler/bench_build.salam` (new), imported by `cli.salam`                                        |
| directory walk for `bench_*.salam`                         | `list_salam_files_in`, `c/src/driver/build.c:367`                                  | `list_salam_files`-equivalent in `compiler/driver.salam`                                           |
| "does this file define X" scan                             | `file_has_entry`, `c/src/driver/build.c:419`                                       | its `driver.salam` counterpart                                                                     |
| langpack bench prefix                                      | `langpack_bench_prefix`, next to `langpack_entry` (`c/src/langpack/langpack.h:34`) | `lp.BenchPrefix`, next to `pub func Entry` (`compiler/langpack.salam:429`) and `EntryFor` (`:443`) |
| build + run the synthesized driver                         | reuse `driver_build` (`build.c:487`)                                               | reuse `dr.Build` (`compiler/driver.salam`)                                                         |

The langpack addition is the only one that touches a data table rather than
code: `langpack_entry` already returns the per-language name of `main`, so
`langpack_bench_prefix` returns the per-language `Bench` prefix from the same
three tables (`c/src/langpack/langpack_data.c` and its `compiler/langpack.salam`
twin). Without it, an `@fa` source could not name a benchmark in Persian, which
would be the only place in the language where that is true.

**Generated driver.** `salam bench` synthesizes a `main` into the scratch dir
(`.salam-build`, `driver.c:74`; `salam_scratch_dir()`'s Salam counterpart in
`compiler/driver.salam`) that imports the discovered files and calls `bench.Run`
for each discovered function, then builds and runs that. This mirrors how the
test suite is driven and avoids needing reflection in the language.

Because the synthesized `main` is ordinary Salam, both compilers produce a
byte-identical driver from the same discovery result. That makes the C-vs-Salam
parity test for Part C trivial: synthesize with each binary, `diff` the two
generated files, and require them to match.

The synthesized file must be written into a **per-invocation subdirectory** of
`.salam-build`. Per `salam_ssh_pkg_bugs_and_landmines`, the shared `.salam-build`
directory collides across concurrent sessions; a bench run that clobbers a
concurrent build is a nasty failure mode.

**Output:**

```
salam bench (c backend, gcc -O2), 8 samples, min-time 100ms

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

Deltas smaller than the combined stddev of the two runs are annotated
"(within noise)" and never trip `--fail-under`. A regression gate that fires on
noise gets disabled by the team within a week, so the gate must be
noise-aware to be useful.

### C.5 JSON schema

```json
{
  "schema": "salam.bench.v1",
  "salam_version": "0.3.0",
  "backend": "c",
  "cc": "gcc",
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

The `host` block is what makes `--baseline` trustworthy: `salam bench` warns
loudly when the baseline's host block differs from the current one, because
comparing timings across machines is meaningless.

### C.6 Relationship to the existing `benchmark/` suite

`benchmark/` (39 cross-language programs, `benchmark/bench/*.salam` runner) stays
as it is. It answers "how does Salam compare to C/Rust/Go", which is a different
question. Two integration points:

1. `benchmark/bench/timing.salam` currently does its own timing. Point it at
   `time.MonoNanos` from section 1.3 so the cross-language suite stops using the
   15.6 ms Windows clock. This is the highest-value single line in this whole
   document for the accuracy of the numbers in `benchmark/RESULTS.md`.
2. `benchmark/bench/report.salam` and `salam bench --json` should emit
   compatible statistics fields so one set of downstream tooling reads both.

**Filename collision warning.** Per
`salam_filename_collision_across_stdlib_packages`, a `.salam` file's C module is
keyed by _filename_, not package. `std/bench/bench.salam` and
`benchmark/bench/bench.salam` are the same filename. They will not normally be
linked into the same program, but `benchmark/bench/run.salam` importing `bench`
would suddenly resolve differently. Rename the harness file to
`std/bench/harness.salam` with `std/bench/bench.salam` as the thin package marker
(the marker file is required anyway for translated dotted imports, per
`salam_stdlib_parent_alias_landmine`), or rename the suite's file. This must be
decided before the first commit, not after.

---

## 5. Cross-cutting requirements

### 5.1 Localization

- Every new `pub` symbol in `std/prof` and `std/bench` needs `@en`, `@fa` and
  `@ar` annotations. Missing ones break translated imports.
- Every new user-visible string in `c/src/` goes through `i18n_tr(...)`, and the
  same key is added to the `fa`/`ar` catalogs in `c/src/i18n/`.
- `compiler/i18n.salam` needs the mirrored entries.
- Report _tables_ are not translated string-by-string; column headers are, and
  numbers go through `sal_format` for digit shaping.

### 5.2 Help text

`c/src/cli/cli_help.c` and `compiler/cli.salam`'s help path both need:

- the two new commands (`prof`, `bench`) in the command list,
- `--time-report`, `--time-trace`, `--profile`, `--profile-out`, `--profile-min`,
  `--profile-exclude` under build options.

Remember `salam_cli_reference`: the CLI is `salam help`, not `salam --help`, so
the reference doc and any examples must use that form.

### 5.3 Tests

New section `bench` in `tools/bash/run-tests.sh` (modelled on the `opencv`
section at line 563, which degrades to SKIP when a dependency is missing):

| Test                                                       | Asserts                                                                                                                                         |
| ---------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------- |
| `tests/en/stdlib/prof_api.salam`                           | hand-called `BeginRegion`/`EndRegion`, `Report()` contains the region name, elapsed > 0                                                         |
| `tests/en/stdlib/bench_stats.salam`                        | feed `bench`'s statistics functions known arrays, check median/stddev to a fixed epsilon                                                        |
| shell: `salam exec --profile=count prog.salam`             | `salam.prof` exists, has a `F` record for every function in the program, call counts are exactly right (counts are deterministic, unlike times) |
| shell: `salam build --profile=fn` then `salam prof report` | exit 0, report names the hot function                                                                                                           |
| shell: `salam prof diff a.prof b.prof`                     | correct sign and magnitude on a hand-written pair of `.prof` files                                                                              |
| shell: `--time-report=json` on both binaries               | schema parity (section A.6)                                                                                                                     |
| `tests/fa/stdlib/prof_api.salam`                           | the Persian-named API resolves and works                                                                                                        |

The deterministic call-count test is the load-bearing one: it validates
`Enter`/`Exit` pairing on every control-flow path, including early `ret`,
`defer`, recursion and lambdas, without depending on timing at all. Give it a
program that exercises each of those shapes deliberately.

**Parity tests, one per part.** Each follows the shape already shipped for Part
A's `timereport/parity`: run both binaries, compare the part of the output that
is deterministic, and SKIP rather than FAIL when `SALAM_SELFHOST` is unset.

| Part        | What is compared                                                                | Why it is deterministic                                          |
| ----------- | ------------------------------------------------------------------------------- | ---------------------------------------------------------------- |
| A (shipped) | the set of phase keys in `--time-report=json`                                   | timings differ, key sets must not                                |
| B           | the whole `.prof` file with the `wall_ns`/`self_ns`/`total_ns` columns stripped | call counts and the slot table are exact, times are not          |
| C           | the synthesized bench driver `main`, byte for byte                              | it is generated from the same discovery result by the same rules |

Part B's comparison is the strongest of the three: identical call counts from
two independently written instrumentation passes means both got `Enter`/`Exit`
pairing right on every control-flow path, which is the only part of this feature
where a silent wrong answer is plausible.

### 5.4 Docs

- `docs/PROFILING.md` (this file) becomes the user guide once implemented.
- `docs/GETTING_STARTED.md` gets a short "measuring performance" pointer.
- `benchmark/README.md` gains a paragraph distinguishing the cross-language suite
  from `salam bench`.

---

## 6. Known landmines for the implementer

Collected from prior work in this repository; each one has cost a debugging session
before.

1. **Build source lists.** Adding `core/timer.c`, `core/prof_self.c` means
   editing `c/Makefile` _and_ the quick-build source lists. Commit `24fd5d43`
   exists because a previous addition missed them.
2. **`pub const` collides at link time across packages.** Prefix the new
   constants and grep the whole stdlib before choosing names.
3. **`pub enum` values cannot cross a package boundary.** Use int consts in
   `std/prof` and `std/bench`.
4. **Nested generics never work.** No `HashMap<int, Vector<Edge>>`. Flat arrays
   and hand-rolled hash tables only.
5. **`until` is `while`, not `do-until`.** When relocating existing `.salam`
   code, copy it verbatim rather than retyping; several inverted loops have been
   found and fixed already.
6. **`salam exec` gets unsigned math wrong.** Do not validate profiler counters
   with the interpreter alone; use `salam build`.
7. **Prefer tcc on Windows for the C backend.** Per the notes on `--cc=gcc`,
   GCC 16.1.0 on this host has a codegen bug broader than `str.Split`. Test
   `--profile` builds on tcc first.
8. **Filename keying across packages.** See section C.6; also check that
   `prof.salam` does not collide with anything (it currently does not; verified
   against `std/`).
9. **Never edit `run-tests.sh` while a run is in flight.**
10. **A concurrent Claude Code session may be working this same repository/branch.**
    Re-check `git status` before trusting file state.

---

## 7. Suggested implementation order

Each step is independently shippable and independently useful.

| Step | Work                                                                                                                                      | Why first                                                                                                    |
| ---- | ----------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------ |
| 1    | Section 1: `salam_mono_ns` in C, `MonoNanos` in `compiler/` and `std/time`, repoint `std/debug` timers and `benchmark/bench/timing.salam` | Fixes a real accuracy bug today, unblocks everything else, tiny surface                                      |
| 2    | Part A: `--time-report` table format only, C compiler                                                                                     | Self-contained, immediately useful to compiler developers                                                    |
| 3    | Part A parity: `compiler/prof_self.salam` + JSON format + `--time-trace`                                                                  | Parity rule; JSON enables the CI check                                                                       |
| 4    | Part B runtime: `std/prof` package + hand-called `BeginRegion`/`EndRegion` + `Dump`                                                       | Testable with zero codegen changes                                                                           |
| 5    | Part B: interpreter instrumentation (`salam exec --profile`) + `salam prof report`/`folded`/`diff`                                        | No codegen risk, delivers a working end-to-end profiler and validates the file format and the report tooling |
| 6    | Part B: C-backend codegen instrumentation via the defer mechanism, both compilers                                                         | The riskiest step, and by now the format and tooling are already proven                                      |
| 7    | Part B: LLVM and JS backends                                                                                                              | Parity                                                                                                       |
| 8    | Part C: `std/bench` + `salam bench` + JSON + `--baseline`                                                                                 | Builds on the step 1 clock and the step 5 reporting conventions                                              |
| 9    | `--profile=graph`, `salam prof graph`, CI regression gate wiring                                                                          | Refinement                                                                                                   |

Steps 1, 2, 4 and 5 together already give users a usable profiler and a faster
feedback loop on compiler performance, which is most of the value.

---

## 8. Implementation notes for §1 and Part A

What the built code does differently from the design above, and why. Each of
these was a real defect found by running the thing, not a preference.

### 8.1 Labels must be copied, not borrowed

The design said scope labels are "borrowed, not copied, so [they] must outlive
the report (source paths out of the arena qualify)". They do not. `driver_build`
frees its arena at the end, and the report prints afterwards from the
`driver_main` wrapper, so every label in the file table was a dangling pointer.
The first working build printed the phase table and then died partway through
"slowest inputs".

`file_acc_t` now owns a `char label[200]` copy, and long paths keep their tail
rather than their head (the interesting part of `.../std/encoding/json/json.salam`
is the end). Trace events point at that owned copy or at the static phase name,
never at the caller's string. The Salam port copies into `_file_label` for the
same reason plus one more: it keeps both implementations producing identical
file lists.

### 8.2 64-bit values are rendered by hand

`%lu` is 32-bit on Windows and `%llu` is not dependable across every host C
compiler this project builds with (tcc included). Every 64-bit number in
`prof_self.c` goes through a local `u64s()` and prints with `%s`. Getting this
wrong would have produced silently truncated nanosecond counts rather than a
visible failure.

### 8.3 The cc phase is charged to the `.salam` file, not the `.c` file

The first working version labelled the `TP_CC` scope with the generated
`.salam-build/salam_mod_text.c` path, which put generated files and source files
in the same "slowest inputs" table under two different naming schemes. `build.c`
now carries a `csrc[]` array parallel to `cfiles[]`, so the host C compiler's
time lands on the `.salam` file the user actually wrote and every phase
aggregates per input. `TP_LINK` still uses the output binary's name, which is
correct: the link is not attributable to one input.

### 8.4 Language rules that shaped `compiler/prof_self.salam`

Three of them, all enforced by the compiler and all worth knowing before writing
a new `compiler/*.salam` file:

- **E085**: module-level `mut` globals must appear before any function or type
  definition. `cli.salam`'s three new globals had to move above `struct FmtCtx`.
- **E088**: once a `pub func` appears, every later function must also be `pub`.
  This is why `prof_self.salam` is laid out as "all private helpers, then all
  public API", and why `cli.salam`'s `run_inner` became `pub func RunCommand`
  rather than a private helper.
- **E083**: top-level `extern:` blocks (including ones inside a platform `if`)
  must precede the rest of the file. `sal_core.salam` therefore declares
  `GetProcessTimes`/`clock` next to its existing extern block, with the function
  bodies at the far end of the file.

Also: arithmetic on a `void*` is not an operator this language has, so
`CpuNanos()` allocates four separate one-`FILETIME` buffers instead of one
32-byte buffer with offsets.

### 8.5 Where the two implementations legitimately differ

The parity test compares the report's _shape_, not its values, and these are the
values that differ by design:

| Field                           | C                                                                     | Self-hosted                                                                                                                               |
| ------------------------------- | --------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------- |
| `peak_rss`                      | real (psapi resolved at runtime via `GetProcAddress`, or `getrusage`) | always 0; there is no runtime-resolved function pointer here, and a link-time psapi dependency is the kind of extern that fails under tcc |
| `ast_nodes`                     | exact (`ast_new` counts every node)                                   | 0; the self-hosted AST is an `at.NodeId` arena with no equivalent single choke point wired up yet                                         |
| `arena_bytes` / `arena_blocks`  | real (`arena_stats()`)                                                | 0; `sal_core.salam`'s `Arena` has no block list to walk                                                                                   |
| `cc` calls                      | one per module                                                        | one, because the self-hosted driver compiles all modules in a single `compile_parallel` scope                                             |
| `lexer`/`parser`/`source` calls | 31                                                                    | 12, because the C counts each sibling file of a multi-file package separately                                                             |

The phase key set is identical, which is what `timereport/parity` asserts.

### 8.6 Building and running the checks

```sh
# C compiler
make -C c CC=gcc

# self-hosted compiler
salam build --stdlib-path=./std -o salamx.exe compiler/main.salam

# the three timereport checks, including C-vs-Salam parity
SALAM=./salam SALAM_SELFHOST=./salamx.exe sh tools/bash/run-tests.sh timereport

# the clock assertions
SALAM=./salam sh tools/bash/run-tests.sh stdlib
```

`timereport/parity` **skips** rather than fails when `SALAM_SELFHOST` is unset,
the same way the `llvm` and `opencv` sections degrade when their optional
dependency is missing.
