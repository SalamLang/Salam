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

#ifndef SALAM_PREPROC_INTERNAL_H
#define SALAM_PREPROC_INTERNAL_H

#include "core/prelude.h"

const char *pp_skip_ws(const char *p);

bool pp_eval(const char *cond, const char *const *defs, int n);

#endif /* SALAM_PREPROC_INTERNAL_H */
