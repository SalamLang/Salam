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

#ifndef SALAM_LANGPACK_LANGPACK_H
#define SALAM_LANGPACK_LANGPACK_H

#include "core/prelude.h"
#include "token/token.h"

typedef struct {
    token_kind_t kind;
    const char  *spelling;
} kw_entry_t;

typedef struct langpack langpack_t;

langpack_t *langpack_load(const char *code);

void        langpack_free(langpack_t *p);

const char *langpack_code(const langpack_t *p);

const char *langpack_entry(const langpack_t *p);

const char *langpack_entry_for(const char *code);

token_kind_t langpack_lookup_keyword(const langpack_t *p, const char *ident);

const char *langpack_canon_word(const langpack_t *p, const char *ident);

const char *langpack_end_spelling(const langpack_t *p);

#endif /* SALAM_LANGPACK_LANGPACK_H */
