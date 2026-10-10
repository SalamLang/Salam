# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured three ways: the LLVM backend at
`-O3`, the C backend compiled by `gcc -O3`, and the default `tcc` toolchain
(no optimizer) as the out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-09T01:10:17Z (took 47 min)
- Commit: `4b437c6`

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
| 1 | **Salam-l** | 14.22 ms | 0.90x | 1.00x | 42 / 42 | 24 / 42 |
| 2 | **Salam-t** | 14.54 ms | 0.92x | 1.02x | 42 / 42 | 19 / 42 |
| 3 | **Salam-g** | 15.50 ms | 0.98x | 1.09x | 42 / 42 | 18 / 42 |
| 4 | C | 15.81 ms | 1.00x | 1.11x | 42 / 42 | 20 / 42 |
| 5 | Rust | 16.08 ms | 1.02x | 1.13x | 42 / 42 | 16 / 42 |
| 6 | C++ | 17.53 ms | 1.11x | 1.23x | 42 / 42 | 1 / 42 |
| 7 | Go | 20.37 ms | 1.29x | 1.43x | 42 / 42 | 0 / 42 |
| 8 | PHP | 193.14 ms | 12.22x | 13.59x | 42 / 42 | 0 / 42 |
| 9 | Python | 983.13 ms | 62.20x | 69.15x | 42 / 42 | 0 / 42 |

```
Salam-l      ███████████████████████████████                 14.22 ms  0.90x C
Salam-t      ████████████████████████████████                14.54 ms  0.92x C
Salam-g      ██████████████████████████████████              15.50 ms  0.98x C
C            ███████████████████████████████████             15.81 ms  1.00x C  <- baseline
Rust         ████████████████████████████████████            16.08 ms  1.02x C
C++          ███████████████████████████████████████         17.53 ms  1.11x C
Go           █████████████████████████████████████████████   20.37 ms  1.29x C
PHP          (off scale)                                    193.14 ms  12.22x C
Python       (off scale)                                    983.13 ms  62.20x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 14.22 ms
  (0.90x C's geomean, 0.88x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  24/42); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.02x slower than the
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
| 01_fib_recursive | 4 | 5 | 4 | 8 | 4 | 4 | 4 | 69 | 112 | **C / Rust / Salam (4)** |
| 02_fib_iterative | 21 | 22 | 22 | 22 | 21 | 21 | 22 | 64 | 841 | **C / Salam (21)** |
| 03_primes_count | 5 | 6 | 5 | 6 | 5 | 5 | 5 | 25 | 204 | **C / Rust / Salam (5)** |
| 04_collatz | 5 | 7 | 5 | 8 | 5 | 5 | 5 | 74 | 446 | **C / Rust / Salam (5)** |
| 05_sum_mod | 13 | 15 | 14 | 15 | 14 | 14 | 14 | 37 | 535 | C (13) |
| 06_gcd_sum | 17 | 18 | 16 | 18 | 16 | 17 | 16 | 60 | 177 | **Rust / Salam (16)** |
| 07_pow_mod | 3 | 4 | 4 | 5 | 3 | 3 | 3 | 27 | 83 | **C / Salam (3)** |
| 08_digit_sum | 4 | 5 | 5 | 5 | 5 | 4 | 4 | 81 | 460 | **C / Salam (4)** |
| 09_perfect_numbers | 10 | 12 | 9 | 12 | 9 | 11 | 9 | 46 | 511 | **Rust / Salam (9)** |
| 10_pi_leibniz | 6 | 7 | 7 | 7 | 6 | 6 | 6 | 63 | 738 | **C / Salam (6)** |
| 11_loop_count | 6 | 7 | 7 | 7 | 7 | 6 | 7 | 18 | 156 | **C / Salam-g (6)** |
| 12_hello_print | 23 | 22 | 4 | 10 | 5 | 5 | 5 | 456 | 124 | Rust (4) |
| 13_array_rw | 25 | 26 | 26 | 26 | 26 | 23 | 26 | 90 | 1263 | **Salam-g (23)** |
| 14_lcg_random | 21 | 22 | 23 | 28 | 23 | 21 | 23 | 83 | 1707 | **C / Salam-g (21)** |
| 15_matrix_mult | 8 | 9 | 7 | 11 | 7 | 8 | 7 | 245 | 1464 | **Rust / Salam (7)** |
| 16_quicksort | 27 | 28 | 26 | 28 | 27 | 29 | 28 | 190 | 736 | Rust (26) |
| 17_n_queens | 43 | 43 | 42 | 50 | 45 | 48 | 44 | 564 | 807 | Rust (42) |
| 18_sieve_eratosthenes | 6 | 8 | 7 | 9 | 7 | 9 | 7 | 122 | 1139 | C (6) |
| 19_mandelbrot | 12 | 13 | 12 | 14 | 12 | 12 | 12 | 160 | 1643 | **C / Rust / Salam (12)** |
| 20_monte_carlo_pi | 31 | 33 | 17 | 38 | 16 | 34 | 13 | 393 | 4731 | **Salam-t (13)** |
| 21_coin_change_dp | 3 | 4 | 4 | 4 | 4 | 4 | 4 | 20 | 156 | C (3) |
| 22_knapsack_01 | 6 | 7 | 5 | 9 | 6 | 6 | 6 | 135 | 1465 | Rust (5) |
| 23_caesar_cipher | 17 | 18 | 14 | 25 | 14 | 20 | 14 | 108 | 1461 | **Rust / Salam (14)** |
| 24_merge_sort | 31 | 31 | 33 | 39 | 32 | 37 | 37 | 323 | 1129 | C / C++ (31) |
| 25_palindrome_count | 15 | 16 | 15 | 19 | 15 | 15 | 15 | 375 | 2219 | **C / Rust / Salam (15)** |
| 26_ackermann | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 13 | 30 | **C / Rust / Salam (2)** |
| 27_edit_distance | 8 | 9 | 10 | 11 | 7 | 8 | 8 | 331 | 3151 | **Salam-l (7)** |
| 28_prime_factorization | 1117 | 1119 | 970 | 1106 | 968 | 1118 | 970 | 5809 | 71473 | **Salam-l (968)** |
| 29_dot_product | 40 | 41 | 43 | 43 | 43 | 40 | 41 | 278 | 3744 | **C / Salam-g (40)** |
| 30_heap_sort | 36 | 36 | 40 | 47 | 35 | 36 | 38 | 651 | 1580 | **Salam-l (35)** |
| 31_binary_search_stress | 1013 | 1079 | 1477 | 951 | 914 | 1090 | 1083 | 6855 | 33014 | **Salam-l (914)** |
| 32_longest_increasing_subsequence | 15 | 16 | 12 | 19 | 12 | 17 | 13 | 313 | 2485 | **Rust / Salam-l (12)** |
| 33_subset_sum_reachability | 19 | 20 | 21 | 25 | 14 | 21 | 16 | 378 | 3347 | **Salam-l (14)** |
| 34_counting_sort | 42 | 42 | 32 | 45 | 32 | 43 | 32 | 317 | 3215 | **Rust / Salam (32)** |
| 35_run_length_stats | 9 | 10 | 8 | 10 | 7 | 9 | 8 | 151 | 1079 | **Salam-l (7)** |
| 36_array_rotation_reversal | 23 | 26 | 25 | 26 | 23 | 23 | 24 | 240 | 1277 | **C / Salam (23)** |
| 37_sqrt_decomposition | 9 | 11 | 9 | 20 | 8 | 10 | 8 | 363 | 1634 | **Salam (8)** |
| 38_game_of_life | 12 | 13 | 38 | 48 | 11 | 11 | 10 | 1465 | 10544 | **Salam-t (10)** |
| 39_trapezoidal_integration | 15 | 16 | 16 | 18 | 16 | 15 | 15 | 522 | 3862 | **C / Salam (15)** |
| 40_matrix_stdlib_matmul | 54 | 56 | 93 | 66 | 16 | 15 | 20 | 1654 | 160 | **Salam-g (15)** |
| 41_matrix_stdlib_smallops | 6 | 7 | 7 | 23 | 53 | 63 | 68 | 591 | 1935 | C (6) |
| 42_tensor_stdlib_matmul | 54 | 56 | 93 | 66 | 9 | 9 | 9 | 1654 | 158 | **Salam (9)** |

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
| 3 | 33_subset_sum_reachability | 14 | C (19) | 0.74x |
| 4 | 27_edit_distance | 7 | C (8) | 0.88x |
| 5 | 35_run_length_stats | 7 | Rust (8) | 0.88x |
| 6 | 37_sqrt_decomposition | 8 | C (9) | 0.89x |
| 7 | 38_game_of_life | 11 | C (12) | 0.92x |
| 8 | 20_monte_carlo_pi | 16 | Rust (17) | 0.94x |
| 9 | 31_binary_search_stress | 914 | Go (951) | 0.96x |
| 10 | 30_heap_sort | 35 | C (36) | 0.97x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 53 | C (6) | 8.83x |
| 2 | 21_coin_change_dp | 4 | C (3) | 1.33x |
| 3 | 12_hello_print | 5 | Rust (4) | 1.25x |
| 4 | 08_digit_sum | 5 | C (4) | 1.25x |
| 5 | 22_knapsack_01 | 6 | Rust (5) | 1.20x |
| 6 | 18_sieve_eratosthenes | 7 | C (6) | 1.17x |
| 7 | 11_loop_count | 7 | C (6) | 1.17x |
| 8 | 14_lcg_random | 23 | C (21) | 1.10x |
| 9 | 05_sum_mod | 14 | C (13) | 1.08x |
| 10 | 29_dot_product | 43 | C (40) | 1.08x |
| 11 | 17_n_queens | 45 | Rust (42) | 1.07x |
| 12 | 39_trapezoidal_integration | 16 | C (15) | 1.07x |
| 13 | 13_array_rw | 26 | C (25) | 1.04x |
| 14 | 16_quicksort | 27 | Rust (26) | 1.04x |
| 15 | 24_merge_sort | 32 | C (31) | 1.03x |
| 16 | 34_counting_sort | 32 | Rust (32) | 1.00x |
| 17 | 36_array_rotation_reversal | 23 | C (23) | 1.00x |
| 18 | 02_fib_iterative | 21 | C (21) | 1.00x |
| 19 | 06_gcd_sum | 16 | Rust (16) | 1.00x |
| 20 | 25_palindrome_count | 15 | C (15) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (12):** 13_array_rw, 20_monte_carlo_pi, 27_edit_distance, 28_prime_factorization, 30_heap_sort, 31_binary_search_stress, 33_subset_sum_reachability, 35_run_length_stats, 37_sqrt_decomposition, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (21):** 01_fib_recursive, 02_fib_iterative, 03_primes_count, 04_collatz, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 14_lcg_random, 15_matrix_mult, 19_mandelbrot, 23_caesar_cipher, 25_palindrome_count, 26_ackermann, 29_dot_product, 32_longest_increasing_subsequence, 34_counting_sort, 36_array_rotation_reversal, 39_trapezoidal_integration.
- **Trailing the fastest (9):** 05_sum_mod (+1 vs C); 12_hello_print (+1 vs Rust); 16_quicksort (+1 vs Rust); 17_n_queens (+2 vs Rust); 18_sieve_eratosthenes (+1 vs C); 21_coin_change_dp (+1 vs C); 22_knapsack_01 (+1 vs Rust); 24_merge_sort (+1 vs C/C++); 41_matrix_stdlib_smallops (+47 vs C).

Salam matches or beats the fastest other compiled language on 33 of 42 programs.

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
