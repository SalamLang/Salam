#include "core/prelude.h"
#include "lexer/lexer_internal.h"

static size_t scan_digits(lx_t *L, char base, bool *bad)
{
    size_t ndig = 0;
    bool prev_us = false, any = false;
    for (;;) {
        
        bool digit = base ? lx_is_base_digit(lx_peek(L), base)
                          : lx_at_digit(L, 0) != 0;
        if (digit) {
            if (base) lx_adv(L); else lx_take_digit(L);
            ndig++; prev_us = false; any = true;
        }
        else if (lx_peek(L) == '_') {
            if (!any || prev_us) *bad = true;
            lx_adv(L); prev_us = true;
        } else break;
    }
    if (prev_us) *bad = true;
    return ndig;
}

static void scan_based_int(lx_t *L, const src_pos_t *b, bool *bad)
{
    lx_adv(L);                                       
    char base = lx_adv(L);                           
    if (scan_digits(L, base, bad) == 0)
        lx_error(L, b, "numeric literal has no digits after its base prefix");
    
    if (lx_is_ident_cont(lx_peek(L))) {
        lx_error(L, b, "invalid digit in numeric literal");
        lx_skip_ident(L);
        *bad = true;
    }
}

static bool scan_decimal(lx_t *L, const src_pos_t *b, bool *bad)
{
    bool is_float = false;
    scan_digits(L, 0, bad);
    if (lx_peek(L) == '.' && lx_at_digit(L, 1)) {
        is_float = true;
        lx_adv(L);
        scan_digits(L, 0, bad);                      
    }
    if (lx_peek(L) == 'e' || lx_peek(L) == 'E') {
        is_float = true;
        lx_adv(L);
        if (lx_peek(L) == '+' || lx_peek(L) == '-') lx_adv(L);
        if (scan_digits(L, 0, bad) == 0)
            lx_error(L, b, "numeric literal has no exponent digits");
    }
    if (lx_is_alpha(lx_peek(L))) {                   
        if (lx_peek(L) == 'f') is_float = true;
        lx_skip_ident(L);
    }
    return is_float;
}


static void normalize_number(const char *text, size_t tlen, char *out, size_t cap)
{
    size_t ci = 0;
    { size_t i = 0; for (; i < tlen && ci < cap - 1; ) {
        if (text[i] == '_') { i++; continue; }
        int val;
        int len = lx_decimal_digit(text + i, tlen - i, &val);
        if (len == 2) { out[ci++] = (char)('0' + val); i += 2; }
        else          { out[ci++] = text[i++]; }
    } }
    out[ci] = '\0';
}

static void decode_number(const char *clean, bool is_float, token_value_t *v, bool *overflow)
{
    *overflow = false;
    if (is_float) {
        v->kind = TV_FLOAT;
        v->as.f = strtod(clean, NULL);
        return;
    }
    int base = 10; const char *d = clean;
    if      (clean[0]=='0' && (clean[1]=='x'||clean[1]=='X')) { base = 16; d = clean + 2; }
    else if (clean[0]=='0' && (clean[1]=='b'||clean[1]=='B')) { base = 2;  d = clean + 2; }
    else if (clean[0]=='0' && (clean[1]=='o'||clean[1]=='O')) { base = 8;  d = clean + 2; }
    v->kind = TV_INT;
    v->as.i = lx_parse_uint(d, base, overflow);
}

void lx_scan_number(lx_t *L)
{
    size_t start = L->pos;
    src_pos_t b = LX_POS(L);
    bool bad = false;
    char c1 = lx_peek2(L, 1);
    bool based = lx_peek(L) == '0' &&
                 (c1=='x'||c1=='X'||c1=='b'||c1=='B'||c1=='o'||c1=='O');
    bool is_float = false;
    if (based) scan_based_int(L, &b, &bad);
    else       is_float = scan_decimal(L, &b, &bad);
    if (bad) lx_error(L, &b, "misplaced '_' separator in numeric literal");
    const char *text = lx_slice(L, start);
    size_t tlen = L->pos - start;                    
    
    char stackbuf[64];
    char *clean = (tlen < sizeof stackbuf) ? stackbuf
                                           : (char *)arena_alloc(L->a, tlen + 1);
    normalize_number(text, tlen, clean, tlen + 1);
    token_value_t v;
    bool overflow;
    decode_number(clean, is_float, &v, &overflow);
    if (overflow)
        lx_error(L, &b, "integer literal is too large for a 64-bit value");
    lx_emit_val(L, is_float ? TK_FLOAT : TK_INT, &b, text, &v);
}
