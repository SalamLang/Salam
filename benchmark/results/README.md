# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured three ways: the LLVM backend at
`-O3`, the C backend compiled by `gcc -O3`, and the default `tcc` toolchain
(no optimizer) as the out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-06T09:45:08Z (took 27 min)
- Commit: `bbbbce4`

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

- **CPU**: Intel(R) Xeon(R) 6973P-C
- **gcc**: gcc (Alpine 15.2.0) 15.2.0
- **g++**: g++ (Alpine 15.2.0) 15.2.0
- **rustc**: rustc 1.96.1 (31fca3adb 2026-06-26) (Alpine Linux Rust 1.96.1-r0)
- **go**: go version go1.26.8 linux/amd64
- **tcc**: tcc version 0.9.28rc (x86_64 Linux)
- **clang**: Alpine clang version 22.1.3
- **python**: Python 3.14.8
- **numpy**: 2.4.6
- **php**: PHP 8.5.11 (cli) (built: Sep 25 2026 15:57:29) (NTS)
- **salam**: 0.5.0

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
| 1 | **Salam-l** | 10.12 ms | 0.89x | 1.00x | 42 / 42 | 29 / 42 |
| 2 | C | 11.40 ms | 1.00x | 1.13x | 42 / 42 | 21 / 42 |
| 3 | Rust | 11.44 ms | 1.00x | 1.13x | 42 / 42 | 20 / 42 |
| 4 | **Salam-t** | 11.45 ms | 1.00x | 1.13x | 42 / 42 | 24 / 42 |
| 5 | **Salam-g** | 12.40 ms | 1.09x | 1.23x | 42 / 42 | 18 / 42 |
| 6 | C++ | 12.53 ms | 1.10x | 1.24x | 42 / 42 | 5 / 42 |
| 7 | Go | 15.40 ms | 1.35x | 1.52x | 42 / 42 | 0 / 42 |
| 8 | PHP | 115.47 ms | 10.13x | 11.41x | 42 / 42 | 0 / 42 |
| 9 | Python | 539.46 ms | 47.30x | 53.31x | 42 / 42 | 0 / 42 |

```
Salam-l      ██████████████████████████████                  10.12 ms  0.89x C
C            █████████████████████████████████               11.40 ms  1.00x C  <- baseline
Rust         █████████████████████████████████               11.44 ms  1.00x C
Salam-t      █████████████████████████████████               11.45 ms  1.00x C
Salam-g      ████████████████████████████████████            12.40 ms  1.09x C
C++          █████████████████████████████████████           12.53 ms  1.10x C
Go           █████████████████████████████████████████████   15.40 ms  1.35x C
PHP          (off scale)                                    115.47 ms  10.13x C
Python       (off scale)                                    539.46 ms  47.30x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 10.12 ms
  (0.89x C's geomean, 0.88x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  29/42); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.13x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 10-47x slower** than the compiled languages on
  this suite and are included for scale only.
- "Fastest / tied-fastest on" counts ties on every side (e.g. a 4-way tie
  adds 1 to all 4 languages), so the column does not sum to 42.

## Per-program results (minimum ms)

| Program | C | C++ | Rust | Go | Salam-l | Salam-g | Salam-t | PHP | Python | Fastest |
|---|---|---|---|---|---|---|---|---|---|---|
| 01_fib_recursive | 2 | 3 | 3 | 5 | 3 | 2 | 3 | 38 | 65 | **C / Salam-g (2)** |
| 02_fib_iterative | 16 | 16 | 16 | 17 | 16 | 16 | 16 | 47 | 471 | **C / C++ / Rust / Salam (16)** |
| 03_primes_count | 5 | 5 | 4 | 5 | 3 | 5 | 3 | 14 | 115 | **Salam (3)** |
| 04_collatz | 5 | 5 | 4 | 6 | 4 | 4 | 4 | 44 | 252 | **Rust / Salam (4)** |
| 05_sum_mod | 10 | 11 | 10 | 11 | 10 | 10 | 10 | 24 | 293 | **C / Rust / Salam (10)** |
| 06_gcd_sum | 18 | 19 | 14 | 19 | 14 | 18 | 14 | 37 | 106 | **Rust / Salam (14)** |
| 07_pow_mod | 2 | 2 | 2 | 3 | 2 | 2 | 2 | 14 | 47 | **C / C++ / Rust / Salam (2)** |
| 08_digit_sum | 3 | 4 | 3 | 4 | 3 | 3 | 3 | 42 | 253 | **C / Rust / Salam (3)** |
| 09_perfect_numbers | 10 | 11 | 6 | 11 | 6 | 10 | 6 | 26 | 283 | **Rust / Salam (6)** |
| 10_pi_leibniz | 4 | 5 | 4 | 5 | 4 | 4 | 4 | 43 | 354 | **C / Rust / Salam (4)** |
| 11_loop_count | 4 | 5 | 5 | 5 | 6 | 5 | 6 | 11 | 82 | C (4) |
| 12_hello_print | 13 | 14 | 2 | 5 | 2 | 144 | 145 | 163 | 62 | **Rust / Salam-l (2)** |
| 13_array_rw | 18 | 19 | 20 | 19 | 20 | 17 | 20 | 57 | 672 | **Salam-g (17)** |
| 14_lcg_random | 16 | 16 | 17 | 20 | 17 | 16 | 17 | 65 | 1009 | **C / C++ / Salam-g (16)** |
| 15_matrix_mult | 4 | 5 | 4 | 8 | 4 | 4 | 4 | 140 | 786 | **C / Rust / Salam (4)** |
| 16_quicksort | 23 | 24 | 22 | 24 | 21 | 25 | 22 | 128 | 398 | **Salam-l (21)** |
| 17_n_queens | 36 | 37 | 38 | 41 | 42 | 45 | 44 | 364 | 506 | C (36) |
| 18_sieve_eratosthenes | 7 | 8 | 8 | 8 | 8 | 8 | 8 | 96 | 685 | C (7) |
| 19_mandelbrot | 9 | 9 | 8 | 9 | 8 | 8 | 8 | 99 | 851 | **Rust / Salam (8)** |
| 20_monte_carlo_pi | 25 | 26 | 14 | 30 | 14 | 25 | 11 | 258 | 2624 | **Salam-t (11)** |
| 21_coin_change_dp | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 12 | 80 | **C / Rust / Salam (2)** |
| 22_knapsack_01 | 3 | 4 | 4 | 5 | 3 | 5 | 4 | 82 | 765 | **C / Salam-l (3)** |
| 23_caesar_cipher | 11 | 11 | 10 | 15 | 10 | 11 | 10 | 74 | 842 | **Rust / Salam (10)** |
| 24_merge_sort | 25 | 26 | 27 | 30 | 27 | 29 | 28 | 213 | 572 | C (25) |
| 25_palindrome_count | 10 | 11 | 10 | 12 | 10 | 10 | 10 | 190 | 1235 | **C / Rust / Salam (10)** |
| 26_ackermann | 1 | 2 | 1 | 2 | 1 | 1 | 1 | 7 | 16 | **C / Rust / Salam (1)** |
| 27_edit_distance | 5 | 6 | 6 | 12 | 4 | 5 | 5 | 174 | 1746 | **Salam-l (4)** |
| 28_prime_factorization | 1231 | 1228 | 759 | 1216 | 760 | 1226 | 768 | 3268 | 39606 | Rust (759) |
| 29_dot_product | 30 | 31 | 31 | 31 | 31 | 31 | 31 | 174 | 2016 | C (30) |
| 30_heap_sort | 30 | 30 | 34 | 36 | 29 | 30 | 31 | 361 | 787 | **Salam-l (29)** |
| 31_binary_search_stress | 857 | 857 | 658 | 844 | 956 | 905 | 926 | 3013 | 18641 | Rust (658) |
| 32_longest_increasing_subsequence | 9 | 11 | 22 | 11 | 8 | 9 | 8 | 198 | 1359 | **Salam (8)** |
| 33_subset_sum_reachability | 9 | 9 | 18 | 14 | 9 | 15 | 9 | 233 | 1754 | **C / C++ / Salam (9)** |
| 34_counting_sort | 29 | 30 | 22 | 33 | 22 | 29 | 22 | 169 | 1785 | **Rust / Salam (22)** |
| 35_run_length_stats | 6 | 7 | 5 | 8 | 5 | 7 | 5 | 85 | 616 | **Rust / Salam (5)** |
| 36_array_rotation_reversal | 18 | 20 | 20 | 20 | 18 | 18 | 18 | 163 | 678 | **C / Salam (18)** |
| 37_sqrt_decomposition | 8 | 9 | 7 | 12 | 7 | 9 | 8 | 231 | 832 | **Rust / Salam-l (7)** |
| 38_game_of_life | 8 | 9 | 25 | 37 | 6 | 7 | 6 | 804 | 6303 | **Salam (6)** |
| 39_trapezoidal_integration | 11 | 12 | 12 | 12 | 12 | 11 | 11 | 290 | 1822 | **C / Salam (11)** |
| 40_matrix_stdlib_matmul | 31 | 27 | 50 | 64 | 10 | 10 | 12 | 1071 | 99 | **Salam (10)** |
| 41_matrix_stdlib_smallops | 4 | 4 | 5 | 17 | 34 | 42 | 45 | 383 | 1013 | C / C++ (4) |
| 42_tensor_stdlib_matmul | 31 | 27 | 51 | 65 | 6 | 6 | 6 | 1072 | 99 | **Salam (6)** |

Bold marks rows where a Salam backend is the fastest or tied for fastest.

### Top 10 fastest programs in Salam-l

Where the LLVM backend (`-O3`) does best **relative to the fastest non-Salam
entry on the same program**: sorted by `vs best other` (Salam-l's minimum
divided by that language's), lowest ratio first. Under 1.00x means Salam-l
wins the row outright. Absolute ms are shown but are not the sort key, since
they mostly track how big each program's workload is.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 42_tensor_stdlib_matmul | 6 | C++ (27) | 0.22x |
| 2 | 40_matrix_stdlib_matmul | 10 | C++ (27) | 0.37x |
| 3 | 03_primes_count | 3 | Rust (4) | 0.75x |
| 4 | 38_game_of_life | 6 | C (8) | 0.75x |
| 5 | 27_edit_distance | 4 | C (5) | 0.80x |
| 6 | 32_longest_increasing_subsequence | 8 | C (9) | 0.89x |
| 7 | 16_quicksort | 21 | Rust (22) | 0.95x |
| 8 | 30_heap_sort | 29 | C (30) | 0.97x |
| 9 | 26_ackermann | 1 | C (1) | 1.00x |
| 10 | 12_hello_print | 2 | Rust (2) | 1.00x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 34 | C (4) | 8.50x |
| 2 | 11_loop_count | 6 | C (4) | 1.50x |
| 3 | 01_fib_recursive | 3 | C (2) | 1.50x |
| 4 | 31_binary_search_stress | 956 | Rust (658) | 1.45x |
| 5 | 17_n_queens | 42 | C (36) | 1.17x |
| 6 | 18_sieve_eratosthenes | 8 | C (7) | 1.14x |
| 7 | 13_array_rw | 20 | C (18) | 1.11x |
| 8 | 39_trapezoidal_integration | 12 | C (11) | 1.09x |
| 9 | 24_merge_sort | 27 | C (25) | 1.08x |
| 10 | 14_lcg_random | 17 | C (16) | 1.06x |
| 11 | 29_dot_product | 31 | C (30) | 1.03x |
| 12 | 28_prime_factorization | 760 | Rust (759) | 1.00x |
| 13 | 34_counting_sort | 22 | Rust (22) | 1.00x |
| 14 | 36_array_rotation_reversal | 18 | C (18) | 1.00x |
| 15 | 02_fib_iterative | 16 | C (16) | 1.00x |
| 16 | 20_monte_carlo_pi | 14 | Rust (14) | 1.00x |
| 17 | 06_gcd_sum | 14 | Rust (14) | 1.00x |
| 18 | 05_sum_mod | 10 | C (10) | 1.00x |
| 19 | 25_palindrome_count | 10 | C (10) | 1.00x |
| 20 | 23_caesar_cipher | 10 | Rust (10) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (10):** 03_primes_count, 13_array_rw, 16_quicksort, 20_monte_carlo_pi, 27_edit_distance, 30_heap_sort, 32_longest_increasing_subsequence, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (24):** 01_fib_recursive, 02_fib_iterative, 04_collatz, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 12_hello_print, 14_lcg_random, 15_matrix_mult, 19_mandelbrot, 21_coin_change_dp, 22_knapsack_01, 23_caesar_cipher, 25_palindrome_count, 26_ackermann, 33_subset_sum_reachability, 34_counting_sort, 35_run_length_stats, 36_array_rotation_reversal, 37_sqrt_decomposition, 39_trapezoidal_integration.
- **Trailing the fastest (8):** 11_loop_count (+1 vs C); 17_n_queens (+6 vs C); 18_sieve_eratosthenes (+1 vs C); 24_merge_sort (+2 vs C); 28_prime_factorization (+1 vs Rust); 29_dot_product (+1 vs C); 31_binary_search_stress (+247 vs Rust); 41_matrix_stdlib_smallops (+30 vs C/C++).

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
