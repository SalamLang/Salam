# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured three ways: the LLVM backend at
`-O3`, the C backend compiled by `gcc -O3`, and the default `tcc` toolchain
(no optimizer) as the out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-08T13:54:07Z (took 26 min)
- Commit: `6ce8fd2`

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

- **CPU**: AMD EPYC 9V45 96-Core Processor
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
| 1 | **Salam-l** | 10.25 ms | 0.91x | 1.00x | 42 / 42 | 28 / 42 |
| 2 | **Salam-t** | 10.32 ms | 0.92x | 1.01x | 42 / 42 | 26 / 42 |
| 3 | C | 11.21 ms | 1.00x | 1.09x | 42 / 42 | 23 / 42 |
| 4 | Rust | 11.21 ms | 1.00x | 1.09x | 42 / 42 | 20 / 42 |
| 5 | **Salam-g** | 11.22 ms | 1.00x | 1.09x | 42 / 42 | 17 / 42 |
| 6 | C++ | 12.47 ms | 1.11x | 1.22x | 42 / 42 | 5 / 42 |
| 7 | Go | 14.12 ms | 1.26x | 1.38x | 42 / 42 | 2 / 42 |
| 8 | PHP | 108.46 ms | 9.67x | 10.58x | 42 / 42 | 0 / 42 |
| 9 | Python | 521.31 ms | 46.50x | 50.86x | 42 / 42 | 0 / 42 |

```
Salam-l      █████████████████████████████████               10.25 ms  0.91x C
Salam-t      █████████████████████████████████               10.32 ms  0.92x C
C            ████████████████████████████████████            11.21 ms  1.00x C  <- baseline
Rust         ████████████████████████████████████            11.21 ms  1.00x C
Salam-g      ████████████████████████████████████            11.22 ms  1.00x C
C++          ████████████████████████████████████████        12.47 ms  1.11x C
Go           █████████████████████████████████████████████   14.12 ms  1.26x C
PHP          (off scale)                                    108.46 ms  9.67x C
Python       (off scale)                                    521.31 ms  46.50x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 10.25 ms
  (0.91x C's geomean, 0.91x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  28/42); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.01x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 10-46x slower** than the compiled languages on
  this suite and are included for scale only.
- "Fastest / tied-fastest on" counts ties on every side (e.g. a 4-way tie
  adds 1 to all 4 languages), so the column does not sum to 42.

## Per-program results (minimum ms)

| Program | C | C++ | Rust | Go | Salam-l | Salam-g | Salam-t | PHP | Python | Fastest |
|---|---|---|---|---|---|---|---|---|---|---|
| 01_fib_recursive | 2 | 3 | 3 | 5 | 3 | 2 | 3 | 40 | 63 | **C / Salam-g (2)** |
| 02_fib_iterative | 16 | 16 | 16 | 17 | 16 | 16 | 16 | 40 | 424 | **C / C++ / Rust / Salam (16)** |
| 03_primes_count | 4 | 5 | 4 | 5 | 4 | 4 | 4 | 15 | 111 | **C / Rust / Salam (4)** |
| 04_collatz | 4 | 5 | 4 | 6 | 4 | 5 | 4 | 45 | 242 | **C / Rust / Salam (4)** |
| 05_sum_mod | 10 | 11 | 11 | 11 | 10 | 10 | 10 | 23 | 273 | **C / Salam (10)** |
| 06_gcd_sum | 15 | 16 | 13 | 16 | 13 | 15 | 13 | 35 | 99 | **Rust / Salam (13)** |
| 07_pow_mod | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 15 | 46 | **C / Rust / Salam (2)** |
| 08_digit_sum | 3 | 4 | 3 | 4 | 3 | 3 | 3 | 40 | 246 | **C / Rust / Salam (3)** |
| 09_perfect_numbers | 8 | 9 | 7 | 9 | 7 | 8 | 7 | 28 | 266 | **Rust / Salam (7)** |
| 10_pi_leibniz | 4 | 5 | 4 | 5 | 4 | 4 | 4 | 35 | 346 | **C / Rust / Salam (4)** |
| 11_loop_count | 5 | 5 | 5 | 5 | 5 | 5 | 5 | 11 | 78 | **C / C++ / Rust / Go / Salam (5)** |
| 12_hello_print | 16 | 13 | 2 | 5 | 7 | 7 | 7 | 317 | 68 | Rust (2) |
| 13_array_rw | 18 | 19 | 19 | 19 | 19 | 17 | 19 | 52 | 616 | **Salam-g (17)** |
| 14_lcg_random | 15 | 17 | 17 | 20 | 17 | 16 | 17 | 51 | 922 | C (15) |
| 15_matrix_mult | 5 | 6 | 4 | 7 | 4 | 6 | 4 | 112 | 722 | **Rust / Salam (4)** |
| 16_quicksort | 22 | 23 | 21 | 22 | 22 | 23 | 22 | 133 | 438 | Rust (21) |
| 17_n_queens | 35 | 37 | 39 | 41 | 40 | 40 | 39 | 312 | 499 | C (35) |
| 18_sieve_eratosthenes | 5 | 5 | 5 | 5 | 5 | 7 | 5 | 111 | 636 | **C / C++ / Rust / Go / Salam (5)** |
| 19_mandelbrot | 9 | 9 | 9 | 10 | 9 | 9 | 9 | 75 | 817 | **C / C++ / Rust / Salam (9)** |
| 20_monte_carlo_pi | 22 | 23 | 12 | 27 | 12 | 24 | 8 | 197 | 2445 | **Salam-t (8)** |
| 21_coin_change_dp | 2 | 3 | 3 | 3 | 3 | 3 | 3 | 13 | 76 | C (2) |
| 22_knapsack_01 | 3 | 4 | 4 | 5 | 4 | 4 | 4 | 75 | 703 | C (3) |
| 23_caesar_cipher | 10 | 11 | 10 | 14 | 10 | 10 | 10 | 61 | 794 | **C / Rust / Salam (10)** |
| 24_merge_sort | 24 | 25 | 25 | 29 | 27 | 28 | 27 | 183 | 573 | C (24) |
| 25_palindrome_count | 9 | 10 | 10 | 13 | 9 | 10 | 9 | 175 | 1171 | **C / Salam (9)** |
| 26_ackermann | 2 | 2 | 2 | 3 | 2 | 2 | 2 | 9 | 19 | **C / C++ / Rust / Salam (2)** |
| 27_edit_distance | 8 | 9 | 9 | 10 | 5 | 6 | 5 | 167 | 1713 | **Salam (5)** |
| 28_prime_factorization | 815 | 891 | 758 | 835 | 744 | 853 | 745 | 3151 | 35739 | **Salam-l (744)** |
| 29_dot_product | 29 | 31 | 30 | 31 | 30 | 29 | 30 | 148 | 1852 | **C / Salam-g (29)** |
| 30_heap_sort | 30 | 30 | 31 | 40 | 27 | 30 | 28 | 320 | 818 | **Salam-l (27)** |
| 31_binary_search_stress | 731 | 736 | 723 | 739 | 680 | 772 | 731 | 4001 | 20654 | **Salam-l (680)** |
| 32_longest_increasing_subsequence | 9 | 10 | 10 | 13 | 9 | 12 | 9 | 177 | 1248 | **C / Salam (9)** |
| 33_subset_sum_reachability | 9 | 10 | 16 | 16 | 7 | 10 | 7 | 207 | 1618 | **Salam (7)** |
| 34_counting_sort | 27 | 28 | 20 | 29 | 20 | 28 | 20 | 149 | 1656 | **Rust / Salam (20)** |
| 35_run_length_stats | 6 | 6 | 5 | 6 | 5 | 6 | 5 | 74 | 553 | **Rust / Salam (5)** |
| 36_array_rotation_reversal | 17 | 18 | 18 | 18 | 16 | 18 | 16 | 132 | 596 | **Salam (16)** |
| 37_sqrt_decomposition | 8 | 9 | 6 | 12 | 6 | 9 | 6 | 204 | 781 | **Rust / Salam (6)** |
| 38_game_of_life | 8 | 9 | 21 | 26 | 6 | 6 | 6 | 771 | 5883 | **Salam (6)** |
| 39_trapezoidal_integration | 10 | 12 | 10 | 11 | 10 | 10 | 10 | 233 | 1807 | **C / Rust / Salam (10)** |
| 40_matrix_stdlib_matmul | 35 | 37 | 55 | 40 | 12 | 11 | 15 | 855 | 110 | **Salam-g (11)** |
| 41_matrix_stdlib_smallops | 4 | 5 | 5 | 15 | 31 | 40 | 42 | 325 | 981 | C (4) |
| 42_tensor_stdlib_matmul | 35 | 36 | 53 | 40 | 7 | 6 | 7 | 851 | 108 | **Salam-g (6)** |

Bold marks rows where a Salam backend is the fastest or tied for fastest.

### Top 10 fastest programs in Salam-l

Where the LLVM backend (`-O3`) does best **relative to the fastest non-Salam
entry on the same program**: sorted by `vs best other` (Salam-l's minimum
divided by that language's), lowest ratio first. Under 1.00x means Salam-l
wins the row outright. Absolute ms are shown but are not the sort key, since
they mostly track how big each program's workload is.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 42_tensor_stdlib_matmul | 7 | C (35) | 0.20x |
| 2 | 40_matrix_stdlib_matmul | 12 | C (35) | 0.34x |
| 3 | 27_edit_distance | 5 | C (8) | 0.63x |
| 4 | 38_game_of_life | 6 | C (8) | 0.75x |
| 5 | 33_subset_sum_reachability | 7 | C (9) | 0.78x |
| 6 | 30_heap_sort | 27 | C (30) | 0.90x |
| 7 | 31_binary_search_stress | 680 | Rust (723) | 0.94x |
| 8 | 36_array_rotation_reversal | 16 | C (17) | 0.94x |
| 9 | 28_prime_factorization | 744 | Rust (758) | 0.98x |
| 10 | 26_ackermann | 2 | C (2) | 1.00x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 31 | C (4) | 7.75x |
| 2 | 12_hello_print | 7 | Rust (2) | 3.50x |
| 3 | 01_fib_recursive | 3 | C (2) | 1.50x |
| 4 | 21_coin_change_dp | 3 | C (2) | 1.50x |
| 5 | 22_knapsack_01 | 4 | C (3) | 1.33x |
| 6 | 17_n_queens | 40 | C (35) | 1.14x |
| 7 | 14_lcg_random | 17 | C (15) | 1.13x |
| 8 | 24_merge_sort | 27 | C (24) | 1.13x |
| 9 | 13_array_rw | 19 | C (18) | 1.06x |
| 10 | 16_quicksort | 22 | Rust (21) | 1.05x |
| 11 | 29_dot_product | 30 | C (29) | 1.03x |
| 12 | 34_counting_sort | 20 | Rust (20) | 1.00x |
| 13 | 02_fib_iterative | 16 | C (16) | 1.00x |
| 14 | 06_gcd_sum | 13 | Rust (13) | 1.00x |
| 15 | 20_monte_carlo_pi | 12 | Rust (12) | 1.00x |
| 16 | 39_trapezoidal_integration | 10 | C (10) | 1.00x |
| 17 | 05_sum_mod | 10 | C (10) | 1.00x |
| 18 | 23_caesar_cipher | 10 | C (10) | 1.00x |
| 19 | 19_mandelbrot | 9 | C (9) | 1.00x |
| 20 | 32_longest_increasing_subsequence | 9 | C (9) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (11):** 13_array_rw, 20_monte_carlo_pi, 27_edit_distance, 28_prime_factorization, 30_heap_sort, 31_binary_search_stress, 33_subset_sum_reachability, 36_array_rotation_reversal, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (23):** 01_fib_recursive, 02_fib_iterative, 03_primes_count, 04_collatz, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 15_matrix_mult, 18_sieve_eratosthenes, 19_mandelbrot, 23_caesar_cipher, 25_palindrome_count, 26_ackermann, 29_dot_product, 32_longest_increasing_subsequence, 34_counting_sort, 35_run_length_stats, 37_sqrt_decomposition, 39_trapezoidal_integration.
- **Trailing the fastest (8):** 12_hello_print (+5 vs Rust); 14_lcg_random (+1 vs C); 16_quicksort (+1 vs Rust); 17_n_queens (+4 vs C); 21_coin_change_dp (+1 vs C); 22_knapsack_01 (+1 vs C); 24_merge_sort (+3 vs C); 41_matrix_stdlib_smallops (+27 vs C).

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
