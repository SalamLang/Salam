# Salam Benchmark Suite Results

46 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured four ways: the LLVM backend at
`-O3`, the native x86-64 backend (`--backend=native`), the C backend compiled
by `gcc -O3`, and the default `tcc` toolchain (no optimizer) as the
out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-11-25T19:58:30Z (took 47 min)
- Commit: `d82cd3c9`

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
- **go**: go version go1.26.11 linux/amd64
- **tcc**: tcc version 0.9.28rc (x86_64 Linux)
- **clang**: Alpine clang version 22.1.3
- **python**: Python 3.14.10
- **numpy**: 2.4.6
- **php**: PHP 8.5.13 (cli) (built: Nov 20 2026 02:07:43) (NTS)
- **salam**: 0.7.1

## Overall ranking (fastest to slowest)

One number per language: the **geometric mean of minimum times across the
programs it completed** (geometric mean is used because it is not skewed by the few
slow/high-variance programs the way an arithmetic mean would be). `Programs`
is how many of the 46 went into that mean: a language that fails to build
one is averaged over the rest, so two rows with different counts are not
measuring the same workload, and a row with 0 has no result at all. Lower is
faster; `vs C` and `vs Salam` express the same geomean as a multiple of C's
and of the fastest Salam backend's (Salam-l). Salam's four variants are
suffixed **-g** (C backend, `gcc -O3`), **-l** (LLVM backend, `-O3`), **-n**
(native x86-64 backend, `--release`), and **-t** (C backend, `tcc`, no
optimizer, the out-of-the-box default).

| Rank | Language | Geomean | vs C | vs Salam | Programs | Fastest / tied-fastest on |
|---|---|---|---|---|---|---|
| 1 | **Salam-l** | 14.75 ms | 0.90x | 1.00x | 42 / 46 | 21 / 46 |
| 2 | **Salam-t** | 14.79 ms | 0.91x | 1.00x | 42 / 46 | 24 / 46 |
| 3 | **Salam-g** | 15.59 ms | 0.95x | 1.06x | 42 / 46 | 21 / 46 |
| 4 | **Salam-n** | 15.85 ms | 0.97x | 1.08x | 42 / 46 | 12 / 46 |
| 5 | C | 16.33 ms | 1.00x | 1.11x | 42 / 46 | 21 / 46 |
| 6 | Rust | 16.36 ms | 1.00x | 1.11x | 42 / 46 | 16 / 46 |
| 7 | C++ | 17.88 ms | 1.10x | 1.21x | 42 / 46 | 2 / 46 |
| 8 | Go | 21.32 ms | 1.31x | 1.45x | 42 / 46 | 1 / 46 |
| 9 | PHP | 191.73 ms | 11.74x | 13.00x | 42 / 46 | 0 / 46 |
| 10 | Python | 968.01 ms | 59.29x | 65.64x | 42 / 46 | 0 / 46 |

```
Salam-l      ███████████████████████████████                 14.75 ms  0.90x C
Salam-t      ███████████████████████████████                 14.79 ms  0.91x C
Salam-g      █████████████████████████████████               15.59 ms  0.95x C
Salam-n      █████████████████████████████████               15.85 ms  0.97x C
C            ██████████████████████████████████              16.33 ms  1.00x C  <- baseline
Rust         ███████████████████████████████████             16.36 ms  1.00x C
C++          ██████████████████████████████████████          17.88 ms  1.10x C
Go           █████████████████████████████████████████████   21.32 ms  1.31x C
PHP          (off scale)                                    191.73 ms  11.74x C
Python       (off scale)                                    968.01 ms  59.29x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 14.75 ms
  (0.90x C's geomean, 0.90x Rust's).
- **Salam-t is fastest or tied-fastest on the most programs** (
  24/46); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.00x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 12-59x slower** than the compiled languages on
  this suite and are included for scale only.
- "Fastest / tied-fastest on" counts ties on every side (e.g. a 4-way tie
  adds 1 to all 4 languages), so the column does not sum to 46.

## Failures

40 program/language pairs produced no timing. They are left out of that
language's geomean, which is why the `Programs` column in the ranking is
worth reading before comparing two rows. Full compiler output for each one
is in `results.json`.

| Program | Language | Status |
|---|---|---|
| 43_vector_collections | C | build failed |
| 43_vector_collections | C++ | build failed |
| 43_vector_collections | Rust | build failed |
| 43_vector_collections | Go | build failed |
| 43_vector_collections | Salam-l | output mismatch |
| 43_vector_collections | Salam-n | output mismatch |
| 43_vector_collections | Salam-g | output mismatch |
| 43_vector_collections | Salam-t | output mismatch |
| 43_vector_collections | PHP | output mismatch |
| 43_vector_collections | Python | output mismatch |
| 44_hashmap_collections | C | build failed |
| 44_hashmap_collections | C++ | build failed |
| 44_hashmap_collections | Rust | build failed |
| 44_hashmap_collections | Go | build failed |
| 44_hashmap_collections | Salam-l | output mismatch |
| 44_hashmap_collections | Salam-n | output mismatch |
| 44_hashmap_collections | Salam-g | output mismatch |
| 44_hashmap_collections | Salam-t | output mismatch |
| 44_hashmap_collections | PHP | output mismatch |
| 44_hashmap_collections | Python | output mismatch |
| 45_hashmap_string_keys | C | build failed |
| 45_hashmap_string_keys | C++ | build failed |
| 45_hashmap_string_keys | Rust | build failed |
| 45_hashmap_string_keys | Go | build failed |
| 45_hashmap_string_keys | Salam-l | output mismatch |
| 45_hashmap_string_keys | Salam-n | output mismatch |
| 45_hashmap_string_keys | Salam-g | output mismatch |
| 45_hashmap_string_keys | Salam-t | output mismatch |
| 45_hashmap_string_keys | PHP | output mismatch |
| 45_hashmap_string_keys | Python | output mismatch |

...and 10 more, in `results.json`.

## Per-program results (minimum ms)

| Program | C | C++ | Rust | Go | Salam-l | Salam-n | Salam-g | Salam-t | PHP | Python | Fastest |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 01_fib_recursive | 4 | 5 | 4 | 8 | 4 | 4 | 3 | 4 | 70 | 117 | **Salam-g (3)** |
| 02_fib_iterative | 21 | 22 | 21 | 23 | 21 | 22 | 21 | 21 | 64 | 826 | **C / Rust / Salam (21)** |
| 03_primes_count | 5 | 6 | 5 | 6 | 5 | 5 | 5 | 5 | 24 | 198 | **C / Rust / Salam (5)** |
| 04_collatz | 5 | 7 | 5 | 8 | 5 | 5 | 5 | 5 | 72 | 439 | **C / Rust / Salam (5)** |
| 05_sum_mod | 13 | 14 | 14 | 15 | 14 | 14 | 13 | 14 | 37 | 532 | **C / Salam-g (13)** |
| 06_gcd_sum | 17 | 18 | 16 | 18 | 16 | 16 | 17 | 16 | 58 | 174 | **Rust / Salam (16)** |
| 07_pow_mod | 3 | 4 | 4 | 5 | 3 | 4 | 3 | 3 | 27 | 79 | **C / Salam (3)** |
| 08_digit_sum | 4 | 5 | 5 | 6 | 5 | 4 | 4 | 4 | 79 | 452 | **C / Salam (4)** |
| 09_perfect_numbers | 10 | 11 | 9 | 11 | 9 | 9 | 10 | 9 | 46 | 497 | **Rust / Salam (9)** |
| 10_pi_leibniz | 6 | 7 | 6 | 7 | 6 | 6 | 6 | 6 | 57 | 738 | **C / Rust / Salam (6)** |
| 11_loop_count | 6 | 7 | 7 | 7 | 7 | 6 | 6 | 7 | 17 | 153 | **C / Salam (6)** |
| 12_hello_print | 22 | 22 | 4 | 9 | 3 | 4 | 3 | 3 | 461 | 121 | **Salam (3)** |
| 13_array_rw | 28 | 28 | 28 | 30 | 28 | 29 | 26 | 29 | 96 | 1253 | **Salam-g (26)** |
| 14_lcg_random | 21 | 22 | 23 | 27 | 23 | 22 | 21 | 22 | 83 | 1670 | **C / Salam-g (21)** |
| 15_matrix_mult | 8 | 9 | 7 | 14 | 7 | 12 | 8 | 7 | 211 | 1423 | **Rust / Salam (7)** |
| 16_quicksort | 27 | 28 | 26 | 30 | 27 | 28 | 28 | 28 | 192 | 667 | Rust (26) |
| 17_n_queens | 42 | 43 | 42 | 46 | 45 | 49 | 47 | 44 | 576 | 794 | C / Rust (42) |
| 18_sieve_eratosthenes | 7 | 8 | 8 | 11 | 8 | 8 | 7 | 8 | 139 | 1145 | **C / Salam-g (7)** |
| 19_mandelbrot | 12 | 13 | 12 | 14 | 12 | 12 | 12 | 12 | 157 | 1658 | **C / Rust / Salam (12)** |
| 20_monte_carlo_pi | 31 | 32 | 17 | 37 | 16 | 28 | 31 | 12 | 386 | 4630 | **Salam-t (12)** |
| 21_coin_change_dp | 3 | 4 | 4 | 5 | 4 | 4 | 4 | 4 | 20 | 151 | C (3) |
| 22_knapsack_01 | 6 | 7 | 5 | 8 | 7 | 7 | 6 | 6 | 134 | 1752 | Rust (5) |
| 23_caesar_cipher | 17 | 18 | 14 | 24 | 14 | 15 | 19 | 13 | 106 | 1414 | **Salam-t (13)** |
| 24_merge_sort | 33 | 33 | 34 | 38 | 32 | 34 | 36 | 37 | 328 | 1047 | **Salam-l (32)** |
| 25_palindrome_count | 15 | 16 | 15 | 18 | 14 | 15 | 14 | 14 | 374 | 2155 | **Salam (14)** |
| 26_ackermann | 2 | 3 | 2 | 4 | 2 | 2 | 2 | 2 | 13 | 29 | **C / Rust / Salam (2)** |
| 27_edit_distance | 8 | 9 | 10 | 13 | 7 | 9 | 8 | 8 | 330 | 3150 | **Salam-l (7)** |
| 28_prime_factorization | 1118 | 1121 | 971 | 1109 | 971 | 958 | 1119 | 971 | 6233 | 69824 | **Salam-n (958)** |
| 29_dot_product | 40 | 41 | 43 | 43 | 43 | 44 | 40 | 41 | 275 | 3676 | **C / Salam-g (40)** |
| 30_heap_sort | 37 | 37 | 40 | 48 | 39 | 40 | 37 | 39 | 629 | 1291 | **C / C++ / Salam-g (37)** |
| 31_binary_search_stress | 1008 | 1009 | 909 | 991 | 977 | 1012 | 1013 | 972 | 5405 | 30332 | Rust (909) |
| 32_longest_increasing_subsequence | 15 | 16 | 12 | 13 | 12 | 17 | 16 | 12 | 314 | 3172 | **Rust / Salam (12)** |
| 33_subset_sum_reachability | 19 | 21 | 21 | 19 | 19 | 21 | 21 | 19 | 383 | 3400 | **C / Go / Salam (19)** |
| 34_counting_sort | 42 | 42 | 32 | 44 | 32 | 40 | 43 | 32 | 323 | 3183 | **Rust / Salam (32)** |
| 35_run_length_stats | 9 | 10 | 8 | 10 | 8 | 9 | 10 | 8 | 154 | 1044 | **Rust / Salam (8)** |
| 36_array_rotation_reversal | 36 | 39 | 38 | 40 | 36 | 37 | 36 | 36 | 265 | 1278 | **C / Salam (36)** |
| 37_sqrt_decomposition | 17 | 17 | 18 | 37 | 18 | 17 | 19 | 17 | 377 | 1520 | **C / C++ / Salam (17)** |
| 38_game_of_life | 12 | 13 | 38 | 59 | 11 | 14 | 11 | 10 | 1369 | 10608 | **Salam-t (10)** |
| 39_trapezoidal_integration | 15 | 16 | 16 | 18 | 16 | 15 | 15 | 15 | 570 | 3789 | **C / Salam (15)** |
| 40_matrix_stdlib_matmul | 54 | 55 | 93 | 91 | 15 | 16 | 15 | 15 | 1617 | 156 | **Salam (15)** |
| 41_matrix_stdlib_smallops | 6 | 7 | 8 | 23 | 51 | 58 | 63 | 68 | 609 | 1895 | C (6) |
| 42_tensor_stdlib_matmul | 54 | 55 | 93 | 90 | 7 | 8 | 8 | 8 | 1621 | 155 | **Salam-l (7)** |
| 43_vector_collections | - | - | - | - | - | - | - | - | - | - |  (-1) |
| 44_hashmap_collections | - | - | - | - | - | - | - | - | - | - |  (-1) |
| 45_hashmap_string_keys | - | - | - | - | - | - | - | - | - | - |  (-1) |
| 46_hashmap_string_keys_prealloc | - | - | - | - | - | - | - | - | - | - |  (-1) |

Bold marks rows where a Salam backend is the fastest or tied for fastest.

### Top 10 fastest programs in Salam-l

Where the LLVM backend (`-O3`) does best **relative to the fastest non-Salam
entry on the same program**: sorted by `vs best other` (Salam-l's minimum
divided by that language's), lowest ratio first. Under 1.00x means Salam-l
wins the row outright. Absolute ms are shown but are not the sort key, since
they mostly track how big each program's workload is.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 42_tensor_stdlib_matmul | 7 | C (54) | 0.13x |
| 2 | 40_matrix_stdlib_matmul | 15 | C (54) | 0.28x |
| 3 | 12_hello_print | 3 | Rust (4) | 0.75x |
| 4 | 27_edit_distance | 7 | C (8) | 0.88x |
| 5 | 38_game_of_life | 11 | C (12) | 0.92x |
| 6 | 25_palindrome_count | 14 | C (15) | 0.93x |
| 7 | 20_monte_carlo_pi | 16 | Rust (17) | 0.94x |
| 8 | 24_merge_sort | 32 | C (33) | 0.97x |
| 9 | 26_ackermann | 2 | C (2) | 1.00x |
| 10 | 07_pow_mod | 3 | C (3) | 1.00x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 51 | C (6) | 8.50x |
| 2 | 22_knapsack_01 | 7 | Rust (5) | 1.40x |
| 3 | 21_coin_change_dp | 4 | C (3) | 1.33x |
| 4 | 08_digit_sum | 5 | C (4) | 1.25x |
| 5 | 11_loop_count | 7 | C (6) | 1.17x |
| 6 | 18_sieve_eratosthenes | 8 | C (7) | 1.14x |
| 7 | 14_lcg_random | 23 | C (21) | 1.10x |
| 8 | 05_sum_mod | 14 | C (13) | 1.08x |
| 9 | 29_dot_product | 43 | C (40) | 1.08x |
| 10 | 31_binary_search_stress | 977 | Rust (909) | 1.07x |
| 11 | 17_n_queens | 45 | C (42) | 1.07x |
| 12 | 39_trapezoidal_integration | 16 | C (15) | 1.07x |
| 13 | 37_sqrt_decomposition | 18 | C (17) | 1.06x |
| 14 | 30_heap_sort | 39 | C (37) | 1.05x |
| 15 | 16_quicksort | 27 | Rust (26) | 1.04x |
| 16 | 28_prime_factorization | 971 | Rust (971) | 1.00x |
| 17 | 36_array_rotation_reversal | 36 | C (36) | 1.00x |
| 18 | 34_counting_sort | 32 | Rust (32) | 1.00x |
| 19 | 13_array_rw | 28 | C (28) | 1.00x |
| 20 | 02_fib_iterative | 21 | C (21) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (12):** 01_fib_recursive, 12_hello_print, 13_array_rw, 20_monte_carlo_pi, 23_caesar_cipher, 24_merge_sort, 25_palindrome_count, 27_edit_distance, 28_prime_factorization, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (24):** 02_fib_iterative, 03_primes_count, 04_collatz, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 14_lcg_random, 15_matrix_mult, 18_sieve_eratosthenes, 19_mandelbrot, 26_ackermann, 29_dot_product, 30_heap_sort, 32_longest_increasing_subsequence, 33_subset_sum_reachability, 34_counting_sort, 35_run_length_stats, 36_array_rotation_reversal, 37_sqrt_decomposition, 39_trapezoidal_integration.
- **Trailing the fastest (6):** 16_quicksort (+1 vs Rust); 17_n_queens (+2 vs C/Rust); 21_coin_change_dp (+1 vs C); 22_knapsack_01 (+1 vs Rust); 31_binary_search_stress (+63 vs Rust); 41_matrix_stdlib_smallops (+45 vs C).

Salam matches or beats the fastest other compiled language on 36 of 46 programs.

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
