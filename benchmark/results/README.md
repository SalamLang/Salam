# Salam Benchmark Suite Results

46 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured four ways: the LLVM backend at
`-O3`, the native x86-64 backend (`--backend=native`), the C backend compiled
by `gcc -O3`, and the default `tcc` toolchain (no optimizer) as the
out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-11-26T22:53:21Z (took 31 min)
- Commit: `25c1c8ab`

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
- **php**: PHP 8.5.13 (cli) (built: Nov 20 2026 02:07:43) (NTS)
- **salam**: 0.7.2

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
| 1 | **Salam-l** | 11.28 ms | 0.91x | 1.00x | 42 / 46 | 26 / 46 |
| 2 | **Salam-t** | 11.49 ms | 0.93x | 1.02x | 42 / 46 | 24 / 46 |
| 3 | **Salam-n** | 11.94 ms | 0.96x | 1.06x | 42 / 46 | 15 / 46 |
| 4 | **Salam-g** | 12.18 ms | 0.98x | 1.08x | 42 / 46 | 15 / 46 |
| 5 | C | 12.41 ms | 1.00x | 1.10x | 42 / 46 | 23 / 46 |
| 6 | Rust | 12.48 ms | 1.01x | 1.11x | 42 / 46 | 21 / 46 |
| 7 | C++ | 13.63 ms | 1.10x | 1.21x | 42 / 46 | 5 / 46 |
| 8 | Go | 16.58 ms | 1.34x | 1.47x | 42 / 46 | 2 / 46 |
| 9 | PHP | 125.95 ms | 10.15x | 11.17x | 42 / 46 | 0 / 46 |
| 10 | Python | 581.86 ms | 46.89x | 51.60x | 42 / 46 | 0 / 46 |

```
Salam-l      ███████████████████████████████                 11.28 ms  0.91x C
Salam-t      ███████████████████████████████                 11.49 ms  0.93x C
Salam-n      ████████████████████████████████                11.94 ms  0.96x C
Salam-g      █████████████████████████████████               12.18 ms  0.98x C
C            ██████████████████████████████████              12.41 ms  1.00x C  <- baseline
Rust         ██████████████████████████████████              12.48 ms  1.01x C
C++          █████████████████████████████████████           13.63 ms  1.10x C
Go           █████████████████████████████████████████████   16.58 ms  1.34x C
PHP          (off scale)                                    125.95 ms  10.15x C
Python       (off scale)                                    581.86 ms  46.89x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 11.28 ms
  (0.91x C's geomean, 0.90x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  26/46); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.02x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 10-47x slower** than the compiled languages on
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
| 01_fib_recursive | 2 | 3 | 3 | 5 | 3 | 3 | 2 | 3 | 41 | 71 | **C / Salam-g (2)** |
| 02_fib_iterative | 17 | 18 | 18 | 18 | 18 | 18 | 17 | 18 | 51 | 498 | **C / Salam-g (17)** |
| 03_primes_count | 5 | 6 | 4 | 6 | 4 | 4 | 5 | 4 | 15 | 124 | **Rust / Salam (4)** |
| 04_collatz | 5 | 6 | 4 | 7 | 4 | 4 | 5 | 4 | 50 | 271 | **Rust / Salam (4)** |
| 05_sum_mod | 11 | 11 | 11 | 11 | 11 | 11 | 11 | 11 | 27 | 306 | **C / C++ / Rust / Go / Salam (11)** |
| 06_gcd_sum | 20 | 21 | 16 | 21 | 17 | 17 | 20 | 16 | 42 | 115 | **Rust / Salam-t (16)** |
| 07_pow_mod | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 2 | 15 | 51 | **C / Rust / Salam (2)** |
| 08_digit_sum | 3 | 4 | 3 | 4 | 3 | 3 | 3 | 3 | 45 | 274 | **C / Rust / Salam (3)** |
| 09_perfect_numbers | 11 | 12 | 7 | 12 | 8 | 8 | 12 | 8 | 29 | 306 | Rust (7) |
| 10_pi_leibniz | 4 | 5 | 4 | 5 | 4 | 4 | 4 | 4 | 41 | 380 | **C / Rust / Salam (4)** |
| 11_loop_count | 5 | 5 | 5 | 5 | 5 | 5 | 5 | 5 | 11 | 87 | **C / C++ / Rust / Go / Salam (5)** |
| 12_hello_print | 13 | 15 | 2 | 5 | 2 | 2 | 3 | 3 | 163 | 67 | **Rust / Salam (2)** |
| 13_array_rw | 21 | 23 | 23 | 23 | 22 | 22 | 20 | 22 | 66 | 712 | **Salam-g (20)** |
| 14_lcg_random | 17 | 18 | 19 | 22 | 19 | 18 | 17 | 18 | 71 | 1074 | **C / Salam-g (17)** |
| 15_matrix_mult | 4 | 5 | 4 | 9 | 4 | 7 | 5 | 4 | 157 | 850 | **C / Rust / Salam (4)** |
| 16_quicksort | 25 | 26 | 24 | 25 | 24 | 24 | 26 | 24 | 140 | 423 | **Rust / Salam (24)** |
| 17_n_queens | 39 | 40 | 42 | 43 | 46 | 48 | 46 | 44 | 393 | 518 | C (39) |
| 18_sieve_eratosthenes | 8 | 9 | 9 | 9 | 8 | 8 | 8 | 8 | 136 | 752 | **C / Salam (8)** |
| 19_mandelbrot | 9 | 9 | 9 | 10 | 9 | 10 | 9 | 9 | 98 | 905 | **C / C++ / Rust / Salam (9)** |
| 20_monte_carlo_pi | 27 | 28 | 15 | 32 | 15 | 26 | 27 | 15 | 279 | 2799 | **Rust / Salam (15)** |
| 21_coin_change_dp | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 2 | 13 | 85 | **C / Rust / Salam (2)** |
| 22_knapsack_01 | 3 | 4 | 4 | 5 | 5 | 4 | 5 | 5 | 87 | 1013 | C (3) |
| 23_caesar_cipher | 11 | 12 | 11 | 14 | 11 | 12 | 11 | 11 | 74 | 906 | **C / Rust / Salam (11)** |
| 24_merge_sort | 28 | 28 | 29 | 33 | 28 | 29 | 31 | 31 | 231 | 623 | **C / C++ / Salam-l (28)** |
| 25_palindrome_count | 11 | 11 | 10 | 13 | 10 | 12 | 11 | 10 | 192 | 1311 | **Rust / Salam (10)** |
| 26_ackermann | 1 | 2 | 1 | 2 | 1 | 1 | 1 | 1 | 8 | 17 | **C / Rust / Salam (1)** |
| 27_edit_distance | 6 | 7 | 6 | 12 | 5 | 11 | 5 | 5 | 184 | 1937 | **Salam (5)** |
| 28_prime_factorization | 1327 | 1325 | 819 | 1316 | 820 | 803 | 1326 | 829 | 3408 | 41994 | **Salam-n (803)** |
| 29_dot_product | 33 | 34 | 34 | 34 | 34 | 36 | 34 | 34 | 185 | 2133 | C (33) |
| 30_heap_sort | 33 | 33 | 37 | 40 | 34 | 34 | 34 | 35 | 402 | 867 | C / C++ (33) |
| 31_binary_search_stress | 1102 | 1042 | 863 | 1038 | 1073 | 1058 | 1034 | 1066 | 4244 | 20553 | Rust (863) |
| 32_longest_increasing_subsequence | 9 | 10 | 22 | 10 | 9 | 10 | 10 | 9 | 213 | 1855 | **C / Salam (9)** |
| 33_subset_sum_reachability | 9 | 10 | 19 | 15 | 18 | 16 | 16 | 18 | 261 | 1842 | C (9) |
| 34_counting_sort | 31 | 32 | 24 | 36 | 24 | 34 | 32 | 24 | 185 | 1880 | **Rust / Salam (24)** |
| 35_run_length_stats | 7 | 8 | 6 | 8 | 6 | 6 | 7 | 5 | 86 | 642 | **Salam-t (5)** |
| 36_array_rotation_reversal | 25 | 29 | 28 | 27 | 25 | 26 | 27 | 26 | 189 | 726 | **C / Salam-l (25)** |
| 37_sqrt_decomposition | 13 | 13 | 12 | 19 | 12 | 12 | 13 | 12 | 258 | 879 | **Rust / Salam (12)** |
| 38_game_of_life | 8 | 9 | 27 | 39 | 6 | 9 | 7 | 6 | 866 | 6668 | **Salam (6)** |
| 39_trapezoidal_integration | 12 | 12 | 13 | 12 | 12 | 12 | 12 | 10 | 297 | 1952 | **Salam-t (10)** |
| 40_matrix_stdlib_matmul | 34 | 32 | 54 | 86 | 10 | 11 | 11 | 11 | 1289 | 100 | **Salam-l (10)** |
| 41_matrix_stdlib_smallops | 4 | 5 | 5 | 19 | 37 | 41 | 46 | 48 | 408 | 1075 | C (4) |
| 42_tensor_stdlib_matmul | 33 | 30 | 54 | 86 | 5 | 5 | 6 | 6 | 1285 | 101 | **Salam (5)** |
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
| 1 | 42_tensor_stdlib_matmul | 5 | C++ (30) | 0.17x |
| 2 | 40_matrix_stdlib_matmul | 10 | C++ (32) | 0.31x |
| 3 | 38_game_of_life | 6 | C (8) | 0.75x |
| 4 | 27_edit_distance | 5 | C (6) | 0.83x |
| 5 | 26_ackermann | 1 | C (1) | 1.00x |
| 6 | 07_pow_mod | 2 | C (2) | 1.00x |
| 7 | 12_hello_print | 2 | Rust (2) | 1.00x |
| 8 | 21_coin_change_dp | 2 | C (2) | 1.00x |
| 9 | 08_digit_sum | 3 | C (3) | 1.00x |
| 10 | 10_pi_leibniz | 4 | C (4) | 1.00x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 37 | C (4) | 9.25x |
| 2 | 33_subset_sum_reachability | 18 | C (9) | 2.00x |
| 3 | 22_knapsack_01 | 5 | C (3) | 1.67x |
| 4 | 01_fib_recursive | 3 | C (2) | 1.50x |
| 5 | 31_binary_search_stress | 1073 | Rust (863) | 1.24x |
| 6 | 17_n_queens | 46 | C (39) | 1.18x |
| 7 | 09_perfect_numbers | 8 | Rust (7) | 1.14x |
| 8 | 14_lcg_random | 19 | C (17) | 1.12x |
| 9 | 06_gcd_sum | 17 | Rust (16) | 1.06x |
| 10 | 02_fib_iterative | 18 | C (17) | 1.06x |
| 11 | 13_array_rw | 22 | C (21) | 1.05x |
| 12 | 29_dot_product | 34 | C (33) | 1.03x |
| 13 | 30_heap_sort | 34 | C (33) | 1.03x |
| 14 | 28_prime_factorization | 820 | Rust (819) | 1.00x |
| 15 | 24_merge_sort | 28 | C (28) | 1.00x |
| 16 | 36_array_rotation_reversal | 25 | C (25) | 1.00x |
| 17 | 34_counting_sort | 24 | Rust (24) | 1.00x |
| 18 | 16_quicksort | 24 | Rust (24) | 1.00x |
| 19 | 20_monte_carlo_pi | 15 | Rust (15) | 1.00x |
| 20 | 39_trapezoidal_integration | 12 | C (12) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (8):** 13_array_rw, 27_edit_distance, 28_prime_factorization, 35_run_length_stats, 38_game_of_life, 39_trapezoidal_integration, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (26):** 01_fib_recursive, 02_fib_iterative, 03_primes_count, 04_collatz, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 10_pi_leibniz, 11_loop_count, 12_hello_print, 14_lcg_random, 15_matrix_mult, 16_quicksort, 18_sieve_eratosthenes, 19_mandelbrot, 20_monte_carlo_pi, 21_coin_change_dp, 23_caesar_cipher, 24_merge_sort, 25_palindrome_count, 26_ackermann, 32_longest_increasing_subsequence, 34_counting_sort, 36_array_rotation_reversal, 37_sqrt_decomposition.
- **Trailing the fastest (8):** 09_perfect_numbers (+1 vs Rust); 17_n_queens (+5 vs C); 22_knapsack_01 (+1 vs C); 29_dot_product (+1 vs C); 30_heap_sort (+1 vs C/C++); 31_binary_search_stress (+171 vs Rust); 33_subset_sum_reachability (+7 vs C); 41_matrix_stdlib_smallops (+33 vs C).

Salam matches or beats the fastest other compiled language on 34 of 46 programs.

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
