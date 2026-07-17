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

static int kw_fold(const char **p)
{
    const unsigned char *s = (const unsigned char *)*p;
    if (s[0] == 0) return 0;
    if ((s[0] == 0xD9 && s[1] == 0x8A) || (s[0] == 0xDB && s[1] == 0x8C)) {
        *p += 2;
        return 0x101;
    }
    if ((s[0] == 0xD9 && s[1] == 0x83) || (s[0] == 0xDA && s[1] == 0xA9)) {
        *p += 2;
        return 0x102;
    }
    *p += 1;
    return s[0];
}

static bool kw_ident_equal(const char *a, const char *b)
{
    for (;;) {
        int ka, kb;
        while (zwnj_at(a) || *a == ' ')
            a += (*a == ' ') ? 1 : 3;
        while (zwnj_at(b) || *b == ' ')
            b += (*b == ' ') ? 1 : 3;
        ka = kw_fold(&a);
        kb = kw_fold(&b);
        if (ka != kb) return false;
        if (ka == 0) return true;
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
        for (; *q; q++) {
            unsigned char c0 = (unsigned char)q[0], c1 = (unsigned char)q[1];
            if (c0 == ' ' || (c0 == 0xE2 && zwnj_at(q)) ||
                (c0 == 0xD9 && (c1 == 0x8A || c1 == 0x83)) ||
                (c0 == 0xDB && c1 == 0x8C) || (c0 == 0xDA && c1 == 0xA9)) {
                has_join = true;
                break;
            }
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
