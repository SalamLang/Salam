---
title: Interpreted or compiled? salam run, salam exec and salam build explained
seo_title: salam run vs salam build vs salam exec: Compiled or Interpreted?
description: The difference between salam check, salam run, salam build and salam exec, how Salam's interpreter and REPL work, and which command to use for each job.
keywords: salam run, salam build, salam exec, compiled vs interpreted, Salam interpreter, Salam REPL, salam check, run a Salam program, bytecode interpreter, compile and run
category: compiler
summary: Salam is a compiled language that also ships an interpreter and a REPL. What each of check, run, build and exec actually does, how they differ, and a simple guide to picking the right one.
date: 2026-11-11
---

**Salam is a compiled language**: `salam build` turns a program into a native executable, and that executable is what you deploy. The toolchain also gives you three other ways to work with the same code. `salam check` stops before generating any code and only reports errors. `salam run` compiles to a temporary executable, runs it and throws it away. `salam exec` runs the program in an interpreter built into the compiler, with no C compiler, LLVM or linker involved. There is also a REPL, `salam cli`, for trying things a line at a time. All of them read the same source and apply the same checks. This article explains what each command does underneath and when to use it.

## The four commands at a glance {#short-answer}

| Command | What it does | Produces | Typical use |
|---|---|---|---|
| `salam check app.salam` | Lex, parse and fully analyze, imports included | Diagnostics only | After every edit |
| `salam run app.salam` | Check, compile, run, delete the executable | Program output | Trying a program, scripts, examples |
| `salam build app.salam` | Check, compile, link | An executable you keep | Releases, benchmarks, deployment |
| `salam exec app.salam` | Check, then run in the interpreter | Program output | Machines without a toolchain, quick logic checks |

The REPL (`salam cli`) is a fifth way in, covered below. One program can go through all of them unchanged. Here is a prime sieve that we will use throughout:

```salam
func count primes(limit: int): int:
    mut composite := Vector {} as Vector<bool>
    defer composite.free()
    repeat limit + 1:
        composite.push(false)
    end
    mut count := 0
    repeat 2 to limit in n:
        if composite.get(n):
            continue
        end
        count++
        mut multiple := n * 2
        until multiple <= limit:
            composite.set(multiple, true)
            multiple += n
        end
    end
    ret count
end

func main:
    each limit in [100, 10000, 1000000]:
        println "primes up to", limit, "=", count primes(limit)
    end
end
```

```output
primes up to 100 = 25
primes up to 10000 = 1229
primes up to 1000000 = 78498
```

`salam run`, `salam exec`, and the executable from `salam build`, with any of the three backends, all print exactly these three lines.

## salam check: find the errors without building anything {#check}

`salam check` runs everything `salam build` does before code generation: lexing, parsing, type checking, the range and ownership analyses, and the same for every imported package. It prints the diagnostics and exits with status 1 if there are any errors, or prints `app.salam: no errors`. It never invokes a C compiler or LLVM, so it is the fastest feedback you can get, and it works on a single file or a whole directory:

```bash
salam check app.salam
salam check src/
salam check app.salam --error-format=json
```

What it cannot see is anything that only happens later: a problem in the generated C, a missing system library at link time. For those you need a real build. The [compiler pipeline article](/blog/how-the-salam-compiler-works/) shows where `check` stops, and the [static typing article](/blog/static-typing-in-salam/) shows what it catches.

## salam run: compile, run, discard {#run}

`salam run app.salam` does a full build into a temporary location, runs the result, passes its exit status back to you and deletes the executable. It is what you use while developing a program, for one-off scripts and for examples. Arguments for the program go after `--`:

```bash
salam run tool.salam -- input.txt --verbose
```

Because it is a real compiled build, `run` takes the same options as `build`: `--backend=native`, `--release`, `--asan` and so on. Inside a project directory, `salam run` with no file finds the entry point (`main.salam`) by itself. A program's exit status is whatever `main` returns: `ret 3` in `main` makes `salam run` exit with 3, which is how a Salam script reports failure to a shell or a CI job.

The cost of `run` is the compile step on every invocation. The C backend caches the object files of unchanged modules between builds, which makes repeated runs of the same program cheaper, but each run still compiles and links.

## salam build: an executable you keep {#build}

`salam build app.salam` produces an executable named after the file (`app`, or `app.exe` for Windows targets), or the name given with `--output=FILE`. With no input it builds the project in the current directory. If nothing has changed since the last build, it prints `up to date` and does nothing.

The options that matter most:

- **Development and release builds.** By default a build keeps the safety checks: array and `Vector` indices are checked at run time, and signed arithmetic is guaranteed to wrap. `--release` removes the bounds checks, lets the optimizer assume signed arithmetic never overflows and raises LLVM's optimization from `-O2` to `-O3`. Test with the default, ship with `--release` when you need the speed.
- **Debugging.** `-g` embeds DWARF debug information, and `salam debug app.salam` builds that way and starts GDB or LLDB. `salam memcheck` builds and runs the program under Valgrind or AddressSanitizer.
- **Backends.** `--backend=c`, `llvm` or `native` picks the code generator; the article [Three ways to generate code](/blog/salam-code-generation-c-llvm-native/) explains the trade-offs.
- **Targets.** `--target=TRIPLE` cross-compiles, for example to Windows from Linux. See [cross-compiling Salam](/blog/cross-compilation-salam/).
- **Libraries.** `--lib` builds a shared library (`.so`, `.dylib`, `.dll`) or, with `--lib=static`, a static archive, together with a C header.

The executable needs nothing from the Salam toolchain to run. You can copy it to another machine with the same operating system and architecture and start it there.

## salam exec: the interpreter {#exec}

`salam exec app.salam` runs the program without producing machine code. After the same front end and checks as every other command, it hands the checked syntax tree to an interpreter that lives inside the `salam` binary (`compiler/interp/`). Despite the "tree-walking interpreter" description in `salam help`, the current interpreter does a little more: the first time a function is called, it is translated into **register bytecode** for a small virtual machine (`interp_vm.salam`), and anything the bytecode compiler does not handle natively falls back to evaluating the tree directly, so the results stay exact. Setting `SALAM_VM=0` turns the bytecode path off. The interpreter's values live in its own heap and are cleaned up by a mark-and-sweep garbage collector, unlike compiled Salam, which has none.

When is that useful?

- **No toolchain.** A `salam` binary without LLVM normally needs a C compiler to build anything. `exec` needs nothing else at all, which matters on a locked-down machine, in a minimal container or when you are just getting started.
- **Quick checks of logic.** For a small program, interpreting can be quicker than compiling, and `--timeout=MS` stops a program that loops forever.
- **Tests.** `salam test --interp` runs a project's tests through the interpreter instead of compiling them.

And its limits. The interpreter is much slower than compiled code on anything compute-heavy, so it is not a way to run production workloads. The repository's own guidance is to treat the compiled build as the source of truth: the interpreter covers the language, but some low-level features (parts of C interop and threading) are only fully supported by the compiled backends. If `exec` and `build` ever disagree, trust `build` and report the difference.

## The REPL: salam cli {#repl}

`salam cli` starts an interactive prompt. Each line is either a definition (a function, a struct, an enum) or a statement, and the REPL keeps them as a session: a definition with a name you used before replaces the old one, and statements accumulate. A session looks like this:

```text
Salam general REPL (lang=en). ':help' for commands.
salam> x := 6
salam> println x * 7
42
salam> func twice(n: int): int:
  ...     ret n * 2
  ... end
defined twice.
salam> println twice(x)
12
salam> :quit
bye.
```

Under the hood the REPL is not an interpreter. Every turn, it writes the whole session out as a program, with your statements placed inside a generated `func main:`, compiles it with the normal compiler, runs it and shows only the output that is new. `:defs` prints that generated program, `:undo` drops the last statement, `:save FILE` and `:load FILE` store and restore a session, `:clear` starts over. Because each turn is a real compile, errors in the REPL are the same compiler errors you get from `salam check`. `salam page` is the matching REPL for the page language that generates HTML and CSS.

## Compiled vs interpreted: what really differs {#compiled-vs-interpreted}

The classic distinction is that a **compiled** language is translated to machine code before it runs and an **interpreted** one is executed by another program as it goes. In practice most languages mix the two: Java compiles to bytecode and then JIT-compiles it, CPython compiles to bytecode and interprets it, and JavaScript engines do several of these at once. "Compiled" and "interpreted" describe implementations more than languages.

Salam's case is clear, though. The language is designed for ahead-of-time compilation: static types, no runtime reflection needed for ordinary code, no garbage collector in the compiled output. The interpreter is a convenience built on the same front end, not a second runtime with its own semantics. For Salam programs, the practical differences are:

| | Compiled (`build`, `run`) | Interpreted (`exec`) |
|---|---|---|
| Needs | A backend toolchain (built into release binaries, or a C compiler) | Only the `salam` binary |
| Speed of the program | Native speed, in the [same range as C and Rust](/speed/) | Much slower on heavy computation |
| Memory | Freed by the program, no collector | Interpreter heap with a garbage collector |
| Feature coverage | The whole language | Nearly all; the compiled build is the reference |
| Output | An executable (kept by `build`) | Nothing but the program's output |

The [native compilation article](/blog/native-compilation-without-vm-or-gc/) explains why the compiled form is what Salam is built around.

## Which command should you use? {#which}

- **After every edit:** `salam check`. It is the fastest loop and shows every error the compiler can find without building.
- **While developing and for scripts:** `salam run`. One command, real compiled behavior, nothing left behind.
- **For anything you ship, time or deploy:** `salam build`, with `--release` once it is tested. Measure performance only on built executables.
- **On a machine with no toolchain, or for a quick look at pure logic:** `salam exec`.
- **To try a one-liner or explore an API:** `salam cli`.

If you are new to Salam, the [first lesson](/learn/start/) uses `salam run` and `salam exec` from the start, the [tools page](/tools/) lists every command, and the [install page](/install/) gets you the compiler in one command.

## Frequently asked questions {#faq}

### Is Salam compiled or interpreted?

Compiled. `salam build` produces native machine code through C, LLVM or Salam's own native backend. The toolchain also includes an interpreter, `salam exec`, for running programs without a C compiler, but compiled builds are the reference.

### What is the difference between salam run and salam build?

Both compile the program the same way. `salam run` runs the result immediately and deletes it; `salam build` keeps the executable so you can run it later, copy it or ship it.

### Does salam run compile the program every time?

Yes, it does a real build each time, though the C backend reuses the cached object files of modules that did not change. For repeated runs of the same program, build it once with `salam build` and run the executable.

### Does Salam have a REPL?

Yes. `salam cli` starts it. It keeps a session of definitions and statements, recompiles the session on each turn and prints only the new output, with commands such as `:defs`, `:undo`, `:save` and `:load`.
