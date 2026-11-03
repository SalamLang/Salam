#include <stdarg.h>

struct Pair { int a; int b; };
struct Mix { double d; int i; };
struct Dbl2 { double x; double y; };
struct Flt3 { float x; float y; float z; };
struct Big { long long a; long long b; long long c; };
struct Tiny { signed char c; };
struct Odd { short s; signed char c; };
struct Three { signed char a; signed char b; signed char c; };
struct Wide { long long a; int b; };
struct Quad { double a; double b; double c; double d; };
struct F1 { float x; };

static const unsigned long long pow10[20] = {
    10000000000000000000ULL, 1000000000000000000ULL, 100000000000000000ULL,
    10000000000000000ULL, 1000000000000000ULL, 100000000000000ULL,
    10000000000000ULL, 1000000000000ULL, 100000000000ULL, 10000000000ULL,
    1000000000ULL, 100000000ULL, 10000000ULL, 1000000ULL, 100000ULL,
    10000ULL, 1000ULL, 100ULL, 10ULL, 1ULL,
};

static char *put_s(char *p, const char *s) {
    while (*s) *p++ = *s++;
    return p;
}

static char *put_i(char *p, long long v) {
    unsigned long long u = (unsigned long long)v;
    int started = 0;
    int k;
    if (v < 0) {
        *p++ = '-';
        u = 0ULL - u;
    }
    for (k = 0; k < 20; k++) {
        char d = '0';
        while (u >= pow10[k]) {
            u -= pow10[k];
            d++;
        }
        if (d != '0' || started || k == 19) {
            *p++ = d;
            started = 1;
        }
    }
    return p;
}

static char *put_f(char *p, double d) {
    char dig[24];
    char tmp[24];
    int c = (int)(d * 100.0);
    int n, m = 0, k;
    if (c < 0) {
        *p++ = '-';
        c = -c;
    }
    n = (int)(put_i(dig, c) - dig);
    for (k = n; k < 3; k++) tmp[m++] = '0';
    for (k = 0; k < n; k++) tmp[m++] = dig[k];
    for (k = 0; k < m; k++) {
        if (k == m - 2) *p++ = '.';
        *p++ = tmp[k];
    }
    return p;
}

static char *put_list(char *p, const char *tag, int n, const long long *v) {
    int k;
    p = put_s(p, tag);
    *p++ = '{';
    for (k = 0; k < n; k++) {
        if (k) *p++ = ',';
        p = put_i(p, v[k]);
    }
    *p++ = '}';
    return p;
}

static char *put_flist(char *p, const char *tag, int n, const double *v) {
    int k;
    p = put_s(p, tag);
    *p++ = '{';
    for (k = 0; k < n; k++) {
        if (k) *p++ = ',';
        p = put_f(p, v[k]);
    }
    *p++ = '}';
    return p;
}

static int vtrace(char *out, const char *fmt, va_list ap) {
    char *p = out;
    const char *f;
    for (f = fmt; *f; f++) {
        if (f != fmt) *p++ = ' ';
        if (*f == 'i') {
            p = put_i(p, va_arg(ap, int));
        } else if (*f == 'l') {
            p = put_i(p, va_arg(ap, long long));
        } else if (*f == 'g') {
            p = put_f(p, va_arg(ap, double));
        } else if (*f == 'p') {
            struct Pair s = va_arg(ap, struct Pair);
            long long v[2] = {s.a, s.b};
            p = put_list(p, "p", 2, v);
        } else if (*f == 'm') {
            struct Mix s = va_arg(ap, struct Mix);
            p = put_s(p, "m{");
            p = put_f(p, s.d);
            *p++ = ',';
            p = put_i(p, s.i);
            *p++ = '}';
        } else if (*f == 'd') {
            struct Dbl2 s = va_arg(ap, struct Dbl2);
            double v[2] = {s.x, s.y};
            p = put_flist(p, "d", 2, v);
        } else if (*f == 'f') {
            struct Flt3 s = va_arg(ap, struct Flt3);
            double v[3] = {s.x, s.y, s.z};
            p = put_flist(p, "f", 3, v);
        } else if (*f == 'b') {
            struct Big s = va_arg(ap, struct Big);
            long long v[3] = {s.a, s.b, s.c};
            p = put_list(p, "b", 3, v);
        } else if (*f == 't') {
            struct Tiny s = va_arg(ap, struct Tiny);
            long long v[1] = {s.c};
            p = put_list(p, "t", 1, v);
        } else if (*f == 'o') {
            struct Odd s = va_arg(ap, struct Odd);
            long long v[2] = {s.s, s.c};
            p = put_list(p, "o", 2, v);
        } else if (*f == 'h') {
            struct Three s = va_arg(ap, struct Three);
            long long v[3] = {s.a, s.b, s.c};
            p = put_list(p, "h", 3, v);
        } else if (*f == 'w') {
            struct Wide s = va_arg(ap, struct Wide);
            long long v[2] = {s.a, s.b};
            p = put_list(p, "w", 2, v);
        } else if (*f == 'q') {
            struct Quad s = va_arg(ap, struct Quad);
            double v[4] = {s.a, s.b, s.c, s.d};
            p = put_flist(p, "q", 4, v);
        } else if (*f == 'e') {
            struct F1 s = va_arg(ap, struct F1);
            double v[1] = {s.x};
            p = put_flist(p, "e", 1, v);
        } else {
            *p++ = '?';
        }
    }
    *p = 0;
    return (int)(p - out);
}

int va_trace(char *out, const char *fmt, ...) {
    va_list ap;
    int n;
    va_start(ap, fmt);
    n = vtrace(out, fmt, ap);
    va_end(ap);
    return n;
}

int va_tagged(char *out, struct Pair tag, const char *fmt, ...) {
    va_list ap;
    long long v[2] = {tag.a, tag.b};
    char *p = put_list(out, "tag", 2, v);
    *p++ = ':';
    va_start(ap, fmt);
    p += vtrace(p, fmt, ap);
    va_end(ap);
    return (int)(p - out);
}

struct Mix va_summix(int n, ...) {
    struct Mix r = {0.0, 0};
    va_list ap;
    int k;
    va_start(ap, n);
    for (k = 0; k < n; k++) {
        struct Mix s = va_arg(ap, struct Mix);
        r.d += s.d;
        r.i += s.i;
    }
    va_end(ap);
    return r;
}

int fixed_after_ints(char *out, long long a, long long b, long long c, long long d, long long e,
                     long long f, long long g, long long h, struct Dbl2 v, struct F1 w) {
    long long n[8] = {a, b, c, d, e, f, g, h};
    double x[3] = {v.x, v.y, w.x};
    char *p = put_list(out, "n", 8, n);
    *p++ = ' ';
    p = put_flist(p, "x", 3, x);
    *p = 0;
    return (int)(p - out);
}
