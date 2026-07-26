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
#include "i18n/i18n.h"

char lx_peek(lx_t *L)
{
    return L->pos < L->n ? L->s[L->pos] : '\0';
}

char lx_peek2(lx_t *L, size_t k)
{
    return L->pos + k < L->n ? L->s[L->pos + k] : '\0';
}

bool lx_end(lx_t *L)
{
    return L->pos >= L->n;
}

char lx_adv(lx_t *L)
{
    char c = L->s[L->pos++];
    if (c == '\n') {
        L->line++;
        L->col = 1;
    } else {
        L->col++;
    }
    return c;
}

const char *lx_slice(lx_t *L, size_t start)
{
    return arena_strndup(L->a, L->s + start, L->pos - start);
}

bool lx_match(lx_t *L, char c)
{
    if (lx_peek(L) == c) {
        lx_adv(L);
        return true;
    }
    return false;
}

void lx_skip_ident(lx_t *L)
{
    while (lx_is_ident_cont(lx_peek(L)))
        lx_adv(L);
}

void lx_skip_to_eol(lx_t *L)
{
    while (!lx_end(L) && lx_peek(L) != '\n')
        lx_adv(L);
}

int lx_at_digit(lx_t *L, size_t k)
{
    size_t p = L->pos + k;
    if (p >= L->n) return 0;
    int val;
    return lx_decimal_digit(L->s + p, L->n - p, &val);
}

int lx_take_digit(lx_t *L)
{
    if (L->pos >= L->n) return -1;
    int val;
    int len = lx_decimal_digit(L->s + L->pos, L->n - L->pos, &val);
    if (len == 0) return -1;
    {
        int i = 0;
        for (; i < len; i++)
            lx_adv(L);
    }
    return val;
}

size_t lx_consume_hex(lx_t *L, int max)
{
    size_t n = 0;
    while ((int)n < max && !lx_end(L) && lx_hex(lx_peek(L)) >= 0) {
        lx_adv(L);
        n++;
    }
    return n;
}

bool lx_expect(lx_t *L, char close, const src_pos_t *begin, const char *errmsg)
{
    if (lx_peek(L) == close) {
        lx_adv(L);
        return true;
    }
    lx_error(L, begin, errmsg);
    return false;
}

size_t lx_trim_ws_end(const char *s, size_t start, size_t end)
{
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r'))
        end--;
    return end;
}

void lx_emit_val(lx_t *L, token_kind_t kind, const src_pos_t *begin, const char *lexeme,
                 const token_value_t *val)
{
    token_t t;
    t.kind = kind;
    t.lexeme = lexeme;
    t.layout_mode = L->layout_mode;
    if (val)
        t.value = *val;
    else
        t.value.kind = TV_NONE;
    t.span.begin = *begin;
    t.span.end = LX_POS(L);
    t.span.length = (uint32_t)(t.span.end.index - begin->index);
    token_stream_push(L->a, L->out, &t);
    L->last = kind;
    L->has_last = true;
    LOG_T_AT(L->log, PH_LEXER, L->file, t.span, "token recognized: %s '%s'",
             token_kind_name(kind), lexeme ? lexeme : "");
}

void lx_emit(lx_t *L, token_kind_t kind, const src_pos_t *begin, const char *lexeme)
{
    lx_emit_val(L, kind, begin, lexeme, NULL);
}

void lx_error(lx_t *L, const src_pos_t *at, const char *msg)
{
    L->had_error = true;

    src_span_t sp;
    sp.begin = *at;
    sp.end = LX_POS(L);
    sp.length = (uint32_t)(sp.end.index - at->index);
    LOG_E_AT(L->log, PH_LEXER, L->file, sp, "%s", i18n_tr(msg));
}
