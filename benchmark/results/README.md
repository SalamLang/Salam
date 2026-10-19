# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured three ways: the LLVM backend at
`-O3`, the C backend compiled by `gcc -O3`, and the default `tcc` toolchain
(no optimizer) as the out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-19T07:59:42Z (took 48 min)
- Commit: `a9ed300`

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
- **php**: PHP 8.5.11 (cli) (built: Sep 25 2026 15:57:29) (NTS)
- **salam**: 0.5.6

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
| 1 | **Salam-l** | 15.82 ms | 0.90x | 1.00x | 42 / 42 | 22 / 42 |
| 2 | **Salam-t** | 16.34 ms | 0.93x | 1.03x | 42 / 42 | 17 / 42 |
| 3 | **Salam-g** | 17.20 ms | 0.98x | 1.09x | 42 / 42 | 17 / 42 |
| 4 | Rust | 17.54 ms | 1.00x | 1.11x | 42 / 42 | 17 / 42 |
| 5 | C | 17.59 ms | 1.00x | 1.11x | 42 / 42 | 21 / 42 |
| 6 | C++ | 19.68 ms | 1.12x | 1.24x | 42 / 42 | 2 / 42 |
| 7 | Go | 22.28 ms | 1.27x | 1.41x | 42 / 42 | 0 / 42 |
| 8 | PHP | 204.96 ms | 11.65x | 12.96x | 42 / 42 | 0 / 42 |
| 9 | Python | 962.68 ms | 54.72x | 60.85x | 42 / 42 | 0 / 42 |

```
Salam-l      ████████████████████████████████                15.82 ms  0.90x C
Salam-t      █████████████████████████████████               16.34 ms  0.93x C
Salam-g      ███████████████████████████████████             17.20 ms  0.98x C
Rust         ███████████████████████████████████             17.54 ms  1.00x C
C            ████████████████████████████████████            17.59 ms  1.00x C  <- baseline
C++          ████████████████████████████████████████        19.68 ms  1.12x C
Go           █████████████████████████████████████████████   22.28 ms  1.27x C
PHP          (off scale)                                    204.96 ms  11.65x C
Python       (off scale)                                    962.68 ms  54.72x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 15.82 ms
  (0.90x C's geomean, 0.90x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  22/42); see the per-program table below.
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
| 01_fib_recursive | 4 | 5 | 5 | 8 | 5 | 4 | 5 | 71 | 110 | **C / Salam-g (4)** |
| 02_fib_iterative | 24 | 25 | 24 | 25 | 24 | 24 | 24 | 73 | 809 | **C / Rust / Salam (24)** |
| 03_primes_count | 6 | 7 | 6 | 7 | 5 | 6 | 6 | 27 | 199 | **Salam-l (5)** |
| 04_collatz | 6 | 8 | 6 | 9 | 6 | 6 | 6 | 73 | 437 | **C / Rust / Salam (6)** |
| 05_sum_mod | 15 | 17 | 16 | 16 | 16 | 15 | 15 | 42 | 513 | **C / Salam (15)** |
| 06_gcd_sum | 20 | 21 | 18 | 21 | 18 | 19 | 18 | 62 | 176 | **Rust / Salam (18)** |
| 07_pow_mod | 4 | 5 | 4 | 5 | 4 | 3 | 4 | 26 | 81 | **Salam-g (3)** |
| 08_digit_sum | 5 | 6 | 5 | 6 | 5 | 5 | 5 | 78 | 449 | **C / Rust / Salam (5)** |
| 09_perfect_numbers | 12 | 12 | 10 | 13 | 10 | 12 | 10 | 51 | 497 | **Rust / Salam (10)** |
| 10_pi_leibniz | 7 | 9 | 7 | 9 | 7 | 7 | 8 | 61 | 702 | **C / Rust / Salam (7)** |
| 11_loop_count | 7 | 9 | 7 | 9 | 8 | 7 | 8 | 19 | 151 | **C / Rust / Salam-g (7)** |
| 12_hello_print | 25 | 24 | 4 | 10 | 5 | 5 | 5 | 788 | 120 | Rust (4) |
| 13_array_rw | 28 | 30 | 29 | 30 | 29 | 26 | 29 | 100 | 1225 | **Salam-g (26)** |
| 14_lcg_random | 24 | 25 | 26 | 31 | 26 | 24 | 26 | 91 | 1697 | **C / Salam-g (24)** |
| 15_matrix_mult | 8 | 9 | 8 | 12 | 8 | 9 | 8 | 227 | 1430 | **C / Rust / Salam (8)** |
| 16_quicksort | 30 | 32 | 30 | 33 | 30 | 32 | 31 | 211 | 738 | **C / Rust / Salam-l (30)** |
| 17_n_queens | 46 | 48 | 49 | 53 | 52 | 54 | 51 | 568 | 791 | C (46) |
| 18_sieve_eratosthenes | 7 | 9 | 8 | 11 | 8 | 10 | 8 | 136 | 1171 | C (7) |
| 19_mandelbrot | 13 | 15 | 14 | 15 | 13 | 14 | 14 | 162 | 1597 | **C / Salam-l (13)** |
| 20_monte_carlo_pi | 35 | 36 | 19 | 41 | 19 | 39 | 14 | 449 | 4458 | **Salam-t (14)** |
| 21_coin_change_dp | 4 | 5 | 4 | 5 | 4 | 4 | 4 | 24 | 150 | **C / Rust / Salam (4)** |
| 22_knapsack_01 | 6 | 8 | 6 | 8 | 7 | 7 | 7 | 145 | 1752 | C / Rust (6) |
| 23_caesar_cipher | 22 | 23 | 16 | 27 | 16 | 22 | 15 | 130 | 1449 | **Salam-t (15)** |
| 24_merge_sort | 34 | 34 | 38 | 40 | 37 | 41 | 42 | 345 | 1047 | C / C++ (34) |
| 25_palindrome_count | 16 | 17 | 16 | 21 | 17 | 16 | 17 | 356 | 2145 | **C / Rust / Salam-g (16)** |
| 26_ackermann | 2 | 3 | 3 | 4 | 2 | 2 | 2 | 14 | 31 | **C / Salam (2)** |
| 27_edit_distance | 9 | 10 | 10 | 13 | 8 | 9 | 9 | 325 | 3145 | **Salam-l (8)** |
| 28_prime_factorization | 1263 | 1264 | 1094 | 1252 | 1092 | 1262 | 1096 | 6421 | 69608 | **Salam-l (1092)** |
| 29_dot_product | 47 | 48 | 49 | 49 | 48 | 46 | 49 | 319 | 3775 | **Salam-g (46)** |
| 30_heap_sort | 40 | 40 | 43 | 52 | 40 | 42 | 42 | 633 | 1336 | **C / C++ / Salam-l (40)** |
| 31_binary_search_stress | 1121 | 1135 | 989 | 991 | 961 | 1083 | 1047 | 7772 | 32799 | **Salam-l (961)** |
| 32_longest_increasing_subsequence | 18 | 19 | 14 | 15 | 14 | 20 | 14 | 334 | 3044 | **Rust / Salam (14)** |
| 33_subset_sum_reachability | 23 | 25 | 24 | 23 | 16 | 23 | 24 | 417 | 3159 | **Salam-l (16)** |
| 34_counting_sort | 47 | 50 | 35 | 51 | 35 | 47 | 35 | 284 | 3137 | **Rust / Salam (35)** |
| 35_run_length_stats | 10 | 12 | 9 | 11 | 9 | 10 | 9 | 142 | 1084 | **Rust / Salam (9)** |
| 36_array_rotation_reversal | 25 | 28 | 27 | 28 | 25 | 25 | 26 | 261 | 1183 | **C / Salam (25)** |
| 37_sqrt_decomposition | 10 | 12 | 10 | 23 | 9 | 11 | 8 | 381 | 1548 | **Salam-t (8)** |
| 38_game_of_life | 13 | 14 | 38 | 57 | 12 | 12 | 11 | 1450 | 10494 | **Salam-t (11)** |
| 39_trapezoidal_integration | 17 | 18 | 16 | 19 | 16 | 17 | 17 | 509 | 3564 | **Rust / Salam-l (16)** |
| 40_matrix_stdlib_matmul | 58 | 60 | 104 | 75 | 17 | 16 | 22 | 1735 | 156 | **Salam-g (16)** |
| 41_matrix_stdlib_smallops | 6 | 7 | 8 | 24 | 50 | 63 | 69 | 610 | 1688 | C (6) |
| 42_tensor_stdlib_matmul | 58 | 60 | 104 | 74 | 11 | 10 | 10 | 1734 | 155 | **Salam (10)** |

Bold marks rows where a Salam backend is the fastest or tied for fastest.

### Top 10 fastest programs in Salam-l

Where the LLVM backend (`-O3`) does best **relative to the fastest non-Salam
entry on the same program**: sorted by `vs best other` (Salam-l's minimum
divided by that language's), lowest ratio first. Under 1.00x means Salam-l
wins the row outright. Absolute ms are shown but are not the sort key, since
they mostly track how big each program's workload is.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 42_tensor_stdlib_matmul | 11 | C (58) | 0.19x |
| 2 | 40_matrix_stdlib_matmul | 17 | C (58) | 0.29x |
| 3 | 33_subset_sum_reachability | 16 | C (23) | 0.70x |
| 4 | 03_primes_count | 5 | C (6) | 0.83x |
| 5 | 27_edit_distance | 8 | C (9) | 0.89x |
| 6 | 37_sqrt_decomposition | 9 | C (10) | 0.90x |
| 7 | 38_game_of_life | 12 | C (13) | 0.92x |
| 8 | 31_binary_search_stress | 961 | Rust (989) | 0.97x |
| 9 | 28_prime_factorization | 1092 | Rust (1094) | 1.00x |
| 10 | 26_ackermann | 2 | C (2) | 1.00x |

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
| 4 | 22_knapsack_01 | 7 | C (6) | 1.17x |
| 5 | 18_sieve_eratosthenes | 8 | C (7) | 1.14x |
| 6 | 11_loop_count | 8 | C (7) | 1.14x |
| 7 | 17_n_queens | 52 | C (46) | 1.13x |
| 8 | 24_merge_sort | 37 | C (34) | 1.09x |
| 9 | 14_lcg_random | 26 | C (24) | 1.08x |
| 10 | 05_sum_mod | 16 | C (15) | 1.07x |
| 11 | 25_palindrome_count | 17 | C (16) | 1.06x |
| 12 | 13_array_rw | 29 | C (28) | 1.04x |
| 13 | 29_dot_product | 48 | C (47) | 1.02x |
| 14 | 30_heap_sort | 40 | C (40) | 1.00x |
| 15 | 34_counting_sort | 35 | Rust (35) | 1.00x |
| 16 | 16_quicksort | 30 | C (30) | 1.00x |
| 17 | 36_array_rotation_reversal | 25 | C (25) | 1.00x |
| 18 | 02_fib_iterative | 24 | C (24) | 1.00x |
| 19 | 20_monte_carlo_pi | 19 | Rust (19) | 1.00x |
| 20 | 06_gcd_sum | 18 | Rust (18) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (14):** 03_primes_count, 07_pow_mod, 13_array_rw, 20_monte_carlo_pi, 23_caesar_cipher, 27_edit_distance, 28_prime_factorization, 29_dot_product, 31_binary_search_stress, 33_subset_sum_reachability, 37_sqrt_decomposition, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (22):** 01_fib_recursive, 02_fib_iterative, 04_collatz, 05_sum_mod, 06_gcd_sum, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 14_lcg_random, 15_matrix_mult, 16_quicksort, 19_mandelbrot, 21_coin_change_dp, 25_palindrome_count, 26_ackermann, 30_heap_sort, 32_longest_increasing_subsequence, 34_counting_sort, 35_run_length_stats, 36_array_rotation_reversal, 39_trapezoidal_integration.
- **Trailing the fastest (6):** 12_hello_print (+1 vs Rust); 17_n_queens (+5 vs C); 18_sieve_eratosthenes (+1 vs C); 22_knapsack_01 (+1 vs C/Rust); 24_merge_sort (+3 vs C/C++); 41_matrix_stdlib_smallops (+44 vs C).

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
