#include "core/sal_format.h"
#include "core/numstr.h"

typedef struct { char *buf; size_t cap; size_t len; } fbuf_t;

static void fb_ch(fbuf_t *fb, char c)
{
    if (fb->len + 1 < fb->cap) fb->buf[fb->len] = c;
    fb->len++;
}

static void fb_str(fbuf_t *fb, const char *s, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) fb_ch(fb, s[i]);
}

static void emit_u64(fbuf_t *fb, uint64_t v, int base, int upper)
{
    char tmp[24];
    const char *dig = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0, j;
    if (v == 0) tmp[i++] = '0';
    else while (v != 0) { tmp[i++] = dig[(int)(v % (unsigned)base)]; v /= (unsigned)base; }
    for (j = i; j > 0; j--) fb_ch(fb, tmp[j - 1]);
}

static void emit_i64(fbuf_t *fb, int64_t v)
{
    if (v < 0) {
        fb_ch(fb, '-');
        emit_u64(fb, (uint64_t)(-(v + 1)) + 1u, 10, 0);
    } else {
        emit_u64(fb, (uint64_t)v, 10, 0);
    }
}

static void deleg_buf(fbuf_t *fb, const char *s, int n)
{
    if (n < 0) return;
    fb_str(fb, s, (size_t)n);
}

static void deleg_int (fbuf_t *fb, const char *sp, int v)           { char t[320]; int n = snprintf(t, sizeof t, sp, v); if (n >= (int)sizeof t) n = (int)sizeof t - 1; deleg_buf(fb, t, n); }

static void deleg_uint(fbuf_t *fb, const char *sp, unsigned v)      { char t[320]; int n = snprintf(t, sizeof t, sp, v); if (n >= (int)sizeof t) n = (int)sizeof t - 1; deleg_buf(fb, t, n); }

static void deleg_long(fbuf_t *fb, const char *sp, long v)          { char t[320]; int n = snprintf(t, sizeof t, sp, v); if (n >= (int)sizeof t) n = (int)sizeof t - 1; deleg_buf(fb, t, n); }

static void deleg_ulong(fbuf_t *fb, const char *sp, unsigned long v){ char t[320]; int n = snprintf(t, sizeof t, sp, v); if (n >= (int)sizeof t) n = (int)sizeof t - 1; deleg_buf(fb, t, n); }

static void deleg_dbl (fbuf_t *fb, const char *sp, double v)        { char t[320]; int n = snprintf(t, sizeof t, sp, v); if (n >= (int)sizeof t) n = (int)sizeof t - 1; deleg_buf(fb, t, n); }

static void deleg_ptr (fbuf_t *fb, const char *sp, void *v)         { char t[320]; int n = snprintf(t, sizeof t, sp, v); if (n >= (int)sizeof t) n = (int)sizeof t - 1; deleg_buf(fb, t, n); }

static void deleg_str(fbuf_t *fb, const char *sp, const char *v)
{
    int n = snprintf(NULL, 0, sp, v);
    if (n < 0) return;
    if ((size_t)n < 320) {
        char t[320];
        snprintf(t, sizeof t, sp, v);
        fb_str(fb, t, (size_t)n);
    } else {
        char *h = (char *)malloc((size_t)n + 1);
        if (h) { snprintf(h, (size_t)n + 1, sp, v); fb_str(fb, h, (size_t)n); free(h); }
    }
}

static int spec_putint(char *dst, int v)
{
    char t[16]; int i = 0, j = 0;
    if (v == 0) { dst[0] = '0'; return 1; }
    while (v > 0) { t[i++] = (char)('0' + v % 10); v /= 10; }
    while (i > 0) dst[j++] = t[--i];
    return j;
}

enum len_kind { LEN_NONE, LEN_L, LEN_LL, LEN_Z, LEN_H, LEN_HH, LEN_OTHER };

int sal_vsnprintf(char *buf, size_t cap, const char *fmt, va_list ap)
{
    fbuf_t fb;
    const char *p = fmt;
    fb.buf = buf; fb.cap = cap; fb.len = 0;

    while (*p != '\0') {
        const char *start;
        int flag_minus, flag_plus, flag_space, flag_zero, flag_hash;
        int width, prec;
        enum len_kind len;
        char conv;
        char spec[40];
        int si;

        if (*p != '%') { fb_ch(&fb, *p++); continue; }

        start = p;
        p++; 
        flag_minus = flag_plus = flag_space = flag_zero = flag_hash = 0;
        width = -1; prec = -1; len = LEN_NONE;

        for (;;) {
            if      (*p == '-') flag_minus = 1;
            else if (*p == '+') flag_plus  = 1;
            else if (*p == ' ') flag_space = 1;
            else if (*p == '0') flag_zero  = 1;
            else if (*p == '#') flag_hash  = 1;
            else break;
            p++;
        }

        if (*p == '*') { width = va_arg(ap, int); p++; if (width < 0) { flag_minus = 1; width = -width; } }
        else if (*p >= '0' && *p <= '9') { width = 0; while (*p >= '0' && *p <= '9') width = width * 10 + (*p++ - '0'); }

        if (*p == '.') {
            p++;
            if (*p == '*') { prec = va_arg(ap, int); p++; }
            else { prec = 0; while (*p >= '0' && *p <= '9') prec = prec * 10 + (*p++ - '0'); }
            if (prec < 0) prec = -1;
        }

        if (*p == 'l') { if (p[1] == 'l') { len = LEN_LL; p += 2; } else { len = LEN_L; p++; } }
        else if (*p == 'z') { len = LEN_Z; p++; }
        else if (*p == 'h') { if (p[1] == 'h') { len = LEN_HH; p += 2; } else { len = LEN_H; p++; } }
        else if (*p == 'j' || *p == 't' || *p == 'L') { len = LEN_OTHER; p++; }
        conv = *p;
        if (conv == '\0') { fb_str(&fb, start, (size_t)(p - start)); break; }
        p++;

        if ((len == LEN_LL || len == LEN_Z) &&
            (conv=='d'||conv=='i'||conv=='u'||conv=='x'||conv=='X'||conv=='o')) {
            if (conv == 'd' || conv == 'i') {
                if (len == LEN_Z) { size_t u = va_arg(ap, size_t); emit_i64(&fb, (int64_t)u); }
                else              { long long v = va_arg(ap, long long); emit_i64(&fb, (int64_t)v); }
            } else {
                uint64_t v = (len == LEN_Z) ? (uint64_t)va_arg(ap, size_t)
                                            : (uint64_t)va_arg(ap, unsigned long long);
                int base = (conv == 'o') ? 8 : (conv == 'd' || conv == 'i' || conv == 'u') ? 10 : 16;
                emit_u64(&fb, v, base, conv == 'X');
            }
            continue;
        }

        if (conv == '%') { fb_ch(&fb, '%'); continue; }

        si = 0;
        spec[si++] = '%';
        if (flag_minus) spec[si++] = '-';
        if (flag_plus)  spec[si++] = '+';
        if (flag_space) spec[si++] = ' ';
        if (flag_zero)  spec[si++] = '0';
        if (flag_hash)  spec[si++] = '#';
        if (width >= 0) si += spec_putint(spec + si, width);
        if (prec  >= 0) { spec[si++] = '.'; si += spec_putint(spec + si, prec); }
        if (len == LEN_L)  spec[si++] = 'l';
        else if (len == LEN_H)  spec[si++] = 'h';
        else if (len == LEN_HH) { spec[si++] = 'h'; spec[si++] = 'h'; }
        spec[si++] = conv;
        spec[si]   = '\0';

        switch (conv) {
            case 'd': case 'i':
                if (len == LEN_L) deleg_long(&fb, spec, va_arg(ap, long));
                else              deleg_int (&fb, spec, va_arg(ap, int));
                break;
            case 'u': case 'o': case 'x': case 'X':
                if (len == LEN_L) deleg_ulong(&fb, spec, va_arg(ap, unsigned long));
                else              deleg_uint (&fb, spec, va_arg(ap, unsigned int));
                break;
            case 'e': case 'E': case 'f': case 'F':
            case 'g': case 'G': case 'a': case 'A':
                deleg_dbl(&fb, spec, va_arg(ap, double));
                break;
            case 'c':
                deleg_int(&fb, spec, va_arg(ap, int));
                break;
            case 's': {
                const char *v = va_arg(ap, char *);
                if (!v) v = "(null)";
                if (width < 0 && prec < 0 && !flag_minus) fb_str(&fb, v, strlen(v)); 
                else deleg_str(&fb, spec, v);
                break;
            }
            case 'p':
                deleg_ptr(&fb, spec, va_arg(ap, void *));
                break;
            default: 
                fb_str(&fb, start, (size_t)(p - start));
                break;
        }
    }

    if (cap > 0) fb.buf[fb.len < cap ? fb.len : cap - 1] = '\0';
    return (int)fb.len;
}

int sal_snprintf(char *buf, size_t cap, const char *fmt, ...)
{
    va_list ap; int n;
    va_start(ap, fmt);
    n = sal_vsnprintf(buf, cap, fmt, ap);
    va_end(ap);
    return n;
}

void sal_vfprintf(FILE *f, const char *fmt, va_list ap)
{
    va_list ap2;
    int n;
    SAL_VA_COPY(ap2, ap);
    n = sal_vsnprintf(NULL, 0, fmt, ap);
    if (n < 0) { va_end(ap2); return; }
    if (n < 1024) {
        char t[1024];
        sal_vsnprintf(t, sizeof t, fmt, ap2);
        fwrite(t, 1, (size_t)n, f);
    } else {
        char *h = (char *)malloc((size_t)n + 1);
        if (h) { sal_vsnprintf(h, (size_t)n + 1, fmt, ap2); fwrite(h, 1, (size_t)n, f); free(h); }
    }
    va_end(ap2);
}
