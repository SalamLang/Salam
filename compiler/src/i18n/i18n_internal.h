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

#ifndef SALAM_I18N_INTERNAL_H
#define SALAM_I18N_INTERNAL_H

#include "core/prelude.h"

typedef struct {
    const char *key;
    const char *val;
} i18n_pair_t;

const char *i18n_active_lang(void);

#endif /* SALAM_I18N_INTERNAL_H */
