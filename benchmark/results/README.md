# Salam Benchmark Suite Results

46 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured four ways: the LLVM backend at
`-O3`, the native x86-64 backend (`--backend=native`), the C backend compiled
by `gcc -O3`, and the default `tcc` toolchain (no optimizer) as the
out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-11-24T09:24:29Z (took 30 min)
- Commit: `c416d928`

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
- **salam**: 0.7.0

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
| 1 | **Salam-l** | 11.19 ms | 0.91x | 1.00x | 42 / 46 | 25 / 46 |
| 2 | **Salam-t** | 11.40 ms | 0.93x | 1.02x | 42 / 46 | 23 / 46 |
| 3 | **Salam-n** | 11.88 ms | 0.97x | 1.06x | 42 / 46 | 21 / 46 |
| 4 | **Salam-g** | 12.09 ms | 0.98x | 1.08x | 42 / 46 | 19 / 46 |
| 5 | C | 12.31 ms | 1.00x | 1.10x | 42 / 46 | 23 / 46 |
| 6 | Rust | 12.37 ms | 1.00x | 1.10x | 42 / 46 | 19 / 46 |
| 7 | C++ | 13.58 ms | 1.10x | 1.21x | 42 / 46 | 5 / 46 |
| 8 | Go | 16.44 ms | 1.34x | 1.47x | 42 / 46 | 3 / 46 |
| 9 | PHP | 123.75 ms | 10.05x | 11.06x | 42 / 46 | 0 / 46 |
| 10 | Python | 577.85 ms | 46.95x | 51.62x | 42 / 46 | 0 / 46 |

```
Salam-l      ███████████████████████████████                 11.19 ms  0.91x C
Salam-t      ███████████████████████████████                 11.40 ms  0.93x C
Salam-n      █████████████████████████████████               11.88 ms  0.97x C
Salam-g      █████████████████████████████████               12.09 ms  0.98x C
C            ██████████████████████████████████              12.31 ms  1.00x C  <- baseline
Rust         ██████████████████████████████████              12.37 ms  1.00x C
C++          █████████████████████████████████████           13.58 ms  1.10x C
Go           █████████████████████████████████████████████   16.44 ms  1.34x C
PHP          (off scale)                                    123.75 ms  10.05x C
Python       (off scale)                                    577.85 ms  46.95x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 11.19 ms
  (0.91x C's geomean, 0.91x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  25/46); see the per-program table below.
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
| 02_fib_iterative | 17 | 18 | 18 | 18 | 18 | 18 | 17 | 18 | 51 | 496 | **C / Salam-g (17)** |
| 03_primes_count | 5 | 6 | 4 | 6 | 4 | 3 | 5 | 4 | 15 | 124 | **Salam-n (3)** |
| 04_collatz | 5 | 6 | 5 | 7 | 4 | 4 | 5 | 4 | 49 | 272 | **Salam (4)** |
| 05_sum_mod | 11 | 11 | 11 | 11 | 11 | 11 | 11 | 11 | 27 | 303 | **C / C++ / Rust / Go / Salam (11)** |
| 06_gcd_sum | 20 | 21 | 15 | 20 | 17 | 17 | 20 | 15 | 41 | 114 | **Rust / Salam-t (15)** |
| 07_pow_mod | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 2 | 14 | 50 | **C / Rust / Salam (2)** |
| 08_digit_sum | 3 | 4 | 3 | 4 | 3 | 3 | 3 | 3 | 43 | 271 | **C / Rust / Salam (3)** |
| 09_perfect_numbers | 11 | 12 | 7 | 12 | 8 | 7 | 11 | 8 | 28 | 304 | **Rust / Salam-n (7)** |
| 10_pi_leibniz | 4 | 5 | 4 | 5 | 4 | 4 | 4 | 4 | 40 | 380 | **C / Rust / Salam (4)** |
| 11_loop_count | 5 | 5 | 5 | 5 | 5 | 5 | 5 | 5 | 11 | 86 | **C / C++ / Rust / Go / Salam (5)** |
| 12_hello_print | 13 | 15 | 2 | 5 | 2 | 2 | 3 | 3 | 162 | 66 | **Rust / Salam (2)** |
| 13_array_rw | 21 | 22 | 22 | 22 | 22 | 22 | 20 | 22 | 63 | 708 | **Salam-g (20)** |
| 14_lcg_random | 17 | 18 | 18 | 22 | 18 | 18 | 17 | 18 | 71 | 1074 | **C / Salam-g (17)** |
| 15_matrix_mult | 4 | 5 | 4 | 9 | 4 | 7 | 5 | 4 | 155 | 848 | **C / Rust / Salam (4)** |
| 16_quicksort | 25 | 25 | 24 | 25 | 24 | 24 | 26 | 24 | 139 | 419 | **Rust / Salam (24)** |
| 17_n_queens | 39 | 40 | 41 | 42 | 46 | 48 | 45 | 44 | 392 | 519 | C (39) |
| 18_sieve_eratosthenes | 8 | 9 | 8 | 9 | 8 | 8 | 8 | 8 | 112 | 721 | **C / Rust / Salam (8)** |
| 19_mandelbrot | 9 | 9 | 9 | 9 | 9 | 9 | 9 | 9 | 95 | 893 | **C / C++ / Rust / Go / Salam (9)** |
| 20_monte_carlo_pi | 27 | 28 | 15 | 32 | 15 | 26 | 27 | 12 | 277 | 2775 | **Salam-t (12)** |
| 21_coin_change_dp | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 2 | 13 | 85 | **C / Rust / Salam (2)** |
| 22_knapsack_01 | 3 | 4 | 4 | 5 | 5 | 4 | 5 | 5 | 87 | 1007 | C (3) |
| 23_caesar_cipher | 12 | 12 | 11 | 15 | 11 | 12 | 11 | 11 | 74 | 909 | **Rust / Salam (11)** |
| 24_merge_sort | 28 | 28 | 29 | 33 | 28 | 29 | 31 | 31 | 231 | 622 | **C / C++ / Salam-l (28)** |
| 25_palindrome_count | 11 | 11 | 10 | 13 | 10 | 12 | 11 | 10 | 192 | 1313 | **Rust / Salam (10)** |
| 26_ackermann | 1 | 2 | 1 | 2 | 1 | 1 | 1 | 1 | 8 | 17 | **C / Rust / Salam (1)** |
| 27_edit_distance | 6 | 7 | 6 | 12 | 5 | 11 | 5 | 5 | 185 | 1926 | **Salam (5)** |
| 28_prime_factorization | 1326 | 1324 | 818 | 1314 | 818 | 803 | 1324 | 828 | 3394 | 41840 | **Salam-n (803)** |
| 29_dot_product | 33 | 34 | 34 | 34 | 33 | 35 | 33 | 34 | 185 | 2103 | **C / Salam (33)** |
| 30_heap_sort | 33 | 33 | 36 | 39 | 34 | 33 | 33 | 34 | 396 | 834 | **C / C++ / Salam (33)** |
| 31_binary_search_stress | 1042 | 1038 | 714 | 1039 | 1078 | 1060 | 1048 | 1066 | 3389 | 18869 | Rust (714) |
| 32_longest_increasing_subsequence | 9 | 10 | 22 | 10 | 9 | 10 | 10 | 9 | 213 | 1864 | **C / Salam (9)** |
| 33_subset_sum_reachability | 9 | 10 | 19 | 15 | 18 | 15 | 16 | 18 | 262 | 1855 | C (9) |
| 34_counting_sort | 31 | 32 | 24 | 36 | 24 | 34 | 32 | 24 | 189 | 1899 | **Rust / Salam (24)** |
| 35_run_length_stats | 7 | 8 | 6 | 8 | 6 | 6 | 7 | 6 | 87 | 643 | **Rust / Salam (6)** |
| 36_array_rotation_reversal | 25 | 28 | 27 | 26 | 25 | 25 | 25 | 25 | 188 | 721 | **C / Salam (25)** |
| 37_sqrt_decomposition | 12 | 13 | 12 | 20 | 12 | 11 | 13 | 12 | 255 | 886 | **Salam-n (11)** |
| 38_game_of_life | 8 | 9 | 27 | 39 | 6 | 9 | 7 | 6 | 865 | 6655 | **Salam (6)** |
| 39_trapezoidal_integration | 12 | 13 | 13 | 13 | 13 | 12 | 12 | 12 | 296 | 1962 | **C / Salam (12)** |
| 40_matrix_stdlib_matmul | 33 | 32 | 54 | 86 | 10 | 11 | 11 | 11 | 1289 | 101 | **Salam-l (10)** |
| 41_matrix_stdlib_smallops | 4 | 5 | 5 | 19 | 38 | 41 | 46 | 49 | 412 | 1076 | C (4) |
| 42_tensor_stdlib_matmul | 35 | 31 | 55 | 86 | 5 | 5 | 6 | 6 | 1307 | 101 | **Salam (5)** |
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
| 1 | 42_tensor_stdlib_matmul | 5 | C++ (31) | 0.16x |
| 2 | 40_matrix_stdlib_matmul | 10 | C++ (32) | 0.31x |
| 3 | 38_game_of_life | 6 | C (8) | 0.75x |
| 4 | 04_collatz | 4 | C (5) | 0.80x |
| 5 | 27_edit_distance | 5 | C (6) | 0.83x |
| 6 | 26_ackermann | 1 | C (1) | 1.00x |
| 7 | 07_pow_mod | 2 | C (2) | 1.00x |
| 8 | 12_hello_print | 2 | Rust (2) | 1.00x |
| 9 | 21_coin_change_dp | 2 | C (2) | 1.00x |
| 10 | 08_digit_sum | 3 | C (3) | 1.00x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 38 | C (4) | 9.50x |
| 2 | 33_subset_sum_reachability | 18 | C (9) | 2.00x |
| 3 | 22_knapsack_01 | 5 | C (3) | 1.67x |
| 4 | 31_binary_search_stress | 1078 | Rust (714) | 1.51x |
| 5 | 01_fib_recursive | 3 | C (2) | 1.50x |
| 6 | 17_n_queens | 46 | C (39) | 1.18x |
| 7 | 09_perfect_numbers | 8 | Rust (7) | 1.14x |
| 8 | 06_gcd_sum | 17 | Rust (15) | 1.13x |
| 9 | 39_trapezoidal_integration | 13 | C (12) | 1.08x |
| 10 | 02_fib_iterative | 18 | C (17) | 1.06x |
| 11 | 14_lcg_random | 18 | C (17) | 1.06x |
| 12 | 13_array_rw | 22 | C (21) | 1.05x |
| 13 | 30_heap_sort | 34 | C (33) | 1.03x |
| 14 | 28_prime_factorization | 818 | Rust (818) | 1.00x |
| 15 | 29_dot_product | 33 | C (33) | 1.00x |
| 16 | 24_merge_sort | 28 | C (28) | 1.00x |
| 17 | 36_array_rotation_reversal | 25 | C (25) | 1.00x |
| 18 | 34_counting_sort | 24 | Rust (24) | 1.00x |
| 19 | 16_quicksort | 24 | Rust (24) | 1.00x |
| 20 | 20_monte_carlo_pi | 15 | Rust (15) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (10):** 03_primes_count, 04_collatz, 13_array_rw, 20_monte_carlo_pi, 27_edit_distance, 28_prime_factorization, 37_sqrt_decomposition, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (27):** 01_fib_recursive, 02_fib_iterative, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 12_hello_print, 14_lcg_random, 15_matrix_mult, 16_quicksort, 18_sieve_eratosthenes, 19_mandelbrot, 21_coin_change_dp, 23_caesar_cipher, 24_merge_sort, 25_palindrome_count, 26_ackermann, 29_dot_product, 30_heap_sort, 32_longest_increasing_subsequence, 34_counting_sort, 35_run_length_stats, 36_array_rotation_reversal, 39_trapezoidal_integration.
- **Trailing the fastest (5):** 17_n_queens (+5 vs C); 22_knapsack_01 (+1 vs C); 31_binary_search_stress (+334 vs Rust); 33_subset_sum_reachability (+6 vs C); 41_matrix_stdlib_smallops (+34 vs C).

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
