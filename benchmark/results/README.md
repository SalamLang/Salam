# Salam Benchmark Suite Results

46 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured four ways: the LLVM backend at
`-O3`, the native x86-64 backend (`--backend=native`), the C backend compiled
by `gcc -O3`, and the default `tcc` toolchain (no optimizer) as the
out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-11-27T11:05:01Z (took 39 min)
- Commit: `f68a7359`

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
| 1 | **Salam-l** | 12.80 ms | 0.90x | 1.00x | 42 / 46 | 25 / 46 |
| 2 | **Salam-t** | 12.95 ms | 0.91x | 1.01x | 42 / 46 | 26 / 46 |
| 3 | **Salam-g** | 13.58 ms | 0.96x | 1.06x | 42 / 46 | 16 / 46 |
| 4 | **Salam-n** | 13.64 ms | 0.96x | 1.07x | 42 / 46 | 15 / 46 |
| 5 | C | 14.16 ms | 1.00x | 1.11x | 42 / 46 | 22 / 46 |
| 6 | Rust | 14.22 ms | 1.00x | 1.11x | 42 / 46 | 21 / 46 |
| 7 | C++ | 15.77 ms | 1.11x | 1.23x | 42 / 46 | 3 / 46 |
| 8 | Go | 18.59 ms | 1.31x | 1.45x | 42 / 46 | 0 / 46 |
| 9 | PHP | 161.11 ms | 11.38x | 12.59x | 42 / 46 | 0 / 46 |
| 10 | Python | 744.41 ms | 52.59x | 58.16x | 42 / 46 | 0 / 46 |

```
Salam-l      ███████████████████████████████                 12.80 ms  0.90x C
Salam-t      ███████████████████████████████                 12.95 ms  0.91x C
Salam-g      █████████████████████████████████               13.58 ms  0.96x C
Salam-n      █████████████████████████████████               13.64 ms  0.96x C
C            ██████████████████████████████████              14.16 ms  1.00x C  <- baseline
Rust         ██████████████████████████████████              14.22 ms  1.00x C
C++          ██████████████████████████████████████          15.77 ms  1.11x C
Go           █████████████████████████████████████████████   18.59 ms  1.31x C
PHP          (off scale)                                    161.11 ms  11.38x C
Python       (off scale)                                    744.41 ms  52.59x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 12.80 ms
  (0.90x C's geomean, 0.90x Rust's).
- **Salam-t is fastest or tied-fastest on the most programs** (
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
| 01_fib_recursive | 3 | 4 | 4 | 6 | 4 | 3 | 3 | 4 | 55 | 84 | **C / Salam (3)** |
| 02_fib_iterative | 18 | 20 | 19 | 20 | 19 | 20 | 19 | 19 | 57 | 624 | C (18) |
| 03_primes_count | 5 | 5 | 4 | 5 | 4 | 4 | 5 | 4 | 21 | 154 | **Rust / Salam (4)** |
| 04_collatz | 5 | 6 | 4 | 7 | 4 | 4 | 5 | 4 | 57 | 335 | **Rust / Salam (4)** |
| 05_sum_mod | 12 | 13 | 12 | 13 | 12 | 13 | 12 | 12 | 33 | 406 | **C / Rust / Salam (12)** |
| 06_gcd_sum | 15 | 16 | 14 | 16 | 15 | 15 | 15 | 14 | 49 | 137 | **Rust / Salam-t (14)** |
| 07_pow_mod | 3 | 4 | 3 | 4 | 3 | 3 | 3 | 3 | 20 | 62 | **C / Rust / Salam (3)** |
| 08_digit_sum | 4 | 5 | 4 | 5 | 4 | 4 | 4 | 4 | 62 | 347 | **C / Rust / Salam (4)** |
| 09_perfect_numbers | 9 | 10 | 8 | 10 | 8 | 8 | 9 | 8 | 40 | 381 | **Rust / Salam (8)** |
| 10_pi_leibniz | 6 | 6 | 6 | 7 | 6 | 6 | 6 | 6 | 48 | 546 | **C / C++ / Rust / Salam (6)** |
| 11_loop_count | 5 | 6 | 6 | 7 | 6 | 6 | 6 | 6 | 15 | 116 | C (5) |
| 12_hello_print | 18 | 18 | 3 | 8 | 3 | 3 | 3 | 3 | 616 | 94 | **Rust / Salam (3)** |
| 13_array_rw | 24 | 26 | 25 | 26 | 25 | 26 | 22 | 25 | 84 | 942 | **Salam-g (22)** |
| 14_lcg_random | 18 | 20 | 20 | 24 | 20 | 20 | 19 | 20 | 71 | 1326 | C (18) |
| 15_matrix_mult | 6 | 7 | 6 | 11 | 6 | 10 | 7 | 6 | 176 | 1115 | **C / Rust / Salam (6)** |
| 16_quicksort | 24 | 25 | 23 | 27 | 24 | 24 | 25 | 25 | 161 | 545 | Rust (23) |
| 17_n_queens | 36 | 37 | 38 | 42 | 41 | 43 | 41 | 40 | 438 | 604 | C (36) |
| 18_sieve_eratosthenes | 6 | 8 | 7 | 10 | 7 | 7 | 6 | 7 | 128 | 912 | **C / Salam-g (6)** |
| 19_mandelbrot | 11 | 12 | 11 | 12 | 11 | 11 | 11 | 11 | 129 | 1292 | **C / Rust / Salam (11)** |
| 20_monte_carlo_pi | 27 | 28 | 15 | 32 | 15 | 26 | 27 | 15 | 338 | 3492 | **Rust / Salam (15)** |
| 21_coin_change_dp | 3 | 4 | 3 | 4 | 3 | 3 | 3 | 3 | 18 | 114 | **C / Rust / Salam (3)** |
| 22_knapsack_01 | 5 | 6 | 5 | 7 | 6 | 6 | 6 | 6 | 112 | 1337 | C / Rust (5) |
| 23_caesar_cipher | 17 | 18 | 12 | 21 | 12 | 13 | 17 | 12 | 101 | 1115 | **Rust / Salam (12)** |
| 24_merge_sort | 28 | 28 | 30 | 33 | 28 | 31 | 31 | 33 | 272 | 763 | **C / C++ / Salam-l (28)** |
| 25_palindrome_count | 12 | 13 | 13 | 16 | 13 | 13 | 13 | 13 | 277 | 1700 | C (12) |
| 26_ackermann | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 2 | 11 | 23 | **C / Rust / Salam (2)** |
| 27_edit_distance | 7 | 8 | 8 | 10 | 7 | 8 | 7 | 7 | 251 | 2472 | **C / Salam (7)** |
| 28_prime_factorization | 979 | 981 | 850 | 971 | 850 | 840 | 980 | 848 | 4987 | 53676 | **Salam-n (840)** |
| 29_dot_product | 37 | 37 | 38 | 38 | 37 | 40 | 36 | 38 | 247 | 2917 | **Salam-g (36)** |
| 30_heap_sort | 32 | 32 | 34 | 41 | 34 | 34 | 32 | 34 | 498 | 1034 | **C / C++ / Salam-g (32)** |
| 31_binary_search_stress | 871 | 876 | 938 | 877 | 871 | 879 | 867 | 830 | 5690 | 27871 | **Salam-t (830)** |
| 32_longest_increasing_subsequence | 14 | 15 | 11 | 12 | 11 | 14 | 14 | 11 | 261 | 2394 | **Rust / Salam (11)** |
| 33_subset_sum_reachability | 17 | 19 | 19 | 19 | 17 | 18 | 18 | 17 | 324 | 2429 | **C / Salam (17)** |
| 34_counting_sort | 37 | 39 | 27 | 40 | 27 | 35 | 37 | 27 | 230 | 2425 | **Rust / Salam (27)** |
| 35_run_length_stats | 8 | 9 | 7 | 8 | 7 | 7 | 8 | 7 | 111 | 817 | **Rust / Salam (7)** |
| 36_array_rotation_reversal | 31 | 36 | 33 | 35 | 30 | 31 | 30 | 30 | 224 | 930 | **Salam (30)** |
| 37_sqrt_decomposition | 16 | 17 | 17 | 34 | 18 | 17 | 19 | 16 | 310 | 1178 | **C / Salam-t (16)** |
| 38_game_of_life | 10 | 11 | 30 | 45 | 9 | 12 | 11 | 9 | 1137 | 8118 | **Salam (9)** |
| 39_trapezoidal_integration | 13 | 14 | 12 | 15 | 12 | 13 | 13 | 12 | 413 | 2800 | **Rust / Salam (12)** |
| 40_matrix_stdlib_matmul | 45 | 46 | 79 | 75 | 13 | 13 | 13 | 13 | 1352 | 120 | **Salam (13)** |
| 41_matrix_stdlib_smallops | 5 | 6 | 6 | 19 | 39 | 45 | 49 | 52 | 474 | 1316 | C (5) |
| 42_tensor_stdlib_matmul | 45 | 46 | 80 | 76 | 6 | 6 | 7 | 7 | 1353 | 118 | **Salam (6)** |
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
| 1 | 42_tensor_stdlib_matmul | 6 | C (45) | 0.13x |
| 2 | 40_matrix_stdlib_matmul | 13 | C (45) | 0.29x |
| 3 | 38_game_of_life | 9 | C (10) | 0.90x |
| 4 | 36_array_rotation_reversal | 30 | C (31) | 0.97x |
| 5 | 26_ackermann | 2 | C (2) | 1.00x |
| 6 | 07_pow_mod | 3 | C (3) | 1.00x |
| 7 | 12_hello_print | 3 | Rust (3) | 1.00x |
| 8 | 21_coin_change_dp | 3 | C (3) | 1.00x |
| 9 | 08_digit_sum | 4 | C (4) | 1.00x |
| 10 | 04_collatz | 4 | Rust (4) | 1.00x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 39 | C (5) | 7.80x |
| 2 | 01_fib_recursive | 4 | C (3) | 1.33x |
| 3 | 22_knapsack_01 | 6 | C (5) | 1.20x |
| 4 | 11_loop_count | 6 | C (5) | 1.20x |
| 5 | 18_sieve_eratosthenes | 7 | C (6) | 1.17x |
| 6 | 17_n_queens | 41 | C (36) | 1.14x |
| 7 | 37_sqrt_decomposition | 18 | C (16) | 1.13x |
| 8 | 14_lcg_random | 20 | C (18) | 1.11x |
| 9 | 25_palindrome_count | 13 | C (12) | 1.08x |
| 10 | 06_gcd_sum | 15 | Rust (14) | 1.07x |
| 11 | 30_heap_sort | 34 | C (32) | 1.06x |
| 12 | 02_fib_iterative | 19 | C (18) | 1.06x |
| 13 | 16_quicksort | 24 | Rust (23) | 1.04x |
| 14 | 13_array_rw | 25 | C (24) | 1.04x |
| 15 | 31_binary_search_stress | 871 | C (871) | 1.00x |
| 16 | 28_prime_factorization | 850 | Rust (850) | 1.00x |
| 17 | 29_dot_product | 37 | C (37) | 1.00x |
| 18 | 24_merge_sort | 28 | C (28) | 1.00x |
| 19 | 34_counting_sort | 27 | Rust (27) | 1.00x |
| 20 | 33_subset_sum_reachability | 17 | C (17) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (8):** 13_array_rw, 28_prime_factorization, 29_dot_product, 31_binary_search_stress, 36_array_rotation_reversal, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (26):** 01_fib_recursive, 03_primes_count, 04_collatz, 05_sum_mod, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 12_hello_print, 15_matrix_mult, 18_sieve_eratosthenes, 19_mandelbrot, 20_monte_carlo_pi, 21_coin_change_dp, 23_caesar_cipher, 24_merge_sort, 26_ackermann, 27_edit_distance, 30_heap_sort, 32_longest_increasing_subsequence, 33_subset_sum_reachability, 34_counting_sort, 35_run_length_stats, 37_sqrt_decomposition, 39_trapezoidal_integration.
- **Trailing the fastest (8):** 02_fib_iterative (+1 vs C); 11_loop_count (+1 vs C); 14_lcg_random (+1 vs C); 16_quicksort (+1 vs Rust); 17_n_queens (+4 vs C); 22_knapsack_01 (+1 vs C/Rust); 25_palindrome_count (+1 vs C); 41_matrix_stdlib_smallops (+34 vs C).

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
