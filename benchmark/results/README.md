# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured three ways: the LLVM backend at
`-O3`, the C backend compiled by `gcc -O3`, and the default `tcc` toolchain
(no optimizer) as the out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-09T08:46:28Z (took 46 min)
- Commit: `a978993`

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
| 1 | **Salam-l** | 14.15 ms | 0.90x | 1.00x | 42 / 42 | 22 / 42 |
| 2 | **Salam-t** | 14.47 ms | 0.92x | 1.02x | 42 / 42 | 22 / 42 |
| 3 | **Salam-g** | 15.42 ms | 0.98x | 1.09x | 42 / 42 | 18 / 42 |
| 4 | C | 15.75 ms | 1.00x | 1.11x | 42 / 42 | 19 / 42 |
| 5 | Rust | 15.77 ms | 1.00x | 1.11x | 42 / 42 | 19 / 42 |
| 6 | C++ | 17.31 ms | 1.10x | 1.22x | 42 / 42 | 2 / 42 |
| 7 | Go | 20.24 ms | 1.29x | 1.43x | 42 / 42 | 0 / 42 |
| 8 | PHP | 191.39 ms | 12.16x | 13.52x | 42 / 42 | 0 / 42 |
| 9 | Python | 967.28 ms | 61.43x | 68.35x | 42 / 42 | 0 / 42 |

```
Salam-l      ███████████████████████████████                 14.15 ms  0.90x C
Salam-t      ████████████████████████████████                14.47 ms  0.92x C
Salam-g      ██████████████████████████████████              15.42 ms  0.98x C
C            ███████████████████████████████████             15.75 ms  1.00x C  <- baseline
Rust         ███████████████████████████████████             15.77 ms  1.00x C
C++          ██████████████████████████████████████          17.31 ms  1.10x C
Go           █████████████████████████████████████████████   20.24 ms  1.29x C
PHP          (off scale)                                    191.39 ms  12.16x C
Python       (off scale)                                    967.28 ms  61.43x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 14.15 ms
  (0.90x C's geomean, 0.90x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  22/42); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.02x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 12-61x slower** than the compiled languages on
  this suite and are included for scale only.
- "Fastest / tied-fastest on" counts ties on every side (e.g. a 4-way tie
  adds 1 to all 4 languages), so the column does not sum to 42.

## Per-program results (minimum ms)

| Program | C | C++ | Rust | Go | Salam-l | Salam-g | Salam-t | PHP | Python | Fastest |
|---|---|---|---|---|---|---|---|---|---|---|
| 01_fib_recursive | 4 | 5 | 4 | 7 | 4 | 4 | 4 | 69 | 113 | **C / Rust / Salam (4)** |
| 02_fib_iterative | 21 | 22 | 22 | 22 | 21 | 21 | 21 | 64 | 840 | **C / Salam (21)** |
| 03_primes_count | 5 | 6 | 5 | 6 | 5 | 5 | 5 | 24 | 200 | **C / Rust / Salam (5)** |
| 04_collatz | 5 | 7 | 5 | 8 | 5 | 5 | 5 | 73 | 443 | **C / Rust / Salam (5)** |
| 05_sum_mod | 13 | 14 | 14 | 14 | 14 | 13 | 14 | 37 | 528 | **C / Salam-g (13)** |
| 06_gcd_sum | 17 | 18 | 16 | 18 | 16 | 17 | 16 | 61 | 175 | **Rust / Salam (16)** |
| 07_pow_mod | 3 | 4 | 3 | 5 | 3 | 3 | 3 | 26 | 81 | **C / Rust / Salam (3)** |
| 08_digit_sum | 4 | 5 | 5 | 5 | 5 | 4 | 4 | 81 | 449 | **C / Salam (4)** |
| 09_perfect_numbers | 10 | 11 | 9 | 12 | 9 | 10 | 9 | 47 | 499 | **Rust / Salam (9)** |
| 10_pi_leibniz | 6 | 7 | 6 | 7 | 6 | 6 | 6 | 63 | 736 | **C / Rust / Salam (6)** |
| 11_loop_count | 6 | 7 | 7 | 7 | 7 | 6 | 7 | 17 | 157 | **C / Salam-g (6)** |
| 12_hello_print | 23 | 22 | 4 | 10 | 5 | 5 | 5 | 454 | 124 | Rust (4) |
| 13_array_rw | 25 | 26 | 26 | 27 | 26 | 23 | 26 | 92 | 1249 | **Salam-g (23)** |
| 14_lcg_random | 21 | 22 | 23 | 28 | 23 | 21 | 23 | 83 | 1699 | **C / Salam-g (21)** |
| 15_matrix_mult | 8 | 9 | 7 | 11 | 7 | 8 | 7 | 243 | 1431 | **Rust / Salam (7)** |
| 16_quicksort | 26 | 27 | 26 | 28 | 26 | 28 | 28 | 190 | 705 | **C / Rust / Salam-l (26)** |
| 17_n_queens | 42 | 43 | 42 | 50 | 45 | 48 | 44 | 566 | 802 | C / Rust (42) |
| 18_sieve_eratosthenes | 8 | 7 | 7 | 8 | 7 | 9 | 7 | 122 | 1140 | **C++ / Rust / Salam (7)** |
| 19_mandelbrot | 12 | 13 | 12 | 14 | 12 | 12 | 12 | 159 | 1677 | **C / Rust / Salam (12)** |
| 20_monte_carlo_pi | 31 | 33 | 17 | 38 | 16 | 34 | 13 | 393 | 4795 | **Salam-t (13)** |
| 21_coin_change_dp | 3 | 4 | 4 | 4 | 4 | 4 | 4 | 21 | 153 | C (3) |
| 22_knapsack_01 | 6 | 7 | 5 | 9 | 6 | 6 | 6 | 135 | 1431 | Rust (5) |
| 23_caesar_cipher | 17 | 18 | 14 | 24 | 14 | 20 | 13 | 108 | 1452 | **Salam-t (13)** |
| 24_merge_sort | 32 | 31 | 33 | 38 | 32 | 36 | 36 | 321 | 1060 | C++ (31) |
| 25_palindrome_count | 14 | 15 | 15 | 18 | 14 | 14 | 14 | 369 | 2187 | **C / Salam (14)** |
| 26_ackermann | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 12 | 27 | **C / Rust / Salam (2)** |
| 27_edit_distance | 8 | 9 | 9 | 11 | 7 | 8 | 7 | 330 | 3172 | **Salam (7)** |
| 28_prime_factorization | 1117 | 1119 | 970 | 1107 | 968 | 1118 | 970 | 5814 | 70445 | **Salam-l (968)** |
| 29_dot_product | 40 | 41 | 43 | 43 | 43 | 40 | 41 | 279 | 3708 | **C / Salam-g (40)** |
| 30_heap_sort | 36 | 36 | 40 | 47 | 35 | 36 | 38 | 635 | 1321 | **Salam-l (35)** |
| 31_binary_search_stress | 888 | 890 | 842 | 889 | 867 | 1017 | 993 | 6481 | 33024 | Rust (842) |
| 32_longest_increasing_subsequence | 16 | 16 | 12 | 19 | 12 | 17 | 13 | 314 | 2421 | **Rust / Salam-l (12)** |
| 33_subset_sum_reachability | 19 | 20 | 21 | 26 | 14 | 21 | 16 | 378 | 3334 | **Salam-l (14)** |
| 34_counting_sort | 42 | 42 | 32 | 45 | 32 | 43 | 32 | 316 | 3202 | **Rust / Salam (32)** |
| 35_run_length_stats | 9 | 10 | 8 | 10 | 8 | 9 | 8 | 150 | 1059 | **Rust / Salam (8)** |
| 36_array_rotation_reversal | 24 | 26 | 26 | 26 | 23 | 24 | 24 | 240 | 1282 | **Salam-l (23)** |
| 37_sqrt_decomposition | 9 | 10 | 9 | 20 | 9 | 10 | 8 | 363 | 1552 | **Salam-t (8)** |
| 38_game_of_life | 12 | 13 | 38 | 48 | 11 | 11 | 10 | 1391 | 10603 | **Salam-t (10)** |
| 39_trapezoidal_integration | 15 | 16 | 16 | 18 | 16 | 15 | 15 | 505 | 3780 | **C / Salam (15)** |
| 40_matrix_stdlib_matmul | 54 | 55 | 93 | 67 | 16 | 15 | 20 | 1654 | 161 | **Salam-g (15)** |
| 41_matrix_stdlib_smallops | 6 | 7 | 8 | 24 | 53 | 63 | 68 | 593 | 1915 | C (6) |
| 42_tensor_stdlib_matmul | 54 | 56 | 94 | 67 | 10 | 9 | 10 | 1670 | 159 | **Salam-g (9)** |

Bold marks rows where a Salam backend is the fastest or tied for fastest.

### Top 10 fastest programs in Salam-l

Where the LLVM backend (`-O3`) does best **relative to the fastest non-Salam
entry on the same program**: sorted by `vs best other` (Salam-l's minimum
divided by that language's), lowest ratio first. Under 1.00x means Salam-l
wins the row outright. Absolute ms are shown but are not the sort key, since
they mostly track how big each program's workload is.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 42_tensor_stdlib_matmul | 10 | C (54) | 0.19x |
| 2 | 40_matrix_stdlib_matmul | 16 | C (54) | 0.30x |
| 3 | 33_subset_sum_reachability | 14 | C (19) | 0.74x |
| 4 | 27_edit_distance | 7 | C (8) | 0.88x |
| 5 | 38_game_of_life | 11 | C (12) | 0.92x |
| 6 | 20_monte_carlo_pi | 16 | Rust (17) | 0.94x |
| 7 | 36_array_rotation_reversal | 23 | C (24) | 0.96x |
| 8 | 30_heap_sort | 35 | C (36) | 0.97x |
| 9 | 28_prime_factorization | 968 | Rust (970) | 1.00x |
| 10 | 26_ackermann | 2 | C (2) | 1.00x |

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
| 6 | 11_loop_count | 7 | C (6) | 1.17x |
| 7 | 14_lcg_random | 23 | C (21) | 1.10x |
| 8 | 05_sum_mod | 14 | C (13) | 1.08x |
| 9 | 29_dot_product | 43 | C (40) | 1.08x |
| 10 | 17_n_queens | 45 | C (42) | 1.07x |
| 11 | 39_trapezoidal_integration | 16 | C (15) | 1.07x |
| 12 | 13_array_rw | 26 | C (25) | 1.04x |
| 13 | 24_merge_sort | 32 | C++ (31) | 1.03x |
| 14 | 31_binary_search_stress | 867 | Rust (842) | 1.03x |
| 15 | 34_counting_sort | 32 | Rust (32) | 1.00x |
| 16 | 16_quicksort | 26 | C (26) | 1.00x |
| 17 | 02_fib_iterative | 21 | C (21) | 1.00x |
| 18 | 06_gcd_sum | 16 | Rust (16) | 1.00x |
| 19 | 25_palindrome_count | 14 | C (14) | 1.00x |
| 20 | 23_caesar_cipher | 14 | Rust (14) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (12):** 13_array_rw, 20_monte_carlo_pi, 23_caesar_cipher, 27_edit_distance, 28_prime_factorization, 30_heap_sort, 33_subset_sum_reachability, 36_array_rotation_reversal, 37_sqrt_decomposition, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (23):** 01_fib_recursive, 02_fib_iterative, 03_primes_count, 04_collatz, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 14_lcg_random, 15_matrix_mult, 16_quicksort, 18_sieve_eratosthenes, 19_mandelbrot, 25_palindrome_count, 26_ackermann, 29_dot_product, 32_longest_increasing_subsequence, 34_counting_sort, 35_run_length_stats, 39_trapezoidal_integration.
- **Trailing the fastest (7):** 12_hello_print (+1 vs Rust); 17_n_queens (+2 vs C/Rust); 21_coin_change_dp (+1 vs C); 22_knapsack_01 (+1 vs Rust); 24_merge_sort (+1 vs C++); 31_binary_search_stress (+25 vs Rust); 41_matrix_stdlib_smallops (+47 vs C).

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
