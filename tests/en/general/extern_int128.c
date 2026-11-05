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

__int128 w_add(__int128 a, __int128 b) {
    return a + b;
}

unsigned __int128 w_mul(unsigned long long a, unsigned long long b) {
    return (unsigned __int128)a * b;
}

long long w_high(unsigned __int128 v) {
    return (long long)(v >> 64);
}

long long w_tail(long long a, long long b, long long c, long long d, long long e, __int128 f, long long g) {
    return a + b * 10 + c * 100 + d * 1000 + e * 10000 + (long long)(f >> 64) * 100000 + g * 1000000;
}

double w_mix(double x, __int128 v, double y) {
    return x + (double)v + y;
}

long long w_odd(long long a, __int128 v) {
    return a + (long long)(v >> 64);
}
