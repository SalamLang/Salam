# Salam Benchmark Suite Results

42 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured four ways: the LLVM backend at
`-O3`, the native x86-64 backend (`--backend=native`), the C backend compiled
by `gcc -O3`, and the default `tcc` toolchain (no optimizer) as the
out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-10-30T03:04:06Z (took 47 min)
- Commit: `933588e`

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
| 1 | **Salam-n** | 13.92 ms | 0.85x | 1.00x | 42 / 42 | 30 / 42 |
| 2 | **Salam-l** | 14.70 ms | 0.90x | 1.06x | 42 / 42 | 16 / 42 |
| 3 | **Salam-t** | 15.11 ms | 0.92x | 1.09x | 42 / 42 | 15 / 42 |
| 4 | **Salam-g** | 15.90 ms | 0.97x | 1.14x | 42 / 42 | 16 / 42 |
| 5 | C | 16.35 ms | 1.00x | 1.18x | 42 / 42 | 18 / 42 |
| 6 | Rust | 16.37 ms | 1.00x | 1.18x | 42 / 42 | 15 / 42 |
| 7 | C++ | 17.90 ms | 1.09x | 1.29x | 42 / 42 | 2 / 42 |
| 8 | Go | 21.34 ms | 1.30x | 1.53x | 42 / 42 | 0 / 42 |
| 9 | PHP | 192.26 ms | 11.76x | 13.82x | 42 / 42 | 0 / 42 |
| 10 | Python | 973.57 ms | 59.54x | 69.96x | 42 / 42 | 0 / 42 |

```
Salam-n      █████████████████████████████                   13.92 ms  0.85x C
Salam-l      ███████████████████████████████                 14.70 ms  0.90x C
Salam-t      ████████████████████████████████                15.11 ms  0.92x C
Salam-g      ██████████████████████████████████              15.90 ms  0.97x C
C            ██████████████████████████████████              16.35 ms  1.00x C  <- baseline
Rust         ███████████████████████████████████             16.37 ms  1.00x C
C++          ██████████████████████████████████████          17.90 ms  1.09x C
Go           █████████████████████████████████████████████   21.34 ms  1.30x C
PHP          (off scale)                                    192.26 ms  11.76x C
Python       (off scale)                                    973.57 ms  59.54x C
```

Takeaways:

- **Fastest Salam backend:** Salam-n, geomean 13.92 ms
  (0.85x C's geomean, 0.85x Rust's).
- **Salam-n is fastest or tied-fastest on the most programs** (
  30/42); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.09x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 12-60x slower** than the compiled languages on
  this suite and are included for scale only.
- "Fastest / tied-fastest on" counts ties on every side (e.g. a 4-way tie
  adds 1 to all 4 languages), so the column does not sum to 42.

## Per-program results (minimum ms)

| Program | C | C++ | Rust | Go | Salam-l | Salam-n | Salam-g | Salam-t | PHP | Python | Fastest |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 01_fib_recursive | 4 | 5 | 4 | 7 | 5 | 4 | 4 | 5 | 68 | 112 | **C / Rust / Salam (4)** |
| 02_fib_iterative | 21 | 22 | 21 | 23 | 21 | 16 | 21 | 21 | 66 | 838 | **Salam-n (16)** |
| 03_primes_count | 5 | 6 | 5 | 6 | 5 | 5 | 5 | 5 | 24 | 199 | **C / Rust / Salam (5)** |
| 04_collatz | 5 | 7 | 5 | 8 | 5 | 5 | 5 | 5 | 76 | 442 | **C / Rust / Salam (5)** |
| 05_sum_mod | 13 | 14 | 14 | 14 | 14 | 11 | 13 | 14 | 37 | 527 | **Salam-n (11)** |
| 06_gcd_sum | 17 | 18 | 16 | 18 | 16 | 16 | 17 | 16 | 64 | 173 | **Rust / Salam (16)** |
| 07_pow_mod | 3 | 4 | 4 | 5 | 3 | 3 | 3 | 3 | 26 | 79 | **C / Salam (3)** |
| 08_digit_sum | 4 | 5 | 5 | 6 | 5 | 4 | 4 | 5 | 79 | 451 | **C / Salam (4)** |
| 09_perfect_numbers | 10 | 11 | 9 | 11 | 9 | 8 | 10 | 9 | 46 | 496 | **Salam-n (8)** |
| 10_pi_leibniz | 6 | 7 | 6 | 7 | 6 | 6 | 6 | 6 | 59 | 737 | **C / Rust / Salam (6)** |
| 11_loop_count | 6 | 7 | 7 | 7 | 7 | 6 | 6 | 7 | 17 | 155 | **C / Salam (6)** |
| 12_hello_print | 22 | 22 | 4 | 9 | 3 | 3 | 3 | 3 | 458 | 125 | **Salam (3)** |
| 13_array_rw | 28 | 29 | 29 | 31 | 29 | 29 | 26 | 29 | 97 | 1244 | **Salam-g (26)** |
| 14_lcg_random | 21 | 22 | 23 | 27 | 23 | 16 | 21 | 23 | 84 | 1679 | **Salam-n (16)** |
| 15_matrix_mult | 8 | 9 | 7 | 14 | 7 | 7 | 8 | 7 | 208 | 1430 | **Rust / Salam (7)** |
| 16_quicksort | 27 | 28 | 26 | 30 | 27 | 27 | 29 | 28 | 190 | 691 | Rust (26) |
| 17_n_queens | 42 | 43 | 42 | 46 | 45 | 46 | 49 | 44 | 568 | 808 | C / Rust (42) |
| 18_sieve_eratosthenes | 7 | 8 | 8 | 11 | 8 | 7 | 10 | 8 | 139 | 1140 | **C / Salam-n (7)** |
| 19_mandelbrot | 12 | 13 | 12 | 14 | 12 | 12 | 12 | 12 | 157 | 1652 | **C / Rust / Salam (12)** |
| 20_monte_carlo_pi | 31 | 32 | 16 | 37 | 16 | 15 | 34 | 12 | 385 | 4693 | **Salam-t (12)** |
| 21_coin_change_dp | 3 | 4 | 4 | 5 | 4 | 4 | 4 | 4 | 21 | 152 | C (3) |
| 22_knapsack_01 | 6 | 7 | 5 | 8 | 6 | 5 | 6 | 7 | 135 | 1770 | **Rust / Salam-n (5)** |
| 23_caesar_cipher | 17 | 18 | 14 | 24 | 13 | 11 | 20 | 14 | 107 | 1445 | **Salam-n (11)** |
| 24_merge_sort | 33 | 33 | 34 | 38 | 34 | 35 | 38 | 38 | 333 | 1065 | C / C++ (33) |
| 25_palindrome_count | 15 | 16 | 15 | 19 | 15 | 15 | 15 | 15 | 367 | 2157 | **C / Rust / Salam (15)** |
| 26_ackermann | 2 | 3 | 2 | 4 | 2 | 2 | 2 | 2 | 13 | 29 | **C / Rust / Salam (2)** |
| 27_edit_distance | 8 | 9 | 9 | 12 | 7 | 8 | 8 | 8 | 331 | 3138 | **Salam-l (7)** |
| 28_prime_factorization | 1118 | 1119 | 970 | 1106 | 971 | 954 | 1120 | 968 | 6277 | 69496 | **Salam-n (954)** |
| 29_dot_product | 40 | 41 | 43 | 43 | 43 | 33 | 40 | 43 | 279 | 3711 | **Salam-n (33)** |
| 30_heap_sort | 37 | 37 | 40 | 48 | 37 | 37 | 37 | 40 | 627 | 1342 | **C / C++ / Salam (37)** |
| 31_binary_search_stress | 1011 | 1011 | 911 | 995 | 914 | 924 | 1036 | 982 | 5778 | 31137 | Rust (911) |
| 32_longest_increasing_subsequence | 15 | 16 | 12 | 14 | 12 | 12 | 16 | 12 | 314 | 3123 | **Rust / Salam (12)** |
| 33_subset_sum_reachability | 19 | 20 | 21 | 19 | 14 | 14 | 21 | 21 | 392 | 3313 | **Salam (14)** |
| 34_counting_sort | 42 | 42 | 32 | 45 | 32 | 26 | 43 | 32 | 324 | 3190 | **Salam-n (26)** |
| 35_run_length_stats | 9 | 10 | 8 | 10 | 8 | 8 | 10 | 8 | 152 | 1051 | **Rust / Salam (8)** |
| 36_array_rotation_reversal | 36 | 39 | 38 | 39 | 37 | 37 | 37 | 36 | 264 | 1311 | **C / Salam-t (36)** |
| 37_sqrt_decomposition | 18 | 18 | 18 | 38 | 18 | 17 | 19 | 18 | 364 | 1580 | **Salam-n (17)** |
| 38_game_of_life | 12 | 13 | 38 | 58 | 11 | 7 | 11 | 10 | 1420 | 10561 | **Salam-n (7)** |
| 39_trapezoidal_integration | 15 | 16 | 16 | 18 | 16 | 15 | 15 | 15 | 535 | 3752 | **C / Salam (15)** |
| 40_matrix_stdlib_matmul | 54 | 55 | 93 | 90 | 16 | 16 | 15 | 20 | 1597 | 156 | **Salam-g (15)** |
| 41_matrix_stdlib_smallops | 6 | 7 | 8 | 23 | 51 | 91 | 62 | 70 | 611 | 1948 | C (6) |
| 42_tensor_stdlib_matmul | 53 | 55 | 93 | 91 | 9 | 10 | 9 | 10 | 1597 | 155 | **Salam (9)** |

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
| 2 | 40_matrix_stdlib_matmul | 16 | C (54) | 0.30x |
| 3 | 33_subset_sum_reachability | 14 | C (19) | 0.74x |
| 4 | 12_hello_print | 3 | Rust (4) | 0.75x |
| 5 | 27_edit_distance | 7 | C (8) | 0.88x |
| 6 | 38_game_of_life | 11 | C (12) | 0.92x |
| 7 | 23_caesar_cipher | 13 | Rust (14) | 0.93x |
| 8 | 26_ackermann | 2 | C (2) | 1.00x |
| 9 | 07_pow_mod | 3 | C (3) | 1.00x |
| 10 | 04_collatz | 5 | C (5) | 1.00x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 51 | C (6) | 8.50x |
| 2 | 21_coin_change_dp | 4 | C (3) | 1.33x |
| 3 | 01_fib_recursive | 5 | C (4) | 1.25x |
| 4 | 08_digit_sum | 5 | C (4) | 1.25x |
| 5 | 22_knapsack_01 | 6 | Rust (5) | 1.20x |
| 6 | 11_loop_count | 7 | C (6) | 1.17x |
| 7 | 18_sieve_eratosthenes | 8 | C (7) | 1.14x |
| 8 | 14_lcg_random | 23 | C (21) | 1.10x |
| 9 | 05_sum_mod | 14 | C (13) | 1.08x |
| 10 | 29_dot_product | 43 | C (40) | 1.08x |
| 11 | 17_n_queens | 45 | C (42) | 1.07x |
| 12 | 39_trapezoidal_integration | 16 | C (15) | 1.07x |
| 13 | 16_quicksort | 27 | Rust (26) | 1.04x |
| 14 | 13_array_rw | 29 | C (28) | 1.04x |
| 15 | 24_merge_sort | 34 | C (33) | 1.03x |
| 16 | 36_array_rotation_reversal | 37 | C (36) | 1.03x |
| 17 | 31_binary_search_stress | 914 | Rust (911) | 1.00x |
| 18 | 28_prime_factorization | 971 | Rust (970) | 1.00x |
| 19 | 30_heap_sort | 37 | C (37) | 1.00x |
| 20 | 34_counting_sort | 32 | Rust (32) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (17):** 02_fib_iterative, 05_sum_mod, 09_perfect_numbers, 12_hello_print, 13_array_rw, 14_lcg_random, 20_monte_carlo_pi, 23_caesar_cipher, 27_edit_distance, 28_prime_factorization, 29_dot_product, 33_subset_sum_reachability, 34_counting_sort, 37_sqrt_decomposition, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (19):** 01_fib_recursive, 03_primes_count, 04_collatz, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 10_pi_leibniz, 11_loop_count, 15_matrix_mult, 18_sieve_eratosthenes, 19_mandelbrot, 22_knapsack_01, 25_palindrome_count, 26_ackermann, 30_heap_sort, 32_longest_increasing_subsequence, 35_run_length_stats, 36_array_rotation_reversal, 39_trapezoidal_integration.
- **Trailing the fastest (6):** 16_quicksort (+1 vs Rust); 17_n_queens (+2 vs C/Rust); 21_coin_change_dp (+1 vs C); 24_merge_sort (+1 vs C/C++); 31_binary_search_stress (+3 vs Rust); 41_matrix_stdlib_smallops (+45 vs C).

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
