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
#include "core/sb.h"
#include "lexer/lexer_internal.h"

/* Canonicalize an identifier: ZWNJ (U+200C) -> space, Arabic yeh (U+064A) ->
 * Persian yeh (U+06CC), Arabic kaf (U+0643) -> Persian keheh (U+06A9), so the
 * same word typed with either spelling normalizes identically. */
static const char *lx_norm_ident(lx_t *L, const char *text)
{
    const char *p;
    bool need = false;
    sb_t b;
    {
        const unsigned char *q = (const unsigned char *)text;
        for (; *q; q++) {
            if (q[0] == 0xE2 && q[1] == 0x80 && q[2] == 0x8C) {
                need = true;
                break;
            }
            if (q[0] == 0xD9 && (q[1] == 0x8A || q[1] == 0x83)) {
                need = true;
                break;
            }
        }
    }
    if (!need) return text;
    sb_init(&b);
    p = text;
    while (*p) {
        unsigned char c0 = (unsigned char)p[0], c1 = (unsigned char)p[1];
        if (c0 == 0xE2 && c1 == 0x80 && (unsigned char)p[2] == 0x8C) {
            sb_putc(&b, ' ');
            p += 3;
        } else if (c0 == 0xD9 && c1 == 0x8A) {
            sb_puts(&b, "\xDB\x8C");
            p += 2;
        } else if (c0 == 0xD9 && c1 == 0x83) {
            sb_puts(&b, "\xDA\xA9");
            p += 2;
        } else {
            sb_putc(&b, *p);
            p++;
        }
    }
    {
        const char *r = arena_strdup(L->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}

void lx_scan_ident(lx_t *L)
{
    size_t start = L->pos;
    src_pos_t b = LX_POS(L);
    lx_skip_ident(L);
    const char *raw = lx_slice(L, start);
    token_kind_t k = langpack_lookup_keyword(L->pack, raw);
    const char *text = raw;

    if (k == TK_IDENT) {
        const char *canon = L->keep_comments ? NULL : langpack_canon_word(L->pack, raw);
        text = canon ? canon : lx_norm_ident(L, raw);
    }
    if (k == TK_KW_TRUE || k == TK_KW_FALSE) {
        token_value_t v;
        v.kind = TV_BOOL;
        v.as.b = (k == TK_KW_TRUE);
        v.slen = 0;
        lx_emit_val(L, k, &b, text, &v);
    } else {
        lx_emit(L, k, &b, text);
    }

    if (k == TK_KW_COMPONENT) L->comp_header = true;
}

void lx_scan_meta(lx_t *L)
{
    src_pos_t b = LX_POS(L);
    lx_adv(L);
    size_t start = L->pos;
    lx_skip_ident(L);
    const char *name = lx_slice(L, start);

    if (!L->keep_comments) {
        const char *canon = langpack_canon_word(L->pack, name);
        if (canon) name = canon;
    }
    lx_emit(L, TK_META, &b, name);
}
