# Salam Benchmark Suite Results

46 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured four ways: the LLVM backend at
`-O3`, the native x86-64 backend (`--backend=native`), the C backend compiled
by `gcc -O3`, and the default `tcc` toolchain (no optimizer) as the
out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-11-24T13:01:51Z (took 29 min)
- Commit: `9fee9afc`

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
| 1 | **Salam-l** | 11.08 ms | 0.91x | 1.00x | 42 / 46 | 27 / 46 |
| 2 | **Salam-t** | 11.29 ms | 0.93x | 1.02x | 42 / 46 | 25 / 46 |
| 3 | **Salam-n** | 11.76 ms | 0.97x | 1.06x | 42 / 46 | 19 / 46 |
| 4 | **Salam-g** | 11.97 ms | 0.99x | 1.08x | 42 / 46 | 20 / 46 |
| 5 | C | 12.14 ms | 1.00x | 1.10x | 42 / 46 | 24 / 46 |
| 6 | Rust | 12.24 ms | 1.01x | 1.11x | 42 / 46 | 20 / 46 |
| 7 | C++ | 13.37 ms | 1.10x | 1.21x | 42 / 46 | 10 / 46 |
| 8 | Go | 16.18 ms | 1.33x | 1.46x | 42 / 46 | 6 / 46 |
| 9 | PHP | 121.95 ms | 10.04x | 11.01x | 42 / 46 | 0 / 46 |
| 10 | Python | 568.72 ms | 46.83x | 51.35x | 42 / 46 | 0 / 46 |

```
Salam-l      ███████████████████████████████                 11.08 ms  0.91x C
Salam-t      ███████████████████████████████                 11.29 ms  0.93x C
Salam-n      █████████████████████████████████               11.76 ms  0.97x C
Salam-g      █████████████████████████████████               11.97 ms  0.99x C
C            ██████████████████████████████████              12.14 ms  1.00x C  <- baseline
Rust         ██████████████████████████████████              12.24 ms  1.01x C
C++          █████████████████████████████████████           13.37 ms  1.10x C
Go           █████████████████████████████████████████████   16.18 ms  1.33x C
PHP          (off scale)                                    121.95 ms  10.04x C
Python       (off scale)                                    568.72 ms  46.83x C
```

Takeaways:

- **Fastest Salam backend:** Salam-l, geomean 11.08 ms
  (0.91x C's geomean, 0.90x Rust's).
- **Salam-l is fastest or tied-fastest on the most programs** (
  27/46); see the per-program table below.
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
| 01_fib_recursive | 2 | 3 | 3 | 5 | 3 | 2 | 2 | 3 | 40 | 69 | **C / Salam (2)** |
| 02_fib_iterative | 17 | 17 | 18 | 17 | 17 | 18 | 17 | 18 | 49 | 490 | **C / C++ / Go / Salam (17)** |
| 03_primes_count | 5 | 6 | 4 | 6 | 4 | 3 | 5 | 4 | 15 | 122 | **Salam-n (3)** |
| 04_collatz | 5 | 6 | 4 | 7 | 4 | 4 | 5 | 4 | 48 | 268 | **Rust / Salam (4)** |
| 05_sum_mod | 10 | 11 | 11 | 11 | 11 | 11 | 11 | 11 | 26 | 299 | C (10) |
| 06_gcd_sum | 20 | 20 | 15 | 20 | 17 | 17 | 20 | 15 | 40 | 112 | **Rust / Salam-t (15)** |
| 07_pow_mod | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 2 | 14 | 49 | **C / Rust / Salam (2)** |
| 08_digit_sum | 3 | 4 | 3 | 4 | 3 | 3 | 3 | 3 | 43 | 269 | **C / Rust / Salam (3)** |
| 09_perfect_numbers | 11 | 12 | 7 | 12 | 8 | 7 | 11 | 8 | 28 | 302 | **Rust / Salam-n (7)** |
| 10_pi_leibniz | 4 | 5 | 4 | 5 | 4 | 4 | 4 | 4 | 39 | 375 | **C / Rust / Salam (4)** |
| 11_loop_count | 5 | 5 | 5 | 5 | 5 | 5 | 5 | 5 | 11 | 84 | **C / C++ / Rust / Go / Salam (5)** |
| 12_hello_print | 13 | 15 | 2 | 5 | 2 | 2 | 3 | 3 | 161 | 64 | **Rust / Salam (2)** |
| 13_array_rw | 21 | 22 | 22 | 22 | 22 | 22 | 19 | 22 | 62 | 701 | **Salam-g (19)** |
| 14_lcg_random | 17 | 17 | 18 | 21 | 18 | 18 | 17 | 18 | 70 | 1068 | **C / C++ / Salam-g (17)** |
| 15_matrix_mult | 4 | 5 | 4 | 9 | 4 | 6 | 4 | 4 | 153 | 838 | **C / Rust / Salam (4)** |
| 16_quicksort | 25 | 25 | 24 | 24 | 24 | 24 | 25 | 24 | 136 | 412 | **Rust / Go / Salam (24)** |
| 17_n_queens | 39 | 39 | 41 | 42 | 46 | 48 | 45 | 43 | 391 | 515 | C / C++ (39) |
| 18_sieve_eratosthenes | 8 | 9 | 8 | 9 | 8 | 8 | 8 | 8 | 103 | 708 | **C / Rust / Salam (8)** |
| 19_mandelbrot | 9 | 9 | 9 | 9 | 9 | 9 | 9 | 9 | 95 | 878 | **C / C++ / Rust / Go / Salam (9)** |
| 20_monte_carlo_pi | 27 | 27 | 15 | 31 | 15 | 25 | 27 | 12 | 275 | 2766 | **Salam-t (12)** |
| 21_coin_change_dp | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 2 | 13 | 83 | **C / Rust / Salam (2)** |
| 22_knapsack_01 | 3 | 4 | 4 | 5 | 5 | 4 | 5 | 5 | 86 | 990 | C (3) |
| 23_caesar_cipher | 11 | 12 | 11 | 14 | 11 | 11 | 11 | 11 | 73 | 899 | **C / Rust / Salam (11)** |
| 24_merge_sort | 28 | 28 | 29 | 32 | 28 | 29 | 31 | 31 | 228 | 608 | **C / C++ / Salam-l (28)** |
| 25_palindrome_count | 11 | 11 | 10 | 13 | 10 | 12 | 11 | 10 | 190 | 1308 | **Rust / Salam (10)** |
| 26_ackermann | 1 | 2 | 1 | 2 | 1 | 1 | 1 | 1 | 8 | 17 | **C / Rust / Salam (1)** |
| 27_edit_distance | 6 | 7 | 6 | 12 | 5 | 11 | 5 | 5 | 183 | 1886 | **Salam (5)** |
| 28_prime_factorization | 1322 | 1319 | 816 | 1310 | 816 | 799 | 1321 | 826 | 3381 | 41717 | **Salam-n (799)** |
| 29_dot_product | 33 | 33 | 34 | 33 | 33 | 35 | 33 | 33 | 185 | 2120 | **C / C++ / Go / Salam (33)** |
| 30_heap_sort | 32 | 32 | 36 | 39 | 33 | 33 | 33 | 34 | 394 | 827 | C / C++ (32) |
| 31_binary_search_stress | 1041 | 1043 | 724 | 1037 | 1079 | 1057 | 1044 | 1065 | 3386 | 17938 | Rust (724) |
| 32_longest_increasing_subsequence | 9 | 10 | 22 | 10 | 9 | 10 | 9 | 9 | 210 | 1849 | **C / Salam (9)** |
| 33_subset_sum_reachability | 9 | 10 | 19 | 15 | 18 | 15 | 16 | 18 | 259 | 1835 | C (9) |
| 34_counting_sort | 31 | 32 | 24 | 35 | 24 | 33 | 31 | 24 | 183 | 1903 | **Rust / Salam (24)** |
| 35_run_length_stats | 7 | 8 | 6 | 8 | 5 | 6 | 7 | 5 | 85 | 640 | **Salam (5)** |
| 36_array_rotation_reversal | 24 | 28 | 26 | 26 | 24 | 25 | 24 | 24 | 178 | 706 | **C / Salam (24)** |
| 37_sqrt_decomposition | 12 | 12 | 11 | 18 | 11 | 11 | 13 | 11 | 241 | 872 | **Rust / Salam (11)** |
| 38_game_of_life | 8 | 9 | 27 | 38 | 6 | 9 | 7 | 6 | 860 | 6653 | **Salam (6)** |
| 39_trapezoidal_integration | 12 | 12 | 12 | 12 | 12 | 12 | 12 | 12 | 292 | 1949 | **C / C++ / Rust / Go / Salam (12)** |
| 40_matrix_stdlib_matmul | 34 | 29 | 54 | 84 | 10 | 11 | 11 | 11 | 1321 | 94 | **Salam-l (10)** |
| 41_matrix_stdlib_smallops | 4 | 4 | 5 | 18 | 37 | 40 | 45 | 48 | 405 | 1068 | C / C++ (4) |
| 42_tensor_stdlib_matmul | 33 | 29 | 54 | 84 | 5 | 5 | 5 | 5 | 1360 | 93 | **Salam (5)** |
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
| 1 | 42_tensor_stdlib_matmul | 5 | C++ (29) | 0.17x |
| 2 | 40_matrix_stdlib_matmul | 10 | C++ (29) | 0.34x |
| 3 | 38_game_of_life | 6 | C (8) | 0.75x |
| 4 | 27_edit_distance | 5 | C (6) | 0.83x |
| 5 | 35_run_length_stats | 5 | Rust (6) | 0.83x |
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
| 1 | 41_matrix_stdlib_smallops | 37 | C (4) | 9.25x |
| 2 | 33_subset_sum_reachability | 18 | C (9) | 2.00x |
| 3 | 22_knapsack_01 | 5 | C (3) | 1.67x |
| 4 | 01_fib_recursive | 3 | C (2) | 1.50x |
| 5 | 31_binary_search_stress | 1079 | Rust (724) | 1.49x |
| 6 | 17_n_queens | 46 | C (39) | 1.18x |
| 7 | 09_perfect_numbers | 8 | Rust (7) | 1.14x |
| 8 | 06_gcd_sum | 17 | Rust (15) | 1.13x |
| 9 | 05_sum_mod | 11 | C (10) | 1.10x |
| 10 | 14_lcg_random | 18 | C (17) | 1.06x |
| 11 | 13_array_rw | 22 | C (21) | 1.05x |
| 12 | 30_heap_sort | 33 | C (32) | 1.03x |
| 13 | 28_prime_factorization | 816 | Rust (816) | 1.00x |
| 14 | 29_dot_product | 33 | C (33) | 1.00x |
| 15 | 24_merge_sort | 28 | C (28) | 1.00x |
| 16 | 36_array_rotation_reversal | 24 | C (24) | 1.00x |
| 17 | 34_counting_sort | 24 | Rust (24) | 1.00x |
| 18 | 16_quicksort | 24 | Rust (24) | 1.00x |
| 19 | 02_fib_iterative | 17 | C (17) | 1.00x |
| 20 | 20_monte_carlo_pi | 15 | Rust (15) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (9):** 03_primes_count, 13_array_rw, 20_monte_carlo_pi, 27_edit_distance, 28_prime_factorization, 35_run_length_stats, 38_game_of_life, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (26):** 01_fib_recursive, 02_fib_iterative, 04_collatz, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 11_loop_count, 12_hello_print, 14_lcg_random, 15_matrix_mult, 16_quicksort, 18_sieve_eratosthenes, 19_mandelbrot, 21_coin_change_dp, 23_caesar_cipher, 24_merge_sort, 25_palindrome_count, 26_ackermann, 29_dot_product, 32_longest_increasing_subsequence, 34_counting_sort, 36_array_rotation_reversal, 37_sqrt_decomposition, 39_trapezoidal_integration.
- **Trailing the fastest (7):** 05_sum_mod (+1 vs C); 17_n_queens (+4 vs C/C++); 22_knapsack_01 (+1 vs C); 30_heap_sort (+1 vs C/C++); 31_binary_search_stress (+320 vs Rust); 33_subset_sum_reachability (+6 vs C); 41_matrix_stdlib_smallops (+33 vs C/C++).

Salam matches or beats the fastest other compiled language on 35 of 46 programs.

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
