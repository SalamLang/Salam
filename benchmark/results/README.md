# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured three ways: the LLVM backend at
`-O3`, the C backend compiled by `gcc -O3`, and the default `tcc` toolchain
(no optimizer) as the out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-15T19:00:17Z (took 45 min)
- Commit: `5c4dee1`

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
- **go**: go version go1.26.9 linux/amd64
- **tcc**: tcc version 0.9.28rc (x86_64 Linux)
- **clang**: Alpine clang version 22.1.3
- **python**: Python 3.14.8
- **numpy**: 2.4.6
- **php**: PHP 8.5.11 (cli) (built: Sep 25 2026 15:57:29) (NTS)
- **salam**: 0.5.4

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
| 1 | **Salam-l** | 13.81 ms | 0.90x | 1.00x | 42 / 42 | 26 / 42 |
| 2 | **Salam-t** | 14.14 ms | 0.92x | 1.02x | 42 / 42 | 21 / 42 |
| 3 | **Salam-g** | 15.09 ms | 0.98x | 1.09x | 42 / 42 | 19 / 42 |
| 4 | C | 15.39 ms | 1.00x | 1.11x | 42 / 42 | 21 / 42 |
| 5 | Rust | 15.42 ms | 1.00x | 1.12x | 42 / 42 | 17 / 42 |
| 6 | C++ | 16.91 ms | 1.10x | 1.22x | 42 / 42 | 1 / 42 |
| 7 | Go | 19.45 ms | 1.26x | 1.41x | 42 / 42 | 0 / 42 |
| 8 | PHP | 186.85 ms | 12.14x | 13.53x | 42 / 42 | 0 / 42 |
| 9 | Python | 968.49 ms | 62.91x | 70.12x | 42 / 42 | 0 / 42 |

```
Salam-l      ████████████████████████████████                13.81 ms  0.90x C
Salam-t      █████████████████████████████████               14.14 ms  0.92x C
Salam-g      ███████████████████████████████████             15.09 ms  0.98x C
C            ████████████████████████████████████            15.39 ms  1.00x C  <- baseline
Rust         ████████████████████████████████████            15.42 ms  1.00x C
C++          ███████████████████████████████████████         16.91 ms  1.10x C
Go           █████████████████████████████████████████████   19.45 ms  1.26x C
PHP          (off scale)                                    186.85 ms  12.14x C
Python       (off scale)                                    968.49 ms  62.91x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 13.81 ms
  (0.90x C's geomean, 0.90x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  26/42); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.02x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 12-63x slower** than the compiled languages on
  this suite and are included for scale only.
- "Fastest / tied-fastest on" counts ties on every side (e.g. a 4-way tie
  adds 1 to all 4 languages), so the column does not sum to 42.

## Per-program results (minimum ms)

| Program | C | C++ | Rust | Go | Salam-l | Salam-g | Salam-t | PHP | Python | Fastest |
|---|---|---|---|---|---|---|---|---|---|---|
| 01_fib_recursive | 4 | 5 | 4 | 7 | 4 | 4 | 4 | 68 | 111 | **C / Rust / Salam (4)** |
| 02_fib_iterative | 21 | 22 | 21 | 22 | 21 | 21 | 21 | 63 | 837 | **C / Rust / Salam (21)** |
| 03_primes_count | 5 | 6 | 5 | 6 | 5 | 5 | 5 | 24 | 197 | **C / Rust / Salam (5)** |
| 04_collatz | 5 | 6 | 5 | 8 | 5 | 5 | 5 | 72 | 439 | **C / Rust / Salam (5)** |
| 05_sum_mod | 13 | 14 | 14 | 14 | 13 | 13 | 14 | 36 | 546 | **C / Salam (13)** |
| 06_gcd_sum | 17 | 18 | 16 | 18 | 16 | 17 | 16 | 59 | 173 | **Rust / Salam (16)** |
| 07_pow_mod | 3 | 4 | 3 | 5 | 3 | 3 | 3 | 26 | 79 | **C / Rust / Salam (3)** |
| 08_digit_sum | 4 | 5 | 5 | 5 | 4 | 4 | 4 | 79 | 451 | **C / Salam (4)** |
| 09_perfect_numbers | 10 | 11 | 9 | 11 | 9 | 10 | 9 | 44 | 493 | **Rust / Salam (9)** |
| 10_pi_leibniz | 6 | 7 | 6 | 7 | 6 | 6 | 6 | 63 | 737 | **C / Rust / Salam (6)** |
| 11_loop_count | 6 | 7 | 7 | 7 | 7 | 6 | 7 | 16 | 153 | **C / Salam-g (6)** |
| 12_hello_print | 23 | 22 | 4 | 8 | 5 | 5 | 5 | 453 | 122 | Rust (4) |
| 13_array_rw | 24 | 25 | 25 | 26 | 25 | 22 | 25 | 88 | 1256 | **Salam-g (22)** |
| 14_lcg_random | 21 | 22 | 22 | 27 | 22 | 21 | 22 | 82 | 1697 | **C / Salam-g (21)** |
| 15_matrix_mult | 7 | 8 | 7 | 11 | 6 | 8 | 7 | 239 | 1455 | **Salam-l (6)** |
| 16_quicksort | 26 | 27 | 25 | 28 | 26 | 28 | 27 | 187 | 655 | Rust (25) |
| 17_n_queens | 42 | 43 | 42 | 45 | 44 | 47 | 44 | 562 | 813 | C / Rust (42) |
| 18_sieve_eratosthenes | 6 | 7 | 7 | 10 | 7 | 9 | 7 | 119 | 1121 | C (6) |
| 19_mandelbrot | 12 | 13 | 12 | 13 | 12 | 12 | 12 | 156 | 1631 | **C / Rust / Salam (12)** |
| 20_monte_carlo_pi | 31 | 32 | 16 | 37 | 16 | 34 | 12 | 395 | 4678 | **Salam-t (12)** |
| 21_coin_change_dp | 3 | 4 | 4 | 4 | 4 | 4 | 4 | 19 | 151 | C (3) |
| 22_knapsack_01 | 6 | 7 | 5 | 7 | 6 | 6 | 6 | 133 | 1733 | Rust (5) |
| 23_caesar_cipher | 17 | 18 | 14 | 24 | 14 | 19 | 13 | 106 | 1449 | **Salam-t (13)** |
| 24_merge_sort | 31 | 31 | 32 | 35 | 31 | 36 | 36 | 319 | 1038 | **C / C++ / Salam-l (31)** |
| 25_palindrome_count | 14 | 15 | 15 | 18 | 14 | 14 | 14 | 375 | 2162 | **C / Salam (14)** |
| 26_ackermann | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 12 | 27 | **C / Rust / Salam (2)** |
| 27_edit_distance | 8 | 9 | 9 | 12 | 7 | 8 | 7 | 330 | 3197 | **Salam (7)** |
| 28_prime_factorization | 1117 | 1118 | 969 | 1105 | 967 | 1117 | 970 | 5798 | 69281 | **Salam-l (967)** |
| 29_dot_product | 39 | 40 | 43 | 42 | 43 | 39 | 40 | 277 | 3725 | **C / Salam-g (39)** |
| 30_heap_sort | 35 | 35 | 39 | 46 | 34 | 35 | 38 | 601 | 1297 | **Salam-l (34)** |
| 31_binary_search_stress | 879 | 879 | 720 | 862 | 829 | 950 | 899 | 4491 | 29926 | Rust (720) |
| 32_longest_increasing_subsequence | 15 | 16 | 12 | 14 | 12 | 17 | 13 | 312 | 3136 | **Rust / Salam-l (12)** |
| 33_subset_sum_reachability | 19 | 20 | 21 | 19 | 14 | 21 | 16 | 377 | 3501 | **Salam-l (14)** |
| 34_counting_sort | 42 | 41 | 32 | 43 | 32 | 43 | 32 | 312 | 3164 | **Rust / Salam (32)** |
| 35_run_length_stats | 9 | 10 | 8 | 9 | 7 | 9 | 8 | 148 | 1053 | **Salam-l (7)** |
| 36_array_rotation_reversal | 22 | 24 | 25 | 25 | 22 | 22 | 22 | 239 | 1276 | **C / Salam (22)** |
| 37_sqrt_decomposition | 8 | 10 | 8 | 19 | 8 | 9 | 7 | 360 | 1602 | **Salam-t (7)** |
| 38_game_of_life | 11 | 13 | 38 | 58 | 10 | 11 | 10 | 1420 | 10598 | **Salam (10)** |
| 39_trapezoidal_integration | 15 | 16 | 16 | 18 | 15 | 15 | 15 | 507 | 3842 | **C / Salam (15)** |
| 40_matrix_stdlib_matmul | 53 | 54 | 93 | 65 | 15 | 14 | 19 | 1647 | 152 | **Salam-g (14)** |
| 41_matrix_stdlib_smallops | 6 | 7 | 7 | 23 | 52 | 62 | 67 | 599 | 1931 | C (6) |
| 42_tensor_stdlib_matmul | 53 | 54 | 93 | 65 | 9 | 9 | 9 | 1645 | 151 | **Salam (9)** |

Bold marks rows where a Salam backend is the fastest or tied for fastest.

### Top 10 fastest programs in Salam-l

Where the LLVM backend (`-O3`) does best **relative to the fastest non-Salam
entry on the same program**: sorted by `vs best other` (Salam-l's minimum
divided by that language's), lowest ratio first. Under 1.00x means Salam-l
wins the row outright. Absolute ms are shown but are not the sort key, since
they mostly track how big each program's workload is.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 42_tensor_stdlib_matmul | 9 | C (53) | 0.17x |
| 2 | 40_matrix_stdlib_matmul | 15 | C (53) | 0.28x |
| 3 | 33_subset_sum_reachability | 14 | C (19) | 0.74x |
| 4 | 15_matrix_mult | 6 | C (7) | 0.86x |
| 5 | 27_edit_distance | 7 | C (8) | 0.88x |
| 6 | 35_run_length_stats | 7 | Rust (8) | 0.88x |
| 7 | 38_game_of_life | 10 | C (11) | 0.91x |
| 8 | 30_heap_sort | 34 | C (35) | 0.97x |
| 9 | 28_prime_factorization | 967 | Rust (969) | 1.00x |
| 10 | 26_ackermann | 2 | C (2) | 1.00x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 52 | C (6) | 8.67x |
| 2 | 21_coin_change_dp | 4 | C (3) | 1.33x |
| 3 | 12_hello_print | 5 | Rust (4) | 1.25x |
| 4 | 22_knapsack_01 | 6 | Rust (5) | 1.20x |
| 5 | 18_sieve_eratosthenes | 7 | C (6) | 1.17x |
| 6 | 11_loop_count | 7 | C (6) | 1.17x |
| 7 | 31_binary_search_stress | 829 | Rust (720) | 1.15x |
| 8 | 29_dot_product | 43 | C (39) | 1.10x |
| 9 | 17_n_queens | 44 | C (42) | 1.05x |
| 10 | 14_lcg_random | 22 | C (21) | 1.05x |
| 11 | 13_array_rw | 25 | C (24) | 1.04x |
| 12 | 16_quicksort | 26 | Rust (25) | 1.04x |
| 13 | 34_counting_sort | 32 | Rust (32) | 1.00x |
| 14 | 24_merge_sort | 31 | C (31) | 1.00x |
| 15 | 36_array_rotation_reversal | 22 | C (22) | 1.00x |
| 16 | 02_fib_iterative | 21 | C (21) | 1.00x |
| 17 | 06_gcd_sum | 16 | Rust (16) | 1.00x |
| 18 | 20_monte_carlo_pi | 16 | Rust (16) | 1.00x |
| 19 | 39_trapezoidal_integration | 15 | C (15) | 1.00x |
| 20 | 25_palindrome_count | 14 | C (14) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (13):** 13_array_rw, 15_matrix_mult, 20_monte_carlo_pi, 23_caesar_cipher, 27_edit_distance, 28_prime_factorization, 30_heap_sort, 33_subset_sum_reachability, 35_run_length_stats, 37_sqrt_decomposition, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (21):** 01_fib_recursive, 02_fib_iterative, 03_primes_count, 04_collatz, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 14_lcg_random, 19_mandelbrot, 24_merge_sort, 25_palindrome_count, 26_ackermann, 29_dot_product, 32_longest_increasing_subsequence, 34_counting_sort, 36_array_rotation_reversal, 39_trapezoidal_integration.
- **Trailing the fastest (8):** 12_hello_print (+1 vs Rust); 16_quicksort (+1 vs Rust); 17_n_queens (+2 vs C/Rust); 18_sieve_eratosthenes (+1 vs C); 21_coin_change_dp (+1 vs C); 22_knapsack_01 (+1 vs Rust); 31_binary_search_stress (+109 vs Rust); 41_matrix_stdlib_smallops (+46 vs C).

Salam matches or beats the fastest other compiled language on 34 of 42 programs.

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
