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

#ifndef SALAM_CORE_TIMER_H
#define SALAM_CORE_TIMER_H

#include "core/prelude.h"

/* Monotonic nanoseconds since an unspecified epoch. Never runs backwards and
 * never jumps when the wall clock is adjusted, so differences of two readings
 * are a valid elapsed time. This is the clock every timing feature in the
 * compiler must use; the wall clock (time(), GetSystemTimeAsFileTime) has a
 * ~15.6 ms update granularity on Windows and is useless below that. */
uint64_t salam_mono_ns(void);

/* CPU time (user + kernel) consumed by this process, in nanoseconds.
 * Returns 0 when the platform cannot report it. */
uint64_t salam_cpu_ns(void);

/* Peak resident set size of this process in bytes, 0 when unavailable. */
uint64_t salam_peak_rss(void);

#endif /* SALAM_CORE_TIMER_H */
