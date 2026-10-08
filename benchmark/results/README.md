# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured three ways: the LLVM backend at
`-O3`, the C backend compiled by `gcc -O3`, and the default `tcc` toolchain
(no optimizer) as the out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-08T20:54:53Z (took 46 min)
- Commit: `6fef3d7`

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
| 1 | **Salam-l** | 14.02 ms | 0.89x | 1.00x | 42 / 42 | 26 / 42 |
| 2 | **Salam-t** | 14.37 ms | 0.91x | 1.02x | 42 / 42 | 21 / 42 |
| 3 | **Salam-g** | 15.33 ms | 0.97x | 1.09x | 42 / 42 | 19 / 42 |
| 4 | C | 15.74 ms | 1.00x | 1.12x | 42 / 42 | 22 / 42 |
| 5 | Rust | 15.84 ms | 1.01x | 1.13x | 42 / 42 | 17 / 42 |
| 6 | C++ | 17.29 ms | 1.10x | 1.23x | 42 / 42 | 2 / 42 |
| 7 | Go | 20.13 ms | 1.28x | 1.44x | 42 / 42 | 0 / 42 |
| 8 | PHP | 190.66 ms | 12.11x | 13.60x | 42 / 42 | 0 / 42 |
| 9 | Python | 972.48 ms | 61.78x | 69.36x | 42 / 42 | 0 / 42 |

```
Salam-l      ███████████████████████████████                 14.02 ms  0.89x C
Salam-t      ████████████████████████████████                14.37 ms  0.91x C
Salam-g      ██████████████████████████████████              15.33 ms  0.97x C
C            ███████████████████████████████████             15.74 ms  1.00x C  <- baseline
Rust         ███████████████████████████████████             15.84 ms  1.01x C
C++          ███████████████████████████████████████         17.29 ms  1.10x C
Go           █████████████████████████████████████████████   20.13 ms  1.28x C
PHP          (off scale)                                    190.66 ms  12.11x C
Python       (off scale)                                    972.48 ms  61.78x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 14.02 ms
  (0.89x C's geomean, 0.88x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  26/42); see the per-program table below.
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
| 01_fib_recursive | 4 | 5 | 4 | 7 | 4 | 4 | 4 | 69 | 111 | **C / Rust / Salam (4)** |
| 02_fib_iterative | 21 | 22 | 22 | 22 | 21 | 21 | 21 | 64 | 851 | **C / Salam (21)** |
| 03_primes_count | 5 | 6 | 5 | 6 | 5 | 5 | 5 | 25 | 198 | **C / Rust / Salam (5)** |
| 04_collatz | 5 | 7 | 5 | 8 | 5 | 5 | 5 | 74 | 442 | **C / Rust / Salam (5)** |
| 05_sum_mod | 13 | 14 | 14 | 15 | 14 | 13 | 14 | 36 | 528 | **C / Salam-g (13)** |
| 06_gcd_sum | 17 | 18 | 16 | 18 | 16 | 17 | 16 | 60 | 175 | **Rust / Salam (16)** |
| 07_pow_mod | 3 | 4 | 3 | 5 | 3 | 3 | 3 | 27 | 81 | **C / Rust / Salam (3)** |
| 08_digit_sum | 4 | 5 | 5 | 5 | 5 | 4 | 4 | 81 | 449 | **C / Salam (4)** |
| 09_perfect_numbers | 10 | 11 | 9 | 11 | 9 | 10 | 9 | 45 | 495 | **Rust / Salam (9)** |
| 10_pi_leibniz | 6 | 7 | 6 | 7 | 6 | 6 | 6 | 63 | 743 | **C / Rust / Salam (6)** |
| 11_loop_count | 6 | 7 | 7 | 7 | 7 | 6 | 7 | 17 | 159 | **C / Salam-g (6)** |
| 12_hello_print | 24 | 22 | 4 | 9 | 5 | 5 | 5 | 455 | 122 | Rust (4) |
| 13_array_rw | 25 | 26 | 26 | 27 | 26 | 23 | 26 | 89 | 1256 | **Salam-g (23)** |
| 14_lcg_random | 21 | 22 | 23 | 28 | 23 | 21 | 23 | 83 | 1757 | **C / Salam-g (21)** |
| 15_matrix_mult | 8 | 9 | 7 | 11 | 7 | 8 | 7 | 239 | 1439 | **Rust / Salam (7)** |
| 16_quicksort | 26 | 29 | 26 | 28 | 26 | 28 | 27 | 190 | 717 | **C / Rust / Salam-l (26)** |
| 17_n_queens | 42 | 43 | 42 | 50 | 45 | 48 | 44 | 567 | 801 | C / Rust (42) |
| 18_sieve_eratosthenes | 7 | 7 | 7 | 8 | 7 | 10 | 7 | 121 | 1123 | **C / C++ / Rust / Salam (7)** |
| 19_mandelbrot | 12 | 13 | 12 | 14 | 12 | 12 | 12 | 155 | 1628 | **C / Rust / Salam (12)** |
| 20_monte_carlo_pi | 31 | 32 | 16 | 38 | 16 | 34 | 12 | 392 | 4811 | **Salam-t (12)** |
| 21_coin_change_dp | 3 | 4 | 4 | 4 | 4 | 4 | 4 | 20 | 153 | C (3) |
| 22_knapsack_01 | 6 | 7 | 5 | 9 | 6 | 6 | 6 | 135 | 1446 | Rust (5) |
| 23_caesar_cipher | 17 | 18 | 14 | 24 | 14 | 19 | 13 | 108 | 1468 | **Salam-t (13)** |
| 24_merge_sort | 32 | 31 | 33 | 39 | 32 | 36 | 37 | 322 | 1085 | C++ (31) |
| 25_palindrome_count | 14 | 16 | 15 | 19 | 14 | 14 | 15 | 373 | 2143 | **C / Salam (14)** |
| 26_ackermann | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 12 | 28 | **C / Rust / Salam (2)** |
| 27_edit_distance | 8 | 9 | 9 | 11 | 7 | 8 | 7 | 331 | 3118 | **Salam (7)** |
| 28_prime_factorization | 1117 | 1118 | 970 | 1105 | 968 | 1118 | 971 | 5809 | 69355 | **Salam-l (968)** |
| 29_dot_product | 40 | 41 | 43 | 43 | 43 | 40 | 41 | 279 | 3729 | **C / Salam-g (40)** |
| 30_heap_sort | 36 | 37 | 39 | 47 | 35 | 36 | 37 | 647 | 1468 | **Salam-l (35)** |
| 31_binary_search_stress | 955 | 930 | 1126 | 885 | 851 | 979 | 976 | 6229 | 32501 | **Salam-l (851)** |
| 32_longest_increasing_subsequence | 16 | 16 | 12 | 19 | 12 | 17 | 13 | 313 | 2369 | **Rust / Salam-l (12)** |
| 33_subset_sum_reachability | 20 | 21 | 21 | 26 | 14 | 21 | 16 | 377 | 3631 | **Salam-l (14)** |
| 34_counting_sort | 42 | 42 | 32 | 46 | 32 | 43 | 32 | 315 | 3289 | **Rust / Salam (32)** |
| 35_run_length_stats | 9 | 10 | 8 | 10 | 7 | 9 | 8 | 151 | 1047 | **Salam-l (7)** |
| 36_array_rotation_reversal | 23 | 26 | 25 | 25 | 23 | 23 | 24 | 239 | 1279 | **C / Salam (23)** |
| 37_sqrt_decomposition | 8 | 10 | 9 | 20 | 8 | 9 | 8 | 362 | 1639 | **C / Salam (8)** |
| 38_game_of_life | 12 | 13 | 38 | 49 | 10 | 11 | 10 | 1382 | 10641 | **Salam (10)** |
| 39_trapezoidal_integration | 15 | 16 | 16 | 18 | 16 | 15 | 15 | 503 | 3832 | **C / Salam (15)** |
| 40_matrix_stdlib_matmul | 54 | 55 | 93 | 67 | 16 | 15 | 20 | 1654 | 159 | **Salam-g (15)** |
| 41_matrix_stdlib_smallops | 6 | 7 | 8 | 24 | 53 | 64 | 68 | 593 | 1888 | C (6) |
| 42_tensor_stdlib_matmul | 54 | 56 | 93 | 66 | 9 | 9 | 9 | 1662 | 155 | **Salam (9)** |

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
| 3 | 33_subset_sum_reachability | 14 | C (20) | 0.70x |
| 4 | 38_game_of_life | 10 | C (12) | 0.83x |
| 5 | 27_edit_distance | 7 | C (8) | 0.88x |
| 6 | 35_run_length_stats | 7 | Rust (8) | 0.88x |
| 7 | 31_binary_search_stress | 851 | Go (885) | 0.96x |
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
| 14 | 34_counting_sort | 32 | Rust (32) | 1.00x |
| 15 | 16_quicksort | 26 | C (26) | 1.00x |
| 16 | 36_array_rotation_reversal | 23 | C (23) | 1.00x |
| 17 | 02_fib_iterative | 21 | C (21) | 1.00x |
| 18 | 06_gcd_sum | 16 | Rust (16) | 1.00x |
| 19 | 20_monte_carlo_pi | 16 | Rust (16) | 1.00x |
| 20 | 25_palindrome_count | 14 | C (14) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (12):** 13_array_rw, 20_monte_carlo_pi, 23_caesar_cipher, 27_edit_distance, 28_prime_factorization, 30_heap_sort, 31_binary_search_stress, 33_subset_sum_reachability, 35_run_length_stats, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (24):** 01_fib_recursive, 02_fib_iterative, 03_primes_count, 04_collatz, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 14_lcg_random, 15_matrix_mult, 16_quicksort, 18_sieve_eratosthenes, 19_mandelbrot, 25_palindrome_count, 26_ackermann, 29_dot_product, 32_longest_increasing_subsequence, 34_counting_sort, 36_array_rotation_reversal, 37_sqrt_decomposition, 39_trapezoidal_integration.
- **Trailing the fastest (6):** 12_hello_print (+1 vs Rust); 17_n_queens (+2 vs C/Rust); 21_coin_change_dp (+1 vs C); 22_knapsack_01 (+1 vs Rust); 24_merge_sort (+1 vs C++); 41_matrix_stdlib_smallops (+47 vs C).

Salam matches or beats the fastest other compiled language on 36 of 42 programs.

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
