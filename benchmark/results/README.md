# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured three ways: the LLVM backend at
`-O3`, the C backend compiled by `gcc -O3`, and the default `tcc` toolchain
(no optimizer) as the out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-23T01:27:27Z (took 37 min)
- Commit: `2fd9327`

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
- **go**: go version go1.26.9 linux/amd64
- **tcc**: tcc version 0.9.28rc (x86_64 Linux)
- **clang**: Alpine clang version 22.1.3
- **python**: Python 3.14.8
- **numpy**: 2.4.6
- **php**: PHP 8.5.12 (cli) (built: Oct 21 2026 16:29:26) (NTS)
- **salam**: 0.5.7

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
| 1 | **Salam-l** | 12.10 ms | 0.90x | 1.00x | 42 / 42 | 26 / 42 |
| 2 | **Salam-t** | 12.56 ms | 0.93x | 1.04x | 42 / 42 | 17 / 42 |
| 3 | **Salam-g** | 13.20 ms | 0.98x | 1.09x | 42 / 42 | 19 / 42 |
| 4 | C | 13.46 ms | 1.00x | 1.11x | 42 / 42 | 19 / 42 |
| 5 | Rust | 13.65 ms | 1.01x | 1.13x | 42 / 42 | 16 / 42 |
| 6 | C++ | 15.07 ms | 1.12x | 1.25x | 42 / 42 | 4 / 42 |
| 7 | Go | 17.20 ms | 1.28x | 1.42x | 42 / 42 | 0 / 42 |
| 8 | PHP | 158.00 ms | 11.74x | 13.05x | 42 / 42 | 0 / 42 |
| 9 | Python | 748.22 ms | 55.60x | 61.82x | 42 / 42 | 0 / 42 |

```
Salam-l      ████████████████████████████████                12.10 ms  0.90x C
Salam-t      █████████████████████████████████               12.56 ms  0.93x C
Salam-g      ███████████████████████████████████             13.20 ms  0.98x C
C            ███████████████████████████████████             13.46 ms  1.00x C  <- baseline
Rust         ████████████████████████████████████            13.65 ms  1.01x C
C++          ███████████████████████████████████████         15.07 ms  1.12x C
Go           █████████████████████████████████████████████   17.20 ms  1.28x C
PHP          (off scale)                                    158.00 ms  11.74x C
Python       (off scale)                                    748.22 ms  55.60x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 12.10 ms
  (0.90x C's geomean, 0.89x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  26/42); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.04x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 12-56x slower** than the compiled languages on
  this suite and are included for scale only.
- "Fastest / tied-fastest on" counts ties on every side (e.g. a 4-way tie
  adds 1 to all 4 languages), so the column does not sum to 42.

## Per-program results (minimum ms)

| Program | C | C++ | Rust | Go | Salam-l | Salam-g | Salam-t | PHP | Python | Fastest |
|---|---|---|---|---|---|---|---|---|---|---|
| 01_fib_recursive | 3 | 4 | 4 | 6 | 4 | 3 | 4 | 55 | 84 | **C / Salam-g (3)** |
| 02_fib_iterative | 18 | 20 | 19 | 20 | 19 | 18 | 19 | 57 | 626 | **C / Salam-g (18)** |
| 03_primes_count | 5 | 5 | 4 | 5 | 4 | 4 | 4 | 21 | 153 | **Rust / Salam (4)** |
| 04_collatz | 5 | 6 | 4 | 7 | 4 | 5 | 4 | 57 | 335 | **Rust / Salam (4)** |
| 05_sum_mod | 12 | 13 | 12 | 13 | 12 | 12 | 12 | 32 | 402 | **C / Rust / Salam (12)** |
| 06_gcd_sum | 15 | 16 | 14 | 16 | 14 | 15 | 14 | 48 | 136 | **Rust / Salam (14)** |
| 07_pow_mod | 3 | 3 | 3 | 4 | 3 | 3 | 3 | 20 | 62 | **C / C++ / Rust / Salam (3)** |
| 08_digit_sum | 4 | 4 | 4 | 5 | 4 | 4 | 4 | 63 | 351 | **C / C++ / Rust / Salam (4)** |
| 09_perfect_numbers | 9 | 10 | 8 | 10 | 7 | 9 | 8 | 39 | 382 | **Salam-l (7)** |
| 10_pi_leibniz | 5 | 6 | 6 | 6 | 5 | 5 | 5 | 47 | 544 | **C / Salam (5)** |
| 11_loop_count | 5 | 6 | 6 | 6 | 6 | 5 | 6 | 14 | 115 | **C / Salam-g (5)** |
| 12_hello_print | 18 | 18 | 3 | 8 | 4 | 4 | 4 | 610 | 93 | Rust (3) |
| 13_array_rw | 22 | 23 | 22 | 23 | 22 | 20 | 22 | 77 | 944 | **Salam-g (20)** |
| 14_lcg_random | 18 | 19 | 20 | 24 | 20 | 18 | 20 | 70 | 1309 | **C / Salam-g (18)** |
| 15_matrix_mult | 6 | 7 | 6 | 10 | 6 | 7 | 6 | 175 | 1096 | **C / Rust / Salam (6)** |
| 16_quicksort | 24 | 24 | 23 | 25 | 23 | 25 | 25 | 161 | 570 | **Rust / Salam-l (23)** |
| 17_n_queens | 36 | 37 | 38 | 42 | 41 | 41 | 40 | 440 | 613 | C (36) |
| 18_sieve_eratosthenes | 5 | 7 | 6 | 9 | 6 | 8 | 6 | 106 | 894 | C (5) |
| 19_mandelbrot | 11 | 12 | 11 | 12 | 10 | 11 | 11 | 126 | 1245 | **Salam-l (10)** |
| 20_monte_carlo_pi | 27 | 28 | 15 | 32 | 14 | 30 | 11 | 345 | 3443 | **Salam-t (11)** |
| 21_coin_change_dp | 3 | 4 | 3 | 4 | 3 | 3 | 3 | 18 | 114 | **C / Rust / Salam (3)** |
| 22_knapsack_01 | 5 | 6 | 5 | 6 | 4 | 6 | 6 | 113 | 1366 | **Salam-l (4)** |
| 23_caesar_cipher | 17 | 18 | 12 | 21 | 12 | 17 | 12 | 94 | 1100 | **Rust / Salam (12)** |
| 24_merge_sort | 26 | 26 | 29 | 31 | 28 | 31 | 32 | 269 | 850 | C / C++ (26) |
| 25_palindrome_count | 12 | 13 | 13 | 16 | 13 | 12 | 13 | 278 | 1668 | **C / Salam-g (12)** |
| 26_ackermann | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 11 | 23 | **C / Rust / Salam (2)** |
| 27_edit_distance | 7 | 8 | 8 | 10 | 6 | 7 | 7 | 250 | 2454 | **Salam-l (6)** |
| 28_prime_factorization | 979 | 980 | 849 | 970 | 847 | 979 | 849 | 4976 | 53937 | **Salam-l (847)** |
| 29_dot_product | 35 | 37 | 38 | 38 | 37 | 35 | 37 | 249 | 2938 | **C / Salam-g (35)** |
| 30_heap_sort | 31 | 31 | 34 | 40 | 31 | 31 | 32 | 493 | 1179 | **C / C++ / Salam (31)** |
| 31_binary_search_stress | 813 | 876 | 1187 | 881 | 787 | 892 | 826 | 5522 | 27505 | **Salam-l (787)** |
| 32_longest_increasing_subsequence | 13 | 15 | 10 | 11 | 10 | 14 | 11 | 260 | 2362 | **Rust / Salam-l (10)** |
| 33_subset_sum_reachability | 17 | 19 | 19 | 18 | 13 | 18 | 17 | 327 | 2427 | **Salam-l (13)** |
| 34_counting_sort | 37 | 39 | 27 | 39 | 27 | 38 | 27 | 223 | 2431 | **Rust / Salam (27)** |
| 35_run_length_stats | 8 | 9 | 7 | 9 | 7 | 7 | 7 | 113 | 839 | **Rust / Salam (7)** |
| 36_array_rotation_reversal | 20 | 21 | 21 | 22 | 20 | 20 | 20 | 200 | 918 | **C / Salam (20)** |
| 37_sqrt_decomposition | 8 | 10 | 8 | 17 | 7 | 8 | 7 | 297 | 1203 | **Salam (7)** |
| 38_game_of_life | 10 | 11 | 30 | 44 | 9 | 9 | 8 | 1147 | 8008 | **Salam-t (8)** |
| 39_trapezoidal_integration | 13 | 14 | 12 | 15 | 12 | 13 | 13 | 397 | 2786 | **Rust / Salam-l (12)** |
| 40_matrix_stdlib_matmul | 45 | 47 | 81 | 58 | 14 | 13 | 18 | 1330 | 121 | **Salam-g (13)** |
| 41_matrix_stdlib_smallops | 5 | 6 | 6 | 19 | 39 | 49 | 52 | 473 | 1312 | C (5) |
| 42_tensor_stdlib_matmul | 45 | 46 | 80 | 58 | 8 | 7 | 8 | 1329 | 122 | **Salam-g (7)** |

Bold marks rows where a Salam backend is the fastest or tied for fastest.

### Top 10 fastest programs in Salam-l

Where the LLVM backend (`-O3`) does best **relative to the fastest non-Salam
entry on the same program**: sorted by `vs best other` (Salam-l's minimum
divided by that language's), lowest ratio first. Under 1.00x means Salam-l
wins the row outright. Absolute ms are shown but are not the sort key, since
they mostly track how big each program's workload is.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 42_tensor_stdlib_matmul | 8 | C (45) | 0.18x |
| 2 | 40_matrix_stdlib_matmul | 14 | C (45) | 0.31x |
| 3 | 33_subset_sum_reachability | 13 | C (17) | 0.76x |
| 4 | 22_knapsack_01 | 4 | C (5) | 0.80x |
| 5 | 27_edit_distance | 6 | C (7) | 0.86x |
| 6 | 09_perfect_numbers | 7 | Rust (8) | 0.88x |
| 7 | 37_sqrt_decomposition | 7 | C (8) | 0.88x |
| 8 | 38_game_of_life | 9 | C (10) | 0.90x |
| 9 | 19_mandelbrot | 10 | C (11) | 0.91x |
| 10 | 20_monte_carlo_pi | 14 | Rust (15) | 0.93x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 39 | C (5) | 7.80x |
| 2 | 01_fib_recursive | 4 | C (3) | 1.33x |
| 3 | 12_hello_print | 4 | Rust (3) | 1.33x |
| 4 | 11_loop_count | 6 | C (5) | 1.20x |
| 5 | 18_sieve_eratosthenes | 6 | C (5) | 1.20x |
| 6 | 17_n_queens | 41 | C (36) | 1.14x |
| 7 | 14_lcg_random | 20 | C (18) | 1.11x |
| 8 | 25_palindrome_count | 13 | C (12) | 1.08x |
| 9 | 24_merge_sort | 28 | C (26) | 1.08x |
| 10 | 29_dot_product | 37 | C (35) | 1.06x |
| 11 | 02_fib_iterative | 19 | C (18) | 1.06x |
| 12 | 30_heap_sort | 31 | C (31) | 1.00x |
| 13 | 34_counting_sort | 27 | Rust (27) | 1.00x |
| 14 | 16_quicksort | 23 | Rust (23) | 1.00x |
| 15 | 13_array_rw | 22 | C (22) | 1.00x |
| 16 | 36_array_rotation_reversal | 20 | C (20) | 1.00x |
| 17 | 06_gcd_sum | 14 | Rust (14) | 1.00x |
| 18 | 39_trapezoidal_integration | 12 | Rust (12) | 1.00x |
| 19 | 05_sum_mod | 12 | C (12) | 1.00x |
| 20 | 23_caesar_cipher | 12 | Rust (12) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (13):** 09_perfect_numbers, 13_array_rw, 19_mandelbrot, 20_monte_carlo_pi, 22_knapsack_01, 27_edit_distance, 28_prime_factorization, 31_binary_search_stress, 33_subset_sum_reachability, 37_sqrt_decomposition, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (24):** 01_fib_recursive, 02_fib_iterative, 03_primes_count, 04_collatz, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 10_pi_leibniz, 11_loop_count, 14_lcg_random, 15_matrix_mult, 16_quicksort, 21_coin_change_dp, 23_caesar_cipher, 25_palindrome_count, 26_ackermann, 29_dot_product, 30_heap_sort, 32_longest_increasing_subsequence, 34_counting_sort, 35_run_length_stats, 36_array_rotation_reversal, 39_trapezoidal_integration.
- **Trailing the fastest (5):** 12_hello_print (+1 vs Rust); 17_n_queens (+4 vs C); 18_sieve_eratosthenes (+1 vs C); 24_merge_sort (+2 vs C/C++); 41_matrix_stdlib_smallops (+34 vs C).

Salam matches or beats the fastest other compiled language on 37 of 42 programs.

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
