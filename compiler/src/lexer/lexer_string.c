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

void lx_scan_string(lx_t *L)
{
    size_t start = L->pos;
    src_pos_t b = LX_POS(L);
    lx_adv(L);
    while (!lx_end(L) && lx_peek(L) != '"') {
        if (lx_peek(L) == '\\') {
            lx_adv(L);
            if (!lx_end(L)) lx_adv(L);
        } else if (lx_peek(L) == '\n')
            break;
        else
            lx_adv(L);
    }
    lx_expect(L, '"', &b, "unterminated string literal");
    const char *text = lx_slice(L, start);
    token_value_t v;
    v.kind = TV_STRING;
    v.as.s = lx_decode_string(L, text, false);
    v.slen = strlen(v.as.s);
    lx_emit_val(L, TK_STRING, &b, text, &v);
}

void lx_scan_triple(lx_t *L)
{
    size_t start = L->pos;
    src_pos_t b = LX_POS(L);
    lx_adv(L);
    lx_adv(L);
    lx_adv(L);
    bool closed = false;
    while (!lx_end(L)) {
        if (lx_peek(L) == '"' && lx_peek2(L, 1) == '"' && lx_peek2(L, 2) == '"') {
            lx_adv(L);
            lx_adv(L);
            lx_adv(L);
            closed = true;
            break;
        }
        lx_adv(L);
    }
    if (!closed) lx_error(L, &b, "unterminated triple-quoted string");
    const char *text = lx_slice(L, start);
    token_value_t v;
    v.kind = TV_STRING;
    v.as.s = lx_decode_string(L, text, true);
    v.slen = strlen(v.as.s);
    lx_emit_val(L, TK_TRIPLE_STRING, &b, text, &v);
}

void lx_scan_char(lx_t *L)
{
    size_t start = L->pos;
    src_pos_t b = LX_POS(L);
    lx_adv(L);
    if (lx_peek(L) == '\\') {
        lx_adv(L);
        if (!lx_end(L)) {
            char esc = lx_adv(L);

            if (esc == 'x')
                lx_consume_hex(L, LX_ESC_HEX_X);
            else if (esc == 'u')
                lx_consume_hex(L, LX_ESC_HEX_U);
            else if (esc == 'U')
                lx_consume_hex(L, LX_ESC_HEX_BIG_U);
        }
    } else if (!lx_end(L) && lx_peek(L) != '\'') {
        lx_adv(L);
    }
    lx_expect(L, '\'', &b, "unterminated char literal");
    const char *text = lx_slice(L, start);
    token_value_t v;
    v.kind = TV_CHAR;
    v.as.i = lx_decode_char(text);
    v.slen = 0;
    lx_emit_val(L, TK_CHAR, &b, text, &v);
}

void lx_scan_raw_string(lx_t *L)
{
    size_t start = L->pos;
    src_pos_t b = LX_POS(L);
    lx_adv(L);
    while (!lx_end(L) && lx_peek(L) != '`')
        lx_adv(L);
    lx_expect(L, '`', &b, "unterminated raw string literal");
    const char *text = lx_slice(L, start);

    size_t tlen = L->pos - start;
    size_t inner = tlen >= 2 ? tlen - 2 : 0;
    char *s = (char *)arena_alloc(L->a, inner + 1);
    memcpy(s, text + 1, inner);
    s[inner] = '\0';
    token_value_t v;
    v.kind = TV_STRING;
    v.as.s = s;
    v.slen = inner;
    lx_emit_val(L, TK_RAW_STRING, &b, text, &v);
}

void lx_scan_utf8_char(lx_t *L)
{
    size_t start = L->pos;
    src_pos_t b = LX_POS(L);
    char quote;
    const char *bytes;
    size_t nbytes;
    const char *text;
    lx_adv(L);
    quote = lx_peek(L);
    lx_adv(L);
    {
        size_t content_start = L->pos;
        if (quote == '"') {
            while (!lx_end(L) && lx_peek(L) != '"') {
                if (lx_peek(L) == '\\') {
                    lx_adv(L);
                    if (!lx_end(L)) lx_adv(L);
                } else if (lx_peek(L) == '\n')
                    break;
                else
                    lx_adv(L);
            }
        } else {
            while (!lx_end(L) && lx_peek(L) != '\'' && lx_peek(L) != '\n')
                lx_adv(L);
        }
        {
            size_t content_len = L->pos - content_start;
            lx_expect(L, quote, &b, "unterminated unicode char literal");
            text = lx_slice(L, start);
            if (quote == '"') {
                const char *decoded = lx_decode_string(L, text + 1, false);
                bytes = decoded;
                nbytes = strlen(decoded);
            } else {
                bytes = arena_strndup(L->a, L->s + content_start, content_len);
                nbytes = content_len;
            }
        }
    }
    if (nbytes == 0) {
        lx_error(L, &b, "empty unicode char literal");
    } else {
        int seq = lx_utf8_seq_len(bytes, nbytes);
        if (seq == 0 || (size_t)seq != nbytes)
            lx_error(L, &b, "unicode char literal must contain exactly one codepoint");
    }
    {
        token_value_t v;
        v.kind = TV_STRING;
        v.as.s = (char *)CONST_CAST(bytes);
        v.slen = nbytes;
        lx_emit_val(L, TK_UTF8_CHAR, &b, text, &v);
    }
}
