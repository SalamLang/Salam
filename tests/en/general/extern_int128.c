/*
 * Salam Programming Language (2024-2026)
 *
 *   +-------------------+
 *   |     S A L A M     |
 *   +-------------------+
 *
 * Designed by Seyyed Ali Mohammadiyeh and the Salam Team
 * Born from a decade of language design experience (since 2018)
 *
 * Repository: https://github.com/SalamLang/Salam
 *
 */

#include <stdint.h>

__int128 w_add(__int128 a, __int128 b) {
    return a + b;
}

unsigned __int128 w_mul(uint64_t a, uint64_t b) {
    return (unsigned __int128)a * b;
}

int64_t w_high(unsigned __int128 v) {
    return (int64_t)(v >> 64);
}

int64_t w_tail(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e, __int128 f, int64_t g) {
    return a + b * 10 + c * 100 + d * 1000 + e * 10000 + (int64_t)(f >> 64) * 100000 + g * 1000000;
}

double w_mix(double x, __int128 v, double y) {
    return x + (double)v + y;
}

int64_t w_odd(int64_t a, __int128 v) {
    return a + (int64_t)(v >> 64);
}
