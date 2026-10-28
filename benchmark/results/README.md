# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured four ways: the LLVM backend at
`-O3`, the native x86-64 backend (`--backend=native`), the C backend compiled
by `gcc -O3`, and the default `tcc` toolchain (no optimizer) as the
out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-27T14:47:31Z (took 29 min)
- Commit: `8cfe206`

## Methodology

Times below are the **minimum wall-clock over 10 timed runs** (plus one
untimed warm-up run per variant). The minimum is the most robust
single-number estimator on a shared machine: it rejects scheduler and
background-load noise, which inflates averages on short-lived processes.
Each program prints a deterministic result that is identical across every
language; all variants are verified against the C output before timing.

For the full per-run distribution (min/avg/max/median/stdev, raw epoch-ns
samples), see [`results.json`](results.json) and [`results.csv`](results.csv).

## Environment

- **CPU**: AMD EPYC 9V45 96-Core Processor
- **gcc**: gcc (Alpine 15.2.0) 15.2.0
- **g++**: g++ (Alpine 15.2.0) 15.2.0
- **rustc**: rustc 1.96.1 (31fca3adb 2026-06-26) (Alpine Linux Rust 1.96.1-r0)
- **go**: go version go1.26.10 linux/amd64
- **tcc**: tcc version 0.9.28rc (x86_64 Linux)
- **clang**: Alpine clang version 22.1.3
- **python**: Python 3.14.10
- **numpy**: 2.4.6
- **php**: PHP 8.5.12 (cli) (built: Oct 21 2026 16:29:26) (NTS)
- **salam**: 0.6.0

## Overall ranking (fastest to slowest)

One number per language: the **geometric mean of minimum times across the
programs it completed** (geometric mean is used because it is not skewed by the few
slow/high-variance programs the way an arithmetic mean would be). `Programs`
is how many of the 42 went into that mean: a language that fails to build
one is averaged over the rest, so two rows with different counts are not
measuring the same workload, and a row with 0 has no result at all. Lower is
faster; `vs C` and `vs Salam` express the same geomean as a multiple of C's
and of the fastest Salam backend's (Salam-l). Salam's four variants are
suffixed **-g** (C backend, `gcc -O3`), **-l** (LLVM backend, `-O3`), **-n**
(native x86-64 backend, `--release`), and **-t** (C backend, `tcc`, no
optimizer, the out-of-the-box default).

| Rank | Language | Geomean | vs C | vs Salam | Programs | Fastest / tied-fastest on |
|---|---|---|---|---|---|---|
| 1 | **Salam-l** | 10.93 ms | 0.90x | 1.00x | 42 / 42 | 27 / 42 |
| 2 | **Salam-t** | 11.38 ms | 0.94x | 1.04x | 42 / 42 | 18 / 42 |
| 3 | **Salam-g** | 11.97 ms | 0.99x | 1.10x | 42 / 42 | 18 / 42 |
| 4 | Rust | 12.00 ms | 0.99x | 1.10x | 42 / 42 | 21 / 42 |
| 5 | C | 12.10 ms | 1.00x | 1.11x | 42 / 42 | 21 / 42 |
| 6 | C++ | 13.47 ms | 1.11x | 1.23x | 42 / 42 | 2 / 42 |
| 7 | Go | 15.07 ms | 1.25x | 1.38x | 42 / 42 | 3 / 42 |
| 8 | PHP | 116.82 ms | 9.65x | 10.69x | 42 / 42 | 0 / 42 |
| 9 | Python | 563.22 ms | 46.54x | 51.55x | 42 / 42 | 0 / 42 |
| 10 | **Salam-n** | n/a | - | - | 0 / 42 | 0 / 42 |

```
Salam-l      █████████████████████████████████               10.93 ms  0.90x C
Salam-t      ██████████████████████████████████              11.38 ms  0.94x C
Salam-g      ████████████████████████████████████            11.97 ms  0.99x C
Rust         ████████████████████████████████████            12.00 ms  0.99x C
C            ████████████████████████████████████            12.10 ms  1.00x C  <- baseline
C++          ████████████████████████████████████████        13.47 ms  1.11x C
Go           █████████████████████████████████████████████   15.07 ms  1.25x C
PHP          (off scale)                                    116.82 ms  9.65x C
Python       (off scale)                                    563.22 ms  46.54x C
Salam-n      (no result)                                          n/a
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 10.93 ms
  (0.90x C's geomean, 0.91x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  27/42); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.04x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 10-47x slower** than the compiled languages on
  this suite and are included for scale only.
- "Fastest / tied-fastest on" counts ties on every side (e.g. a 4-way tie
  adds 1 to all 4 languages), so the column does not sum to 42.
- **Salam-n produced no usable timing on any program** and is ranked
  last with no geomean; see the failures table below.

## Failures

42 program/language pairs produced no timing. They are left out of that
language's geomean, which is why the `Programs` column in the ranking is
worth reading before comparing two rows. Full compiler output for each one
is in `results.json`.

| Program | Language | Status |
|---|---|---|
| 01_fib_recursive | Salam-n | build failed |
| 02_fib_iterative | Salam-n | build failed |
| 03_primes_count | Salam-n | build failed |
| 04_collatz | Salam-n | build failed |
| 05_sum_mod | Salam-n | build failed |
| 06_gcd_sum | Salam-n | build failed |
| 07_pow_mod | Salam-n | build failed |
| 08_digit_sum | Salam-n | build failed |
| 09_perfect_numbers | Salam-n | build failed |
| 10_pi_leibniz | Salam-n | build failed |
| 11_loop_count | Salam-n | build failed |
| 12_hello_print | Salam-n | build failed |
| 13_array_rw | Salam-n | build failed |
| 14_lcg_random | Salam-n | build failed |
| 15_matrix_mult | Salam-n | build failed |
| 16_quicksort | Salam-n | build failed |
| 17_n_queens | Salam-n | build failed |
| 18_sieve_eratosthenes | Salam-n | build failed |
| 19_mandelbrot | Salam-n | build failed |
| 20_monte_carlo_pi | Salam-n | build failed |
| 21_coin_change_dp | Salam-n | build failed |
| 22_knapsack_01 | Salam-n | build failed |
| 23_caesar_cipher | Salam-n | build failed |
| 24_merge_sort | Salam-n | build failed |
| 25_palindrome_count | Salam-n | build failed |
| 26_ackermann | Salam-n | build failed |
| 27_edit_distance | Salam-n | build failed |
| 28_prime_factorization | Salam-n | build failed |
| 29_dot_product | Salam-n | build failed |
| 30_heap_sort | Salam-n | build failed |

...and 12 more, in `results.json`.

## Per-program results (minimum ms)

| Program | C | C++ | Rust | Go | Salam-l | Salam-n | Salam-g | Salam-t | PHP | Python | Fastest |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 01_fib_recursive | 2 | 3 | 3 | 5 | 3 | - | 2 | 3 | 37 | 60 | **C / Salam-g (2)** |
| 02_fib_iterative | 15 | 17 | 16 | 17 | 15 | - | 15 | 15 | 39 | 416 | **C / Salam (15)** |
| 03_primes_count | 4 | 5 | 4 | 5 | 4 | - | 4 | 4 | 16 | 107 | **C / Rust / Salam (4)** |
| 04_collatz | 4 | 5 | 4 | 6 | 4 | - | 4 | 4 | 42 | 246 | **C / Rust / Salam (4)** |
| 05_sum_mod | 10 | 11 | 10 | 10 | 10 | - | 10 | 10 | 24 | 254 | **C / Rust / Go / Salam (10)** |
| 06_gcd_sum | 15 | 16 | 13 | 16 | 13 | - | 15 | 14 | 34 | 95 | **Rust / Salam-l (13)** |
| 07_pow_mod | 2 | 3 | 3 | 4 | 2 | - | 2 | 2 | 19 | 52 | **C / Salam (2)** |
| 08_digit_sum | 4 | 5 | 4 | 4 | 4 | - | 4 | 4 | 47 | 278 | **C / Rust / Go / Salam (4)** |
| 09_perfect_numbers | 9 | 10 | 8 | 11 | 8 | - | 9 | 8 | 31 | 304 | **Rust / Salam (8)** |
| 10_pi_leibniz | 5 | 6 | 5 | 6 | 5 | - | 5 | 5 | 41 | 388 | **C / Rust / Salam (5)** |
| 11_loop_count | 5 | 6 | 5 | 6 | 6 | - | 5 | 6 | 13 | 88 | **C / Rust / Salam-g (5)** |
| 12_hello_print | 18 | 16 | 3 | 6 | 8 | - | 8 | 8 | 376 | 76 | Rust (3) |
| 13_array_rw | 21 | 22 | 22 | 22 | 22 | - | 20 | 22 | 60 | 709 | **Salam-g (20)** |
| 14_lcg_random | 18 | 19 | 19 | 23 | 19 | - | 18 | 19 | 60 | 1048 | **C / Salam-g (18)** |
| 15_matrix_mult | 6 | 7 | 5 | 8 | 5 | - | 6 | 5 | 130 | 837 | **Rust / Salam (5)** |
| 16_quicksort | 26 | 26 | 25 | 26 | 25 | - | 26 | 27 | 133 | 463 | **Rust / Salam-l (25)** |
| 17_n_queens | 36 | 38 | 38 | 40 | 41 | - | 40 | 40 | 318 | 518 | C (36) |
| 18_sieve_eratosthenes | 5 | 6 | 5 | 6 | 5 | - | 6 | 5 | 113 | 648 | **C / Rust / Salam (5)** |
| 19_mandelbrot | 9 | 9 | 9 | 10 | 9 | - | 9 | 9 | 76 | 843 | **C / C++ / Rust / Salam (9)** |
| 20_monte_carlo_pi | 23 | 23 | 12 | 28 | 12 | - | 24 | 8 | 205 | 2586 | **Salam-t (8)** |
| 21_coin_change_dp | 3 | 3 | 3 | 3 | 3 | - | 3 | 3 | 14 | 77 | **C / C++ / Rust / Go / Salam (3)** |
| 22_knapsack_01 | 3 | 4 | 4 | 5 | 3 | - | 4 | 5 | 77 | 888 | **C / Salam-l (3)** |
| 23_caesar_cipher | 10 | 12 | 10 | 14 | 10 | - | 11 | 10 | 61 | 768 | **C / Rust / Salam (10)** |
| 24_merge_sort | 24 | 25 | 25 | 29 | 25 | - | 27 | 28 | 183 | 567 | C (24) |
| 25_palindrome_count | 9 | 10 | 9 | 12 | 9 | - | 9 | 9 | 172 | 1107 | **C / Rust / Salam (9)** |
| 26_ackermann | 1 | 2 | 2 | 2 | 2 | - | 1 | 2 | 9 | 21 | **C / Salam-g (1)** |
| 27_edit_distance | 8 | 8 | 8 | 10 | 4 | - | 6 | 5 | 153 | 1573 | **Salam-l (4)** |
| 28_prime_factorization | 814 | 836 | 701 | 801 | 705 | - | 815 | 709 | 2989 | 35373 | Rust (701) |
| 29_dot_product | 29 | 31 | 30 | 31 | 30 | - | 29 | 30 | 148 | 1802 | **C / Salam-g (29)** |
| 30_heap_sort | 28 | 28 | 29 | 36 | 26 | - | 28 | 28 | 307 | 690 | **Salam-l (26)** |
| 31_binary_search_stress | 731 | 724 | 584 | 734 | 668 | - | 806 | 824 | 4589 | 26120 | Rust (584) |
| 32_longest_increasing_subsequence | 11 | 12 | 12 | 13 | 10 | - | 11 | 11 | 202 | 1919 | **Salam-l (10)** |
| 33_subset_sum_reachability | 10 | 12 | 19 | 14 | 9 | - | 12 | 12 | 235 | 1826 | **Salam-l (9)** |
| 34_counting_sort | 33 | 34 | 23 | 35 | 24 | - | 33 | 25 | 183 | 1953 | Rust (23) |
| 35_run_length_stats | 7 | 7 | 6 | 8 | 6 | - | 7 | 6 | 85 | 641 | **Rust / Salam (6)** |
| 36_array_rotation_reversal | 20 | 21 | 20 | 22 | 19 | - | 20 | 20 | 160 | 706 | **Salam-l (19)** |
| 37_sqrt_decomposition | 11 | 12 | 10 | 16 | 8 | - | 12 | 9 | 248 | 894 | **Salam-l (8)** |
| 38_game_of_life | 9 | 10 | 25 | 33 | 7 | - | 8 | 7 | 976 | 6722 | **Salam (7)** |
| 39_trapezoidal_integration | 12 | 14 | 11 | 14 | 11 | - | 12 | 12 | 292 | 2202 | **Rust / Salam-l (11)** |
| 40_matrix_stdlib_matmul | 43 | 44 | 64 | 47 | 14 | - | 13 | 17 | 995 | 122 | **Salam-g (13)** |
| 41_matrix_stdlib_smallops | 4 | 5 | 5 | 16 | 36 | - | 45 | 47 | 376 | 1174 | C (4) |
| 42_tensor_stdlib_matmul | 40 | 38 | 56 | 43 | 7 | - | 7 | 7 | 860 | 119 | **Salam (7)** |

Bold marks rows where a Salam backend is the fastest or tied for fastest.

### Top 10 fastest programs in Salam-l

Where the LLVM backend (`-O3`) does best **relative to the fastest non-Salam
entry on the same program**: sorted by `vs best other` (Salam-l's minimum
divided by that language's), lowest ratio first. Under 1.00x means Salam-l
wins the row outright. Absolute ms are shown but are not the sort key, since
they mostly track how big each program's workload is.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 42_tensor_stdlib_matmul | 7 | C++ (38) | 0.18x |
| 2 | 40_matrix_stdlib_matmul | 14 | C (43) | 0.33x |
| 3 | 27_edit_distance | 4 | C (8) | 0.50x |
| 4 | 38_game_of_life | 7 | C (9) | 0.78x |
| 5 | 37_sqrt_decomposition | 8 | Rust (10) | 0.80x |
| 6 | 33_subset_sum_reachability | 9 | C (10) | 0.90x |
| 7 | 32_longest_increasing_subsequence | 10 | C (11) | 0.91x |
| 8 | 30_heap_sort | 26 | C (28) | 0.93x |
| 9 | 36_array_rotation_reversal | 19 | C (20) | 0.95x |
| 10 | 07_pow_mod | 2 | C (2) | 1.00x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 36 | C (4) | 9.00x |
| 2 | 12_hello_print | 8 | Rust (3) | 2.67x |
| 3 | 26_ackermann | 2 | C (1) | 2.00x |
| 4 | 01_fib_recursive | 3 | C (2) | 1.50x |
| 5 | 11_loop_count | 6 | C (5) | 1.20x |
| 6 | 31_binary_search_stress | 668 | Rust (584) | 1.14x |
| 7 | 17_n_queens | 41 | C (36) | 1.14x |
| 8 | 14_lcg_random | 19 | C (18) | 1.06x |
| 9 | 13_array_rw | 22 | C (21) | 1.05x |
| 10 | 34_counting_sort | 24 | Rust (23) | 1.04x |
| 11 | 24_merge_sort | 25 | C (24) | 1.04x |
| 12 | 29_dot_product | 30 | C (29) | 1.03x |
| 13 | 28_prime_factorization | 705 | Rust (701) | 1.01x |
| 14 | 16_quicksort | 25 | Rust (25) | 1.00x |
| 15 | 02_fib_iterative | 15 | C (15) | 1.00x |
| 16 | 06_gcd_sum | 13 | Rust (13) | 1.00x |
| 17 | 20_monte_carlo_pi | 12 | Rust (12) | 1.00x |
| 18 | 39_trapezoidal_integration | 11 | Rust (11) | 1.00x |
| 19 | 05_sum_mod | 10 | C (10) | 1.00x |
| 20 | 23_caesar_cipher | 10 | C (10) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (11):** 13_array_rw, 20_monte_carlo_pi, 27_edit_distance, 30_heap_sort, 32_longest_increasing_subsequence, 33_subset_sum_reachability, 36_array_rotation_reversal, 37_sqrt_decomposition, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (24):** 01_fib_recursive, 02_fib_iterative, 03_primes_count, 04_collatz, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 14_lcg_random, 15_matrix_mult, 16_quicksort, 18_sieve_eratosthenes, 19_mandelbrot, 21_coin_change_dp, 22_knapsack_01, 23_caesar_cipher, 25_palindrome_count, 26_ackermann, 29_dot_product, 35_run_length_stats, 39_trapezoidal_integration.
- **Trailing the fastest (7):** 12_hello_print (+5 vs Rust); 17_n_queens (+4 vs C); 24_merge_sort (+1 vs C); 28_prime_factorization (+4 vs Rust); 31_binary_search_stress (+84 vs Rust); 34_counting_sort (+1 vs Rust); 41_matrix_stdlib_smallops (+32 vs C).

Salam matches or beats the fastest other compiled language on 35 of 42 programs.

Notes:

- `12_hello_print` prints a constant line one million times, so it measures
  how each runtime buffers stdout more than how good its generated code is.
- The `*_stdlib_*` programs use each language's own library where it has one
  (Salam's `matrix`/`tensor`, NumPy in Python); C, C++, Rust, Go and PHP use
  plain loops. Those rows compare libraries, not compilers.
- Python and PHP are included for scale only; they are consistently much
  slower and are never the fastest on this suite.
- Measurements were taken on a single machine under normal background load;
  small gaps between the compiled languages can be within run-to-run noise.
