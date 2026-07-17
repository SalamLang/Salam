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

static bool zwnj_at(const char *s)
{
    return (unsigned char)s[0] == 0xE2 && (unsigned char)s[1] == 0x80 &&
           (unsigned char)s[2] == 0x8C;
}

static bool kw_ident_equal(const char *a, const char *b)
{
    for (;;) {
        while (zwnj_at(a) || *a == ' ')
            a += (*a == ' ') ? 1 : 3;
        while (zwnj_at(b) || *b == ' ')
            b += (*b == ' ') ? 1 : 3;
        if (*a != *b) return false;
        if (*a == '\0') return true;
        a++;
        b++;
    }
}

token_kind_t langpack_lookup_keyword(const langpack_t *p, const char *ident)
{
    const kw_entry_t *e = p->keywords;
    bool has_join = false;
    for (; e->spelling != NULL; e++)
        if (strcmp(e->spelling, ident) == 0) return e->kind;
    {
        const char *q = ident;
        for (; *q; q++)
            if (*q == ' ' || ((unsigned char)*q == 0xE2 && zwnj_at(q))) {
                has_join = true;
                break;
            }
    }
    if (has_join)
        for (e = p->keywords; e->spelling != NULL; e++)
            if (kw_ident_equal(e->spelling, ident)) return e->kind;
    return TK_IDENT;
}

const char *langpack_canon_word(const langpack_t *p, const char *ident)
{
    if (!p->contextual) return NULL;
    {
        const ctx_entry_t *e = p->contextual;
        for (; e->spelling != NULL; e++)
            if (strcmp(e->spelling, ident) == 0) return e->canon;
    }
    return NULL;
}
