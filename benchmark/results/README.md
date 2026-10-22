# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured three ways: the LLVM backend at
`-O3`, the C backend compiled by `gcc -O3`, and the default `tcc` toolchain
(no optimizer) as the out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-22T20:41:59Z (took 36 min)
- Commit: `a2567e1`

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

- **CPU**: INTEL(R) XEON(R) PLATINUM 8573C
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
| 1 | **Salam-l** | 13.09 ms | 0.90x | 1.00x | 42 / 42 | 19 / 42 |
| 2 | **Salam-t** | 13.50 ms | 0.93x | 1.03x | 42 / 42 | 21 / 42 |
| 3 | Rust | 14.44 ms | 0.99x | 1.10x | 42 / 42 | 13 / 42 |
| 4 | C | 14.55 ms | 1.00x | 1.11x | 42 / 42 | 17 / 42 |
| 5 | **Salam-g** | 14.77 ms | 1.02x | 1.13x | 42 / 42 | 12 / 42 |
| 6 | C++ | 15.95 ms | 1.10x | 1.22x | 42 / 42 | 2 / 42 |
| 7 | Go | 19.70 ms | 1.35x | 1.50x | 42 / 42 | 1 / 42 |
| 8 | PHP | 146.39 ms | 10.06x | 11.18x | 42 / 42 | 0 / 42 |
| 9 | Python | 701.86 ms | 48.24x | 53.60x | 42 / 42 | 0 / 42 |

```
Salam-l      ██████████████████████████████                  13.09 ms  0.90x C
Salam-t      ███████████████████████████████                 13.50 ms  0.93x C
Rust         █████████████████████████████████               14.44 ms  0.99x C
C            █████████████████████████████████               14.55 ms  1.00x C  <- baseline
Salam-g      ██████████████████████████████████              14.77 ms  1.02x C
C++          ████████████████████████████████████            15.95 ms  1.10x C
Go           █████████████████████████████████████████████   19.70 ms  1.35x C
PHP          (off scale)                                    146.39 ms  10.06x C
Python       (off scale)                                    701.86 ms  48.24x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 13.09 ms
  (0.90x C's geomean, 0.91x Rust's).
- **Salam-t is fastest or tied-fastest on the most programs** (
  21/42); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.03x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 10-48x slower** than the compiled languages on
  this suite and are included for scale only.
- "Fastest / tied-fastest on" counts ties on every side (e.g. a 4-way tie
  adds 1 to all 4 languages), so the column does not sum to 42.

## Per-program results (minimum ms)

| Program | C | C++ | Rust | Go | Salam-l | Salam-g | Salam-t | PHP | Python | Fastest |
|---|---|---|---|---|---|---|---|---|---|---|
| 01_fib_recursive | 3 | 4 | 4 | 7 | 4 | 3 | 4 | 49 | 86 | **C / Salam-g (3)** |
| 02_fib_iterative | 20 | 21 | 21 | 21 | 21 | 20 | 21 | 62 | 575 | **C / Salam-g (20)** |
| 03_primes_count | 6 | 6 | 4 | 7 | 4 | 6 | 4 | 18 | 147 | **Rust / Salam (4)** |
| 04_collatz | 6 | 6 | 5 | 8 | 5 | 6 | 5 | 61 | 326 | **Rust / Salam (5)** |
| 05_sum_mod | 12 | 13 | 13 | 14 | 13 | 13 | 12 | 29 | 371 | **C / Salam-t (12)** |
| 06_gcd_sum | 23 | 23 | 19 | 24 | 17 | 23 | 18 | 49 | 135 | **Salam-l (17)** |
| 07_pow_mod | 2 | 3 | 2 | 4 | 2 | 2 | 2 | 17 | 61 | **C / Rust / Salam (2)** |
| 08_digit_sum | 3 | 4 | 4 | 5 | 4 | 4 | 4 | 49 | 326 | C (3) |
| 09_perfect_numbers | 13 | 13 | 7 | 15 | 8 | 13 | 7 | 33 | 370 | **Rust / Salam-t (7)** |
| 10_pi_leibniz | 5 | 6 | 5 | 6 | 5 | 5 | 5 | 45 | 448 | **C / Rust / Salam (5)** |
| 11_loop_count | 6 | 7 | 6 | 7 | 7 | 6 | 6 | 13 | 98 | **C / Rust / Salam (6)** |
| 12_hello_print | 17 | 19 | 2 | 6 | 8 | 8 | 8 | 208 | 77 | Rust (2) |
| 13_array_rw | 23 | 24 | 24 | 24 | 25 | 21 | 24 | 69 | 852 | **Salam-g (21)** |
| 14_lcg_random | 21 | 20 | 23 | 28 | 22 | 22 | 23 | 87 | 1291 | C++ (20) |
| 15_matrix_mult | 5 | 6 | 4 | 10 | 4 | 5 | 4 | 170 | 1014 | **Rust / Salam (4)** |
| 16_quicksort | 30 | 29 | 29 | 29 | 25 | 31 | 27 | 162 | 492 | **Salam-l (25)** |
| 17_n_queens | 45 | 47 | 47 | 50 | 59 | 54 | 58 | 469 | 626 | C (45) |
| 18_sieve_eratosthenes | 9 | 10 | 9 | 11 | 9 | 13 | 9 | 113 | 908 | **C / Rust / Salam (9)** |
| 19_mandelbrot | 10 | 12 | 11 | 12 | 11 | 11 | 11 | 114 | 1068 | C (10) |
| 20_monte_carlo_pi | 32 | 33 | 19 | 38 | 17 | 32 | 14 | 348 | 3493 | **Salam-t (14)** |
| 21_coin_change_dp | 3 | 4 | 3 | 4 | 3 | 3 | 3 | 16 | 107 | **C / Rust / Salam (3)** |
| 22_knapsack_01 | 5 | 6 | 5 | 7 | 4 | 6 | 7 | 104 | 1257 | **Salam-l (4)** |
| 23_caesar_cipher | 14 | 15 | 14 | 19 | 14 | 14 | 13 | 101 | 1108 | **Salam-t (13)** |
| 24_merge_sort | 33 | 33 | 34 | 37 | 34 | 38 | 38 | 274 | 729 | C / C++ (33) |
| 25_palindrome_count | 14 | 15 | 13 | 17 | 13 | 13 | 13 | 222 | 1601 | **Rust / Salam (13)** |
| 26_ackermann | 1 | 2 | 2 | 3 | 2 | 1 | 1 | 9 | 20 | **C / Salam (1)** |
| 27_edit_distance | 7 | 7 | 7 | 16 | 6 | 6 | 5 | 223 | 2271 | **Salam-t (5)** |
| 28_prime_factorization | 1565 | 1574 | 959 | 1558 | 989 | 1544 | 953 | 4037 | 51082 | **Salam-t (953)** |
| 29_dot_product | 39 | 40 | 40 | 39 | 40 | 39 | 41 | 230 | 2591 | **C / Go / Salam-g (39)** |
| 30_heap_sort | 38 | 39 | 43 | 48 | 35 | 40 | 40 | 478 | 1012 | **Salam-l (35)** |
| 31_binary_search_stress | 1037 | 1020 | 701 | 1034 | 1000 | 1106 | 1114 | 3700 | 24319 | Rust (701) |
| 32_longest_increasing_subsequence | 15 | 16 | 27 | 13 | 11 | 15 | 12 | 255 | 2311 | **Salam-l (11)** |
| 33_subset_sum_reachability | 17 | 18 | 23 | 20 | 12 | 21 | 21 | 313 | 2305 | **Salam-l (12)** |
| 34_counting_sort | 38 | 38 | 29 | 43 | 28 | 37 | 28 | 223 | 2354 | **Salam (28)** |
| 35_run_length_stats | 8 | 9 | 6 | 9 | 6 | 8 | 6 | 105 | 806 | **Rust / Salam (6)** |
| 36_array_rotation_reversal | 21 | 24 | 25 | 26 | 24 | 22 | 23 | 215 | 873 | C (21) |
| 37_sqrt_decomposition | 9 | 11 | 9 | 18 | 8 | 10 | 9 | 304 | 1077 | **Salam-l (8)** |
| 38_game_of_life | 11 | 12 | 34 | 54 | 10 | 11 | 7 | 1039 | 7907 | **Salam-t (7)** |
| 39_trapezoidal_integration | 13 | 14 | 14 | 15 | 14 | 13 | 13 | 386 | 2336 | **C / Salam (13)** |
| 40_matrix_stdlib_matmul | 39 | 36 | 67 | 83 | 12 | 14 | 15 | 1398 | 133 | **Salam-l (12)** |
| 41_matrix_stdlib_smallops | 5 | 6 | 6 | 24 | 51 | 57 | 61 | 500 | 1337 | C (5) |
| 42_tensor_stdlib_matmul | 37 | 36 | 66 | 83 | 8 | 8 | 8 | 1369 | 124 | **Salam (8)** |

Bold marks rows where a Salam backend is the fastest or tied for fastest.

### Top 10 fastest programs in Salam-l

Where the LLVM backend (`-O3`) does best **relative to the fastest non-Salam
entry on the same program**: sorted by `vs best other` (Salam-l's minimum
divided by that language's), lowest ratio first. Under 1.00x means Salam-l
wins the row outright. Absolute ms are shown but are not the sort key, since
they mostly track how big each program's workload is.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 42_tensor_stdlib_matmul | 8 | C++ (36) | 0.22x |
| 2 | 40_matrix_stdlib_matmul | 12 | C++ (36) | 0.33x |
| 3 | 33_subset_sum_reachability | 12 | C (17) | 0.71x |
| 4 | 22_knapsack_01 | 4 | C (5) | 0.80x |
| 5 | 32_longest_increasing_subsequence | 11 | Go (13) | 0.85x |
| 6 | 27_edit_distance | 6 | C (7) | 0.86x |
| 7 | 16_quicksort | 25 | C++ (29) | 0.86x |
| 8 | 37_sqrt_decomposition | 8 | C (9) | 0.89x |
| 9 | 20_monte_carlo_pi | 17 | Rust (19) | 0.89x |
| 10 | 06_gcd_sum | 17 | Rust (19) | 0.89x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 51 | C (5) | 10.20x |
| 2 | 12_hello_print | 8 | Rust (2) | 4.00x |
| 3 | 26_ackermann | 2 | C (1) | 2.00x |
| 4 | 31_binary_search_stress | 1000 | Rust (701) | 1.43x |
| 5 | 01_fib_recursive | 4 | C (3) | 1.33x |
| 6 | 08_digit_sum | 4 | C (3) | 1.33x |
| 7 | 17_n_queens | 59 | C (45) | 1.31x |
| 8 | 11_loop_count | 7 | C (6) | 1.17x |
| 9 | 36_array_rotation_reversal | 24 | C (21) | 1.14x |
| 10 | 09_perfect_numbers | 8 | Rust (7) | 1.14x |
| 11 | 14_lcg_random | 22 | C++ (20) | 1.10x |
| 12 | 19_mandelbrot | 11 | C (10) | 1.10x |
| 13 | 13_array_rw | 25 | C (23) | 1.09x |
| 14 | 05_sum_mod | 13 | C (12) | 1.08x |
| 15 | 39_trapezoidal_integration | 14 | C (13) | 1.08x |
| 16 | 02_fib_iterative | 21 | C (20) | 1.05x |
| 17 | 28_prime_factorization | 989 | Rust (959) | 1.03x |
| 18 | 24_merge_sort | 34 | C (33) | 1.03x |
| 19 | 29_dot_product | 40 | C (39) | 1.03x |
| 20 | 23_caesar_cipher | 14 | C (14) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (16):** 06_gcd_sum, 13_array_rw, 16_quicksort, 20_monte_carlo_pi, 22_knapsack_01, 23_caesar_cipher, 27_edit_distance, 28_prime_factorization, 30_heap_sort, 32_longest_increasing_subsequence, 33_subset_sum_reachability, 34_counting_sort, 37_sqrt_decomposition, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (17):** 01_fib_recursive, 02_fib_iterative, 03_primes_count, 04_collatz, 05_sum_mod, 07_pow_mod, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 15_matrix_mult, 18_sieve_eratosthenes, 21_coin_change_dp, 25_palindrome_count, 26_ackermann, 29_dot_product, 35_run_length_stats, 39_trapezoidal_integration.
- **Trailing the fastest (9):** 08_digit_sum (+1 vs C); 12_hello_print (+6 vs Rust); 14_lcg_random (+2 vs C++); 17_n_queens (+9 vs C); 19_mandelbrot (+1 vs C); 24_merge_sort (+1 vs C/C++); 31_binary_search_stress (+299 vs Rust); 36_array_rotation_reversal (+1 vs C); 41_matrix_stdlib_smallops (+46 vs C).

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
