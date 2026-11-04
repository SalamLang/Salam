struct Pair {
    int a;
    int b;
};

struct Rect {
    int a;
    int b;
    int c;
    int d;
};

struct Tiny3 {
    signed char a;
    signed char b;
    signed char c;
};

struct Flt2 {
    float a;
    float b;
};

struct Flt3 {
    float a;
    float b;
    float c;
};

struct Dbl2 {
    double a;
    double b;
};

struct Dbl4 {
    double a;
    double b;
    double c;
    double d;
};

struct FltInt {
    float a;
    int b;
};

struct DblInt {
    double a;
    int b;
};

struct IntDbl {
    int a;
    double b;
};

struct Big3 {
    long long a;
    long long b;
    long long c;
};

struct Pair ex_Pair(struct Pair a, int k);
struct Pair ex_Pair_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Pair a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Pair b, int k);

struct Pair c_Pair(struct Pair a, int k) {
    a.a += k;
    a.b += k;
    return a;
}

struct Pair c_Pair_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Pair a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Pair b, int k) {
    a.a += b.a + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.b += b.b + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    return a;
}

void *get_Pair(void) {
    return (void *)c_Pair;
}

void *get_Pair_many(void) {
    return (void *)c_Pair_many;
}

struct Pair call_Pair(void *p) {
    struct Pair (*f)(struct Pair, int) = (struct Pair (*)(struct Pair, int))p;
    return f((struct Pair){2, 4}, 1);
}

struct Pair call_Pair_many(void *p) {
    struct Pair (*f)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Pair, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Pair, int) = (struct Pair (*)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Pair, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Pair, int))p;
    return f(1, 2, 3, 4, 5, (struct Pair){2, 4}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Pair){5, 10}, 1);
}

struct Pair callex_Pair(void) {
    return ex_Pair((struct Pair){2, 4}, 1);
}

struct Pair callex_Pair_many(void) {
    return ex_Pair_many(1, 2, 3, 4, 5, (struct Pair){2, 4}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Pair){5, 10}, 1);
}

struct Rect ex_Rect(struct Rect a, int k);
struct Rect ex_Rect_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Rect a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Rect b, int k);

struct Rect c_Rect(struct Rect a, int k) {
    a.a += k;
    a.b += k;
    a.c += k;
    a.d += k;
    return a;
}

struct Rect c_Rect_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Rect a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Rect b, int k) {
    a.a += b.a + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.b += b.b + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.c += b.c + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.d += b.d + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    return a;
}

void *get_Rect(void) {
    return (void *)c_Rect;
}

void *get_Rect_many(void) {
    return (void *)c_Rect_many;
}

struct Rect call_Rect(void *p) {
    struct Rect (*f)(struct Rect, int) = (struct Rect (*)(struct Rect, int))p;
    return f((struct Rect){2, 4, 6, 8}, 1);
}

struct Rect call_Rect_many(void *p) {
    struct Rect (*f)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Rect, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Rect, int) = (struct Rect (*)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Rect, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Rect, int))p;
    return f(1, 2, 3, 4, 5, (struct Rect){2, 4, 6, 8}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Rect){5, 10, 15, 20}, 1);
}

struct Rect callex_Rect(void) {
    return ex_Rect((struct Rect){2, 4, 6, 8}, 1);
}

struct Rect callex_Rect_many(void) {
    return ex_Rect_many(1, 2, 3, 4, 5, (struct Rect){2, 4, 6, 8}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Rect){5, 10, 15, 20}, 1);
}

struct Tiny3 ex_Tiny3(struct Tiny3 a, int k);
struct Tiny3 ex_Tiny3_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Tiny3 a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Tiny3 b, int k);

struct Tiny3 c_Tiny3(struct Tiny3 a, int k) {
    a.a += k;
    a.b += k;
    a.c += k;
    return a;
}

struct Tiny3 c_Tiny3_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Tiny3 a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Tiny3 b, int k) {
    a.a += b.a + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.b += b.b + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.c += b.c + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    return a;
}

void *get_Tiny3(void) {
    return (void *)c_Tiny3;
}

void *get_Tiny3_many(void) {
    return (void *)c_Tiny3_many;
}

struct Tiny3 call_Tiny3(void *p) {
    struct Tiny3 (*f)(struct Tiny3, int) = (struct Tiny3 (*)(struct Tiny3, int))p;
    return f((struct Tiny3){2, 4, 6}, 1);
}

struct Tiny3 call_Tiny3_many(void *p) {
    struct Tiny3 (*f)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Tiny3, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Tiny3, int) = (struct Tiny3 (*)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Tiny3, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Tiny3, int))p;
    return f(1, 2, 3, 4, 5, (struct Tiny3){2, 4, 6}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Tiny3){5, 10, 15}, 1);
}

struct Tiny3 callex_Tiny3(void) {
    return ex_Tiny3((struct Tiny3){2, 4, 6}, 1);
}

struct Tiny3 callex_Tiny3_many(void) {
    return ex_Tiny3_many(1, 2, 3, 4, 5, (struct Tiny3){2, 4, 6}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Tiny3){5, 10, 15}, 1);
}

struct Flt2 ex_Flt2(struct Flt2 a, int k);
struct Flt2 ex_Flt2_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Flt2 a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Flt2 b, int k);

struct Flt2 c_Flt2(struct Flt2 a, int k) {
    a.a += k;
    a.b += k;
    return a;
}

struct Flt2 c_Flt2_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Flt2 a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Flt2 b, int k) {
    a.a += b.a + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.b += b.b + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    return a;
}

void *get_Flt2(void) {
    return (void *)c_Flt2;
}

void *get_Flt2_many(void) {
    return (void *)c_Flt2_many;
}

struct Flt2 call_Flt2(void *p) {
    struct Flt2 (*f)(struct Flt2, int) = (struct Flt2 (*)(struct Flt2, int))p;
    return f((struct Flt2){2, 4}, 1);
}

struct Flt2 call_Flt2_many(void *p) {
    struct Flt2 (*f)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Flt2, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Flt2, int) = (struct Flt2 (*)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Flt2, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Flt2, int))p;
    return f(1, 2, 3, 4, 5, (struct Flt2){2, 4}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Flt2){5, 10}, 1);
}

struct Flt2 callex_Flt2(void) {
    return ex_Flt2((struct Flt2){2, 4}, 1);
}

struct Flt2 callex_Flt2_many(void) {
    return ex_Flt2_many(1, 2, 3, 4, 5, (struct Flt2){2, 4}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Flt2){5, 10}, 1);
}

struct Flt3 ex_Flt3(struct Flt3 a, int k);
struct Flt3 ex_Flt3_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Flt3 a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Flt3 b, int k);

struct Flt3 c_Flt3(struct Flt3 a, int k) {
    a.a += k;
    a.b += k;
    a.c += k;
    return a;
}

struct Flt3 c_Flt3_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Flt3 a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Flt3 b, int k) {
    a.a += b.a + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.b += b.b + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.c += b.c + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    return a;
}

void *get_Flt3(void) {
    return (void *)c_Flt3;
}

void *get_Flt3_many(void) {
    return (void *)c_Flt3_many;
}

struct Flt3 call_Flt3(void *p) {
    struct Flt3 (*f)(struct Flt3, int) = (struct Flt3 (*)(struct Flt3, int))p;
    return f((struct Flt3){2, 4, 6}, 1);
}

struct Flt3 call_Flt3_many(void *p) {
    struct Flt3 (*f)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Flt3, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Flt3, int) = (struct Flt3 (*)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Flt3, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Flt3, int))p;
    return f(1, 2, 3, 4, 5, (struct Flt3){2, 4, 6}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Flt3){5, 10, 15}, 1);
}

struct Flt3 callex_Flt3(void) {
    return ex_Flt3((struct Flt3){2, 4, 6}, 1);
}

struct Flt3 callex_Flt3_many(void) {
    return ex_Flt3_many(1, 2, 3, 4, 5, (struct Flt3){2, 4, 6}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Flt3){5, 10, 15}, 1);
}

struct Dbl2 ex_Dbl2(struct Dbl2 a, int k);
struct Dbl2 ex_Dbl2_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Dbl2 a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Dbl2 b, int k);

struct Dbl2 c_Dbl2(struct Dbl2 a, int k) {
    a.a += k;
    a.b += k;
    return a;
}

struct Dbl2 c_Dbl2_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Dbl2 a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Dbl2 b, int k) {
    a.a += b.a + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.b += b.b + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    return a;
}

void *get_Dbl2(void) {
    return (void *)c_Dbl2;
}

void *get_Dbl2_many(void) {
    return (void *)c_Dbl2_many;
}

struct Dbl2 call_Dbl2(void *p) {
    struct Dbl2 (*f)(struct Dbl2, int) = (struct Dbl2 (*)(struct Dbl2, int))p;
    return f((struct Dbl2){2, 4}, 1);
}

struct Dbl2 call_Dbl2_many(void *p) {
    struct Dbl2 (*f)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Dbl2, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Dbl2, int) = (struct Dbl2 (*)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Dbl2, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Dbl2, int))p;
    return f(1, 2, 3, 4, 5, (struct Dbl2){2, 4}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Dbl2){5, 10}, 1);
}

struct Dbl2 callex_Dbl2(void) {
    return ex_Dbl2((struct Dbl2){2, 4}, 1);
}

struct Dbl2 callex_Dbl2_many(void) {
    return ex_Dbl2_many(1, 2, 3, 4, 5, (struct Dbl2){2, 4}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Dbl2){5, 10}, 1);
}

struct Dbl4 ex_Dbl4(struct Dbl4 a, int k);
struct Dbl4 ex_Dbl4_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Dbl4 a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Dbl4 b, int k);

struct Dbl4 c_Dbl4(struct Dbl4 a, int k) {
    a.a += k;
    a.b += k;
    a.c += k;
    a.d += k;
    return a;
}

struct Dbl4 c_Dbl4_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Dbl4 a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Dbl4 b, int k) {
    a.a += b.a + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.b += b.b + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.c += b.c + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.d += b.d + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    return a;
}

void *get_Dbl4(void) {
    return (void *)c_Dbl4;
}

void *get_Dbl4_many(void) {
    return (void *)c_Dbl4_many;
}

struct Dbl4 call_Dbl4(void *p) {
    struct Dbl4 (*f)(struct Dbl4, int) = (struct Dbl4 (*)(struct Dbl4, int))p;
    return f((struct Dbl4){2, 4, 6, 8}, 1);
}

struct Dbl4 call_Dbl4_many(void *p) {
    struct Dbl4 (*f)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Dbl4, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Dbl4, int) = (struct Dbl4 (*)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Dbl4, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Dbl4, int))p;
    return f(1, 2, 3, 4, 5, (struct Dbl4){2, 4, 6, 8}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Dbl4){5, 10, 15, 20}, 1);
}

struct Dbl4 callex_Dbl4(void) {
    return ex_Dbl4((struct Dbl4){2, 4, 6, 8}, 1);
}

struct Dbl4 callex_Dbl4_many(void) {
    return ex_Dbl4_many(1, 2, 3, 4, 5, (struct Dbl4){2, 4, 6, 8}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Dbl4){5, 10, 15, 20}, 1);
}

struct FltInt ex_FltInt(struct FltInt a, int k);
struct FltInt ex_FltInt_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct FltInt a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct FltInt b, int k);

struct FltInt c_FltInt(struct FltInt a, int k) {
    a.a += k;
    a.b += k;
    return a;
}

struct FltInt c_FltInt_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct FltInt a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct FltInt b, int k) {
    a.a += b.a + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.b += b.b + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    return a;
}

void *get_FltInt(void) {
    return (void *)c_FltInt;
}

void *get_FltInt_many(void) {
    return (void *)c_FltInt_many;
}

struct FltInt call_FltInt(void *p) {
    struct FltInt (*f)(struct FltInt, int) = (struct FltInt (*)(struct FltInt, int))p;
    return f((struct FltInt){2, 4}, 1);
}

struct FltInt call_FltInt_many(void *p) {
    struct FltInt (*f)(long long g0, long long g1, long long g2, long long g3, long long g4, struct FltInt, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct FltInt, int) = (struct FltInt (*)(long long g0, long long g1, long long g2, long long g3, long long g4, struct FltInt, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct FltInt, int))p;
    return f(1, 2, 3, 4, 5, (struct FltInt){2, 4}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct FltInt){5, 10}, 1);
}

struct FltInt callex_FltInt(void) {
    return ex_FltInt((struct FltInt){2, 4}, 1);
}

struct FltInt callex_FltInt_many(void) {
    return ex_FltInt_many(1, 2, 3, 4, 5, (struct FltInt){2, 4}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct FltInt){5, 10}, 1);
}

struct DblInt ex_DblInt(struct DblInt a, int k);
struct DblInt ex_DblInt_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct DblInt a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct DblInt b, int k);

struct DblInt c_DblInt(struct DblInt a, int k) {
    a.a += k;
    a.b += k;
    return a;
}

struct DblInt c_DblInt_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct DblInt a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct DblInt b, int k) {
    a.a += b.a + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.b += b.b + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    return a;
}

void *get_DblInt(void) {
    return (void *)c_DblInt;
}

void *get_DblInt_many(void) {
    return (void *)c_DblInt_many;
}

struct DblInt call_DblInt(void *p) {
    struct DblInt (*f)(struct DblInt, int) = (struct DblInt (*)(struct DblInt, int))p;
    return f((struct DblInt){2, 4}, 1);
}

struct DblInt call_DblInt_many(void *p) {
    struct DblInt (*f)(long long g0, long long g1, long long g2, long long g3, long long g4, struct DblInt, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct DblInt, int) = (struct DblInt (*)(long long g0, long long g1, long long g2, long long g3, long long g4, struct DblInt, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct DblInt, int))p;
    return f(1, 2, 3, 4, 5, (struct DblInt){2, 4}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct DblInt){5, 10}, 1);
}

struct DblInt callex_DblInt(void) {
    return ex_DblInt((struct DblInt){2, 4}, 1);
}

struct DblInt callex_DblInt_many(void) {
    return ex_DblInt_many(1, 2, 3, 4, 5, (struct DblInt){2, 4}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct DblInt){5, 10}, 1);
}

struct IntDbl ex_IntDbl(struct IntDbl a, int k);
struct IntDbl ex_IntDbl_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct IntDbl a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct IntDbl b, int k);

struct IntDbl c_IntDbl(struct IntDbl a, int k) {
    a.a += k;
    a.b += k;
    return a;
}

struct IntDbl c_IntDbl_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct IntDbl a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct IntDbl b, int k) {
    a.a += b.a + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.b += b.b + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    return a;
}

void *get_IntDbl(void) {
    return (void *)c_IntDbl;
}

void *get_IntDbl_many(void) {
    return (void *)c_IntDbl_many;
}

struct IntDbl call_IntDbl(void *p) {
    struct IntDbl (*f)(struct IntDbl, int) = (struct IntDbl (*)(struct IntDbl, int))p;
    return f((struct IntDbl){2, 4}, 1);
}

struct IntDbl call_IntDbl_many(void *p) {
    struct IntDbl (*f)(long long g0, long long g1, long long g2, long long g3, long long g4, struct IntDbl, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct IntDbl, int) = (struct IntDbl (*)(long long g0, long long g1, long long g2, long long g3, long long g4, struct IntDbl, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct IntDbl, int))p;
    return f(1, 2, 3, 4, 5, (struct IntDbl){2, 4}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct IntDbl){5, 10}, 1);
}

struct IntDbl callex_IntDbl(void) {
    return ex_IntDbl((struct IntDbl){2, 4}, 1);
}

struct IntDbl callex_IntDbl_many(void) {
    return ex_IntDbl_many(1, 2, 3, 4, 5, (struct IntDbl){2, 4}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct IntDbl){5, 10}, 1);
}

struct Big3 ex_Big3(struct Big3 a, int k);
struct Big3 ex_Big3_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Big3 a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Big3 b, int k);

struct Big3 c_Big3(struct Big3 a, int k) {
    a.a += k;
    a.b += k;
    a.c += k;
    return a;
}

struct Big3 c_Big3_many(long long g0, long long g1, long long g2, long long g3, long long g4, struct Big3 a, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Big3 b, int k) {
    a.a += b.a + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.b += b.b + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    a.c += b.c + k + g0 + g1 + g2 + g3 + g4 + f0 + f1 + f2 + f3 + f4 + f5 + f6;
    return a;
}

void *get_Big3(void) {
    return (void *)c_Big3;
}

void *get_Big3_many(void) {
    return (void *)c_Big3_many;
}

struct Big3 call_Big3(void *p) {
    struct Big3 (*f)(struct Big3, int) = (struct Big3 (*)(struct Big3, int))p;
    return f((struct Big3){2, 4, 6}, 1);
}

struct Big3 call_Big3_many(void *p) {
    struct Big3 (*f)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Big3, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Big3, int) = (struct Big3 (*)(long long g0, long long g1, long long g2, long long g3, long long g4, struct Big3, double f0, double f1, double f2, double f3, double f4, double f5, double f6, struct Big3, int))p;
    return f(1, 2, 3, 4, 5, (struct Big3){2, 4, 6}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Big3){5, 10, 15}, 1);
}

struct Big3 callex_Big3(void) {
    return ex_Big3((struct Big3){2, 4, 6}, 1);
}

struct Big3 callex_Big3_many(void) {
    return ex_Big3_many(1, 2, 3, 4, 5, (struct Big3){2, 4, 6}, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, (struct Big3){5, 10, 15}, 1);
}
