# Testing Salam programs

`salam test` discovers and runs test files. It needs no configuration: point it
at a directory (the default is the current one) and it walks it recursively.

```sh
salam test                     # every test file under the current directory
salam test src tests           # only these directories
salam test math_test.salam     # one file
```

## Which files are tests

Two names are recognised, in English and Persian projects alike:

- `*_test.salam` (for example `math_test.salam`)
- `*.test.salam` (for example `parser.test.salam`)

Directories named `.git`, `node_modules`, `build`, `out`, `target`, `dist`,
`vendor`, `bin` and `obj` are skipped. Files are sorted and de-duplicated, so
the output is stable. `salam test --list` prints the discovered files without
running anything.

## Three kinds of test file

### 1. Unit tests: `test_` functions

A file of `test_` functions (Persian: `آزمون_...` or the Persian test prefix)
is compiled together with a generated `main` that runs every test through the
`testing` package.

```salam
import testing

func test_add():
    testing.AssertEqInt(2 + 2, 4)
end
```

- A test function takes no parameters and returns nothing.
- A unit test file must not define `main`; the runner writes its own.
- `salam test --run=part` keeps the tests whose name contains `part`;
  `--run=^name$` matches exactly.

### 2. Whole-program tests: `func main`

A file with `main` and no test functions is run as a program. Its exit code is
the test result (0 = pass), and `// EXPECT` directives check its output.

```salam
// EXPECT: hello
// EXPECT-NOT: goodbye
// EXPECT-EXIT: 0
func main:
    println "hello"
end
```

### 3. Compile-check tests: `// EXPECT-CODE`

A file with a compile-check directive is type-checked instead of run. This is
how compiler diagnostics are tested.

```salam
// EXPECT-CODE: E059
func main:
    unused := 1
end
```

`// EXPECT: E059` (a bare diagnostic code) also selects check mode.

## Directives

Directives are comments at the top of the file, one per line, before any code
(license block comments are skipped). English and Persian spellings are
equivalent. Values are separated from the name by `:`. A directive-shaped
comment inside a function body is not a directive.

| Directive (English / Persian) | Meaning |
| --- | --- |
| `EXPECT` / `انتظار` / `توقع` | Standard output must contain this text |
| `EXPECT-NOT` / `انتظار-نه` / `توقع-نه` | Standard output must not contain this text |
| `EXPECT-LINE` / `انتظار-خط` / `توقع-خط` | Some line of standard output must equal this text |
| `EXPECT-STDERR` / `انتظار-خطا` / `توقع-خطا` | Standard error must contain this text |
| `EXPECT-STDERR-NOT` / `انتظار-خطا-نه` / `توقع-خطا-نه` | Standard error must not contain this text |
| `EXPECT-EXIT` / `انتظار-کد` / `توقع-کد` | Exit code must be this number |
| `EXPECT-CODE` / `انتظار-کد-خطا` / `توقع-کد-خطا` | The compiler must report this diagnostic code (check mode) |
| `EXPECT-ERROR` / `انتظار-خطا-کد` / `توقع-خطا-کد` | Alias of `EXPECT-CODE` |
| `EXPECT-MSG` / `انتظار-پیام` / `توقع-پیام` | The compiler output must contain this text (check mode) |
| `EXPECT-WARNINGS` / `انتظار-هشدارها` / `توقع-هشدارها` | Exact number of compiler warnings (check mode) |
| `EXPECT-FIXES` / `انتظار-اصلاحها` / `توقع-اصلاحها` | Exact number of suggested fixes (check mode) |
| `EXPECT-FILE` / `انتظار-پرونده` / `توقع-پرونده` | Standard output must equal the contents of this file (path relative to the test file) |
| `SKIP` / `رد شدن` / `ردشدن` | Skip this file; the value is the reason |
| `ONLY-OS` / `فقط-سیستم` / `فقط سیستم` | Run only on this OS (`linux`, `mac`, `windows`, ...) |
| `SKIP-OS` / `رد-سیستم` / `رد سیستم` | Skip on this OS |
| `ONLY-ARCH` / `فقط-معماری` / `فقط معماری` | Run only on this architecture (`x64`, `arm64`, ...) |
| `SKIP-ARCH` / `رد-معماری` / `رد معماری` | Skip on this architecture |
| `TIMEOUT` / `مهلت` | Deadline for running the file, in milliseconds (compilation gets up to five extra minutes) |
| `ARGS` / `آرگومانها` / `آرگومان ها` | Extra arguments for the compiler in check mode |

Persian digits are accepted in numeric values, so `// انتظار-کد: ۰` and
`// EXPECT-EXIT: 0` are the same.

## Options

| Option | Effect |
| --- | --- |
| `--run=NAME` | Run only tests whose name contains `NAME`; `^name$` matches exactly |
| `--format=pretty\|teamcity` | Human output (default) or TeamCity service messages |
| `--jobs=N` | Run at most N files at once (default: one per CPU core, at most 256) |
| `--no-parallel` | Run one file at a time |
| `--fail-fast` | Stop starting new files after the first failure |
| `--list` | Print the discovered files and exit |
| `--verbose` | Print the output of passing tests too |
| `--timeout=MS` | Deadline for each test file: starting the compiler and running the file, in milliseconds (default: no limit) |
| `--report=FILE` | Write a machine-readable summary to FILE (one tab-separated line per file) |
| `--interp` | Run tests with the interpreter instead of compiling them |

Parallel runs are deterministic: each file's output is printed in sorted file
order after it finishes, so two runs produce the same text. Each test file is
compiled into its own temporary directory, so parallel builds never share state.

## Exit codes

- `0`: every selected test passed (skipped files do not fail the run)
- `1`: at least one test failed, or `--run` matched nothing
- `2`: the command itself is wrong (a file cannot be read, a test file has both
  test functions and `main`, an unknown `--format`)

## Reporting

`--format=teamcity` emits `testSuiteStarted`, `testStarted`, `testFailed`,
`testFinished` and `testIgnored` messages for editors and CI. `--report=FILE`
writes one tab-separated line per file: status (`pass`/`fail`/`skip`/`nomatch`/
`notrun`), path, test count, passed, failed, and a reason. It is useful from
scripts that need a summary without parsing text. A single-file run (`--one`)
writes the same information in the older three-line form instead.
