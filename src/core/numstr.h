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

#ifndef SALAM_CORE_NUMSTR_H
#define SALAM_CORE_NUMSTR_H

#include "core/prelude.h"

#define SAL_NUMSTR_MAX 21

char *sal_u64toa(uint64_t v, char *buf);

char *sal_i64toa(int64_t  v, char *buf);

#endif /* SALAM_CORE_NUMSTR_H */
