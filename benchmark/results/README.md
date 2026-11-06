# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured four ways: the LLVM backend at
`-O3`, the native x86-64 backend (`--backend=native`), the C backend compiled
by `gcc -O3`, and the default `tcc` toolchain (no optimizer) as the
out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-11-06T21:58:06Z (took 43 min)
- Commit: `d683b2a9`

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

- **CPU**: Intel(R) Xeon(R) Platinum 8370C CPU @ 2.80GHz
- **gcc**: gcc (Alpine 15.2.0) 15.2.0
- **g++**: g++ (Alpine 15.2.0) 15.2.0
- **rustc**: rustc 1.96.1 (31fca3adb 2026-06-26) (Alpine Linux Rust 1.96.1-r0)
- **go**: go version go1.26.10 linux/amd64
- **tcc**: tcc version 0.9.28rc (x86_64 Linux)
- **clang**: Alpine clang version 22.1.3
- **python**: Python 3.14.10
- **numpy**: 2.4.6
- **php**: PHP 8.5.12 (cli) (built: Oct 21 2026 16:29:26) (NTS)
- **salam**: 0.6.2

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
| 1 | **Salam-n** | 12.40 ms | 0.81x | 1.00x | 42 / 42 | 31 / 42 |
| 2 | **Salam-l** | 13.42 ms | 0.87x | 1.08x | 42 / 42 | 19 / 42 |
| 3 | **Salam-t** | 14.33 ms | 0.93x | 1.16x | 42 / 42 | 10 / 42 |
| 4 | Rust | 15.17 ms | 0.99x | 1.22x | 42 / 42 | 11 / 42 |
| 5 | **Salam-g** | 15.27 ms | 1.00x | 1.23x | 42 / 42 | 11 / 42 |
| 6 | C | 15.34 ms | 1.00x | 1.24x | 42 / 42 | 11 / 42 |
| 7 | C++ | 16.94 ms | 1.10x | 1.37x | 42 / 42 | 2 / 42 |
| 8 | Go | 20.84 ms | 1.36x | 1.68x | 42 / 42 | 0 / 42 |
| 9 | PHP | 177.83 ms | 11.59x | 14.34x | 42 / 42 | 0 / 42 |
| 10 | Python | 857.95 ms | 55.91x | 69.17x | 42 / 42 | 0 / 42 |

```
Salam-n      ███████████████████████████                     12.40 ms  0.81x C
Salam-l      █████████████████████████████                   13.42 ms  0.87x C
Salam-t      ███████████████████████████████                 14.33 ms  0.93x C
Rust         █████████████████████████████████               15.17 ms  0.99x C
Salam-g      █████████████████████████████████               15.27 ms  1.00x C
C            █████████████████████████████████               15.34 ms  1.00x C  <- baseline
C++          █████████████████████████████████████           16.94 ms  1.10x C
Go           █████████████████████████████████████████████   20.84 ms  1.36x C
PHP          (off scale)                                    177.83 ms  11.59x C
Python       (off scale)                                    857.95 ms  55.91x C
```

Takeaways:

- **Fastest Salam backend:** Salam-n, geomean 12.40 ms
  (0.81x C's geomean, 0.82x Rust's).
- **Salam-n is fastest or tied-fastest on the most programs** (
  31/42); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.16x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 12-56x slower** than the compiled languages on
  this suite and are included for scale only.
- "Fastest / tied-fastest on" counts ties on every side (e.g. a 4-way tie
  adds 1 to all 4 languages), so the column does not sum to 42.

## Per-program results (minimum ms)

| Program | C | C++ | Rust | Go | Salam-l | Salam-n | Salam-g | Salam-t | PHP | Python | Fastest |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 01_fib_recursive | 3 | 4 | 4 | 7 | 4 | 3 | 3 | 4 | 66 | 105 | **C / Salam (3)** |
| 02_fib_iterative | 21 | 22 | 20 | 20 | 20 | 15 | 19 | 20 | 75 | 734 | **Salam-n (15)** |
| 03_primes_count | 6 | 7 | 4 | 6 | 4 | 4 | 6 | 4 | 21 | 182 | **Rust / Salam (4)** |
| 04_collatz | 6 | 6 | 5 | 8 | 5 | 4 | 7 | 5 | 64 | 393 | **Salam-n (4)** |
| 05_sum_mod | 12 | 13 | 13 | 13 | 13 | 10 | 12 | 13 | 40 | 463 | **Salam-n (10)** |
| 06_gcd_sum | 22 | 23 | 18 | 23 | 17 | 17 | 22 | 17 | 56 | 162 | **Salam (17)** |
| 07_pow_mod | 2 | 3 | 3 | 4 | 2 | 2 | 2 | 2 | 20 | 71 | **C / Salam (2)** |
| 08_digit_sum | 4 | 5 | 4 | 5 | 4 | 4 | 4 | 4 | 64 | 406 | **C / Rust / Salam (4)** |
| 09_perfect_numbers | 12 | 13 | 8 | 13 | 8 | 7 | 12 | 8 | 40 | 454 | **Salam-n (7)** |
| 10_pi_leibniz | 6 | 6 | 6 | 6 | 5 | 5 | 6 | 5 | 58 | 582 | **Salam (5)** |
| 11_loop_count | 6 | 7 | 6 | 6 | 6 | 6 | 5 | 6 | 15 | 128 | **Salam-g (5)** |
| 12_hello_print | 18 | 18 | 2 | 8 | 2 | 3 | 4 | 4 | 204 | 97 | **Rust / Salam-l (2)** |
| 13_array_rw | 24 | 25 | 26 | 26 | 26 | 25 | 23 | 26 | 101 | 1073 | **Salam-g (23)** |
| 14_lcg_random | 19 | 20 | 20 | 25 | 20 | 14 | 19 | 20 | 91 | 1609 | **Salam-n (14)** |
| 15_matrix_mult | 7 | 7 | 4 | 12 | 4 | 4 | 7 | 5 | 227 | 1255 | **Rust / Salam (4)** |
| 16_quicksort | 28 | 29 | 27 | 29 | 26 | 27 | 29 | 27 | 181 | 595 | **Salam-l (26)** |
| 17_n_queens | 46 | 47 | 49 | 50 | 52 | 50 | 54 | 54 | 528 | 720 | C (46) |
| 18_sieve_eratosthenes | 10 | 11 | 10 | 13 | 10 | 10 | 13 | 10 | 177 | 1150 | **C / Rust / Salam (10)** |
| 19_mandelbrot | 14 | 15 | 14 | 15 | 14 | 15 | 14 | 14 | 119 | 1416 | **C / Rust / Salam (14)** |
| 20_monte_carlo_pi | 29 | 30 | 16 | 38 | 16 | 15 | 30 | 16 | 336 | 4460 | **Salam-n (15)** |
| 21_coin_change_dp | 3 | 4 | 3 | 4 | 3 | 3 | 3 | 3 | 19 | 125 | **C / Rust / Salam (3)** |
| 22_knapsack_01 | 5 | 6 | 5 | 8 | 5 | 4 | 6 | 7 | 132 | 1576 | **Salam-n (4)** |
| 23_caesar_cipher | 14 | 15 | 13 | 19 | 13 | 9 | 14 | 13 | 98 | 1369 | **Salam-n (9)** |
| 24_merge_sort | 33 | 33 | 34 | 38 | 33 | 33 | 36 | 36 | 327 | 862 | **C / C++ / Salam (33)** |
| 25_palindrome_count | 15 | 16 | 15 | 20 | 15 | 17 | 16 | 15 | 286 | 1963 | **C / Rust / Salam (15)** |
| 26_ackermann | 2 | 2 | 2 | 3 | 2 | 2 | 2 | 2 | 10 | 23 | **C / C++ / Rust / Salam (2)** |
| 27_edit_distance | 7 | 8 | 8 | 16 | 6 | 7 | 8 | 7 | 294 | 2862 | **Salam-l (6)** |
| 28_prime_factorization | 1472 | 1471 | 911 | 1460 | 910 | 894 | 1476 | 912 | 5303 | 63393 | **Salam-n (894)** |
| 29_dot_product | 38 | 39 | 40 | 40 | 40 | 27 | 38 | 40 | 303 | 3206 | **Salam-n (27)** |
| 30_heap_sort | 37 | 37 | 42 | 49 | 36 | 36 | 38 | 38 | 544 | 1182 | **Salam (36)** |
| 31_binary_search_stress | 1054 | 1065 | 818 | 1040 | 978 | 990 | 1046 | 1087 | 4400 | 28002 | Rust (818) |
| 32_longest_increasing_subsequence | 14 | 14 | 23 | 13 | 12 | 11 | 14 | 13 | 319 | 2898 | **Salam-n (11)** |
| 33_subset_sum_reachability | 16 | 17 | 25 | 20 | 12 | 11 | 19 | 22 | 358 | 2800 | **Salam-n (11)** |
| 34_counting_sort | 36 | 37 | 28 | 42 | 28 | 23 | 37 | 28 | 278 | 2914 | **Salam-n (23)** |
| 35_run_length_stats | 8 | 9 | 7 | 10 | 7 | 6 | 9 | 7 | 131 | 958 | **Salam-n (6)** |
| 36_array_rotation_reversal | 31 | 34 | 33 | 35 | 30 | 30 | 30 | 31 | 270 | 1016 | **Salam (30)** |
| 37_sqrt_decomposition | 15 | 16 | 15 | 36 | 15 | 14 | 16 | 15 | 407 | 1324 | **Salam-n (14)** |
| 38_game_of_life | 12 | 13 | 35 | 53 | 9 | 7 | 10 | 9 | 1174 | 9771 | **Salam-n (7)** |
| 39_trapezoidal_integration | 17 | 18 | 15 | 18 | 15 | 17 | 17 | 17 | 420 | 2932 | **Rust / Salam-l (15)** |
| 40_matrix_stdlib_matmul | 42 | 47 | 79 | 96 | 15 | 14 | 14 | 17 | 2141 | 139 | **Salam (14)** |
| 41_matrix_stdlib_smallops | 5 | 6 | 6 | 23 | 51 | 54 | 72 | 80 | 591 | 1754 | C (5) |
| 42_tensor_stdlib_matmul | 42 | 47 | 79 | 96 | 8 | 8 | 8 | 9 | 2177 | 140 | **Salam (8)** |

Bold marks rows where a Salam backend is the fastest or tied for fastest.

### Top 10 fastest programs in Salam-l

Where the LLVM backend (`-O3`) does best **relative to the fastest non-Salam
entry on the same program**: sorted by `vs best other` (Salam-l's minimum
divided by that language's), lowest ratio first. Under 1.00x means Salam-l
wins the row outright. Absolute ms are shown but are not the sort key, since
they mostly track how big each program's workload is.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 42_tensor_stdlib_matmul | 8 | C (42) | 0.19x |
| 2 | 40_matrix_stdlib_matmul | 15 | C (42) | 0.36x |
| 3 | 38_game_of_life | 9 | C (12) | 0.75x |
| 4 | 33_subset_sum_reachability | 12 | C (16) | 0.75x |
| 5 | 10_pi_leibniz | 5 | C (6) | 0.83x |
| 6 | 27_edit_distance | 6 | C (7) | 0.86x |
| 7 | 32_longest_increasing_subsequence | 12 | Go (13) | 0.92x |
| 8 | 06_gcd_sum | 17 | Rust (18) | 0.94x |
| 9 | 16_quicksort | 26 | Rust (27) | 0.96x |
| 10 | 36_array_rotation_reversal | 30 | C (31) | 0.97x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 51 | C (5) | 10.20x |
| 2 | 01_fib_recursive | 4 | C (3) | 1.33x |
| 3 | 31_binary_search_stress | 978 | Rust (818) | 1.20x |
| 4 | 17_n_queens | 52 | C (46) | 1.13x |
| 5 | 13_array_rw | 26 | C (24) | 1.08x |
| 6 | 05_sum_mod | 13 | C (12) | 1.08x |
| 7 | 29_dot_product | 40 | C (38) | 1.05x |
| 8 | 14_lcg_random | 20 | C (19) | 1.05x |
| 9 | 24_merge_sort | 33 | C (33) | 1.00x |
| 10 | 34_counting_sort | 28 | Rust (28) | 1.00x |
| 11 | 02_fib_iterative | 20 | Rust (20) | 1.00x |
| 12 | 20_monte_carlo_pi | 16 | Rust (16) | 1.00x |
| 13 | 39_trapezoidal_integration | 15 | Rust (15) | 1.00x |
| 14 | 37_sqrt_decomposition | 15 | C (15) | 1.00x |
| 15 | 25_palindrome_count | 15 | C (15) | 1.00x |
| 16 | 19_mandelbrot | 14 | C (14) | 1.00x |
| 17 | 23_caesar_cipher | 13 | Rust (13) | 1.00x |
| 18 | 18_sieve_eratosthenes | 10 | C (10) | 1.00x |
| 19 | 09_perfect_numbers | 8 | Rust (8) | 1.00x |
| 20 | 35_run_length_stats | 7 | Rust (7) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (26):** 02_fib_iterative, 04_collatz, 05_sum_mod, 06_gcd_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 13_array_rw, 14_lcg_random, 16_quicksort, 20_monte_carlo_pi, 22_knapsack_01, 23_caesar_cipher, 27_edit_distance, 28_prime_factorization, 29_dot_product, 30_heap_sort, 32_longest_increasing_subsequence, 33_subset_sum_reachability, 34_counting_sort, 35_run_length_stats, 36_array_rotation_reversal, 37_sqrt_decomposition, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (13):** 01_fib_recursive, 03_primes_count, 07_pow_mod, 08_digit_sum, 12_hello_print, 15_matrix_mult, 18_sieve_eratosthenes, 19_mandelbrot, 21_coin_change_dp, 24_merge_sort, 25_palindrome_count, 26_ackermann, 39_trapezoidal_integration.
- **Trailing the fastest (3):** 17_n_queens (+4 vs C); 31_binary_search_stress (+160 vs Rust); 41_matrix_stdlib_smallops (+46 vs C).

Salam matches or beats the fastest other compiled language on 39 of 42 programs.

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
