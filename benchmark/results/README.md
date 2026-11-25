# Salam Benchmark Suite Results

46 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured four ways: the LLVM backend at
`-O3`, the native x86-64 backend (`--backend=native`), the C backend compiled
by `gcc -O3`, and the default `tcc` toolchain (no optimizer) as the
out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-11-25T21:22:41Z (took 49 min)
- Commit: `b40afb97`

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
| 1 | **Salam-l** | 16.19 ms | 0.90x | 1.00x | 42 / 46 | 26 / 46 |
| 2 | **Salam-t** | 16.42 ms | 0.91x | 1.01x | 42 / 46 | 19 / 46 |
| 3 | **Salam-g** | 17.26 ms | 0.96x | 1.07x | 42 / 46 | 18 / 46 |
| 4 | **Salam-n** | 17.32 ms | 0.96x | 1.07x | 42 / 46 | 15 / 46 |
| 5 | Rust | 18.00 ms | 1.00x | 1.11x | 42 / 46 | 18 / 46 |
| 6 | C | 18.05 ms | 1.00x | 1.11x | 42 / 46 | 20 / 46 |
| 7 | C++ | 19.95 ms | 1.11x | 1.23x | 42 / 46 | 2 / 46 |
| 8 | Go | 23.51 ms | 1.30x | 1.45x | 42 / 46 | 0 / 46 |
| 9 | PHP | 204.48 ms | 11.33x | 12.63x | 42 / 46 | 0 / 46 |
| 10 | Python | 953.50 ms | 52.84x | 58.91x | 42 / 46 | 0 / 46 |

```
Salam-l      ███████████████████████████████                 16.19 ms  0.90x C
Salam-t      ███████████████████████████████                 16.42 ms  0.91x C
Salam-g      █████████████████████████████████               17.26 ms  0.96x C
Salam-n      █████████████████████████████████               17.32 ms  0.96x C
Rust         ██████████████████████████████████              18.00 ms  1.00x C
C            ███████████████████████████████████             18.05 ms  1.00x C  <- baseline
C++          ██████████████████████████████████████          19.95 ms  1.11x C
Go           █████████████████████████████████████████████   23.51 ms  1.30x C
PHP          (off scale)                                    204.48 ms  11.33x C
Python       (off scale)                                    953.50 ms  52.84x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 16.19 ms
  (0.90x C's geomean, 0.90x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  26/46); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.01x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 11-53x slower** than the compiled languages on
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
| 01_fib_recursive | 4 | 5 | 5 | 8 | 5 | 4 | 4 | 5 | 71 | 108 | **C / Salam (4)** |
| 02_fib_iterative | 24 | 25 | 24 | 25 | 24 | 25 | 24 | 24 | 73 | 809 | **C / Rust / Salam (24)** |
| 03_primes_count | 6 | 7 | 6 | 7 | 5 | 5 | 6 | 5 | 26 | 199 | **Salam (5)** |
| 04_collatz | 6 | 7 | 6 | 9 | 5 | 6 | 6 | 6 | 73 | 434 | **Salam-l (5)** |
| 05_sum_mod | 15 | 16 | 16 | 16 | 15 | 16 | 15 | 15 | 41 | 515 | **C / Salam (15)** |
| 06_gcd_sum | 19 | 20 | 18 | 21 | 19 | 19 | 19 | 18 | 61 | 177 | **Rust / Salam-t (18)** |
| 07_pow_mod | 4 | 5 | 4 | 5 | 4 | 4 | 4 | 4 | 25 | 80 | **C / Rust / Salam (4)** |
| 08_digit_sum | 5 | 6 | 5 | 6 | 5 | 4 | 5 | 5 | 78 | 455 | **Salam-n (4)** |
| 09_perfect_numbers | 11 | 13 | 10 | 13 | 10 | 10 | 12 | 11 | 50 | 491 | **Rust / Salam (10)** |
| 10_pi_leibniz | 7 | 8 | 7 | 8 | 7 | 7 | 7 | 7 | 62 | 705 | **C / Rust / Salam (7)** |
| 11_loop_count | 7 | 8 | 7 | 8 | 8 | 7 | 7 | 8 | 18 | 150 | **C / Rust / Salam (7)** |
| 12_hello_print | 23 | 23 | 4 | 10 | 3 | 4 | 3 | 3 | 682 | 120 | **Salam (3)** |
| 13_array_rw | 30 | 32 | 31 | 33 | 31 | 32 | 28 | 31 | 107 | 1215 | **Salam-g (28)** |
| 14_lcg_random | 24 | 25 | 26 | 31 | 25 | 25 | 24 | 25 | 90 | 1681 | **C / Salam-g (24)** |
| 15_matrix_mult | 8 | 9 | 8 | 15 | 8 | 13 | 9 | 8 | 225 | 1434 | **C / Rust / Salam (8)** |
| 16_quicksort | 31 | 32 | 30 | 34 | 30 | 31 | 31 | 31 | 207 | 662 | **Rust / Salam-l (30)** |
| 17_n_queens | 46 | 48 | 49 | 54 | 52 | 55 | 53 | 52 | 567 | 766 | C (46) |
| 18_sieve_eratosthenes | 8 | 9 | 9 | 12 | 8 | 9 | 8 | 8 | 150 | 1167 | **C / Salam (8)** |
| 19_mandelbrot | 13 | 15 | 13 | 15 | 13 | 13 | 13 | 14 | 179 | 1643 | **C / Rust / Salam (13)** |
| 20_monte_carlo_pi | 35 | 35 | 19 | 41 | 18 | 32 | 35 | 14 | 440 | 4502 | **Salam-t (14)** |
| 21_coin_change_dp | 4 | 5 | 4 | 6 | 4 | 4 | 4 | 4 | 23 | 146 | **C / Rust / Salam (4)** |
| 22_knapsack_01 | 6 | 8 | 6 | 9 | 7 | 8 | 7 | 7 | 144 | 1735 | C / Rust (6) |
| 23_caesar_cipher | 22 | 23 | 15 | 27 | 15 | 17 | 22 | 15 | 129 | 1423 | **Rust / Salam (15)** |
| 24_merge_sort | 36 | 36 | 38 | 42 | 37 | 39 | 40 | 42 | 350 | 982 | C / C++ (36) |
| 25_palindrome_count | 16 | 17 | 17 | 21 | 17 | 17 | 16 | 17 | 357 | 2163 | **C / Salam-g (16)** |
| 26_ackermann | 2 | 3 | 3 | 4 | 2 | 2 | 2 | 2 | 14 | 30 | **C / Salam (2)** |
| 27_edit_distance | 9 | 10 | 10 | 13 | 8 | 11 | 9 | 9 | 324 | 3191 | **Salam-l (8)** |
| 28_prime_factorization | 1264 | 1264 | 1096 | 1252 | 1097 | 1082 | 1264 | 1093 | 6429 | 69200 | **Salam-n (1082)** |
| 29_dot_product | 46 | 48 | 49 | 49 | 48 | 51 | 47 | 49 | 318 | 3721 | C (46) |
| 30_heap_sort | 41 | 41 | 44 | 53 | 43 | 43 | 41 | 43 | 633 | 1309 | **C / C++ / Salam-g (41)** |
| 31_binary_search_stress | 1123 | 1133 | 1067 | 1105 | 1067 | 1112 | 1121 | 1069 | 6574 | 33054 | **Rust / Salam-l (1067)** |
| 32_longest_increasing_subsequence | 17 | 18 | 13 | 15 | 13 | 18 | 18 | 14 | 335 | 3089 | **Rust / Salam-l (13)** |
| 33_subset_sum_reachability | 22 | 24 | 24 | 24 | 21 | 24 | 23 | 22 | 417 | 3124 | **Salam-l (21)** |
| 34_counting_sort | 47 | 50 | 35 | 51 | 35 | 45 | 47 | 35 | 296 | 3108 | **Rust / Salam (35)** |
| 35_run_length_stats | 10 | 12 | 9 | 11 | 9 | 9 | 11 | 9 | 142 | 1064 | **Rust / Salam (9)** |
| 36_array_rotation_reversal | 39 | 44 | 40 | 41 | 39 | 39 | 38 | 38 | 288 | 1187 | **Salam (38)** |
| 37_sqrt_decomposition | 19 | 21 | 19 | 41 | 19 | 19 | 21 | 19 | 394 | 1517 | **C / Rust / Salam (19)** |
| 38_game_of_life | 13 | 14 | 38 | 58 | 11 | 15 | 14 | 11 | 1470 | 10360 | **Salam (11)** |
| 39_trapezoidal_integration | 17 | 18 | 16 | 19 | 16 | 16 | 17 | 17 | 568 | 3571 | **Rust / Salam (16)** |
| 40_matrix_stdlib_matmul | 58 | 60 | 104 | 98 | 17 | 17 | 17 | 17 | 1731 | 153 | **Salam (17)** |
| 41_matrix_stdlib_smallops | 6 | 7 | 8 | 24 | 51 | 58 | 64 | 68 | 613 | 1670 | C (6) |
| 42_tensor_stdlib_matmul | 58 | 59 | 104 | 98 | 8 | 9 | 8 | 9 | 1732 | 153 | **Salam (8)** |
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
| 1 | 42_tensor_stdlib_matmul | 8 | C (58) | 0.14x |
| 2 | 40_matrix_stdlib_matmul | 17 | C (58) | 0.29x |
| 3 | 12_hello_print | 3 | Rust (4) | 0.75x |
| 4 | 04_collatz | 5 | C (6) | 0.83x |
| 5 | 03_primes_count | 5 | C (6) | 0.83x |
| 6 | 38_game_of_life | 11 | C (13) | 0.85x |
| 7 | 27_edit_distance | 8 | C (9) | 0.89x |
| 8 | 20_monte_carlo_pi | 18 | Rust (19) | 0.95x |
| 9 | 33_subset_sum_reachability | 21 | C (22) | 0.95x |
| 10 | 26_ackermann | 2 | C (2) | 1.00x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 51 | C (6) | 8.50x |
| 2 | 01_fib_recursive | 5 | C (4) | 1.25x |
| 3 | 22_knapsack_01 | 7 | C (6) | 1.17x |
| 4 | 11_loop_count | 8 | C (7) | 1.14x |
| 5 | 17_n_queens | 52 | C (46) | 1.13x |
| 6 | 25_palindrome_count | 17 | C (16) | 1.06x |
| 7 | 06_gcd_sum | 19 | Rust (18) | 1.06x |
| 8 | 30_heap_sort | 43 | C (41) | 1.05x |
| 9 | 29_dot_product | 48 | C (46) | 1.04x |
| 10 | 14_lcg_random | 25 | C (24) | 1.04x |
| 11 | 13_array_rw | 31 | C (30) | 1.03x |
| 12 | 24_merge_sort | 37 | C (36) | 1.03x |
| 13 | 28_prime_factorization | 1097 | Rust (1096) | 1.00x |
| 14 | 31_binary_search_stress | 1067 | Rust (1067) | 1.00x |
| 15 | 36_array_rotation_reversal | 39 | C (39) | 1.00x |
| 16 | 34_counting_sort | 35 | Rust (35) | 1.00x |
| 17 | 16_quicksort | 30 | Rust (30) | 1.00x |
| 18 | 02_fib_iterative | 24 | C (24) | 1.00x |
| 19 | 37_sqrt_decomposition | 19 | C (19) | 1.00x |
| 20 | 39_trapezoidal_integration | 16 | Rust (16) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (13):** 03_primes_count, 04_collatz, 08_digit_sum, 12_hello_print, 13_array_rw, 20_monte_carlo_pi, 27_edit_distance, 28_prime_factorization, 33_subset_sum_reachability, 36_array_rotation_reversal, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (24):** 01_fib_recursive, 02_fib_iterative, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 14_lcg_random, 15_matrix_mult, 16_quicksort, 18_sieve_eratosthenes, 19_mandelbrot, 21_coin_change_dp, 23_caesar_cipher, 25_palindrome_count, 26_ackermann, 30_heap_sort, 31_binary_search_stress, 32_longest_increasing_subsequence, 34_counting_sort, 35_run_length_stats, 37_sqrt_decomposition, 39_trapezoidal_integration.
- **Trailing the fastest (5):** 17_n_queens (+6 vs C); 22_knapsack_01 (+1 vs C/Rust); 24_merge_sort (+1 vs C/C++); 29_dot_product (+1 vs C); 41_matrix_stdlib_smallops (+45 vs C).

Salam matches or beats the fastest other compiled language on 37 of 46 programs.

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
