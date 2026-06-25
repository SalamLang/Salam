#include "core/prelude.h"
#include "lexer/lexer_internal.h"

static int simple_escape(char e)
{
    switch (e) {
        case 'n':  return '\n';
        case 't':  return '\t';
        case 'r':  return '\r';
        case '0':  return '\0';
        case 'a':  return '\a';
        case 'b':  return '\b';
        case 'f':  return '\f';
        case 'v':  return '\v';
        case '\\': return '\\';
        case '"':  return '"';
        case '\'': return '\'';
        default:   return -1;
    }
}

static int decode_hex_run(const char *p, size_t len, int max, uint32_t *cp)
{
    uint32_t v = 0;
    int n = 0;
    while (n < max && (size_t)n < len) {
        int d = lx_hex(p[n]);
        if (d < 0) break;
        v = v * 16 + (uint32_t)d;
        n++;
    }
    *cp = v;
    return n;
}

const char *lx_decode_string(lx_t *L, const char *lex, bool triple)
{
    size_t lexlen = strlen(lex);
    size_t q = triple ? 3 : 1;
    if (lexlen < 2 * q) return arena_strdup(L->a, "");
    const char *c = lex + q;
    size_t clen = lexlen - 2 * q;
    char *out = (char *)arena_alloc(L->a, clen + 1);
    size_t oi = 0;
    { size_t i = 0; for (; i < clen; i++) {
        char ch = c[i];
        if (ch != '\\' || i + 1 >= clen) { out[oi++] = ch; continue; }
        char e = c[++i];  
        int simple = simple_escape(e);
        if (simple >= 0) {
            out[oi++] = (char)simple;
        } else if (e == 'x') {
            
            int hi = (i + 1 < clen) ? lx_hex(c[i + 1]) : -1;
            int lo = (i + 2 < clen) ? lx_hex(c[i + 2]) : -1;
            if (hi >= 0 && lo >= 0) { out[oi++] = (char)(hi * 16 + lo); i += 2; }
            else                    { out[oi++] = 'x'; }
        } else if (e == 'u' || e == 'U') {
            
            uint32_t cp;
            int width = (e == 'u') ? LX_ESC_HEX_U : LX_ESC_HEX_BIG_U;
            int n = decode_hex_run(c + i + 1, clen - (i + 1), width, &cp);
            i += (size_t)n;
            char tmp[4]; size_t nb = lx_utf8_encode(cp, tmp);
            memcpy(out + oi, tmp, nb); oi += nb;
        } else {
            out[oi++] = e;  
        }
    } }
    out[oi] = '\0';
    return out;
}

uint64_t lx_decode_char(const char *lex)
{
    size_t lexlen = strlen(lex);
    if (lexlen < 3) return 0;            
    const char *c = lex + 1;            
    if (c[0] != '\\') return (unsigned char)c[0];
    int simple = simple_escape(c[1]);
    if (simple >= 0) return (uint64_t)simple;
    if (c[1] == 'x') {
        
        int hi = (lexlen > 3) ? lx_hex(c[2]) : -1;
        int lo = (lexlen > 4) ? lx_hex(c[3]) : -1;
        if (hi >= 0 && lo >= 0) return (uint64_t)(hi * 16 + lo);
        return 0;
    }
    if (c[1] == 'u' || c[1] == 'U') {
        
        uint32_t cp;
        size_t avail = lexlen > 3 ? lexlen - 3 : 0;   
        int width = (c[1] == 'u') ? LX_ESC_HEX_U : LX_ESC_HEX_BIG_U;
        decode_hex_run(c + 2, avail, width, &cp);
        return (uint64_t)cp;
    }
    return (unsigned char)c[1];
}
