# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured three ways: the LLVM backend at
`-O3`, the C backend compiled by `gcc -O3`, and the default `tcc` toolchain
(no optimizer) as the out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-22T22:59:04Z (took 46 min)
- Commit: `c35b22e`

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
| 1 | **Salam-l** | 13.96 ms | 0.90x | 1.00x | 42 / 42 | 22 / 42 |
| 2 | **Salam-t** | 14.38 ms | 0.92x | 1.03x | 42 / 42 | 21 / 42 |
| 3 | **Salam-g** | 15.28 ms | 0.98x | 1.09x | 42 / 42 | 18 / 42 |
| 4 | Rust | 15.56 ms | 1.00x | 1.11x | 42 / 42 | 18 / 42 |
| 5 | C | 15.58 ms | 1.00x | 1.12x | 42 / 42 | 21 / 42 |
| 6 | C++ | 17.17 ms | 1.10x | 1.23x | 42 / 42 | 1 / 42 |
| 7 | Go | 19.78 ms | 1.27x | 1.42x | 42 / 42 | 0 / 42 |
| 8 | PHP | 189.22 ms | 12.15x | 13.55x | 42 / 42 | 0 / 42 |
| 9 | Python | 974.52 ms | 62.56x | 69.79x | 42 / 42 | 0 / 42 |

```
Salam-l      ████████████████████████████████                13.96 ms  0.90x C
Salam-t      █████████████████████████████████               14.38 ms  0.92x C
Salam-g      ███████████████████████████████████             15.28 ms  0.98x C
Rust         ███████████████████████████████████             15.56 ms  1.00x C
C            ███████████████████████████████████             15.58 ms  1.00x C  <- baseline
C++          ███████████████████████████████████████         17.17 ms  1.10x C
Go           █████████████████████████████████████████████   19.78 ms  1.27x C
PHP          (off scale)                                    189.22 ms  12.15x C
Python       (off scale)                                    974.52 ms  62.56x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 13.96 ms
  (0.90x C's geomean, 0.90x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  22/42); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.03x slower than the
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
| 01_fib_recursive | 3 | 4 | 4 | 7 | 4 | 4 | 4 | 68 | 112 | C (3) |
| 02_fib_iterative | 21 | 22 | 21 | 22 | 21 | 21 | 21 | 65 | 844 | **C / Rust / Salam (21)** |
| 03_primes_count | 5 | 6 | 5 | 6 | 5 | 5 | 5 | 24 | 200 | **C / Rust / Salam (5)** |
| 04_collatz | 5 | 7 | 5 | 8 | 5 | 5 | 5 | 77 | 439 | **C / Rust / Salam (5)** |
| 05_sum_mod | 13 | 14 | 14 | 14 | 14 | 13 | 14 | 37 | 528 | **C / Salam-g (13)** |
| 06_gcd_sum | 17 | 18 | 16 | 18 | 16 | 17 | 16 | 62 | 175 | **Rust / Salam (16)** |
| 07_pow_mod | 3 | 4 | 3 | 5 | 3 | 3 | 3 | 26 | 80 | **C / Rust / Salam (3)** |
| 08_digit_sum | 4 | 5 | 5 | 5 | 4 | 4 | 4 | 79 | 453 | **C / Salam (4)** |
| 09_perfect_numbers | 10 | 11 | 9 | 11 | 9 | 10 | 9 | 45 | 500 | **Rust / Salam (9)** |
| 10_pi_leibniz | 6 | 7 | 6 | 7 | 6 | 6 | 6 | 57 | 749 | **C / Rust / Salam (6)** |
| 11_loop_count | 6 | 7 | 7 | 7 | 7 | 6 | 7 | 17 | 157 | **C / Salam-g (6)** |
| 12_hello_print | 22 | 22 | 4 | 9 | 5 | 5 | 5 | 451 | 125 | Rust (4) |
| 13_array_rw | 25 | 26 | 26 | 26 | 26 | 23 | 26 | 90 | 1244 | **Salam-g (23)** |
| 14_lcg_random | 21 | 22 | 23 | 27 | 22 | 21 | 22 | 84 | 1688 | **C / Salam-g (21)** |
| 15_matrix_mult | 7 | 8 | 7 | 11 | 7 | 8 | 7 | 240 | 1445 | **C / Rust / Salam (7)** |
| 16_quicksort | 27 | 28 | 26 | 28 | 26 | 29 | 27 | 189 | 673 | **Rust / Salam-l (26)** |
| 17_n_queens | 43 | 43 | 42 | 46 | 45 | 47 | 45 | 572 | 809 | Rust (42) |
| 18_sieve_eratosthenes | 7 | 7 | 7 | 10 | 7 | 9 | 7 | 123 | 1131 | **C / C++ / Rust / Salam (7)** |
| 19_mandelbrot | 12 | 13 | 12 | 14 | 12 | 12 | 12 | 158 | 1651 | **C / Rust / Salam (12)** |
| 20_monte_carlo_pi | 31 | 32 | 16 | 37 | 16 | 34 | 12 | 381 | 4740 | **Salam-t (12)** |
| 21_coin_change_dp | 3 | 4 | 4 | 4 | 4 | 4 | 4 | 20 | 153 | C (3) |
| 22_knapsack_01 | 6 | 7 | 5 | 8 | 6 | 6 | 7 | 134 | 1743 | Rust (5) |
| 23_caesar_cipher | 17 | 18 | 14 | 24 | 14 | 19 | 13 | 108 | 1445 | **Salam-t (13)** |
| 24_merge_sort | 31 | 32 | 33 | 37 | 32 | 38 | 37 | 326 | 1044 | C (31) |
| 25_palindrome_count | 14 | 15 | 15 | 18 | 14 | 14 | 14 | 352 | 2172 | **C / Salam (14)** |
| 26_ackermann | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 12 | 28 | **C / Rust / Salam (2)** |
| 27_edit_distance | 8 | 9 | 10 | 12 | 7 | 8 | 7 | 332 | 3189 | **Salam (7)** |
| 28_prime_factorization | 1117 | 1119 | 970 | 1107 | 969 | 1118 | 972 | 6236 | 70133 | **Salam-l (969)** |
| 29_dot_product | 40 | 41 | 43 | 43 | 43 | 40 | 41 | 283 | 3758 | **C / Salam-g (40)** |
| 30_heap_sort | 36 | 37 | 40 | 47 | 37 | 36 | 38 | 633 | 1304 | **C / Salam-g (36)** |
| 31_binary_search_stress | 883 | 884 | 721 | 864 | 826 | 952 | 900 | 4846 | 30890 | Rust (721) |
| 32_longest_increasing_subsequence | 16 | 17 | 12 | 13 | 12 | 15 | 13 | 313 | 3160 | **Rust / Salam-l (12)** |
| 33_subset_sum_reachability | 20 | 20 | 21 | 20 | 14 | 22 | 19 | 390 | 3631 | **Salam-l (14)** |
| 34_counting_sort | 42 | 41 | 32 | 43 | 33 | 45 | 32 | 311 | 3173 | **Rust / Salam-t (32)** |
| 35_run_length_stats | 9 | 10 | 8 | 10 | 7 | 9 | 8 | 151 | 1050 | **Salam-l (7)** |
| 36_array_rotation_reversal | 23 | 25 | 24 | 26 | 22 | 24 | 23 | 237 | 1259 | **Salam-l (22)** |
| 37_sqrt_decomposition | 8 | 10 | 8 | 19 | 7 | 10 | 7 | 350 | 1554 | **Salam (7)** |
| 38_game_of_life | 11 | 13 | 38 | 58 | 11 | 11 | 10 | 1418 | 10550 | **Salam-t (10)** |
| 39_trapezoidal_integration | 15 | 16 | 16 | 17 | 16 | 15 | 15 | 520 | 3851 | **C / Salam (15)** |
| 40_matrix_stdlib_matmul | 53 | 55 | 93 | 66 | 15 | 14 | 19 | 1642 | 154 | **Salam-g (14)** |
| 41_matrix_stdlib_smallops | 6 | 7 | 7 | 23 | 52 | 62 | 68 | 603 | 1897 | C (6) |
| 42_tensor_stdlib_matmul | 53 | 54 | 93 | 66 | 9 | 9 | 9 | 1643 | 154 | **Salam (9)** |

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
| 3 | 33_subset_sum_reachability | 14 | C (20) | 0.70x |
| 4 | 27_edit_distance | 7 | C (8) | 0.88x |
| 5 | 35_run_length_stats | 7 | Rust (8) | 0.88x |
| 6 | 37_sqrt_decomposition | 7 | C (8) | 0.88x |
| 7 | 36_array_rotation_reversal | 22 | C (23) | 0.96x |
| 8 | 28_prime_factorization | 969 | Rust (970) | 1.00x |
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
| 2 | 01_fib_recursive | 4 | C (3) | 1.33x |
| 3 | 21_coin_change_dp | 4 | C (3) | 1.33x |
| 4 | 12_hello_print | 5 | Rust (4) | 1.25x |
| 5 | 22_knapsack_01 | 6 | Rust (5) | 1.20x |
| 6 | 11_loop_count | 7 | C (6) | 1.17x |
| 7 | 31_binary_search_stress | 826 | Rust (721) | 1.15x |
| 8 | 05_sum_mod | 14 | C (13) | 1.08x |
| 9 | 29_dot_product | 43 | C (40) | 1.08x |
| 10 | 17_n_queens | 45 | Rust (42) | 1.07x |
| 11 | 39_trapezoidal_integration | 16 | C (15) | 1.07x |
| 12 | 14_lcg_random | 22 | C (21) | 1.05x |
| 13 | 13_array_rw | 26 | C (25) | 1.04x |
| 14 | 24_merge_sort | 32 | C (31) | 1.03x |
| 15 | 34_counting_sort | 33 | Rust (32) | 1.03x |
| 16 | 30_heap_sort | 37 | C (36) | 1.03x |
| 17 | 16_quicksort | 26 | Rust (26) | 1.00x |
| 18 | 02_fib_iterative | 21 | C (21) | 1.00x |
| 19 | 06_gcd_sum | 16 | Rust (16) | 1.00x |
| 20 | 20_monte_carlo_pi | 16 | Rust (16) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (12):** 13_array_rw, 20_monte_carlo_pi, 23_caesar_cipher, 27_edit_distance, 28_prime_factorization, 33_subset_sum_reachability, 35_run_length_stats, 36_array_rotation_reversal, 37_sqrt_decomposition, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (22):** 02_fib_iterative, 03_primes_count, 04_collatz, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 14_lcg_random, 15_matrix_mult, 16_quicksort, 18_sieve_eratosthenes, 19_mandelbrot, 25_palindrome_count, 26_ackermann, 29_dot_product, 30_heap_sort, 32_longest_increasing_subsequence, 34_counting_sort, 39_trapezoidal_integration.
- **Trailing the fastest (8):** 01_fib_recursive (+1 vs C); 12_hello_print (+1 vs Rust); 17_n_queens (+3 vs Rust); 21_coin_change_dp (+1 vs C); 22_knapsack_01 (+1 vs Rust); 24_merge_sort (+1 vs C); 31_binary_search_stress (+105 vs Rust); 41_matrix_stdlib_smallops (+46 vs C).

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
