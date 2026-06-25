#include "core/prelude.h"
#include "lexer/lexer_internal.h"

void lx_scan_string(lx_t *L)
{
    size_t start = L->pos;
    src_pos_t b = LX_POS(L);
    lx_adv(L);  
    while (!lx_end(L) && lx_peek(L) != '"') {
        if (lx_peek(L) == '\\') { lx_adv(L); if (!lx_end(L)) lx_adv(L); }  
        else if (lx_peek(L) == '\n') break;                               
        else lx_adv(L);
    }
    lx_expect(L, '"', &b, "unterminated string literal");
    const char *text = lx_slice(L, start);
    token_value_t v; v.kind = TV_STRING; v.as.s = lx_decode_string(L, text, false);
    lx_emit_val(L, TK_STRING, &b, text, &v);
}

void lx_scan_triple(lx_t *L)
{
    size_t start = L->pos;
    src_pos_t b = LX_POS(L);
    lx_adv(L); lx_adv(L); lx_adv(L);  
    bool closed = false;
    while (!lx_end(L)) {
        if (lx_peek(L) == '"' && lx_peek2(L,1) == '"' && lx_peek2(L,2) == '"') {
            lx_adv(L); lx_adv(L); lx_adv(L);
            closed = true;
            break;
        }
        lx_adv(L);
    }
    if (!closed) lx_error(L, &b, "unterminated triple-quoted string");
    const char *text = lx_slice(L, start);
    token_value_t v; v.kind = TV_STRING; v.as.s = lx_decode_string(L, text, true);
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
            
            if      (esc == 'x') lx_consume_hex(L, LX_ESC_HEX_X);
            else if (esc == 'u') lx_consume_hex(L, LX_ESC_HEX_U);
            else if (esc == 'U') lx_consume_hex(L, LX_ESC_HEX_BIG_U);
        }
    } else if (!lx_end(L) && lx_peek(L) != '\'') {
        lx_adv(L);  
    }
    lx_expect(L, '\'', &b, "unterminated char literal");
    const char *text = lx_slice(L, start);
    token_value_t v; v.kind = TV_CHAR; v.as.i = lx_decode_char(text);
    lx_emit_val(L, TK_CHAR, &b, text, &v);
}

void lx_scan_raw_string(lx_t *L)
{
    size_t start = L->pos;
    src_pos_t b = LX_POS(L);
    lx_adv(L);  
    while (!lx_end(L) && lx_peek(L) != '`') lx_adv(L);
    lx_expect(L, '`', &b, "unterminated raw string literal");
    const char *text = lx_slice(L, start);
    
    size_t tlen = L->pos - start;
    size_t inner = tlen >= 2 ? tlen - 2 : 0;
    char *s = (char *)arena_alloc(L->a, inner + 1);
    memcpy(s, text + 1, inner);
    s[inner] = '\0';
    token_value_t v; v.kind = TV_STRING; v.as.s = s;
    lx_emit_val(L, TK_RAW_STRING, &b, text, &v);
}

void lx_scan_utf8_char(lx_t *L)
{
    size_t start = L->pos;
    src_pos_t b = LX_POS(L);
    lx_adv(L);  
    lx_adv(L);  
    size_t content_start = L->pos;
    
    while (!lx_end(L) && lx_peek(L) != '\'' && lx_peek(L) != '\n') lx_adv(L);
    size_t content_end = L->pos;
    lx_expect(L, '\'', &b, "unterminated UTF-8 char literal");
    const char *text = lx_slice(L, start);
    size_t content_len = content_end - content_start;
    if (content_len == 0) {
        lx_error(L, &b, "empty UTF-8 char literal");
    } else {
        
        int seq = lx_utf8_seq_len(L->s + content_start, content_len);
        if (seq == 0 || (size_t)seq != content_len)
            lx_error(L, &b, "UTF-8 char literal must contain exactly one codepoint");
    }
    char *s = arena_strndup(L->a, L->s + content_start, content_len);
    token_value_t v; v.kind = TV_STRING; v.as.s = s;
    lx_emit_val(L, TK_UTF8_CHAR, &b, text, &v);
}
