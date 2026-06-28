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
#include "lexer/lexer_internal.h"

void lx_scan_ident(lx_t *L)
{
    size_t start = L->pos;
    src_pos_t b = LX_POS(L);
    lx_skip_ident(L);
    const char *text = lx_slice(L, start);
    token_kind_t k = langpack_lookup_keyword(L->pack, text);

    if (k == TK_IDENT && !L->keep_comments) {
        const char *canon = langpack_canon_word(L->pack, text);
        if (canon) text = canon;
    }
    if (k == TK_KW_TRUE || k == TK_KW_FALSE) {
        token_value_t v; v.kind = TV_BOOL; v.as.b = (k == TK_KW_TRUE);
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
