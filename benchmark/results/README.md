# Salam Benchmark Suite Results

46 CPU- and IO-bound programs implemented identically in C, C++, Rust, Go,
Python, PHP, and Salam. Salam is measured four ways: the LLVM backend at
`-O3`, the native x86-64 backend (`--backend=native`), the C backend compiled
by `gcc -O3`, and the default `tcc` toolchain (no optimizer) as the
out-of-the-box reference. C and C++ run at `-O3` too.

- Date: 2026-11-19T07:52:40Z (took 29 min)
- Commit: `81efe12e`

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

- **CPU**: AMD EPYC 9V45 96-Core Processor
- **gcc**: gcc (Alpine 15.2.0) 15.2.0
- **g++**: g++ (Alpine 15.2.0) 15.2.0
- **rustc**: rustc 1.96.1 (31fca3adb 2026-06-26) (Alpine Linux Rust 1.96.1-r0)
- **go**: go version go1.26.11 linux/amd64
- **tcc**: tcc version 0.9.28rc (x86_64 Linux)
- **clang**: Alpine clang version 22.1.3
- **python**: Python 3.14.10
- **numpy**: 2.4.6
- **php**: PHP 8.5.12 (cli) (built: Oct 21 2026 16:29:26) (NTS)
- **salam**: 0.6.18

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
| 1 | **Salam-n** | 9.86 ms | 0.83x | 1.00x | 42 / 46 | 28 / 46 |
| 2 | **Salam-l** | 10.45 ms | 0.88x | 1.06x | 42 / 46 | 19 / 46 |
| 3 | **Salam-t** | 10.70 ms | 0.91x | 1.08x | 42 / 46 | 19 / 46 |
| 4 | **Salam-g** | 11.21 ms | 0.95x | 1.14x | 42 / 46 | 14 / 46 |
| 5 | C | 11.81 ms | 1.00x | 1.20x | 42 / 46 | 13 / 46 |
| 6 | Rust | 11.93 ms | 1.01x | 1.21x | 42 / 46 | 14 / 46 |
| 7 | C++ | 13.14 ms | 1.11x | 1.33x | 42 / 46 | 3 / 46 |
| 8 | Go | 14.92 ms | 1.26x | 1.51x | 42 / 46 | 1 / 46 |
| 9 | PHP | 110.32 ms | 9.34x | 11.19x | 42 / 46 | 0 / 46 |
| 10 | Python | 527.67 ms | 44.66x | 53.51x | 42 / 46 | 0 / 46 |

```
Salam-n      ██████████████████████████████                   9.86 ms  0.83x C
Salam-l      ████████████████████████████████                10.45 ms  0.88x C
Salam-t      ████████████████████████████████                10.70 ms  0.91x C
Salam-g      ██████████████████████████████████              11.21 ms  0.95x C
C            ████████████████████████████████████            11.81 ms  1.00x C  <- baseline
Rust         ████████████████████████████████████            11.93 ms  1.01x C
C++          ████████████████████████████████████████        13.14 ms  1.11x C
Go           █████████████████████████████████████████████   14.92 ms  1.26x C
PHP          (off scale)                                    110.32 ms  9.34x C
Python       (off scale)                                    527.67 ms  44.66x C
```

Takeaways:

- **Fastest Salam backend:** Salam-n, geomean 9.86 ms
  (0.83x C's geomean, 0.83x Rust's).
- **Salam-n is fastest or tied-fastest on the most programs** (
  28/46); see the per-program table below.
- **`salam build` with no flags** (Salam-t, tcc, no optimizer) is 1.08x slower than the
  fastest Salam backend, so it is the out-of-the-box baseline, not the number
  to compare against other optimized languages -- pass `-O3` / use the LLVM
  backend for like-for-like comparisons.
- **Python and PHP are 9-45x slower** than the compiled languages on
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
| 01_fib_recursive | 2 | 3 | 3 | 5 | 3 | 2 | 2 | 3 | 36 | 60 | **C / Salam (2)** |
| 02_fib_iterative | 15 | 16 | 15 | 16 | 15 | 11 | 15 | 15 | 39 | 409 | **Salam-n (11)** |
| 03_primes_count | 4 | 5 | 4 | 5 | 4 | 3 | 4 | 4 | 15 | 105 | **Salam-n (3)** |
| 04_collatz | 4 | 5 | 4 | 6 | 4 | 3 | 4 | 4 | 42 | 224 | **Salam-n (3)** |
| 05_sum_mod | 10 | 10 | 10 | 10 | 9 | 8 | 10 | 10 | 23 | 255 | **Salam-n (8)** |
| 06_gcd_sum | 14 | 15 | 12 | 15 | 13 | 12 | 14 | 12 | 34 | 95 | **Rust / Salam (12)** |
| 07_pow_mod | 2 | 3 | 2 | 3 | 2 | 2 | 2 | 2 | 15 | 44 | **C / Rust / Salam (2)** |
| 08_digit_sum | 3 | 4 | 3 | 4 | 3 | 3 | 3 | 3 | 37 | 234 | **C / Rust / Salam (3)** |
| 09_perfect_numbers | 8 | 9 | 6 | 9 | 6 | 6 | 8 | 6 | 28 | 262 | **Rust / Salam (6)** |
| 10_pi_leibniz | 4 | 5 | 4 | 5 | 4 | 4 | 4 | 4 | 35 | 333 | **C / Rust / Salam (4)** |
| 11_loop_count | 5 | 5 | 5 | 5 | 5 | 5 | 4 | 5 | 11 | 77 | **Salam-g (4)** |
| 12_hello_print | 13 | 13 | 2 | 5 | 2 | 2 | 2 | 2 | 315 | 67 | **Rust / Salam (2)** |
| 13_array_rw | 21 | 22 | 21 | 22 | 21 | 21 | 19 | 20 | 55 | 595 | **Salam-g (19)** |
| 14_lcg_random | 16 | 17 | 17 | 20 | 16 | 12 | 15 | 16 | 51 | 900 | **Salam-n (12)** |
| 15_matrix_mult | 5 | 6 | 4 | 7 | 4 | 5 | 6 | 4 | 111 | 709 | **Rust / Salam (4)** |
| 16_quicksort | 22 | 24 | 21 | 23 | 22 | 21 | 23 | 22 | 113 | 374 | **Rust / Salam-n (21)** |
| 17_n_queens | 35 | 36 | 36 | 37 | 36 | 36 | 39 | 36 | 304 | 482 | C (35) |
| 18_sieve_eratosthenes | 5 | 6 | 5 | 6 | 5 | 5 | 5 | 5 | 121 | 661 | **C / Rust / Salam (5)** |
| 19_mandelbrot | 8 | 9 | 8 | 9 | 9 | 8 | 8 | 8 | 73 | 834 | **C / Rust / Salam (8)** |
| 20_monte_carlo_pi | 22 | 23 | 12 | 26 | 12 | 10 | 22 | 8 | 197 | 2477 | **Salam-t (8)** |
| 21_coin_change_dp | 3 | 3 | 3 | 4 | 3 | 3 | 3 | 3 | 14 | 75 | **C / C++ / Rust / Salam (3)** |
| 22_knapsack_01 | 3 | 4 | 4 | 5 | 3 | 3 | 4 | 4 | 74 | 861 | **C / Salam (3)** |
| 23_caesar_cipher | 11 | 11 | 10 | 14 | 10 | 8 | 10 | 10 | 62 | 797 | **Salam-n (8)** |
| 24_merge_sort | 26 | 27 | 26 | 29 | 27 | 27 | 28 | 29 | 189 | 528 | C / Rust (26) |
| 25_palindrome_count | 9 | 10 | 10 | 12 | 10 | 9 | 10 | 9 | 167 | 1133 | **C / Salam (9)** |
| 26_ackermann | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 8 | 18 | **C / C++ / Rust / Go / Salam (2)** |
| 27_edit_distance | 7 | 8 | 8 | 10 | 5 | 6 | 6 | 5 | 156 | 1645 | **Salam (5)** |
| 28_prime_factorization | 811 | 833 | 703 | 811 | 713 | 694 | 812 | 713 | 3053 | 37183 | **Salam-n (694)** |
| 29_dot_product | 32 | 33 | 32 | 34 | 32 | 24 | 31 | 32 | 158 | 1949 | **Salam-n (24)** |
| 30_heap_sort | 31 | 30 | 32 | 40 | 30 | 33 | 30 | 32 | 348 | 953 | **C++ / Salam (30)** |
| 31_binary_search_stress | 810 | 825 | 1175 | 809 | 737 | 759 | 831 | 877 | 4487 | 23453 | **Salam-l (737)** |
| 32_longest_increasing_subsequence | 10 | 12 | 12 | 12 | 9 | 9 | 11 | 9 | 196 | 1816 | **Salam (9)** |
| 33_subset_sum_reachability | 10 | 12 | 18 | 14 | 8 | 10 | 12 | 12 | 229 | 1766 | **Salam-l (8)** |
| 34_counting_sort | 32 | 33 | 23 | 34 | 23 | 22 | 32 | 24 | 179 | 1819 | **Salam-n (22)** |
| 35_run_length_stats | 7 | 7 | 6 | 8 | 5 | 5 | 7 | 5 | 80 | 601 | **Salam (5)** |
| 36_array_rotation_reversal | 29 | 32 | 29 | 32 | 30 | 29 | 29 | 28 | 166 | 676 | **Salam-t (28)** |
| 37_sqrt_decomposition | 18 | 19 | 17 | 32 | 17 | 17 | 20 | 17 | 238 | 813 | **Rust / Salam (17)** |
| 38_game_of_life | 9 | 10 | 23 | 32 | 7 | 5 | 7 | 7 | 891 | 5899 | **Salam-n (5)** |
| 39_trapezoidal_integration | 10 | 12 | 10 | 12 | 9 | 10 | 10 | 10 | 242 | 1755 | **Salam-l (9)** |
| 40_matrix_stdlib_matmul | 36 | 37 | 56 | 43 | 13 | 12 | 11 | 12 | 862 | 110 | **Salam-g (11)** |
| 41_matrix_stdlib_smallops | 4 | 5 | 5 | 14 | 31 | 32 | 44 | 46 | 316 | 968 | C (4) |
| 42_tensor_stdlib_matmul | 35 | 36 | 53 | 42 | 7 | 7 | 7 | 7 | 840 | 113 | **Salam (7)** |
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
| 1 | 42_tensor_stdlib_matmul | 7 | C (35) | 0.20x |
| 2 | 40_matrix_stdlib_matmul | 13 | C (36) | 0.36x |
| 3 | 27_edit_distance | 5 | C (7) | 0.71x |
| 4 | 38_game_of_life | 7 | C (9) | 0.78x |
| 5 | 33_subset_sum_reachability | 8 | C (10) | 0.80x |
| 6 | 35_run_length_stats | 5 | Rust (6) | 0.83x |
| 7 | 32_longest_increasing_subsequence | 9 | C (10) | 0.90x |
| 8 | 05_sum_mod | 9 | C (10) | 0.90x |
| 9 | 39_trapezoidal_integration | 9 | C (10) | 0.90x |
| 10 | 31_binary_search_stress | 737 | Go (809) | 0.91x |

### Top 20 slowest programs in Salam-l

The same ranking inverted: the programs where Salam-l gives up the most
against the best other language, worst ratio first. These are the ones worth
profiling. Ranked over the 42 programs that have both a Salam-l and a
non-Salam timing.

| Rank | Program | Salam-l (ms) | Best other language | vs best other |
|---|---|---|---|---|
| 1 | 41_matrix_stdlib_smallops | 31 | C (4) | 7.75x |
| 2 | 01_fib_recursive | 3 | C (2) | 1.50x |
| 3 | 19_mandelbrot | 9 | C (8) | 1.13x |
| 4 | 25_palindrome_count | 10 | C (9) | 1.11x |
| 5 | 06_gcd_sum | 13 | Rust (12) | 1.08x |
| 6 | 16_quicksort | 22 | Rust (21) | 1.05x |
| 7 | 24_merge_sort | 27 | C (26) | 1.04x |
| 8 | 36_array_rotation_reversal | 30 | C (29) | 1.03x |
| 9 | 17_n_queens | 36 | C (35) | 1.03x |
| 10 | 28_prime_factorization | 713 | Rust (703) | 1.01x |
| 11 | 29_dot_product | 32 | C (32) | 1.00x |
| 12 | 30_heap_sort | 30 | C++ (30) | 1.00x |
| 13 | 34_counting_sort | 23 | Rust (23) | 1.00x |
| 14 | 13_array_rw | 21 | C (21) | 1.00x |
| 15 | 37_sqrt_decomposition | 17 | Rust (17) | 1.00x |
| 16 | 14_lcg_random | 16 | C (16) | 1.00x |
| 17 | 02_fib_iterative | 15 | C (15) | 1.00x |
| 18 | 20_monte_carlo_pi | 12 | Rust (12) | 1.00x |
| 19 | 23_caesar_cipher | 10 | Rust (10) | 1.00x |
| 20 | 09_perfect_numbers | 6 | Rust (6) | 1.00x |

## Summary

Taking the **best Salam backend per program** against the best of the other
compiled languages (C / C++ / Rust / Go):

- **Fastest outright (22):** 02_fib_iterative, 03_primes_count, 04_collatz, 05_sum_mod, 11_loop_count, 13_array_rw, 14_lcg_random, 20_monte_carlo_pi, 23_caesar_cipher, 27_edit_distance, 28_prime_factorization, 29_dot_product, 31_binary_search_stress, 32_longest_increasing_subsequence, 33_subset_sum_reachability, 34_counting_sort, 35_run_length_stats, 36_array_rotation_reversal, 38_game_of_life, 39_trapezoidal_integration, 40_matrix_stdlib_matmul, 42_tensor_stdlib_matmul.
- **Tied for fastest (17):** 01_fib_recursive, 06_gcd_sum, 07_pow_mod, 08_digit_sum, 09_perfect_numbers, 10_pi_leibniz, 12_hello_print, 15_matrix_mult, 16_quicksort, 18_sieve_eratosthenes, 19_mandelbrot, 21_coin_change_dp, 22_knapsack_01, 25_palindrome_count, 26_ackermann, 30_heap_sort, 37_sqrt_decomposition.
- **Trailing the fastest (3):** 17_n_queens (+1 vs C); 24_merge_sort (+1 vs C/Rust); 41_matrix_stdlib_smallops (+27 vs C).

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
