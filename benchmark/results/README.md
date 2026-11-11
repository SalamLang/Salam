# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured four ways: the LLVM backend at
`-O3`, the native x86-64 backend (`--backend=native`), the C backend compiled
by `gcc -O3`, and the default `tcc` toolchain (no optimizer) as the
out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-11-11T17:03:42Z (took 57 min)
- Commit: `e854528f`

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

- **CPU**: AMD EPYC 7763 64-Core Processor
- **gcc**: gcc (Alpine 15.2.0) 15.2.0
- **g++**: g++ (Alpine 15.2.0) 15.2.0
- **rustc**: rustc 1.96.1 (31fca3adb 2026-06-26) (Alpine Linux Rust 1.96.1-r0)
- **go**: go version go1.26.10 linux/amd64
- **tcc**: tcc version 0.9.28rc (x86_64 Linux)
- **clang**: Alpine clang version 22.1.3
- **python**: Python 3.14.10
- **numpy**: 2.4.6
- **php**: PHP 8.5.12 (cli) (built: Oct 21 2026 16:29:26) (NTS)
- **salam**: 0.6.3

## Overall ranking (fastest to slowest)

One number per language: the **geometric mean of minimum times across the
programs it completed** (geometric mean is used because it is not skewed by the few
slow/high-variance programs the way an arithmetic mean would be). `Programs`
is how many of the 42 went into that mean: a language that fails to build
one is averaged over the rest, so two rows with different counts are not
measuring the same workload, and a row with 0 has no result at all. Lower is
faster; `vs C` and `vs Salam` express the same geomean as a multiple of C's
and of the fastest Salam backend's (Salam-n). Salam's four variants are
suffixed **-g** (C backend, `gcc -O3`), **-l** (LLVM backend, `-O3`), **-n**
(native x86-64 backend, `--release`), and **-t** (C backend, `tcc`, no
optimizer, the out-of-the-box default).

| Rank | Language | Geomean | vs C | vs Salam | Programs | Fastest / tied-fastest on |
|---|---|---|---|---|---|---|
| 1 | **Salam-n** | 14.07 ms | 0.85x | 1.00x | 42 / 42 | 27 / 42 |
| 2 | **Salam-l** | 14.86 ms | 0.90x | 1.06x | 42 / 42 | 20 / 42 |
| 3 | **Salam-t** | 15.23 ms | 0.92x | 1.08x | 42 / 42 | 19 / 42 |
| 4 | **Salam-g** | 15.98 ms | 0.96x | 1.14x | 42 / 42 | 18 / 42 |
| 5 | C | 16.60 ms | 1.00x | 1.18x | 42 / 42 | 16 / 42 |
| 6 | Rust | 16.82 ms | 1.01x | 1.20x | 42 / 42 | 13 / 42 |
| 7 | C++ | 18.28 ms | 1.10x | 1.30x | 42 / 42 | 1 / 42 |
| 8 | Go | 21.82 ms | 1.31x | 1.55x | 42 / 42 | 0 / 42 |
| 9 | PHP | 194.85 ms | 11.74x | 13.84x | 42 / 42 | 0 / 42 |
| 10 | Python | 1009.46 ms | 60.83x | 71.72x | 42 / 42 | 0 / 42 |

```
Salam-n      █████████████████████████████                   14.07 ms  0.85x C
Salam-l      ███████████████████████████████                 14.86 ms  0.90x C
Salam-t      ███████████████████████████████                 15.23 ms  0.92x C
Salam-g      █████████████████████████████████               15.98 ms  0.96x C
C            ██████████████████████████████████              16.60 ms  1.00x C  <- baseline
Rust         ███████████████████████████████████             16.82 ms  1.01x C
C++          ██████████████████████████████████████          18.28 ms  1.10x C
Go           █████████████████████████████████████████████   21.82 ms  1.31x C
PHP          (off scale)                                    194.85 ms  11.74x C
Python       (off scale)                                   1009.46 ms  60.83x C
```

Takeaways:

- **Fastest Salam backend:** Salam-n, geomean 14.07 ms
  (0.85x C's geomean, 0.84x Rust's).
- **Salam-n is fastest or tied-fastest on the most programs** (
  27/42); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.08x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 12-61x slower** than the compiled languages on
  this suite and are included for scale only.
- "Fastest / tied-fastest on" counts ties on every side (e.g. a 4-way tie
  adds 1 to all 4 languages), so the column does not sum to 42.

## Per-program results (minimum ms)

| Program | C | C++ | Rust | Go | Salam-l | Salam-n | Salam-g | Salam-t | PHP | Python | Fastest |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 01_fib_recursive | 4 | 5 | 4 | 8 | 5 | 4 | 3 | 5 | 68 | 112 | **Salam-g (3)** |
| 02_fib_iterative | 21 | 22 | 22 | 23 | 21 | 16 | 21 | 22 | 67 | 878 | **Salam-n (16)** |
| 03_primes_count | 5 | 6 | 5 | 6 | 5 | 5 | 5 | 5 | 25 | 204 | **C / Rust / Salam (5)** |
| 04_collatz | 5 | 7 | 5 | 8 | 5 | 5 | 5 | 5 | 77 | 448 | **C / Rust / Salam (5)** |
| 05_sum_mod | 14 | 15 | 15 | 15 | 14 | 11 | 14 | 14 | 38 | 569 | **Salam-n (11)** |
| 06_gcd_sum | 17 | 18 | 16 | 19 | 16 | 16 | 17 | 16 | 65 | 176 | **Rust / Salam (16)** |
| 07_pow_mod | 3 | 4 | 4 | 5 | 3 | 3 | 3 | 3 | 27 | 80 | **C / Salam (3)** |
| 08_digit_sum | 4 | 5 | 5 | 6 | 5 | 4 | 4 | 4 | 79 | 470 | **C / Salam (4)** |
| 09_perfect_numbers | 10 | 11 | 9 | 12 | 9 | 9 | 10 | 9 | 47 | 517 | **Rust / Salam (9)** |
| 10_pi_leibniz | 6 | 7 | 7 | 7 | 6 | 6 | 6 | 6 | 59 | 767 | **C / Salam (6)** |
| 11_loop_count | 6 | 8 | 7 | 8 | 7 | 7 | 6 | 7 | 18 | 161 | **C / Salam-g (6)** |
| 12_hello_print | 22 | 22 | 4 | 9 | 3 | 3 | 3 | 3 | 445 | 126 | **Salam (3)** |
| 13_array_rw | 28 | 29 | 29 | 30 | 29 | 29 | 26 | 29 | 97 | 1329 | **Salam-g (26)** |
| 14_lcg_random | 21 | 22 | 23 | 27 | 23 | 16 | 21 | 23 | 85 | 1721 | **Salam-n (16)** |
| 15_matrix_mult | 8 | 9 | 7 | 14 | 7 | 8 | 8 | 7 | 209 | 1490 | **Rust / Salam (7)** |
| 16_quicksort | 28 | 29 | 27 | 30 | 27 | 27 | 28 | 28 | 194 | 736 | **Rust / Salam (27)** |
| 17_n_queens | 43 | 44 | 43 | 46 | 45 | 46 | 49 | 45 | 575 | 819 | C / Rust (43) |
| 18_sieve_eratosthenes | 8 | 9 | 8 | 12 | 8 | 8 | 8 | 8 | 139 | 1150 | **C / Rust / Salam (8)** |
| 19_mandelbrot | 12 | 14 | 13 | 14 | 12 | 12 | 13 | 12 | 159 | 1709 | **C / Salam (12)** |
| 20_monte_carlo_pi | 31 | 33 | 17 | 38 | 16 | 15 | 31 | 13 | 386 | 4769 | **Salam-t (13)** |
| 21_coin_change_dp | 4 | 5 | 4 | 5 | 4 | 4 | 4 | 4 | 21 | 161 | **C / Rust / Salam (4)** |
| 22_knapsack_01 | 6 | 7 | 6 | 8 | 6 | 5 | 7 | 7 | 135 | 1869 | **Salam-n (5)** |
| 23_caesar_cipher | 17 | 18 | 14 | 24 | 14 | 11 | 20 | 14 | 110 | 1484 | **Salam-n (11)** |
| 24_merge_sort | 33 | 34 | 35 | 39 | 33 | 35 | 37 | 39 | 336 | 1111 | **C / Salam-l (33)** |
| 25_palindrome_count | 15 | 16 | 15 | 19 | 15 | 15 | 15 | 15 | 363 | 2236 | **C / Rust / Salam (15)** |
| 26_ackermann | 2 | 3 | 2 | 4 | 2 | 2 | 2 | 2 | 14 | 30 | **C / Rust / Salam (2)** |
| 27_edit_distance | 8 | 9 | 9 | 12 | 7 | 8 | 8 | 8 | 329 | 3331 | **Salam-l (7)** |
| 28_prime_factorization | 1117 | 1119 | 971 | 1107 | 968 | 954 | 1118 | 972 | 6261 | 73848 | **Salam-n (954)** |
| 29_dot_product | 40 | 41 | 43 | 43 | 43 | 34 | 40 | 42 | 279 | 3935 | **Salam-n (34)** |
| 30_heap_sort | 37 | 37 | 41 | 49 | 37 | 38 | 37 | 39 | 645 | 1492 | **C / C++ / Salam (37)** |
| 31_binary_search_stress | 1099 | 1099 | 1465 | 1089 | 966 | 985 | 1088 | 1079 | 6735 | 35097 | **Salam-l (966)** |
| 32_longest_increasing_subsequence | 16 | 17 | 12 | 14 | 12 | 12 | 16 | 14 | 314 | 3361 | **Rust / Salam (12)** |
| 33_subset_sum_reachability | 20 | 20 | 21 | 20 | 14 | 13 | 21 | 20 | 391 | 3305 | **Salam-n (13)** |
| 34_counting_sort | 42 | 42 | 33 | 45 | 33 | 27 | 44 | 33 | 326 | 3242 | **Salam-n (27)** |
| 35_run_length_stats | 9 | 10 | 8 | 10 | 8 | 8 | 10 | 8 | 152 | 1093 | **Rust / Salam (8)** |
| 36_array_rotation_reversal | 37 | 40 | 39 | 39 | 37 | 38 | 36 | 36 | 264 | 1306 | **Salam (36)** |
| 37_sqrt_decomposition | 19 | 19 | 20 | 39 | 19 | 19 | 20 | 18 | 367 | 1604 | **Salam-t (18)** |
| 38_game_of_life | 12 | 13 | 39 | 59 | 10 | 7 | 11 | 10 | 1422 | 10599 | **Salam-n (7)** |
| 39_trapezoidal_integration | 15 | 16 | 16 | 18 | 16 | 15 | 15 | 15 | 517 | 3883 | **C / Salam (15)** |
| 40_matrix_stdlib_matmul | 55 | 56 | 94 | 91 | 17 | 16 | 15 | 16 | 1607 | 161 | **Salam-g (15)** |
| 41_matrix_stdlib_smallops | 6 | 7 | 8 | 23 | 52 | 56 | 72 | 76 | 607 | 1916 | C (6) |
| 42_tensor_stdlib_matmul | 55 | 56 | 94 | 91 | 10 | 10 | 9 | 10 | 1600 | 161 | **Salam-g (9)** |

Bold marks rows where a Salam backend is the fastest or tied for fastest.

### Top 10 fastest programs in Salam-l

Where the LLVM backend (`-O3`) does best **relative to the fastest non-Salam
entry on the same program**: sorted by `vs best other` (Salam-l's minimum
divided by that language's), lowest ratio first. Under 1.00x means Salam-l
wins the row outright. Absolute ms are shown but are not the sort key, since
they mostly track how big each program's workload is.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 42_tensor_stdlib_matmul | 10 | C (55) | 0.18x |
| 2 | 40_matrix_stdlib_matmul | 17 | C (55) | 0.31x |
| 3 | 33_subset_sum_reachability | 14 | C (20) | 0.70x |
| 4 | 12_hello_print | 3 | Rust (4) | 0.75x |
| 5 | 38_game_of_life | 10 | C (12) | 0.83x |
| 6 | 27_edit_distance | 7 | C (8) | 0.88x |
| 7 | 31_binary_search_stress | 966 | Go (1089) | 0.89x |
| 8 | 20_monte_carlo_pi | 16 | Rust (17) | 0.94x |
| 9 | 28_prime_factorization | 968 | Rust (971) | 1.00x |
| 10 | 26_ackermann | 2 | C (2) | 1.00x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 52 | C (6) | 8.67x |
| 2 | 01_fib_recursive | 5 | C (4) | 1.25x |
| 3 | 08_digit_sum | 5 | C (4) | 1.25x |
| 4 | 11_loop_count | 7 | C (6) | 1.17x |
| 5 | 14_lcg_random | 23 | C (21) | 1.10x |
| 6 | 29_dot_product | 43 | C (40) | 1.08x |
| 7 | 39_trapezoidal_integration | 16 | C (15) | 1.07x |
| 8 | 17_n_queens | 45 | C (43) | 1.05x |
| 9 | 13_array_rw | 29 | C (28) | 1.04x |
| 10 | 36_array_rotation_reversal | 37 | C (37) | 1.00x |
| 11 | 30_heap_sort | 37 | C (37) | 1.00x |
| 12 | 24_merge_sort | 33 | C (33) | 1.00x |
| 13 | 34_counting_sort | 33 | Rust (33) | 1.00x |
| 14 | 16_quicksort | 27 | Rust (27) | 1.00x |
| 15 | 02_fib_iterative | 21 | C (21) | 1.00x |
| 16 | 37_sqrt_decomposition | 19 | C (19) | 1.00x |
| 17 | 06_gcd_sum | 16 | Rust (16) | 1.00x |
| 18 | 25_palindrome_count | 15 | C (15) | 1.00x |
| 19 | 05_sum_mod | 14 | C (14) | 1.00x |
| 20 | 23_caesar_cipher | 14 | Rust (14) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (20):** 01_fib_recursive, 02_fib_iterative, 05_sum_mod, 12_hello_print, 13_array_rw, 14_lcg_random, 20_monte_carlo_pi, 22_knapsack_01, 23_caesar_cipher, 27_edit_distance, 28_prime_factorization, 29_dot_product, 31_binary_search_stress, 33_subset_sum_reachability, 34_counting_sort, 36_array_rotation_reversal, 37_sqrt_decomposition, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (20):** 03_primes_count, 04_collatz, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 15_matrix_mult, 16_quicksort, 18_sieve_eratosthenes, 19_mandelbrot, 21_coin_change_dp, 24_merge_sort, 25_palindrome_count, 26_ackermann, 30_heap_sort, 32_longest_increasing_subsequence, 35_run_length_stats, 39_trapezoidal_integration.
- **Trailing the fastest (2):** 17_n_queens (+2 vs C/Rust); 41_matrix_stdlib_smallops (+46 vs C).

Salam matches or beats the fastest other compiled language on 40 of 42 programs.

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
