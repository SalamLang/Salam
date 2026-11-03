struct Mix { double d; int i; };
struct Flt3 { float x; float y; float z; };
struct Dbl2 { double x; double y; };
struct Tiny { signed char c; };
struct F1 { float x; };
struct Big { long long a; long long b; long long c; };
struct Wide { long long a; int b; };
struct FI { float f; int i; };
struct Quad { double a; double b; double c; double d; };

struct Mix mix_make(double d, int i) {
    struct Mix r;
    r.d = d * 2.0;
    r.i = i + 1;
    return r;
}

struct Flt3 flt3_scale(struct Flt3 f, float k) {
    struct Flt3 r;
    r.x = f.x * k;
    r.y = f.y * k;
    r.z = f.z * k;
    return r;
}

struct Dbl2 dbl2_swap(struct Dbl2 d) {
    struct Dbl2 r;
    r.x = d.y;
    r.y = d.x;
    return r;
}

struct Tiny tiny_neg(struct Tiny t) {
    struct Tiny r;
    r.c = (signed char)-t.c;
    return r;
}

struct F1 f1_twice(struct F1 x) {
    struct F1 r;
    r.x = x.x * 2.0f;
    return r;
}

struct Big big_sum(struct Big a, struct Big b) {
    struct Big r;
    r.a = a.a + b.a;
    r.b = a.b + b.b;
    r.c = a.c + b.c;
    return r;
}

struct Wide wide_make(long long a, int b) {
    struct Wide r;
    r.a = a * 3;
    r.b = b - 1;
    return r;
}

struct FI fi_mix(struct FI a, struct Mix m, struct FI b) {
    struct FI r;
    r.f = a.f + b.f + (float)m.d;
    r.i = a.i + b.i + m.i;
    return r;
}

struct Quad quad_rev(struct Quad q) {
    struct Quad r;
    r.a = q.d;
    r.b = q.c;
    r.c = q.b;
    r.d = q.a;
    return r;
}

double many_fp(double a, double b, double c, double d, double e, double f, double g,
               struct Dbl2 h, struct FI i, double j) {
    return a + b + c + d + e + f + g + h.x * 10.0 + h.y * 100.0 + i.f * 1000.0 + (double)i.i * 10000.0 + j;
}
