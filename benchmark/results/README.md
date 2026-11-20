# Salam Benchmark Suite Results

46 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured four ways: the LLVM backend at
`-O3`, the native x86-64 backend (`--backend=native`), the C backend compiled
by `gcc -O3`, and the default `tcc` toolchain (no optimizer) as the
out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-11-20T16:23:53Z (took 32 min)
- Commit: `837d1287`

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

- **CPU**: Intel(R) Xeon(R) 6975P-C
- **gcc**: gcc (Alpine 15.2.0) 15.2.0
- **g++**: g++ (Alpine 15.2.0) 15.2.0
- **rustc**: rustc 1.96.1 (31fca3adb 2026-06-26) (Alpine Linux Rust 1.96.1-r0)
- **go**: go version go1.26.11 linux/amd64
- **tcc**: tcc version 0.9.28rc (x86_64 Linux)
- **clang**: Alpine clang version 22.1.3
- **python**: Python 3.14.10
- **numpy**: 2.4.6
- **php**: PHP 8.5.12 (cli) (built: Oct 21 2026 16:29:26) (NTS)
- **salam**: 0.6.19

## Overall ranking (fastest to slowest)

One number per language: the **geometric mean of minimum times across the
programs it completed** (geometric mean is used because it is not skewed by the few
slow/high-variance programs the way an arithmetic mean would be). `Programs`
is how many of the 46 went into that mean: a language that fails to build
one is averaged over the rest, so two rows with different counts are not
measuring the same workload, and a row with 0 has no result at all. Lower is
faster; `vs C` and `vs Salam` express the same geomean as a multiple of C's
and of the fastest Salam backend's (Salam-n). Salam's four variants are
suffixed **-g** (C backend, `gcc -O3`), **-l** (LLVM backend, `-O3`), **-n**
(native x86-64 backend, `--release`), and **-t** (C backend, `tcc`, no
optimizer, the out-of-the-box default).

| Rank | Language | Geomean | vs C | vs Salam | Programs | Fastest / tied-fastest on |
|---|---|---|---|---|---|---|
| 1 | **Salam-n** | 10.56 ms | 0.83x | 1.00x | 42 / 46 | 29 / 46 |
| 2 | **Salam-l** | 11.30 ms | 0.89x | 1.07x | 42 / 46 | 19 / 46 |
| 3 | **Salam-t** | 11.93 ms | 0.93x | 1.13x | 42 / 46 | 14 / 46 |
| 4 | **Salam-g** | 12.71 ms | 1.00x | 1.20x | 42 / 46 | 10 / 46 |
| 5 | C | 12.76 ms | 1.00x | 1.21x | 42 / 46 | 14 / 46 |
| 6 | Rust | 12.86 ms | 1.01x | 1.22x | 42 / 46 | 14 / 46 |
| 7 | C++ | 14.06 ms | 1.10x | 1.33x | 42 / 46 | 2 / 46 |
| 8 | Go | 17.17 ms | 1.35x | 1.63x | 42 / 46 | 0 / 46 |
| 9 | PHP | 128.44 ms | 10.07x | 12.17x | 42 / 46 | 0 / 46 |
| 10 | Python | 587.87 ms | 46.07x | 55.68x | 42 / 46 | 0 / 46 |

```
Salam-n      ████████████████████████████                    10.56 ms  0.83x C
Salam-l      ██████████████████████████████                  11.30 ms  0.89x C
Salam-t      ███████████████████████████████                 11.93 ms  0.93x C
Salam-g      █████████████████████████████████               12.71 ms  1.00x C
C            █████████████████████████████████               12.76 ms  1.00x C  <- baseline
Rust         ██████████████████████████████████              12.86 ms  1.01x C
C++          █████████████████████████████████████           14.06 ms  1.10x C
Go           █████████████████████████████████████████████   17.17 ms  1.35x C
PHP          (off scale)                                    128.44 ms  10.07x C
Python       (off scale)                                    587.87 ms  46.07x C
```

Takeaways:

- **Fastest Salam backend:** Salam-n, geomean 10.56 ms
  (0.83x C's geomean, 0.82x Rust's).
- **Salam-n is fastest or tied-fastest on the most programs** (
  29/46); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.13x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 10-46x slower** than the compiled languages on
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
| 01_fib_recursive | 2 | 3 | 3 | 5 | 3 | 3 | 2 | 3 | 42 | 71 | **C / Salam-g (2)** |
| 02_fib_iterative | 17 | 18 | 18 | 18 | 18 | 13 | 17 | 18 | 51 | 497 | **Salam-n (13)** |
| 03_primes_count | 5 | 6 | 4 | 6 | 4 | 4 | 5 | 4 | 16 | 124 | **Rust / Salam (4)** |
| 04_collatz | 5 | 6 | 5 | 7 | 5 | 4 | 6 | 5 | 49 | 270 | **Salam-n (4)** |
| 05_sum_mod | 11 | 12 | 11 | 12 | 11 | 9 | 11 | 11 | 27 | 305 | **Salam-n (9)** |
| 06_gcd_sum | 20 | 21 | 16 | 21 | 16 | 16 | 20 | 16 | 43 | 115 | **Rust / Salam (16)** |
| 07_pow_mod | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 2 | 16 | 52 | **C / Rust / Salam (2)** |
| 08_digit_sum | 3 | 4 | 3 | 4 | 3 | 3 | 3 | 3 | 45 | 273 | **C / Rust / Salam (3)** |
| 09_perfect_numbers | 11 | 12 | 7 | 12 | 7 | 7 | 11 | 7 | 29 | 306 | **Rust / Salam (7)** |
| 10_pi_leibniz | 5 | 5 | 5 | 5 | 5 | 4 | 5 | 5 | 41 | 379 | **Salam-n (4)** |
| 11_loop_count | 5 | 6 | 5 | 6 | 6 | 5 | 5 | 6 | 12 | 87 | **C / Rust / Salam (5)** |
| 12_hello_print | 13 | 16 | 2 | 6 | 2 | 2 | 3 | 3 | 162 | 67 | **Rust / Salam (2)** |
| 13_array_rw | 22 | 24 | 24 | 24 | 24 | 23 | 21 | 23 | 66 | 719 | **Salam-g (21)** |
| 14_lcg_random | 17 | 18 | 19 | 22 | 19 | 13 | 17 | 19 | 70 | 1092 | **Salam-n (13)** |
| 15_matrix_mult | 5 | 5 | 4 | 9 | 4 | 5 | 5 | 4 | 155 | 841 | **Rust / Salam (4)** |
| 16_quicksort | 25 | 27 | 25 | 25 | 23 | 24 | 26 | 25 | 139 | 438 | **Salam-l (23)** |
| 17_n_queens | 39 | 40 | 42 | 43 | 46 | 43 | 48 | 43 | 379 | 522 | C (39) |
| 18_sieve_eratosthenes | 8 | 9 | 9 | 9 | 9 | 8 | 9 | 9 | 164 | 764 | **C / Salam-n (8)** |
| 19_mandelbrot | 9 | 10 | 9 | 10 | 9 | 10 | 9 | 9 | 99 | 896 | **C / Rust / Salam (9)** |
| 20_monte_carlo_pi | 27 | 28 | 16 | 32 | 15 | 13 | 27 | 12 | 277 | 3019 | **Salam-t (12)** |
| 21_coin_change_dp | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 2 | 13 | 85 | **C / Rust / Salam (2)** |
| 22_knapsack_01 | 3 | 4 | 4 | 5 | 3 | 3 | 5 | 5 | 88 | 1012 | **C / Salam (3)** |
| 23_caesar_cipher | 12 | 12 | 11 | 15 | 11 | 8 | 12 | 11 | 80 | 908 | **Salam-n (8)** |
| 24_merge_sort | 29 | 29 | 30 | 34 | 29 | 30 | 32 | 31 | 236 | 621 | **C / C++ / Salam-l (29)** |
| 25_palindrome_count | 11 | 12 | 11 | 13 | 10 | 12 | 11 | 11 | 196 | 1318 | **Salam-l (10)** |
| 26_ackermann | 1 | 2 | 1 | 2 | 1 | 1 | 1 | 1 | 8 | 17 | **C / Rust / Salam (1)** |
| 27_edit_distance | 6 | 7 | 6 | 13 | 5 | 4 | 5 | 5 | 187 | 1846 | **Salam-n (4)** |
| 28_prime_factorization | 1324 | 1323 | 817 | 1313 | 819 | 800 | 1321 | 819 | 3472 | 42013 | **Salam-n (800)** |
| 29_dot_product | 33 | 34 | 34 | 34 | 34 | 25 | 33 | 34 | 191 | 2115 | **Salam-n (25)** |
| 30_heap_sort | 33 | 33 | 37 | 41 | 33 | 33 | 34 | 35 | 408 | 902 | **C / C++ / Salam (33)** |
| 31_binary_search_stress | 1089 | 1085 | 854 | 1084 | 1067 | 1045 | 1099 | 1103 | 4378 | 22975 | Rust (854) |
| 32_longest_increasing_subsequence | 9 | 10 | 22 | 10 | 9 | 8 | 10 | 9 | 212 | 1866 | **Salam-n (8)** |
| 33_subset_sum_reachability | 9 | 10 | 19 | 16 | 10 | 8 | 17 | 19 | 255 | 1841 | **Salam-n (8)** |
| 34_counting_sort | 32 | 33 | 24 | 36 | 24 | 21 | 32 | 24 | 185 | 1917 | **Salam-n (21)** |
| 35_run_length_stats | 7 | 8 | 6 | 9 | 6 | 6 | 8 | 6 | 88 | 648 | **Rust / Salam (6)** |
| 36_array_rotation_reversal | 33 | 34 | 34 | 35 | 33 | 33 | 30 | 33 | 197 | 748 | **Salam-g (30)** |
| 37_sqrt_decomposition | 14 | 14 | 13 | 22 | 13 | 13 | 14 | 13 | 264 | 892 | **Rust / Salam (13)** |
| 38_game_of_life | 9 | 10 | 27 | 39 | 6 | 5 | 7 | 6 | 867 | 6657 | **Salam-n (5)** |
| 39_trapezoidal_integration | 12 | 13 | 13 | 13 | 13 | 12 | 12 | 12 | 316 | 1965 | **C / Salam (12)** |
| 40_matrix_stdlib_matmul | 34 | 32 | 54 | 86 | 11 | 11 | 12 | 11 | 1319 | 105 | **Salam (11)** |
| 41_matrix_stdlib_smallops | 4 | 5 | 5 | 19 | 38 | 41 | 53 | 64 | 411 | 1069 | C (4) |
| 42_tensor_stdlib_matmul | 35 | 32 | 55 | 86 | 6 | 7 | 7 | 7 | 1328 | 105 | **Salam-l (6)** |
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
| 1 | 42_tensor_stdlib_matmul | 6 | C++ (32) | 0.19x |
| 2 | 40_matrix_stdlib_matmul | 11 | C++ (32) | 0.34x |
| 3 | 38_game_of_life | 6 | C (9) | 0.67x |
| 4 | 27_edit_distance | 5 | C (6) | 0.83x |
| 5 | 25_palindrome_count | 10 | C (11) | 0.91x |
| 6 | 16_quicksort | 23 | C (25) | 0.92x |
| 7 | 20_monte_carlo_pi | 15 | Rust (16) | 0.94x |
| 8 | 26_ackermann | 1 | C (1) | 1.00x |
| 9 | 12_hello_print | 2 | Rust (2) | 1.00x |
| 10 | 07_pow_mod | 2 | C (2) | 1.00x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 38 | C (4) | 9.50x |
| 2 | 01_fib_recursive | 3 | C (2) | 1.50x |
| 3 | 31_binary_search_stress | 1067 | Rust (854) | 1.25x |
| 4 | 11_loop_count | 6 | C (5) | 1.20x |
| 5 | 17_n_queens | 46 | C (39) | 1.18x |
| 6 | 18_sieve_eratosthenes | 9 | C (8) | 1.13x |
| 7 | 14_lcg_random | 19 | C (17) | 1.12x |
| 8 | 33_subset_sum_reachability | 10 | C (9) | 1.11x |
| 9 | 13_array_rw | 24 | C (22) | 1.09x |
| 10 | 39_trapezoidal_integration | 13 | C (12) | 1.08x |
| 11 | 02_fib_iterative | 18 | C (17) | 1.06x |
| 12 | 29_dot_product | 34 | C (33) | 1.03x |
| 13 | 28_prime_factorization | 819 | Rust (817) | 1.00x |
| 14 | 36_array_rotation_reversal | 33 | C (33) | 1.00x |
| 15 | 30_heap_sort | 33 | C (33) | 1.00x |
| 16 | 24_merge_sort | 29 | C (29) | 1.00x |
| 17 | 34_counting_sort | 24 | Rust (24) | 1.00x |
| 18 | 06_gcd_sum | 16 | Rust (16) | 1.00x |
| 19 | 37_sqrt_decomposition | 13 | Rust (13) | 1.00x |
| 20 | 05_sum_mod | 11 | C (11) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (20):** 02_fib_iterative, 04_collatz, 05_sum_mod, 10_pi_leibniz, 13_array_rw, 14_lcg_random, 16_quicksort, 20_monte_carlo_pi, 23_caesar_cipher, 25_palindrome_count, 27_edit_distance, 28_prime_factorization, 29_dot_product, 32_longest_increasing_subsequence, 33_subset_sum_reachability, 34_counting_sort, 36_array_rotation_reversal, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (19):** 01_fib_recursive, 03_primes_count, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 11_loop_count, 12_hello_print, 15_matrix_mult, 18_sieve_eratosthenes, 19_mandelbrot, 21_coin_change_dp, 22_knapsack_01, 24_merge_sort, 26_ackermann, 30_heap_sort, 35_run_length_stats, 37_sqrt_decomposition, 39_trapezoidal_integration.
- **Trailing the fastest (3):** 17_n_queens (+4 vs C); 31_binary_search_stress (+191 vs Rust); 41_matrix_stdlib_smallops (+34 vs C).

Salam matches or beats the fastest other compiled language on 39 of 46 programs.

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
