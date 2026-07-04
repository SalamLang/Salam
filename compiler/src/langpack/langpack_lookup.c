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

#include "core/prelude.h"
#include "langpack/langpack.h"
#include "langpack/langpack_internal.h"

token_kind_t langpack_lookup_keyword(const langpack_t *p, const char *ident)
{
    /* Only this pack's own keywords are recognised: a Persian file must be
     * written in Persian and an English one in English. Each source file is
     * lexed in its own detected language (see langpack_detect), so the stdlib
     * (English) and a Persian program each get the right keyword set. */
    { const kw_entry_t *e = p->keywords; for (; e->spelling != NULL; e++) {
        if (strcmp(e->spelling, ident) == 0) return e->kind;
    } }
    return TK_IDENT;
}

const char *langpack_canon_word(const langpack_t *p, const char *ident)
{
    if (!p->contextual) return NULL;     
    { const ctx_entry_t *e = p->contextual; for (; e->spelling != NULL; e++)
        if (strcmp(e->spelling, ident) == 0) return e->canon; }
    return NULL;
}
