---
title: Is Salam really faster than C and Rust? A careful reading of the benchmark
seo_title: Is Salam Faster Than C and Rust? Reading the Benchmark
description: Salam's benchmark puts it in the same range as C and Rust, not ahead of them: how the programs were timed, what the averages hide, where Salam loses, and how to rerun it.
keywords: Salam performance, programming language benchmark, Salam vs C speed, Salam vs Rust speed, benchmark methodology, geometric mean benchmark, compiled language performance, microbenchmark pitfalls, HTTP router benchmark, how to read benchmarks
category: compiler
summary: A methodology-first walk through Salam's published benchmark: what was measured, how, what the averages hide, where Salam loses, and why the honest summary is the same range as C and Rust.
date: 2026-11-11
---

No, the benchmark does not show that Salam is faster than C or Rust in any general sense. It shows that on 39 small programs that use the same algorithm in every language, Salam's optimized build finished in the same range as C and Rust: an average of 10.6 ms for Salam, 11.7 ms for C and 11.3 ms for Rust. Salam was ahead of the fastest of C, C++, Rust and Go on 6 programs, level on 27 and behind on 6. Those gaps are small, the programs are short, and part of every number is process start-up rather than generated code.

That is still a meaningful result for a young language: it places Salam with the compiled languages and far from the interpreted ones, as you would expect from [native compilation without a virtual machine or garbage collector](/blog/native-compilation-without-vm-or-gc/). This article explains how the numbers were produced, where they mislead, and how to check them yourself.

## The short answer {#short-answer}

| Claim | Supported by the benchmark? |
|---|---|
| Salam compiles to code in the same performance range as C, C++, Rust and Go | Yes, on this suite |
| Salam is much faster than Python and PHP | Yes: about 56 times faster than Python and 9.6 times faster than PHP on this suite |
| Salam is faster than C or Rust | No. The average gap is within what start-up time and run-to-run noise can explain |
| Salam is fast for large applications, memory-heavy work or long builds | Not measured |

If you remember one sentence, make it the one the [speed page](/speed/) itself uses: between compiled languages, read the results as the same range, not as a ranking.

## What the benchmark measures {#what-is-measured}

The suite lives in the repository under `benchmark/`. It has 42 programs, each written in C, C++, Rust, Go, Python, PHP and Salam. The rules are simple and worth stating:

- **Same algorithm everywhere.** Each language implements the same steps, written the way that language is normally written. The suite does not let one language use a cleverer algorithm.
- **Same output everywhere.** Every program prints a deterministic result, and every version's output is checked against C's before any timing happens. A fast wrong answer does not count.
- **Mostly small CPU-bound programs.** Recursive and iterative Fibonacci, prime counting, sorting, dynamic programming, string work, matrix multiplication, a game of life, numeric integration, and one program that prints a line a million times.
- **Three library programs.** Programs 40 to 42 use Salam's own `matrix` and `tensor` packages, NumPy in Python, and hand-written loops in the other languages. They compare libraries, not compilers, and are left out of the headline averages.

The run behind the published numbers was made on 27 October 2026 at commit `8cfe206`, on a GitHub Actions `ubuntu-24.04` runner (a shared cloud virtual machine with an AMD EPYC 9V45) inside an Alpine Linux Docker image. The tools were gcc 15.2.0, rustc 1.96.1, Go 1.26.10, Python 3.14.10, PHP 8.5.12 and a development build of Salam 0.6.0. C and C++ were built with `gcc -O3`, Rust with `-C opt-level=3`, and Salam with its LLVM backend at `-O3`.

## How each time is taken {#method}

Each program runs once untimed as a warm-up, then 10 times with the wall clock running, and the fastest of the 10 is kept. The per-language average is the geometric mean of those fastest times.

Both choices are defensible, and both have consequences you should know:

- **The minimum** filters out interference from other work on a shared machine, which only ever makes a run slower. It answers "how fast can this program go here", not "how fast is it usually". The mean, median and spread of every program are in the raw data, so you can check whether a conclusion holds for them too.
- **The geometric mean** keeps one slow program from dominating. Program 28 takes 700 to 840 ms depending on the language and program 26 around 2 ms; an arithmetic mean would be a measurement of program 28. With a geometric mean, a 10% gain on any program moves the average by the same amount. The flip side is that the average no longer says anything about total time.

Wall-clock time of a whole process includes starting it: loading the executable, the dynamic linker if there is one, the C runtime's start-up, and for Go, its runtime. For a program that runs for seconds this is noise. For one that runs for 3 ms, it is a large part of the result.

## The results, read carefully {#results}

Here are the geometric means over the 39 same-algorithm programs, as published on the speed page:

| Language | Geometric mean |
|---|---|
| Salam | 10.63 ms |
| Rust | 11.29 ms |
| C | 11.68 ms |
| C++ | 13.06 ms |
| Go | 14.23 ms |
| PHP | 102 ms |
| Python | 598 ms |

With all 42 programs the order does not change: Salam 10.93 ms, Rust 12.00, C 12.10, C++ 13.47, Go 15.07.

The gap between the compiled languages is a few tenths of a millisecond on programs that take around 10 ms. The gap to the interpreted languages is an order of magnitude or two. Those are different kinds of results, and the first should not be read as a ranking.

Two observations from the same run make that concrete:

1. **A Salam build with no optimizer at all averaged 11.0 ms**, compiled through tcc, a small C compiler that does almost no optimization. That is about the same as gcc's fully optimized C. If an unoptimized build lands level with `-O3`, most rows in the average are not measuring the optimizer.
2. **The same run also measured Salam through its C backend compiled by gcc**, and that build came out level with C. That is what you would expect: when two programs go through the same optimizer, they produce similar machine code.

Look down the per-program table on the speed page with that in mind and a pattern appears: on several rows the LLVM-built languages move together. On `20_monte_carlo_pi` Rust and Salam both took 12.4 ms while C took 22.9; on `28_prime_factorization` Rust took 701.4 ms, Salam 704.8 and C 813.6. Those rows say more about LLVM against GCC on that code than about Salam against C. The article on [Salam's three backends](/blog/salam-code-generation-c-llvm-native/) explains why the backend matters this much.

The library rows (40 to 42) are reported separately for a similar reason. Salam's `matrix` package multiplied the matrices in 14.3 ms against 42.5 ms for C's plain loops, but on `41_matrix_stdlib_smallops` Salam took 36.4 ms against 4.1 ms for C, where gcc removes most of the loop work. Those rows compare a library ([matrix](/std/matrix/), [tensor](/std/ai/tensor/)) with a hand-written loop, not one compiler with another.

## Ahead, level and behind {#ahead-level-behind}

The speed page counts a program as **level** when Salam is within 5%, or within half a millisecond, of the fastest of C, C++, Rust and Go on that program. Gaps that small flip between runs. With that rule, over the 39 programs:

- **Ahead on 6:** edit distance (4.5 ms vs 7.8 for C), game of life (7.2 vs 9.1 for C), sqrt decomposition (8.1 vs 9.8 for Rust), subset sum (8.5 vs 10.3 for C), longest increasing subsequence (9.7 vs 10.8 for C) and heap sort (26.2 vs 28.1 for C).
- **Level on 27**, including iterative Fibonacci, prime counting, the sieve, quicksort, merge sort, Mandelbrot, matrix multiplication and string work.
- **Behind on 6:** LCG random numbers (19.4 vs 17.9 for C), the loop counter (5.9 vs 5.3 for C), binary search (667.7 vs 583.9 for Rust), n-queens (41.1 vs 35.7 for C), recursive Fibonacci (3.1 vs 2.4 for C) and printing a line a million times (8.2 vs 2.6 for Rust).

The comparison is against the best of four languages on each row, so "behind" can still mean beating three of them. Selection works in Salam's favor too: the same run measured Salam several ways, and the speed page reports its LLVM build, which was the fastest of them in that run. Reporting the best configuration of one language against a single configuration of the others gives that language a small edge. The repository's `benchmark/results/README.md` is regenerated by later runs on other machines and summarizes them differently: it picks the best of four Salam builds on each row and counts ties at whole-millisecond resolution, a looser rule than the speed page's. Its absolute numbers differ from the speed page and its summary is more favorable to Salam; the speed page's stricter rule is the one to quote.

## Why short programs mislead {#short-programs}

Most programs in the suite finish in 2 to 30 ms, and the median C time is 10.5 ms. On that scale:

- **Start-up is a real share of each number.** `01_fib_recursive` computes `fib(30)` and takes 2.4 ms in C and 3.1 ms in Salam; a fraction of a millisecond spent before `main` decides the row.
- **I/O policy can decide a row.** `12_hello_print` prints a constant line a million times: Rust took 2.6 ms, Salam 8.2 and C 17.7. That measures how each runtime buffers standard output, not how good its loops are.
- **Noise is a few percent.** Each time is the minimum of 10 runs on a shared virtual machine, and gaps under about 5% can flip on the next run.

None of this makes the benchmark useless. It is good at separating compiled from interpreted languages, at catching performance regressions in the compiler from one release to the next, and at finding programs where Salam is clearly behind and worth profiling. It is not good at ranking compiled languages that are within 10% of each other.

## One benchmark program, side by side {#side-by-side}

Here is `27_edit_distance`, Salam's best row against C (4.5 ms vs 7.8 ms). It computes the Levenshtein distance between two generated sequences of 2,000 symbols with a two-row dynamic programming table. This is the Salam source exactly as it is in `benchmark/programs/27_edit_distance/prog.salam`, minus the license header:

```salam
func main:
    LEN1 := 2000
    LEN2 := 2000

    mut seq1 := Vector {} as Vector<i64> mut i := 0
    until i < LEN1:
        seq1.push(((i as i64) * 31 + 7) % 26)
        i++
    end

    mut seq2 := Vector {} as Vector<i64> mut j := 0
    until j < LEN2:
        seq2.push(((j as i64) * 17 + 11) % 26)
        j++
    end

    mut prev := Vector {} as Vector<i64> j = 0
    until j <= LEN2:
        prev.push(j as i64)
        j++
    end

    mut cur := Vector {} as Vector<i64> cur.zeros(LEN2 + 1)

    i = 1
    until i <= LEN1:
        cur.set(0, i as i64)
        j = 1
        until j <= LEN2:
            mut cost := 0 as i64
            if seq1.get(i - 1) == seq2.get(j - 1):
                cost = 0 as i64
            else:
                cost = 1 as i64
            end
            del_cost := prev.get(j) + 1
            ins_cost := cur.get(j - 1) + 1
            sub_cost := prev.get(j - 1) + cost
            mut m := del_cost
            if ins_cost < m:
                m = ins_cost
            end
            if sub_cost < m:
                m = sub_cost
            end
            cur.set(j, m)
            j++
        end
        j = 0
        until j <= LEN2:
            prev.set(j, cur.get(j))
            j++
        end
        i++
    end

    println prev.get(LEN2)
end
```

```output
1846
```

And the core of the C version from the same directory:

```c
for (i = 1; i <= LEN1; i = i + 1) {
    cur[0] = i;
    for (j = 1; j <= LEN2; j = j + 1) {
        long long cost;
        long long del_cost;
        long long ins_cost;
        long long sub_cost;
        long long m;
        if (seq1[i - 1] == seq2[j - 1]) {
            cost = 0;
        } else {
            cost = 1;
        }
        del_cost = prev[j] + 1;
        ins_cost = cur[j - 1] + 1;
        sub_cost = prev[j - 1] + cost;
        m = del_cost;
        if (ins_cost < m) {
            m = ins_cost;
        }
        if (sub_cost < m) {
            m = sub_cost;
        }
        cur[j] = m;
    }
    for (j = 0; j <= LEN2; j = j + 1) {
        prev[j] = cur[j];
    }
}
```

The two are the same algorithm step for step: same table, same three costs, same copy of the row at the end. (Salam writes `until cond:` for a loop that runs while the condition holds; the benchmark programs are written in a C-like style, with two statements sharing a line in places.) About four million inner iterations finish in single-digit milliseconds in both. What the benchmark does not tell you is why Salam's version is faster on this machine. It might be code generation, memory layout, start-up or something else; answering that takes a profiler and larger inputs, not a table of minimum times. Treat a single row like this as a reason to investigate, never as a conclusion.

## The HTTP router benchmark {#http-router}

The speed page also reports a second, separate benchmark: nine HTTP servers answering the same eleven routes. On 28 October 2026, on a shared 6-vCPU AMD EPYC sandbox with servers pinned to 4 cores and the load generator to 2, Salam's `net/http` averaged 94,850 requests per second (geometric mean over 11 routes) against 62,493 for Go's Fiber framework, 34,987 for Go's standard `net/http`, 22,536 for Node with Express and 11,992 for PHP's built-in server. The load was 64 keep-alive connections, 10 seconds per route after a 3-second warm-up, with routes interleaved across all servers.

Read these with the same care:

- The machine had a load average around 11 from unrelated work. The page gives two figures for the margin over Fiber, 16% on a quiet machine and 52% in the full run under load, because background load favors servers that keep few threads, Salam's included.
- On some routes the margin is thin: on `/` Salam's 91,136 requests per second were 4% ahead of Fiber's 87,651. The static-file route measures caching as much as the server, because Salam keeps static files in memory after the first request while nginx in this setup opened the file each time.
- The harness and its raw results are in `tests/en/apps/httpbench`. Its `BENCHMARK.md` now holds a later run, made on a 4-vCPU GitHub-hosted runner, in which the margin over Fiber is much narrower and Fiber is ahead on some routes, the static-file route most clearly. That is exactly the sensitivity to hardware and load the page warns about.

The honest summary is that Salam's HTTP server is in the same league as the fastest Go frameworks on this kind of test, and well ahead of Node and PHP's built-in server. Do not compare these requests-per-second numbers with the program timings above, and do not plan capacity from them without rerunning on your own hardware. The article on [fast web servers with Salam](/blog/fast-web-servers-with-salam/) covers the server itself.

## How to reproduce it {#reproduce}

Every program, the runner (itself written in Salam) and the Docker image that defines the toolchains are in the repository:

```bash
git clone https://github.com/SalamLang/Salam
cd Salam
docker compose -f benchmark/docker/docker-compose.yml run --rm bench
docker compose -f benchmark/docker/docker-compose.yml run --rm bench 27_edit_distance 12_hello_print
```

The first command runs all 42 programs and writes `README.md`, `results.json` and `results.csv` to `benchmark/results/`; naming programs runs only those, and the `BENCH_RUNS` environment variable changes the number of timed runs. Each new run overwrites those files, so the run behind the speed page is in the repository history as the commit "benchmark: results for v0.6.0 (8cfe206)" (pull request #2407). For the HTTP benchmark, `tests/en/apps/httpbench/bench/run.sh` builds the servers and the load generator; `DURATION`, `CONNS`, `THREADS`, `SERVERS` and `ROUTES` control it.

To go further than the suite: use a quiet machine, grow the inputs until each program runs for a second or more so start-up stops mattering, report the median and spread next to the minimum, and profile any row that differs by more than noise before explaining it.

## Frequently asked questions {#faq}

### Is Salam faster than C?

Not in a general sense. On the project's 39 same-algorithm programs Salam averaged 10.6 ms and C 11.7 ms, a difference small enough that start-up time and noise can explain it. Salam was behind C on several programs, such as n-queens and recursive Fibonacci. See [Salam vs C](/blog/salam-vs-c/) for the differences that matter more than speed.

### Is Salam faster than Rust?

On the same suite Salam averaged 10.6 ms and Rust 11.3 ms, which is the same range. Rust was clearly ahead on binary search and on printing a million lines, and the two often land close together because both compile through LLVM. [Salam vs Rust](/blog/salam-vs-rust/) compares the languages beyond speed, starting with memory safety.

### How much faster is Salam than Python?

About 56 times on this suite (geometric mean 10.6 ms against 598 ms), and about 9.6 times faster than PHP. That gap is large and stable, because it reflects compiled code against an interpreter, not small differences between compilers. The [Salam vs Python](/blog/salam-vs-python/) comparison covers what Python still does better.

### Can I trust a benchmark published by a language's own project?

Trust it as far as you can check it. This one publishes its programs, its runner, its Docker image and the raw time of every run, states where Salam loses, and labels its own limits. Rerunning it on your hardware takes two commands, and that is the best way to settle any doubt.
