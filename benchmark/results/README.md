# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured three ways: the LLVM backend at
`-O3`, the C backend compiled by `gcc -O3`, and the default `tcc` toolchain
(no optimizer) as the out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-04T16:35:46Z (took 13 min)
- Commit: `470160b`

## Methodology

Times below are the **minimum wall-clock over 1 timed run(s)** (plus one
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
- **go**: go version go1.26.8 linux/amd64
- **tcc**: tcc version 0.9.28rc (x86_64 Linux)
- **clang**: Alpine clang version 22.1.3
- **python**: Python 3.14.8
- **numpy**: 2.4.6
- **php**: PHP 8.5.11 (cli) (built: Sep 25 2026 15:57:29) (NTS)
- **salam**: 0.4.9

## Overall ranking (fastest to slowest)

One number per language: the **geometric mean of minimum times across the
programs it completed** (geometric mean is used because it is not skewed by the few
slow/high-variance programs the way an arithmetic mean would be). `Programs`
is how many of the 42 went into that mean: a language that fails to build
one is averaged over the rest, so two rows with different counts are not
measuring the same workload, and a row with 0 has no result at all. Lower is
faster; `vs C` and `vs Salam` express the same geomean as a multiple of C's
and of the fastest Salam backend's (Salam-l). Salam's three variants are
suffixed **-g** (C backend, `gcc -O3`), **-l** (LLVM backend, `-O3`), and
**-t** (C backend, `tcc`, no optimizer, the out-of-the-box default).

| Rank | Language | Geomean | vs C | vs Salam | Programs | Fastest / tied-fastest on |
|---|---|---|---|---|---|---|
| 1 | **Salam-l** | 14.27 ms | 0.88x | 1.00x | 42 / 42 | 24 / 42 |
| 2 | Rust | 16.08 ms | 1.00x | 1.13x | 42 / 42 | 15 / 42 |
| 3 | C | 16.14 ms | 1.00x | 1.13x | 42 / 42 | 19 / 42 |
| 4 | **Salam-t** | 16.32 ms | 1.01x | 1.14x | 42 / 42 | 21 / 42 |
| 5 | **Salam-g** | 17.14 ms | 1.06x | 1.20x | 42 / 42 | 21 / 42 |
| 6 | C++ | 17.62 ms | 1.09x | 1.23x | 42 / 42 | 2 / 42 |
| 7 | Go | 20.56 ms | 1.27x | 1.44x | 42 / 42 | 0 / 42 |
| 8 | PHP | 191.75 ms | 11.88x | 13.44x | 42 / 42 | 0 / 42 |
| 9 | Python | 997.97 ms | 61.83x | 69.94x | 42 / 42 | 0 / 42 |

```
Salam-l      ███████████████████████████████                 14.27 ms  0.88x C
Rust         ███████████████████████████████████             16.08 ms  1.00x C
C            ███████████████████████████████████             16.14 ms  1.00x C  <- baseline
Salam-t      ████████████████████████████████████            16.32 ms  1.01x C
Salam-g      ██████████████████████████████████████          17.14 ms  1.06x C
C++          ███████████████████████████████████████         17.62 ms  1.09x C
Go           █████████████████████████████████████████████   20.56 ms  1.27x C
PHP          (off scale)                                    191.75 ms  11.88x C
Python       (off scale)                                    997.97 ms  61.83x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 14.27 ms
  (0.88x C's geomean, 0.89x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  24/42); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.14x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 12-62x slower** than the compiled languages on
  this suite and are included for scale only.
- "Fastest / tied-fastest on" counts ties on every side (e.g. a 4-way tie
  adds 1 to all 4 languages), so the column does not sum to 42.

## Per-program results (minimum ms)

| Program | C | C++ | Rust | Go | Salam-l | Salam-g | Salam-t | PHP | Python | Fastest |
|---|---|---|---|---|---|---|---|---|---|---|
| 01_fib_recursive | 4 | 5 | 4 | 7 | 4 | 4 | 4 | 68 | 117 | **C / Rust / Salam (4)** |
| 02_fib_iterative | 21 | 26 | 21 | 22 | 21 | 21 | 21 | 63 | 887 | **C / Rust / Salam (21)** |
| 03_primes_count | 5 | 6 | 5 | 6 | 5 | 5 | 5 | 25 | 217 | **C / Rust / Salam (5)** |
| 04_collatz | 5 | 7 | 5 | 8 | 5 | 5 | 5 | 74 | 442 | **C / Rust / Salam (5)** |
| 05_sum_mod | 13 | 14 | 15 | 14 | 14 | 13 | 14 | 37 | 559 | **C / Salam-g (13)** |
| 06_gcd_sum | 17 | 19 | 16 | 18 | 16 | 17 | 16 | 63 | 175 | **Rust / Salam (16)** |
| 07_pow_mod | 3 | 4 | 4 | 5 | 3 | 3 | 3 | 27 | 82 | **C / Salam (3)** |
| 08_digit_sum | 4 | 5 | 5 | 6 | 5 | 4 | 5 | 82 | 462 | **C / Salam-g (4)** |
| 09_perfect_numbers | 10 | 11 | 9 | 11 | 9 | 10 | 9 | 45 | 509 | **Rust / Salam (9)** |
| 10_pi_leibniz | 6 | 7 | 7 | 7 | 6 | 6 | 6 | 60 | 764 | **C / Salam (6)** |
| 11_loop_count | 6 | 7 | 7 | 8 | 7 | 6 | 7 | 17 | 157 | **C / Salam-g (6)** |
| 12_hello_print | 24 | 22 | 4 | 10 | 3 | 402 | 400 | 466 | 129 | **Salam-l (3)** |
| 13_array_rw | 25 | 26 | 26 | 27 | 27 | 24 | 26 | 90 | 1261 | **Salam-g (24)** |
| 14_lcg_random | 21 | 22 | 23 | 28 | 23 | 21 | 22 | 85 | 1694 | **C / Salam-g (21)** |
| 15_matrix_mult | 8 | 9 | 7 | 11 | 7 | 8 | 7 | 246 | 1553 | **Rust / Salam (7)** |
| 16_quicksort | 27 | 28 | 26 | 28 | 26 | 28 | 28 | 190 | 698 | **Rust / Salam-l (26)** |
| 17_n_queens | 43 | 43 | 42 | 50 | 45 | 49 | 44 | 574 | 814 | Rust (42) |
| 18_sieve_eratosthenes | 9 | 9 | 8 | 9 | 8 | 8 | 7 | 122 | 1390 | **Salam-t (7)** |
| 19_mandelbrot | 12 | 13 | 12 | 14 | 12 | 12 | 12 | 165 | 1666 | **C / Rust / Salam (12)** |
| 20_monte_carlo_pi | 31 | 32 | 16 | 37 | 16 | 31 | 12 | 409 | 4682 | **Salam-t (12)** |
| 21_coin_change_dp | 3 | 4 | 4 | 4 | 4 | 4 | 4 | 21 | 155 | C (3) |
| 22_knapsack_01 | 6 | 7 | 6 | 9 | 6 | 6 | 6 | 134 | 1446 | **C / Rust / Salam (6)** |
| 23_caesar_cipher | 17 | 18 | 14 | 24 | 13 | 17 | 14 | 113 | 1447 | **Salam-l (13)** |
| 24_merge_sort | 32 | 32 | 33 | 39 | 33 | 37 | 37 | 326 | 1063 | C / C++ (32) |
| 25_palindrome_count | 15 | 16 | 15 | 19 | 14 | 14 | 15 | 379 | 2335 | **Salam (14)** |
| 26_ackermann | 2 | 3 | 2 | 4 | 2 | 2 | 2 | 12 | 28 | **C / Rust / Salam (2)** |
| 27_edit_distance | 8 | 9 | 10 | 12 | 7 | 9 | 8 | 332 | 3211 | **Salam-l (7)** |
| 28_prime_factorization | 1118 | 1120 | 972 | 1110 | 971 | 1119 | 970 | 5814 | 70559 | **Salam-t (970)** |
| 29_dot_product | 40 | 41 | 43 | 43 | 43 | 40 | 43 | 285 | 3826 | **C / Salam-g (40)** |
| 30_heap_sort | 37 | 37 | 40 | 48 | 36 | 35 | 40 | 609 | 1330 | **Salam-g (35)** |
| 31_binary_search_stress | 893 | 897 | 863 | 889 | 844 | 947 | 921 | 5290 | 34816 | **Salam-l (844)** |
| 32_longest_increasing_subsequence | 16 | 17 | 12 | 19 | 13 | 20 | 12 | 315 | 2558 | **Rust / Salam-t (12)** |
| 33_subset_sum_reachability | 21 | 22 | 22 | 27 | 14 | 22 | 16 | 380 | 3940 | **Salam-l (14)** |
| 34_counting_sort | 42 | 42 | 33 | 45 | 32 | 42 | 33 | 319 | 3512 | **Salam-l (32)** |
| 35_run_length_stats | 10 | 10 | 8 | 10 | 8 | 9 | 8 | 150 | 1105 | **Rust / Salam (8)** |
| 36_array_rotation_reversal | 24 | 26 | 25 | 26 | 23 | 24 | 25 | 238 | 1295 | **Salam-l (23)** |
| 37_sqrt_decomposition | 10 | 10 | 10 | 20 | 10 | 10 | 10 | 361 | 1621 | **C / C++ / Rust / Salam (10)** |
| 38_game_of_life | 12 | 13 | 39 | 50 | 11 | 11 | 10 | 1432 | 10796 | **Salam-t (10)** |
| 39_trapezoidal_integration | 15 | 16 | 16 | 18 | 16 | 15 | 15 | 505 | 4078 | **C / Salam (15)** |
| 40_matrix_stdlib_matmul | 54 | 56 | 94 | 67 | 16 | 15 | 20 | 1676 | 160 | **Salam-g (15)** |
| 41_matrix_stdlib_smallops | 6 | 7 | 8 | 24 | 52 | 60 | 70 | 598 | 1900 | C (6) |
| 42_tensor_stdlib_matmul | 54 | 55 | 93 | 68 | 9 | 9 | 9 | 1706 | 156 | **Salam (9)** |

Bold marks rows where a Salam backend is the fastest or tied for fastest.

### Top 10 fastest programs in Salam-l

Where the LLVM backend (`-O3`) does best **relative to the fastest non-Salam
entry on the same program**: sorted by `vs best other` (Salam-l's minimum
divided by that language's), lowest ratio first. Under 1.00x means Salam-l
wins the row outright. Absolute ms are shown but are not the sort key, since
they mostly track how big each program's workload is.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 42_tensor_stdlib_matmul | 9 | C (54) | 0.17x |
| 2 | 40_matrix_stdlib_matmul | 16 | C (54) | 0.30x |
| 3 | 33_subset_sum_reachability | 14 | C (21) | 0.67x |
| 4 | 12_hello_print | 3 | Rust (4) | 0.75x |
| 5 | 27_edit_distance | 7 | C (8) | 0.88x |
| 6 | 38_game_of_life | 11 | C (12) | 0.92x |
| 7 | 23_caesar_cipher | 13 | Rust (14) | 0.93x |
| 8 | 25_palindrome_count | 14 | C (15) | 0.93x |
| 9 | 36_array_rotation_reversal | 23 | C (24) | 0.96x |
| 10 | 34_counting_sort | 32 | Rust (33) | 0.97x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 52 | C (6) | 8.67x |
| 2 | 21_coin_change_dp | 4 | C (3) | 1.33x |
| 3 | 08_digit_sum | 5 | C (4) | 1.25x |
| 4 | 11_loop_count | 7 | C (6) | 1.17x |
| 5 | 14_lcg_random | 23 | C (21) | 1.10x |
| 6 | 32_longest_increasing_subsequence | 13 | Rust (12) | 1.08x |
| 7 | 13_array_rw | 27 | C (25) | 1.08x |
| 8 | 05_sum_mod | 14 | C (13) | 1.08x |
| 9 | 29_dot_product | 43 | C (40) | 1.08x |
| 10 | 17_n_queens | 45 | Rust (42) | 1.07x |
| 11 | 39_trapezoidal_integration | 16 | C (15) | 1.07x |
| 12 | 24_merge_sort | 33 | C (32) | 1.03x |
| 13 | 16_quicksort | 26 | Rust (26) | 1.00x |
| 14 | 02_fib_iterative | 21 | C (21) | 1.00x |
| 15 | 06_gcd_sum | 16 | Rust (16) | 1.00x |
| 16 | 20_monte_carlo_pi | 16 | Rust (16) | 1.00x |
| 17 | 19_mandelbrot | 12 | C (12) | 1.00x |
| 18 | 37_sqrt_decomposition | 10 | C (10) | 1.00x |
| 19 | 09_perfect_numbers | 9 | Rust (9) | 1.00x |
| 20 | 35_run_length_stats | 8 | Rust (8) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (16):** 12_hello_print, 13_array_rw, 18_sieve_eratosthenes, 20_monte_carlo_pi, 23_caesar_cipher, 25_palindrome_count, 27_edit_distance, 28_prime_factorization, 30_heap_sort, 31_binary_search_stress, 33_subset_sum_reachability, 34_counting_sort, 36_array_rotation_reversal, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (22):** 01_fib_recursive, 02_fib_iterative, 03_primes_count, 04_collatz, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 14_lcg_random, 15_matrix_mult, 16_quicksort, 19_mandelbrot, 22_knapsack_01, 26_ackermann, 29_dot_product, 32_longest_increasing_subsequence, 35_run_length_stats, 37_sqrt_decomposition, 39_trapezoidal_integration.
- **Trailing the fastest (4):** 17_n_queens (+2 vs Rust); 21_coin_change_dp (+1 vs C); 24_merge_sort (+1 vs C/C++); 41_matrix_stdlib_smallops (+46 vs C).

Salam matches or beats the fastest other compiled language on 38 of 42 programs.

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
