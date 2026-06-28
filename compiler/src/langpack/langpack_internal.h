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

#ifndef SALAM_LANGPACK_INTERNAL_H
#define SALAM_LANGPACK_INTERNAL_H

#include "core/prelude.h"
#include "langpack/langpack.h"
#include "token/token.h"

typedef struct { const char *canon; const char *spelling; } ctx_entry_t;

struct langpack {
    const char       *code;
    const kw_entry_t *keywords;
    const char       *entry;
    const ctx_entry_t *contextual;
};

extern struct langpack g_lang_en;

extern struct langpack g_lang_fa;

extern const kw_entry_t k_lang_en[];

#endif /* SALAM_LANGPACK_INTERNAL_H */
