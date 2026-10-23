# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured three ways: the LLVM backend at
`-O3`, the C backend compiled by `gcc -O3`, and the default `tcc` toolchain
(no optimizer) as the out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-23T12:37:15Z (took 47 min)
- Commit: `b44a73d`

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
- **salam**: 0.5.8

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
| 1 | **Salam-l** | 14.16 ms | 0.90x | 1.00x | 42 / 42 | 25 / 42 |
| 2 | **Salam-t** | 14.49 ms | 0.92x | 1.02x | 42 / 42 | 25 / 42 |
| 3 | **Salam-g** | 15.36 ms | 0.98x | 1.08x | 42 / 42 | 20 / 42 |
| 4 | C | 15.67 ms | 1.00x | 1.11x | 42 / 42 | 22 / 42 |
| 5 | Rust | 15.88 ms | 1.01x | 1.12x | 42 / 42 | 21 / 42 |
| 6 | C++ | 17.31 ms | 1.10x | 1.22x | 42 / 42 | 2 / 42 |
| 7 | Go | 20.10 ms | 1.28x | 1.42x | 42 / 42 | 0 / 42 |
| 8 | PHP | 191.53 ms | 12.22x | 13.53x | 42 / 42 | 0 / 42 |
| 9 | Python | 981.94 ms | 62.66x | 69.36x | 42 / 42 | 0 / 42 |

```
Salam-l      ████████████████████████████████                14.16 ms  0.90x C
Salam-t      ████████████████████████████████                14.49 ms  0.92x C
Salam-g      ██████████████████████████████████              15.36 ms  0.98x C
C            ███████████████████████████████████             15.67 ms  1.00x C  <- baseline
Rust         ████████████████████████████████████            15.88 ms  1.01x C
C++          ███████████████████████████████████████         17.31 ms  1.10x C
Go           █████████████████████████████████████████████   20.10 ms  1.28x C
PHP          (off scale)                                    191.53 ms  12.22x C
Python       (off scale)                                    981.94 ms  62.66x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 14.16 ms
  (0.90x C's geomean, 0.89x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  25/42); see the per-program table below.
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
| 01_fib_recursive | 4 | 5 | 4 | 8 | 4 | 4 | 4 | 68 | 112 | **C / Rust / Salam (4)** |
| 02_fib_iterative | 21 | 22 | 21 | 23 | 21 | 21 | 21 | 66 | 838 | **C / Rust / Salam (21)** |
| 03_primes_count | 5 | 6 | 5 | 6 | 5 | 5 | 5 | 25 | 198 | **C / Rust / Salam (5)** |
| 04_collatz | 6 | 7 | 5 | 8 | 5 | 5 | 5 | 77 | 443 | **Rust / Salam (5)** |
| 05_sum_mod | 14 | 15 | 15 | 15 | 14 | 14 | 14 | 38 | 533 | **C / Salam (14)** |
| 06_gcd_sum | 17 | 18 | 16 | 18 | 16 | 17 | 16 | 65 | 175 | **Rust / Salam (16)** |
| 07_pow_mod | 3 | 4 | 3 | 5 | 3 | 3 | 3 | 26 | 80 | **C / Rust / Salam (3)** |
| 08_digit_sum | 4 | 5 | 5 | 6 | 5 | 4 | 4 | 79 | 449 | **C / Salam (4)** |
| 09_perfect_numbers | 10 | 11 | 9 | 12 | 9 | 10 | 9 | 46 | 496 | **Rust / Salam (9)** |
| 10_pi_leibniz | 6 | 7 | 6 | 7 | 6 | 6 | 6 | 58 | 738 | **C / Rust / Salam (6)** |
| 11_loop_count | 6 | 7 | 7 | 7 | 7 | 6 | 7 | 17 | 155 | **C / Salam-g (6)** |
| 12_hello_print | 22 | 22 | 4 | 9 | 5 | 5 | 5 | 450 | 123 | Rust (4) |
| 13_array_rw | 25 | 26 | 26 | 26 | 26 | 23 | 25 | 90 | 1256 | **Salam-g (23)** |
| 14_lcg_random | 21 | 22 | 23 | 27 | 22 | 21 | 23 | 85 | 1692 | **C / Salam-g (21)** |
| 15_matrix_mult | 7 | 9 | 7 | 11 | 7 | 8 | 7 | 243 | 1427 | **C / Rust / Salam (7)** |
| 16_quicksort | 27 | 28 | 26 | 29 | 26 | 29 | 27 | 189 | 703 | **Rust / Salam-l (26)** |
| 17_n_queens | 42 | 43 | 42 | 45 | 45 | 47 | 45 | 579 | 805 | C / Rust (42) |
| 18_sieve_eratosthenes | 7 | 8 | 7 | 10 | 7 | 10 | 7 | 118 | 1124 | **C / Rust / Salam (7)** |
| 19_mandelbrot | 12 | 13 | 12 | 14 | 12 | 12 | 12 | 158 | 1658 | **C / Rust / Salam (12)** |
| 20_monte_carlo_pi | 31 | 32 | 16 | 37 | 16 | 34 | 13 | 388 | 4760 | **Salam-t (13)** |
| 21_coin_change_dp | 3 | 4 | 4 | 4 | 4 | 4 | 4 | 20 | 153 | C (3) |
| 22_knapsack_01 | 6 | 7 | 5 | 8 | 6 | 6 | 7 | 134 | 1738 | Rust (5) |
| 23_caesar_cipher | 17 | 18 | 14 | 24 | 14 | 20 | 14 | 110 | 1435 | **Rust / Salam (14)** |
| 24_merge_sort | 31 | 31 | 33 | 36 | 32 | 37 | 37 | 328 | 1102 | C / C++ (31) |
| 25_palindrome_count | 15 | 16 | 15 | 19 | 15 | 15 | 15 | 363 | 2173 | **C / Rust / Salam (15)** |
| 26_ackermann | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 12 | 29 | **C / Rust / Salam (2)** |
| 27_edit_distance | 8 | 9 | 9 | 12 | 7 | 8 | 7 | 331 | 3130 | **Salam (7)** |
| 28_prime_factorization | 1117 | 1118 | 969 | 1106 | 968 | 1118 | 970 | 6243 | 69306 | **Salam-l (968)** |
| 29_dot_product | 40 | 41 | 43 | 43 | 43 | 40 | 41 | 280 | 3743 | **C / Salam-g (40)** |
| 30_heap_sort | 36 | 36 | 40 | 47 | 37 | 36 | 38 | 634 | 1449 | **C / C++ / Salam-g (36)** |
| 31_binary_search_stress | 924 | 920 | 1047 | 908 | 858 | 1011 | 934 | 5968 | 32490 | **Salam-l (858)** |
| 32_longest_increasing_subsequence | 16 | 16 | 12 | 14 | 12 | 16 | 13 | 314 | 3163 | **Rust / Salam-l (12)** |
| 33_subset_sum_reachability | 19 | 20 | 21 | 20 | 14 | 21 | 19 | 392 | 3523 | **Salam-l (14)** |
| 34_counting_sort | 42 | 42 | 32 | 44 | 33 | 45 | 32 | 313 | 3221 | **Rust / Salam-t (32)** |
| 35_run_length_stats | 9 | 10 | 8 | 10 | 8 | 9 | 8 | 152 | 1057 | **Rust / Salam (8)** |
| 36_array_rotation_reversal | 23 | 25 | 25 | 26 | 23 | 23 | 23 | 237 | 1281 | **C / Salam (23)** |
| 37_sqrt_decomposition | 8 | 10 | 8 | 20 | 8 | 9 | 7 | 350 | 1619 | **Salam-t (7)** |
| 38_game_of_life | 11 | 13 | 38 | 59 | 11 | 11 | 10 | 1436 | 10549 | **Salam-t (10)** |
| 39_trapezoidal_integration | 15 | 16 | 16 | 18 | 16 | 15 | 15 | 517 | 3902 | **C / Salam (15)** |
| 40_matrix_stdlib_matmul | 54 | 55 | 93 | 68 | 15 | 15 | 19 | 1651 | 159 | **Salam (15)** |
| 41_matrix_stdlib_smallops | 6 | 7 | 7 | 23 | 53 | 63 | 68 | 606 | 1932 | C (6) |
| 42_tensor_stdlib_matmul | 54 | 55 | 93 | 68 | 9 | 9 | 9 | 1646 | 158 | **Salam (9)** |

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
| 2 | 40_matrix_stdlib_matmul | 15 | C (54) | 0.28x |
| 3 | 33_subset_sum_reachability | 14 | C (19) | 0.74x |
| 4 | 27_edit_distance | 7 | C (8) | 0.88x |
| 5 | 31_binary_search_stress | 858 | Go (908) | 0.94x |
| 6 | 28_prime_factorization | 968 | Rust (969) | 1.00x |
| 7 | 26_ackermann | 2 | C (2) | 1.00x |
| 8 | 07_pow_mod | 3 | C (3) | 1.00x |
| 9 | 01_fib_recursive | 4 | C (4) | 1.00x |
| 10 | 04_collatz | 5 | Rust (5) | 1.00x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 53 | C (6) | 8.83x |
| 2 | 21_coin_change_dp | 4 | C (3) | 1.33x |
| 3 | 08_digit_sum | 5 | C (4) | 1.25x |
| 4 | 12_hello_print | 5 | Rust (4) | 1.25x |
| 5 | 22_knapsack_01 | 6 | Rust (5) | 1.20x |
| 6 | 11_loop_count | 7 | C (6) | 1.17x |
| 7 | 29_dot_product | 43 | C (40) | 1.08x |
| 8 | 17_n_queens | 45 | C (42) | 1.07x |
| 9 | 39_trapezoidal_integration | 16 | C (15) | 1.07x |
| 10 | 14_lcg_random | 22 | C (21) | 1.05x |
| 11 | 13_array_rw | 26 | C (25) | 1.04x |
| 12 | 24_merge_sort | 32 | C (31) | 1.03x |
| 13 | 34_counting_sort | 33 | Rust (32) | 1.03x |
| 14 | 30_heap_sort | 37 | C (36) | 1.03x |
| 15 | 16_quicksort | 26 | Rust (26) | 1.00x |
| 16 | 36_array_rotation_reversal | 23 | C (23) | 1.00x |
| 17 | 02_fib_iterative | 21 | C (21) | 1.00x |
| 18 | 20_monte_carlo_pi | 16 | Rust (16) | 1.00x |
| 19 | 06_gcd_sum | 16 | Rust (16) | 1.00x |
| 20 | 25_palindrome_count | 15 | C (15) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (10):** 13_array_rw, 20_monte_carlo_pi, 27_edit_distance, 28_prime_factorization, 31_binary_search_stress, 33_subset_sum_reachability, 37_sqrt_decomposition, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (26):** 01_fib_recursive, 02_fib_iterative, 03_primes_count, 04_collatz, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 14_lcg_random, 15_matrix_mult, 16_quicksort, 18_sieve_eratosthenes, 19_mandelbrot, 23_caesar_cipher, 25_palindrome_count, 26_ackermann, 29_dot_product, 30_heap_sort, 32_longest_increasing_subsequence, 34_counting_sort, 35_run_length_stats, 36_array_rotation_reversal, 39_trapezoidal_integration.
- **Trailing the fastest (6):** 12_hello_print (+1 vs Rust); 17_n_queens (+3 vs C/Rust); 21_coin_change_dp (+1 vs C); 22_knapsack_01 (+1 vs Rust); 24_merge_sort (+1 vs C/C++); 41_matrix_stdlib_smallops (+47 vs C).

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
