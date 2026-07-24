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
#include "fmt/fmt_internal.h"

bool fmt_is_value_end(token_kind_t k)
{
    switch (k) {
    case TK_IDENT:
    case TK_INT:
    case TK_FLOAT:
    case TK_STRING:
    case TK_TRIPLE_STRING:
    case TK_RAW_STRING:
    case TK_CHAR:
    case TK_UTF8_CHAR:
    case TK_RPAREN:
    case TK_RBRACKET:
    case TK_RBRACE:
    case TK_KW_TRUE:
    case TK_KW_FALSE:
    case TK_KW_NULL:
    case TK_KW_THIS:
        return true;
    default:
        return false;
    }
}

bool fmt_is_open(token_kind_t k)
{
    return k == TK_LPAREN || k == TK_LBRACKET || k == TK_LBRACE;
}

bool fmt_is_close(token_kind_t k)
{
    return k == TK_RPAREN || k == TK_RBRACKET || k == TK_RBRACE;
}

bool fmt_gap_in_source(const token_t *a, const token_t *b)
{
    return b->span.begin.index > a->span.end.index;
}

bool fmt_need_space(const token_t *pt, const token_t *ct, bool prev_unary)
{
    token_kind_t p = pt->kind, c = ct->kind;

    if (p == TK_DOT || c == TK_DOT) return false;

    if (c == TK_COMMA || c == TK_COLON || c == TK_SEMICOLON) return false;
    if (c == TK_RPAREN || c == TK_RBRACKET) return false;

    if ((c == TK_PLUS_PLUS || c == TK_MINUS_MINUS) && fmt_is_value_end(p)) return false;

    if (p == TK_LPAREN || p == TK_LBRACKET) return false;

    if (p == TK_LBRACE && c == TK_RBRACE) return false;

    if (prev_unary) return false;

    if (c == TK_LPAREN) return !fmt_is_value_end(p);
    if (c == TK_LBRACKET) return !fmt_is_value_end(p);

    if (p == TK_LT || p == TK_GT || c == TK_LT || c == TK_GT || p == TK_SHL ||
        p == TK_SHR || c == TK_SHL || c == TK_SHR)
        return fmt_gap_in_source(pt, ct);

    if (p == TK_STAR || c == TK_STAR) return fmt_gap_in_source(pt, ct);
    return true;
}

bool fmt_is_prefix(token_kind_t k, const token_t *prev)
{
    if (k == TK_NOT || k == TK_TILDE) return true;
    if (k == TK_MINUS || k == TK_STAR || k == TK_AMP)
        return prev == NULL || !fmt_is_value_end(prev->kind);
    if (k == TK_PLUS_PLUS || k == TK_MINUS_MINUS)
        return prev == NULL || !fmt_is_value_end(prev->kind);
    return false;
}
