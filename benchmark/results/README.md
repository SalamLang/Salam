# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured four ways: the LLVM backend at
`-O3`, the native x86-64 backend (`--backend=native`), the C backend compiled
by `gcc -O3`, and the default `tcc` toolchain (no optimizer) as the
out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-31T01:42:17Z (took 49 min)
- Commit: `3fba6e8`

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
- **salam**: 0.6.0

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
| 1 | **Salam-n** | 14.00 ms | 0.85x | 1.00x | 42 / 42 | 30 / 42 |
| 2 | **Salam-l** | 14.80 ms | 0.90x | 1.06x | 42 / 42 | 18 / 42 |
| 3 | **Salam-t** | 15.21 ms | 0.92x | 1.09x | 42 / 42 | 15 / 42 |
| 4 | **Salam-g** | 16.02 ms | 0.97x | 1.14x | 42 / 42 | 16 / 42 |
| 5 | C | 16.50 ms | 1.00x | 1.18x | 42 / 42 | 16 / 42 |
| 6 | Rust | 16.69 ms | 1.01x | 1.19x | 42 / 42 | 14 / 42 |
| 7 | C++ | 18.11 ms | 1.10x | 1.29x | 42 / 42 | 2 / 42 |
| 8 | Go | 21.65 ms | 1.31x | 1.55x | 42 / 42 | 0 / 42 |
| 9 | PHP | 195.30 ms | 11.83x | 13.95x | 42 / 42 | 0 / 42 |
| 10 | Python | 980.82 ms | 59.43x | 70.06x | 42 / 42 | 0 / 42 |

```
Salam-n      █████████████████████████████                   14.00 ms  0.85x C
Salam-l      ███████████████████████████████                 14.80 ms  0.90x C
Salam-t      ████████████████████████████████                15.21 ms  0.92x C
Salam-g      █████████████████████████████████               16.02 ms  0.97x C
C            ██████████████████████████████████              16.50 ms  1.00x C  <- baseline
Rust         ███████████████████████████████████             16.69 ms  1.01x C
C++          ██████████████████████████████████████          18.11 ms  1.10x C
Go           █████████████████████████████████████████████   21.65 ms  1.31x C
PHP          (off scale)                                    195.30 ms  11.83x C
Python       (off scale)                                    980.82 ms  59.43x C
```

Takeaways:

- **Fastest Salam backend:** Salam-n, geomean 14.00 ms
  (0.85x C's geomean, 0.84x Rust's).
- **Salam-n is fastest or tied-fastest on the most programs** (
  30/42); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.09x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 12-59x slower** than the compiled languages on
  this suite and are included for scale only.
- "Fastest / tied-fastest on" counts ties on every side (e.g. a 4-way tie
  adds 1 to all 4 languages), so the column does not sum to 42.

## Per-program results (minimum ms)

| Program | C | C++ | Rust | Go | Salam-l | Salam-n | Salam-g | Salam-t | PHP | Python | Fastest |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 01_fib_recursive | 4 | 5 | 4 | 8 | 5 | 4 | 4 | 5 | 68 | 113 | **C / Rust / Salam (4)** |
| 02_fib_iterative | 21 | 22 | 22 | 23 | 21 | 16 | 21 | 21 | 67 | 829 | **Salam-n (16)** |
| 03_primes_count | 5 | 6 | 5 | 6 | 5 | 5 | 5 | 5 | 29 | 200 | **C / Rust / Salam (5)** |
| 04_collatz | 6 | 7 | 5 | 8 | 5 | 5 | 5 | 5 | 77 | 441 | **Rust / Salam (5)** |
| 05_sum_mod | 13 | 15 | 15 | 15 | 14 | 11 | 14 | 14 | 38 | 528 | **Salam-n (11)** |
| 06_gcd_sum | 17 | 19 | 16 | 18 | 16 | 16 | 17 | 16 | 65 | 173 | **Rust / Salam (16)** |
| 07_pow_mod | 3 | 4 | 4 | 5 | 3 | 3 | 3 | 3 | 26 | 80 | **C / Salam (3)** |
| 08_digit_sum | 4 | 5 | 5 | 6 | 5 | 4 | 4 | 5 | 80 | 453 | **C / Salam (4)** |
| 09_perfect_numbers | 10 | 11 | 9 | 12 | 9 | 9 | 10 | 9 | 47 | 498 | **Rust / Salam (9)** |
| 10_pi_leibniz | 6 | 8 | 7 | 8 | 6 | 6 | 6 | 6 | 59 | 743 | **C / Salam (6)** |
| 11_loop_count | 6 | 7 | 7 | 7 | 7 | 7 | 6 | 7 | 17 | 156 | **C / Salam-g (6)** |
| 12_hello_print | 22 | 22 | 4 | 9 | 3 | 3 | 3 | 3 | 449 | 121 | **Salam (3)** |
| 13_array_rw | 28 | 29 | 29 | 31 | 29 | 29 | 26 | 29 | 97 | 1250 | **Salam-g (26)** |
| 14_lcg_random | 21 | 22 | 23 | 27 | 23 | 16 | 21 | 23 | 85 | 1685 | **Salam-n (16)** |
| 15_matrix_mult | 8 | 9 | 7 | 14 | 7 | 7 | 8 | 7 | 208 | 1439 | **Rust / Salam (7)** |
| 16_quicksort | 27 | 28 | 26 | 30 | 27 | 27 | 29 | 28 | 191 | 686 | Rust (26) |
| 17_n_queens | 42 | 43 | 42 | 46 | 45 | 46 | 49 | 44 | 568 | 810 | C / Rust (42) |
| 18_sieve_eratosthenes | 8 | 9 | 8 | 12 | 8 | 7 | 11 | 8 | 140 | 1165 | **Salam-n (7)** |
| 19_mandelbrot | 12 | 13 | 12 | 14 | 12 | 12 | 12 | 12 | 157 | 1667 | **C / Rust / Salam (12)** |
| 20_monte_carlo_pi | 31 | 32 | 16 | 38 | 16 | 15 | 34 | 12 | 379 | 4699 | **Salam-t (12)** |
| 21_coin_change_dp | 3 | 4 | 4 | 5 | 4 | 4 | 4 | 4 | 21 | 153 | C (3) |
| 22_knapsack_01 | 6 | 7 | 5 | 8 | 6 | 5 | 6 | 6 | 134 | 1743 | **Rust / Salam-n (5)** |
| 23_caesar_cipher | 17 | 18 | 14 | 24 | 13 | 11 | 20 | 14 | 107 | 1432 | **Salam-n (11)** |
| 24_merge_sort | 33 | 33 | 34 | 38 | 34 | 35 | 38 | 37 | 330 | 1060 | C / C++ (33) |
| 25_palindrome_count | 15 | 16 | 15 | 18 | 15 | 15 | 15 | 15 | 357 | 2180 | **C / Rust / Salam (15)** |
| 26_ackermann | 2 | 3 | 2 | 4 | 2 | 2 | 2 | 2 | 13 | 29 | **C / Rust / Salam (2)** |
| 27_edit_distance | 8 | 9 | 10 | 12 | 7 | 8 | 8 | 8 | 331 | 3170 | **Salam-l (7)** |
| 28_prime_factorization | 1118 | 1120 | 970 | 1107 | 972 | 955 | 1120 | 969 | 7358 | 69550 | **Salam-n (955)** |
| 29_dot_product | 40 | 41 | 43 | 43 | 43 | 34 | 40 | 43 | 280 | 3721 | **Salam-n (34)** |
| 30_heap_sort | 37 | 37 | 41 | 49 | 37 | 37 | 38 | 41 | 641 | 1464 | **C / C++ / Salam (37)** |
| 31_binary_search_stress | 1094 | 1080 | 1400 | 1080 | 961 | 949 | 1076 | 1017 | 6688 | 33388 | **Salam-n (949)** |
| 32_longest_increasing_subsequence | 15 | 16 | 12 | 14 | 12 | 12 | 16 | 12 | 315 | 3148 | **Rust / Salam (12)** |
| 33_subset_sum_reachability | 20 | 21 | 22 | 20 | 14 | 14 | 21 | 21 | 392 | 3294 | **Salam (14)** |
| 34_counting_sort | 42 | 42 | 32 | 45 | 33 | 27 | 43 | 32 | 326 | 3189 | **Salam-n (27)** |
| 35_run_length_stats | 9 | 10 | 8 | 10 | 8 | 8 | 10 | 8 | 152 | 1055 | **Rust / Salam (8)** |
| 36_array_rotation_reversal | 36 | 39 | 39 | 40 | 36 | 37 | 36 | 37 | 264 | 1314 | **C / Salam (36)** |
| 37_sqrt_decomposition | 18 | 18 | 19 | 38 | 18 | 17 | 19 | 19 | 365 | 1607 | **Salam-n (17)** |
| 38_game_of_life | 12 | 13 | 38 | 59 | 11 | 7 | 11 | 10 | 1416 | 10756 | **Salam-n (7)** |
| 39_trapezoidal_integration | 15 | 16 | 16 | 18 | 16 | 15 | 15 | 15 | 520 | 3772 | **C / Salam (15)** |
| 40_matrix_stdlib_matmul | 54 | 55 | 94 | 92 | 16 | 16 | 15 | 20 | 1603 | 159 | **Salam-g (15)** |
| 41_matrix_stdlib_smallops | 6 | 7 | 8 | 23 | 52 | 92 | 63 | 70 | 606 | 1941 | C (6) |
| 42_tensor_stdlib_matmul | 54 | 56 | 94 | 91 | 9 | 10 | 9 | 10 | 1603 | 157 | **Salam (9)** |

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
| 4 | 12_hello_print | 3 | Rust (4) | 0.75x |
| 5 | 27_edit_distance | 7 | C (8) | 0.88x |
| 6 | 31_binary_search_stress | 961 | C++ (1080) | 0.89x |
| 7 | 38_game_of_life | 11 | C (12) | 0.92x |
| 8 | 23_caesar_cipher | 13 | Rust (14) | 0.93x |
| 9 | 26_ackermann | 2 | C (2) | 1.00x |
| 10 | 07_pow_mod | 3 | C (3) | 1.00x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 52 | C (6) | 8.67x |
| 2 | 21_coin_change_dp | 4 | C (3) | 1.33x |
| 3 | 01_fib_recursive | 5 | C (4) | 1.25x |
| 4 | 08_digit_sum | 5 | C (4) | 1.25x |
| 5 | 22_knapsack_01 | 6 | Rust (5) | 1.20x |
| 6 | 11_loop_count | 7 | C (6) | 1.17x |
| 7 | 14_lcg_random | 23 | C (21) | 1.10x |
| 8 | 05_sum_mod | 14 | C (13) | 1.08x |
| 9 | 29_dot_product | 43 | C (40) | 1.08x |
| 10 | 17_n_queens | 45 | C (42) | 1.07x |
| 11 | 39_trapezoidal_integration | 16 | C (15) | 1.07x |
| 12 | 16_quicksort | 27 | Rust (26) | 1.04x |
| 13 | 13_array_rw | 29 | C (28) | 1.04x |
| 14 | 34_counting_sort | 33 | Rust (32) | 1.03x |
| 15 | 24_merge_sort | 34 | C (33) | 1.03x |
| 16 | 28_prime_factorization | 972 | Rust (970) | 1.00x |
| 17 | 30_heap_sort | 37 | C (37) | 1.00x |
| 18 | 36_array_rotation_reversal | 36 | C (36) | 1.00x |
| 19 | 02_fib_iterative | 21 | C (21) | 1.00x |
| 20 | 37_sqrt_decomposition | 18 | C (18) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (18):** 02_fib_iterative, 05_sum_mod, 12_hello_print, 13_array_rw, 14_lcg_random, 18_sieve_eratosthenes, 20_monte_carlo_pi, 23_caesar_cipher, 27_edit_distance, 28_prime_factorization, 29_dot_product, 31_binary_search_stress, 33_subset_sum_reachability, 34_counting_sort, 37_sqrt_decomposition, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (19):** 01_fib_recursive, 03_primes_count, 04_collatz, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 15_matrix_mult, 19_mandelbrot, 22_knapsack_01, 25_palindrome_count, 26_ackermann, 30_heap_sort, 32_longest_increasing_subsequence, 35_run_length_stats, 36_array_rotation_reversal, 39_trapezoidal_integration.
- **Trailing the fastest (5):** 16_quicksort (+1 vs Rust); 17_n_queens (+2 vs C/Rust); 21_coin_change_dp (+1 vs C); 24_merge_sort (+1 vs C/C++); 41_matrix_stdlib_smallops (+46 vs C).

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
