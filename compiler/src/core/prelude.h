/*
 * Salam Programming Language (2024–2026)
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

#ifndef SALAM_CORE_PRELUDE_H
#define SALAM_CORE_PRELUDE_H

#include <stdlib.h>   /* size_t, NULL, malloc/free/abort, strtol */
#include <string.h>   /* memcpy, strlen, strcmp */
#include <stdio.h>    /* FILE, fprintf, vsnprintf, snprintf */
#include <stdarg.h>   /* va_list */
#include "core/sal_types.h" /* uint32_t/int64_t/bool: <stdint.h>+<stdbool.h> */
#include <ctype.h>    /* isdigit, isalpha, isspace, isalnum */

#define CONST_CAST(p) ((void *)(uintptr_t)(const void *)(p))

SAL_INLINE char *sal_strdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (!p) abort();
    memcpy(p, s, n);
    return p;
}

SAL_INLINE size_t salam_size_add(size_t a, size_t b)
{
    if (a > SIZE_MAX - b) abort();
    return a + b;
}

SAL_INLINE size_t salam_size_mul(size_t a, size_t b)
{
    if (b != 0 && a > SIZE_MAX / b) abort();
    return a * b;
}

SAL_INLINE size_t salam_grow_cap(size_t cap, size_t need, size_t seed)
{
    size_t ncap = cap ? cap : (seed ? seed : 1);
    while (ncap < need) {
        if (ncap > SIZE_MAX / 2) return need;
        ncap *= 2;
    }
    return ncap;
}

#endif /* SALAM_CORE_PRELUDE_H */
