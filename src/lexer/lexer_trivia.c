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

static bool token_continues(token_kind_t k)
{
    switch (k) {
        /* TK_STAR is intentionally excluded: a line-final `*` is virtually
         * always a pointer-type suffix (`void*`, `T*`) that ends a statement,
         * not a dangling multiply. Treating it as a continuation swallows the
         * statement terminator and merges the next statement onto the line. */
        case TK_PLUS: case TK_MINUS: case TK_SLASH: case TK_PERCENT:
        case TK_POWER: case TK_EQ: case TK_NE: case TK_LT: case TK_GT: case TK_LE:
        case TK_GE: case TK_AND: case TK_OR: case TK_NOT:
        case TK_ASSIGN: case TK_PLUS_EQ: case TK_MINUS_EQ: case TK_STAR_EQ:
        case TK_SLASH_EQ: case TK_PERCENT_EQ:
        case TK_COMMA: case TK_DOT: case TK_COLON: case TK_KW_AS:
        case TK_LPAREN: case TK_LBRACKET: case TK_LBRACE:
            return true;
        default:
            return false;
    }
}

void lx_newline_terminator(lx_t *L)
{
    if (L->group_depth != 0 || !L->has_last ||
        L->last == TK_STMT_END || token_continues(L->last))
        return;
    token_t t;
    t.kind = TK_STMT_END;
    t.lexeme = "\\n";
    t.layout_mode = false;
    t.value.kind = TV_NONE;
    t.span.begin = LX_POS(L); t.span.end = t.span.begin; t.span.length = 0;
    token_stream_push(L->a, L->out, &t);
    L->last = TK_STMT_END;
    L->has_last = true;
    L->comp_header = false;   
    LOG_T_AT(L->log, PH_LEXER, L->file, t.span, "statement terminator (newline)");
}

bool lx_try_line_comment(lx_t *L)
{
    if (lx_peek(L) != '/' || lx_peek2(L, 1) != '/') return false;
    size_t start = L->pos;
    src_pos_t b = LX_POS(L);
    lx_skip_to_eol(L);
    if (L->keep_comments) {
        size_t end = lx_trim_ws_end(L->s, start, L->pos);
        lx_emit(L, TK_COMMENT_LINE, &b, arena_strndup(L->a, L->s + start, end - start));
    }
    return true;
}

static void scan_block_comment(lx_t *L)
{
    size_t start = L->pos;
    src_pos_t b = LX_POS(L);
    lx_adv(L); lx_adv(L);
    bool saw_newline = false;
    int depth = 1;                                   
    while (!lx_end(L)) {
        if (lx_peek(L) == '/' && lx_peek2(L, 1) == '*') { lx_adv(L); lx_adv(L); depth++; continue; }
        if (lx_peek(L) == '*' && lx_peek2(L, 1) == '/') { lx_adv(L); lx_adv(L); if (--depth == 0) break; continue; }
        if (lx_peek(L) == '\n') saw_newline = true;
        lx_adv(L);
    }

    if (depth != 0) lx_error(L, &b, "unterminated block comment");
    if (L->keep_comments) lx_emit(L, TK_COMMENT_BLOCK, &b, lx_slice(L, start));
    if (saw_newline) lx_newline_terminator(L);
}

void lx_skip_trivia(lx_t *L)
{
    for (;;) {
        char c = lx_peek(L);
        if (c == '\0') return;
        if (c == ' ' || c == '\t' || c == '\r') { lx_adv(L); continue; }
        if (c == '\n') { lx_adv(L); lx_newline_terminator(L); continue; }
        if (lx_try_line_comment(L)) continue;
        if (c == '/' && lx_peek2(L, 1) == '*') { scan_block_comment(L); continue; }
        return;
    }
}
