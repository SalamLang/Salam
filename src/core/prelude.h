#ifndef SALAM_CORE_PRELUDE_H
#define SALAM_CORE_PRELUDE_H
#include <stdlib.h>   /* size_t, NULL, malloc/free/abort, strtol */
#include <string.h>   /* memcpy, strlen, strcmp */
#include <stdio.h>    /* FILE, fprintf, vsnprintf, snprintf */
#include <stdarg.h>   /* va_list */
#include <stdint.h>   /* uint32_t, int64_t, ... */
#include <stdbool.h>  /* bool, true, false */
#include <ctype.h>    /* isdigit, isalpha, isspace (no size_t typedef) */
static inline size_t salam_size_add(size_t a, size_t b)
{
    if (a > SIZE_MAX - b) abort();
    return a + b;
}

static inline size_t salam_size_mul(size_t a, size_t b)
{
    if (b != 0 && a > SIZE_MAX / b) abort();
    return a * b;
}

static inline size_t salam_grow_cap(size_t cap, size_t need, size_t seed)
{
    size_t ncap = cap ? cap : (seed ? seed : 1);
    while (ncap < need) {
        if (ncap > SIZE_MAX / 2) return need;
        ncap *= 2;
    }
    return ncap;
}
#endif /* SALAM_CORE_PRELUDE_H */
