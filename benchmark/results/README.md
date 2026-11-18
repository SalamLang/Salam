# Salam Benchmark Suite Results

46 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured four ways: the LLVM backend at
`-O3`, the native x86-64 backend (`--backend=native`), the C backend compiled
by `gcc -O3`, and the default `tcc` toolchain (no optimizer) as the
out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-11-18T02:14:33Z (took 30 min)
- Commit: `0991f06f`

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
- **salam**: 0.6.17

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
| 1 | **Salam-n** | 10.04 ms | 0.82x | 1.00x | 42 / 46 | 32 / 46 |
| 2 | **Salam-l** | 10.82 ms | 0.88x | 1.08x | 42 / 46 | 21 / 46 |
| 3 | **Salam-t** | 11.43 ms | 0.93x | 1.14x | 42 / 46 | 16 / 46 |
| 4 | **Salam-g** | 12.21 ms | 1.00x | 1.22x | 42 / 46 | 11 / 46 |
| 5 | C | 12.24 ms | 1.00x | 1.22x | 42 / 46 | 15 / 46 |
| 6 | Rust | 12.28 ms | 1.00x | 1.22x | 42 / 46 | 14 / 46 |
| 7 | C++ | 13.50 ms | 1.10x | 1.34x | 42 / 46 | 4 / 46 |
| 8 | Go | 16.36 ms | 1.34x | 1.63x | 42 / 46 | 1 / 46 |
| 9 | PHP | 123.02 ms | 10.05x | 12.25x | 42 / 46 | 0 / 46 |
| 10 | Python | 577.82 ms | 47.20x | 57.54x | 42 / 46 | 0 / 46 |

```
Salam-n      ████████████████████████████                    10.04 ms  0.82x C
Salam-l      ██████████████████████████████                  10.82 ms  0.88x C
Salam-t      ███████████████████████████████                 11.43 ms  0.93x C
Salam-g      ██████████████████████████████████              12.21 ms  1.00x C
C            ██████████████████████████████████              12.24 ms  1.00x C  <- baseline
Rust         ██████████████████████████████████              12.28 ms  1.00x C
C++          █████████████████████████████████████           13.50 ms  1.10x C
Go           █████████████████████████████████████████████   16.36 ms  1.34x C
PHP          (off scale)                                    123.02 ms  10.05x C
Python       (off scale)                                    577.82 ms  47.20x C
```

Takeaways:

- **Fastest Salam backend:** Salam-n, geomean 10.04 ms
  (0.82x C's geomean, 0.82x Rust's).
- **Salam-n is fastest or tied-fastest on the most programs** (
  32/46); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.14x slower than the
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
| 01_fib_recursive | 2 | 3 | 3 | 5 | 3 | 2 | 2 | 3 | 40 | 70 | **C / Salam (2)** |
| 02_fib_iterative | 17 | 17 | 18 | 17 | 17 | 13 | 17 | 18 | 49 | 497 | **Salam-n (13)** |
| 03_primes_count | 5 | 6 | 4 | 6 | 4 | 3 | 5 | 4 | 15 | 123 | **Salam-n (3)** |
| 04_collatz | 5 | 6 | 4 | 7 | 4 | 4 | 6 | 4 | 48 | 268 | **Rust / Salam (4)** |
| 05_sum_mod | 11 | 11 | 11 | 11 | 11 | 9 | 11 | 11 | 26 | 308 | **Salam-n (9)** |
| 06_gcd_sum | 20 | 20 | 15 | 20 | 15 | 15 | 20 | 15 | 41 | 113 | **Rust / Salam (15)** |
| 07_pow_mod | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 2 | 15 | 49 | **C / Rust / Salam (2)** |
| 08_digit_sum | 3 | 4 | 3 | 4 | 3 | 3 | 3 | 3 | 42 | 269 | **C / Rust / Salam (3)** |
| 09_perfect_numbers | 11 | 12 | 7 | 12 | 7 | 6 | 11 | 7 | 28 | 304 | **Salam-n (6)** |
| 10_pi_leibniz | 4 | 5 | 4 | 5 | 4 | 4 | 4 | 4 | 38 | 379 | **C / Rust / Salam (4)** |
| 11_loop_count | 5 | 5 | 5 | 5 | 5 | 5 | 5 | 5 | 11 | 85 | **C / C++ / Rust / Go / Salam (5)** |
| 12_hello_print | 13 | 15 | 2 | 5 | 2 | 2 | 3 | 3 | 160 | 64 | **Rust / Salam (2)** |
| 13_array_rw | 21 | 22 | 22 | 22 | 22 | 22 | 19 | 22 | 62 | 714 | **Salam-g (19)** |
| 14_lcg_random | 17 | 18 | 18 | 22 | 18 | 13 | 17 | 18 | 70 | 1100 | **Salam-n (13)** |
| 15_matrix_mult | 4 | 5 | 4 | 9 | 4 | 4 | 4 | 4 | 156 | 855 | **C / Rust / Salam (4)** |
| 16_quicksort | 25 | 25 | 24 | 25 | 22 | 23 | 26 | 24 | 138 | 420 | **Salam-l (22)** |
| 17_n_queens | 39 | 40 | 41 | 42 | 45 | 43 | 48 | 43 | 380 | 519 | C (39) |
| 18_sieve_eratosthenes | 8 | 9 | 8 | 9 | 8 | 7 | 8 | 8 | 111 | 732 | **Salam-n (7)** |
| 19_mandelbrot | 9 | 9 | 9 | 10 | 9 | 9 | 9 | 9 | 99 | 893 | **C / C++ / Rust / Salam (9)** |
| 20_monte_carlo_pi | 27 | 28 | 15 | 32 | 15 | 13 | 27 | 12 | 277 | 3027 | **Salam-t (12)** |
| 21_coin_change_dp | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 2 | 13 | 84 | **C / Rust / Salam (2)** |
| 22_knapsack_01 | 3 | 4 | 4 | 5 | 3 | 3 | 5 | 5 | 87 | 1012 | **C / Salam (3)** |
| 23_caesar_cipher | 11 | 12 | 11 | 15 | 11 | 8 | 11 | 11 | 78 | 907 | **Salam-n (8)** |
| 24_merge_sort | 28 | 28 | 30 | 34 | 28 | 29 | 31 | 31 | 235 | 613 | **C / C++ / Salam-l (28)** |
| 25_palindrome_count | 11 | 11 | 10 | 13 | 10 | 12 | 11 | 10 | 190 | 1343 | **Rust / Salam (10)** |
| 26_ackermann | 1 | 2 | 1 | 2 | 1 | 1 | 1 | 1 | 8 | 17 | **C / Rust / Salam (1)** |
| 27_edit_distance | 6 | 7 | 6 | 12 | 5 | 4 | 5 | 5 | 187 | 1855 | **Salam-n (4)** |
| 28_prime_factorization | 1326 | 1324 | 818 | 1314 | 821 | 802 | 1322 | 821 | 3385 | 41989 | **Salam-n (802)** |
| 29_dot_product | 33 | 33 | 34 | 33 | 33 | 25 | 33 | 34 | 186 | 2108 | **Salam-n (25)** |
| 30_heap_sort | 32 | 32 | 36 | 39 | 32 | 32 | 33 | 34 | 401 | 834 | **C / C++ / Salam (32)** |
| 31_binary_search_stress | 1039 | 1074 | 724 | 1035 | 1020 | 984 | 1071 | 1060 | 3325 | 18035 | Rust (724) |
| 32_longest_increasing_subsequence | 9 | 10 | 22 | 10 | 8 | 8 | 9 | 9 | 210 | 1856 | **Salam (8)** |
| 33_subset_sum_reachability | 9 | 10 | 19 | 15 | 10 | 8 | 16 | 18 | 249 | 1833 | **Salam-n (8)** |
| 34_counting_sort | 31 | 32 | 24 | 35 | 24 | 20 | 31 | 24 | 186 | 1904 | **Salam-n (20)** |
| 35_run_length_stats | 7 | 8 | 6 | 8 | 5 | 6 | 7 | 5 | 84 | 644 | **Salam (5)** |
| 36_array_rotation_reversal | 24 | 28 | 26 | 25 | 24 | 25 | 24 | 24 | 180 | 743 | **C / Salam (24)** |
| 37_sqrt_decomposition | 12 | 13 | 11 | 18 | 11 | 11 | 13 | 11 | 244 | 887 | **Rust / Salam (11)** |
| 38_game_of_life | 8 | 9 | 27 | 41 | 6 | 5 | 7 | 6 | 866 | 6639 | **Salam-n (5)** |
| 39_trapezoidal_integration | 12 | 12 | 12 | 12 | 12 | 11 | 12 | 12 | 315 | 1974 | **Salam-n (11)** |
| 40_matrix_stdlib_matmul | 33 | 31 | 53 | 85 | 11 | 10 | 11 | 11 | 1363 | 104 | **Salam-n (10)** |
| 41_matrix_stdlib_smallops | 4 | 5 | 5 | 19 | 38 | 40 | 53 | 63 | 415 | 1069 | C (4) |
| 42_tensor_stdlib_matmul | 34 | 30 | 54 | 86 | 6 | 6 | 7 | 6 | 1356 | 98 | **Salam (6)** |
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
| 1 | 42_tensor_stdlib_matmul | 6 | C++ (30) | 0.20x |
| 2 | 40_matrix_stdlib_matmul | 11 | C++ (31) | 0.35x |
| 3 | 38_game_of_life | 6 | C (8) | 0.75x |
| 4 | 27_edit_distance | 5 | C (6) | 0.83x |
| 5 | 35_run_length_stats | 5 | Rust (6) | 0.83x |
| 6 | 32_longest_increasing_subsequence | 8 | C (9) | 0.89x |
| 7 | 16_quicksort | 22 | Rust (24) | 0.92x |
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
| 3 | 31_binary_search_stress | 1020 | Rust (724) | 1.41x |
| 4 | 17_n_queens | 45 | C (39) | 1.15x |
| 5 | 33_subset_sum_reachability | 10 | C (9) | 1.11x |
| 6 | 14_lcg_random | 18 | C (17) | 1.06x |
| 7 | 13_array_rw | 22 | C (21) | 1.05x |
| 8 | 28_prime_factorization | 821 | Rust (818) | 1.00x |
| 9 | 29_dot_product | 33 | C (33) | 1.00x |
| 10 | 30_heap_sort | 32 | C (32) | 1.00x |
| 11 | 24_merge_sort | 28 | C (28) | 1.00x |
| 12 | 36_array_rotation_reversal | 24 | C (24) | 1.00x |
| 13 | 34_counting_sort | 24 | Rust (24) | 1.00x |
| 14 | 02_fib_iterative | 17 | C (17) | 1.00x |
| 15 | 20_monte_carlo_pi | 15 | Rust (15) | 1.00x |
| 16 | 06_gcd_sum | 15 | Rust (15) | 1.00x |
| 17 | 39_trapezoidal_integration | 12 | C (12) | 1.00x |
| 18 | 37_sqrt_decomposition | 11 | Rust (11) | 1.00x |
| 19 | 05_sum_mod | 11 | C (11) | 1.00x |
| 20 | 23_caesar_cipher | 11 | C (11) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (21):** 02_fib_iterative, 03_primes_count, 05_sum_mod, 09_perfect_numbers, 13_array_rw, 14_lcg_random, 16_quicksort, 18_sieve_eratosthenes, 20_monte_carlo_pi, 23_caesar_cipher, 27_edit_distance, 28_prime_factorization, 29_dot_product, 32_longest_increasing_subsequence, 33_subset_sum_reachability, 34_counting_sort, 35_run_length_stats, 38_game_of_life, 39_trapezoidal_integration, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (18):** 01_fib_recursive, 04_collatz, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 10_pi_leibniz, 11_loop_count, 12_hello_print, 15_matrix_mult, 19_mandelbrot, 21_coin_change_dp, 22_knapsack_01, 24_merge_sort, 25_palindrome_count, 26_ackermann, 30_heap_sort, 36_array_rotation_reversal, 37_sqrt_decomposition.
- **Trailing the fastest (3):** 17_n_queens (+4 vs C); 31_binary_search_stress (+260 vs Rust); 41_matrix_stdlib_smallops (+34 vs C).

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
