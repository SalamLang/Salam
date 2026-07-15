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

bool lx_scan_unicode_op(lx_t *L)
{
    const unsigned char *s = (const unsigned char *)(L->s + L->pos);
    size_t rem = L->n - L->pos;
    token_kind_t k;
    if (rem >= 3 && s[0] == 0xEF && s[1] == 0xBC && (s[2] == 0x8B || s[2] == 0x8D))
        k = (s[2] == 0x8B) ? TK_PLUS : TK_MINUS;
    else if (rem >= 3 && s[0] == 0xE2 && s[1] == 0x88 && s[2] == 0x92)
        k = TK_MINUS;
    else
        return false;
    src_pos_t b = LX_POS(L);
    size_t start = L->pos;
    lx_adv(L);
    lx_adv(L);
    lx_adv(L);
    lx_emit(L, k, &b, lx_slice(L, start));
    return true;
}

void lx_scan_op(lx_t *L)
{
    token_kind_t prev = L->last;
    size_t start = L->pos;
    src_pos_t b = LX_POS(L);
    char c = lx_adv(L);
    token_kind_t k;
    switch (c) {
    case '(':
        L->group_depth++;
        k = TK_LPAREN;
        break;
    case ')':
        if (L->group_depth > 0) L->group_depth--;
        k = TK_RPAREN;
        break;

    case '[':
        k = TK_LBRACKET;
        break;
    case ']':
        k = TK_RBRACKET;
        break;
    case '{':
        k = TK_LBRACE;
        break;
    case '}':
        k = TK_RBRACE;
        break;
    case ',':
        k = TK_COMMA;
        break;
    case ':':
        k = TK_COLON;
        break;
    case '.':

        if (lx_peek(L) == '.' && lx_peek2(L, 1) == '.') {
            lx_adv(L);
            lx_adv(L);
            k = TK_ELLIPSIS;
        } else
            k = TK_DOT;
        break;
    case '+':
        k = lx_match(L, '=') ? TK_PLUS_EQ : TK_PLUS;
        break;
    case '-':
        k = lx_match(L, '=') ? TK_MINUS_EQ : TK_MINUS;
        break;
    case '*':
        if (lx_match(L, '*'))
            k = lx_match(L, '=') ? TK_POWER_EQ : TK_POWER;
        else if (lx_match(L, '='))
            k = TK_STAR_EQ;
        else
            k = TK_STAR;
        break;
    case '/':
        k = lx_match(L, '=') ? TK_SLASH_EQ : TK_SLASH;
        break;
    case '%':
        k = lx_match(L, '=') ? TK_PERCENT_EQ : TK_PERCENT;
        break;
    case '=':
        if (lx_match(L, '='))
            k = TK_EQ;
        else if (lx_match(L, '>'))
            k = TK_FAT_ARROW;
        else
            k = TK_ASSIGN;
        break;
    case '!':
        if (lx_match(L, '='))
            k = TK_NE;

        else if (lx_peek(L) == '_' && lx_peek2(L, 1) == '_' &&
                 !lx_is_ident_cont(lx_peek2(L, 2))) {
            lx_adv(L);
            lx_adv(L);
            k = TK_KW_PRINTERRLN;
        } else if (lx_peek(L) == '_' && !lx_is_ident_cont(lx_peek2(L, 1))) {
            lx_adv(L);
            k = TK_KW_PRINTERR;
        } else
            k = TK_NOT;
        break;
    case '<':
        k = lx_match(L, '=') ? TK_LE : TK_LT;
        break;
    case '>':
        k = lx_match(L, '=') ? TK_GE : TK_GT;
        break;
    case '&':
        k = lx_match(L, '&') ? TK_AND : TK_AMP;
        break;
    case '|':
        if (lx_match(L, '|'))
            k = TK_OR;
        else {
            lx_error(L, &b, "unexpected '|' (use '||')");
            return;
        }
        break;
    case ';':
        lx_error(
            L, &b,
            "unexpected ';' (statements end at a newline; 'for' uses ',' between clauses)");
        return;
    default:
        lx_error(L, &b, "unexpected character");
        return;
    }
    lx_emit(L, k, &b, lx_slice(L, start));

    if (k == TK_COLON &&
        (prev == TK_KW_LAYOUT || (L->comp_header && L->group_depth == 0))) {
        L->layout_mode = true;
        L->layout_depth = 1;
        L->layout_sub = LX_SUB_NAME;
        L->comp_header = false;
        LOG_D(L->log, PH_LEXER, "entering layout mode");
    }

    if (k == TK_STMT_END && L->group_depth == 0) L->comp_header = false;
}
