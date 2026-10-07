# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured three ways: the LLVM backend at
`-O3`, the C backend compiled by `gcc -O3`, and the default `tcc` toolchain
(no optimizer) as the out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-07T20:34:49Z (took 46 min)
- Commit: `b58be05`

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

- **CPU**: AMD EPYC 9V74 80-Core Processor
- **gcc**: gcc (Alpine 15.2.0) 15.2.0
- **g++**: g++ (Alpine 15.2.0) 15.2.0
- **rustc**: rustc 1.96.1 (31fca3adb 2026-06-26) (Alpine Linux Rust 1.96.1-r0)
- **go**: go version go1.26.8 linux/amd64
- **tcc**: tcc version 0.9.28rc (x86_64 Linux)
- **clang**: Alpine clang version 22.1.3
- **python**: Python 3.14.8
- **numpy**: 2.4.6
- **php**: PHP 8.5.11 (cli) (built: Sep 25 2026 15:57:29) (NTS)
- **salam**: 0.5.1

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
| 1 | **Salam-l** | 15.45 ms | 0.89x | 1.00x | 42 / 42 | 26 / 42 |
| 2 | **Salam-t** | 15.92 ms | 0.92x | 1.03x | 42 / 42 | 23 / 42 |
| 3 | **Salam-g** | 16.94 ms | 0.98x | 1.10x | 42 / 42 | 15 / 42 |
| 4 | Rust | 17.20 ms | 1.00x | 1.11x | 42 / 42 | 17 / 42 |
| 5 | C | 17.27 ms | 1.00x | 1.12x | 42 / 42 | 19 / 42 |
| 6 | C++ | 19.12 ms | 1.11x | 1.24x | 42 / 42 | 2 / 42 |
| 7 | Go | 22.11 ms | 1.28x | 1.43x | 42 / 42 | 1 / 42 |
| 8 | PHP | 201.74 ms | 11.68x | 13.05x | 42 / 42 | 0 / 42 |
| 9 | Python | 945.23 ms | 54.74x | 61.17x | 42 / 42 | 0 / 42 |

```
Salam-l      ███████████████████████████████                 15.45 ms  0.89x C
Salam-t      ████████████████████████████████                15.92 ms  0.92x C
Salam-g      ██████████████████████████████████              16.94 ms  0.98x C
Rust         ███████████████████████████████████             17.20 ms  1.00x C
C            ███████████████████████████████████             17.27 ms  1.00x C  <- baseline
C++          ███████████████████████████████████████         19.12 ms  1.11x C
Go           █████████████████████████████████████████████   22.11 ms  1.28x C
PHP          (off scale)                                    201.74 ms  11.68x C
Python       (off scale)                                    945.23 ms  54.74x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 15.45 ms
  (0.89x C's geomean, 0.90x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  26/42); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.03x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 12-55x slower** than the compiled languages on
  this suite and are included for scale only.
- "Fastest / tied-fastest on" counts ties on every side (e.g. a 4-way tie
  adds 1 to all 4 languages), so the column does not sum to 42.

## Per-program results (minimum ms)

| Program | C | C++ | Rust | Go | Salam-l | Salam-g | Salam-t | PHP | Python | Fastest |
|---|---|---|---|---|---|---|---|---|---|---|
| 01_fib_recursive | 4 | 5 | 5 | 8 | 5 | 4 | 5 | 70 | 108 | **C / Salam-g (4)** |
| 02_fib_iterative | 23 | 24 | 24 | 25 | 24 | 23 | 24 | 73 | 805 | **C / Salam-g (23)** |
| 03_primes_count | 6 | 7 | 5 | 7 | 5 | 6 | 5 | 27 | 198 | **Rust / Salam (5)** |
| 04_collatz | 6 | 7 | 6 | 8 | 5 | 6 | 5 | 72 | 428 | **Salam (5)** |
| 05_sum_mod | 15 | 16 | 16 | 16 | 15 | 15 | 15 | 41 | 543 | **C / Salam (15)** |
| 06_gcd_sum | 19 | 20 | 18 | 20 | 18 | 19 | 18 | 62 | 176 | **Rust / Salam (18)** |
| 07_pow_mod | 4 | 5 | 4 | 5 | 4 | 4 | 4 | 26 | 80 | **C / Rust / Salam (4)** |
| 08_digit_sum | 5 | 6 | 5 | 6 | 5 | 5 | 5 | 80 | 450 | **C / Rust / Salam (5)** |
| 09_perfect_numbers | 12 | 13 | 10 | 13 | 10 | 11 | 10 | 50 | 493 | **Rust / Salam (10)** |
| 10_pi_leibniz | 7 | 8 | 7 | 8 | 7 | 7 | 7 | 60 | 714 | **C / Rust / Salam (7)** |
| 11_loop_count | 7 | 8 | 7 | 8 | 7 | 7 | 7 | 18 | 147 | **C / Rust / Salam (7)** |
| 12_hello_print | 24 | 23 | 4 | 10 | 5 | 5 | 5 | 787 | 119 | Rust (4) |
| 13_array_rw | 27 | 29 | 29 | 29 | 29 | 26 | 28 | 99 | 1243 | **Salam-g (26)** |
| 14_lcg_random | 23 | 24 | 25 | 31 | 25 | 23 | 25 | 90 | 1684 | **C / Salam-g (23)** |
| 15_matrix_mult | 8 | 9 | 8 | 12 | 8 | 9 | 8 | 227 | 1428 | **C / Rust / Salam (8)** |
| 16_quicksort | 30 | 31 | 29 | 32 | 29 | 31 | 31 | 209 | 678 | **Rust / Salam-l (29)** |
| 17_n_queens | 46 | 47 | 49 | 55 | 52 | 53 | 50 | 562 | 796 | C (46) |
| 18_sieve_eratosthenes | 7 | 8 | 8 | 9 | 7 | 10 | 7 | 130 | 1165 | **C / Salam (7)** |
| 19_mandelbrot | 13 | 14 | 13 | 14 | 13 | 13 | 14 | 165 | 1606 | **C / Rust / Salam (13)** |
| 20_monte_carlo_pi | 34 | 35 | 18 | 40 | 18 | 38 | 14 | 444 | 4459 | **Salam-t (14)** |
| 21_coin_change_dp | 4 | 5 | 4 | 4 | 4 | 4 | 4 | 22 | 147 | **C / Rust / Go / Salam (4)** |
| 22_knapsack_01 | 6 | 7 | 6 | 10 | 6 | 7 | 7 | 144 | 1401 | **C / Rust / Salam-l (6)** |
| 23_caesar_cipher | 22 | 23 | 15 | 27 | 15 | 22 | 15 | 128 | 1418 | **Rust / Salam (15)** |
| 24_merge_sort | 34 | 34 | 37 | 41 | 37 | 40 | 41 | 344 | 997 | C / C++ (34) |
| 25_palindrome_count | 15 | 16 | 16 | 20 | 17 | 16 | 17 | 356 | 2171 | C (15) |
| 26_ackermann | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 13 | 29 | **C / Rust / Salam (2)** |
| 27_edit_distance | 9 | 10 | 10 | 12 | 8 | 9 | 8 | 321 | 3194 | **Salam (8)** |
| 28_prime_factorization | 1262 | 1263 | 1094 | 1248 | 1092 | 1261 | 1091 | 6415 | 69268 | **Salam-t (1091)** |
| 29_dot_product | 47 | 48 | 49 | 49 | 48 | 46 | 48 | 325 | 3779 | **Salam-g (46)** |
| 30_heap_sort | 40 | 39 | 43 | 51 | 39 | 40 | 43 | 613 | 1334 | **C++ / Salam-l (39)** |
| 31_binary_search_stress | 1011 | 1014 | 846 | 984 | 930 | 1067 | 997 | 5920 | 32986 | Rust (846) |
| 32_longest_increasing_subsequence | 18 | 19 | 14 | 22 | 14 | 20 | 13 | 334 | 2386 | **Salam-t (13)** |
| 33_subset_sum_reachability | 22 | 24 | 24 | 27 | 17 | 23 | 17 | 416 | 3124 | **Salam (17)** |
| 34_counting_sort | 47 | 50 | 35 | 50 | 35 | 48 | 35 | 288 | 3178 | **Rust / Salam (35)** |
| 35_run_length_stats | 10 | 12 | 10 | 11 | 9 | 10 | 9 | 143 | 1064 | **Salam (9)** |
| 36_array_rotation_reversal | 24 | 28 | 27 | 27 | 25 | 25 | 25 | 259 | 1183 | C (24) |
| 37_sqrt_decomposition | 9 | 11 | 9 | 21 | 8 | 10 | 9 | 379 | 1546 | **Salam-l (8)** |
| 38_game_of_life | 13 | 14 | 38 | 55 | 11 | 12 | 11 | 1495 | 10397 | **Salam (11)** |
| 39_trapezoidal_integration | 17 | 18 | 16 | 19 | 15 | 17 | 17 | 508 | 3543 | **Salam-l (15)** |
| 40_matrix_stdlib_matmul | 58 | 59 | 103 | 74 | 17 | 16 | 22 | 1723 | 153 | **Salam-g (16)** |
| 41_matrix_stdlib_smallops | 6 | 7 | 8 | 24 | 50 | 63 | 69 | 610 | 1679 | C (6) |
| 42_tensor_stdlib_matmul | 58 | 59 | 103 | 74 | 10 | 10 | 10 | 1725 | 150 | **Salam (10)** |

Bold marks rows where a Salam backend is the fastest or tied for fastest.

### Top 10 fastest programs in Salam-l

Where the LLVM backend (`-O3`) does best **relative to the fastest non-Salam
entry on the same program**: sorted by `vs best other` (Salam-l's minimum
divided by that language's), lowest ratio first. Under 1.00x means Salam-l
wins the row outright. Absolute ms are shown but are not the sort key, since
they mostly track how big each program's workload is.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 42_tensor_stdlib_matmul | 10 | C (58) | 0.17x |
| 2 | 40_matrix_stdlib_matmul | 17 | C (58) | 0.29x |
| 3 | 33_subset_sum_reachability | 17 | C (22) | 0.77x |
| 4 | 04_collatz | 5 | C (6) | 0.83x |
| 5 | 38_game_of_life | 11 | C (13) | 0.85x |
| 6 | 27_edit_distance | 8 | C (9) | 0.89x |
| 7 | 37_sqrt_decomposition | 8 | C (9) | 0.89x |
| 8 | 35_run_length_stats | 9 | C (10) | 0.90x |
| 9 | 39_trapezoidal_integration | 15 | Rust (16) | 0.94x |
| 10 | 28_prime_factorization | 1092 | Rust (1094) | 1.00x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 50 | C (6) | 8.33x |
| 2 | 01_fib_recursive | 5 | C (4) | 1.25x |
| 3 | 12_hello_print | 5 | Rust (4) | 1.25x |
| 4 | 25_palindrome_count | 17 | C (15) | 1.13x |
| 5 | 17_n_queens | 52 | C (46) | 1.13x |
| 6 | 31_binary_search_stress | 930 | Rust (846) | 1.10x |
| 7 | 24_merge_sort | 37 | C (34) | 1.09x |
| 8 | 14_lcg_random | 25 | C (23) | 1.09x |
| 9 | 13_array_rw | 29 | C (27) | 1.07x |
| 10 | 02_fib_iterative | 24 | C (23) | 1.04x |
| 11 | 36_array_rotation_reversal | 25 | C (24) | 1.04x |
| 12 | 29_dot_product | 48 | C (47) | 1.02x |
| 13 | 30_heap_sort | 39 | C++ (39) | 1.00x |
| 14 | 34_counting_sort | 35 | Rust (35) | 1.00x |
| 15 | 16_quicksort | 29 | Rust (29) | 1.00x |
| 16 | 20_monte_carlo_pi | 18 | Rust (18) | 1.00x |
| 17 | 06_gcd_sum | 18 | Rust (18) | 1.00x |
| 18 | 05_sum_mod | 15 | C (15) | 1.00x |
| 19 | 23_caesar_cipher | 15 | Rust (15) | 1.00x |
| 20 | 32_longest_increasing_subsequence | 14 | Rust (14) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (14):** 04_collatz, 13_array_rw, 20_monte_carlo_pi, 27_edit_distance, 28_prime_factorization, 29_dot_product, 32_longest_increasing_subsequence, 33_subset_sum_reachability, 35_run_length_stats, 37_sqrt_decomposition, 38_game_of_life, 39_trapezoidal_integration, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (21):** 01_fib_recursive, 02_fib_iterative, 03_primes_count, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 14_lcg_random, 15_matrix_mult, 16_quicksort, 18_sieve_eratosthenes, 19_mandelbrot, 21_coin_change_dp, 22_knapsack_01, 23_caesar_cipher, 26_ackermann, 30_heap_sort, 34_counting_sort.
- **Trailing the fastest (7):** 12_hello_print (+1 vs Rust); 17_n_queens (+4 vs C); 24_merge_sort (+3 vs C/C++); 25_palindrome_count (+1 vs C); 31_binary_search_stress (+84 vs Rust); 36_array_rotation_reversal (+1 vs C); 41_matrix_stdlib_smallops (+44 vs C).

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
