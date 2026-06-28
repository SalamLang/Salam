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

#ifndef SALAM_CORE_SAL_FORMAT_H
#define SALAM_CORE_SAL_FORMAT_H

#include "core/prelude.h"
#include <stdarg.h>

int  sal_vsnprintf(char *buf, size_t cap, const char *fmt, va_list ap);

int  sal_snprintf (char *buf, size_t cap, const char *fmt, ...);

size_t sal_catf (char *buf, size_t cap, size_t off, const char *fmt, ...);

void sal_vfprintf (FILE *f, const char *fmt, va_list ap);

#endif /* SALAM_CORE_SAL_FORMAT_H */
