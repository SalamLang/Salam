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
#include "semantic/sema_internal.h"

#define FOLD_I64_MAX 9223372036854775807LL
#define FOLD_I64_MIN (-FOLD_I64_MAX - 1LL)

static bool lit_int(const ast_node_t *n, long long *out)
{
    if (!n || n->kind != AST_LITERAL || n->op != TK_INT) return false;
    {
        uint64_t u = n->value.as.i;
        bool uns = n->type_str && (n->type_str[0] == 'u' || !strcmp(n->type_str, "size"));
        if (uns && u > (uint64_t)FOLD_I64_MAX) return false;
        *out = (long long)u;
    }
    return true;
}

static bool lit_flt(const ast_node_t *n, double *out)
{
    if (!n || n->kind != AST_LITERAL) return false;
    if (n->op == TK_FLOAT) {
        *out = n->value.as.f;
        return true;
    }
    if (n->op == TK_INT) {
        long long v;
        if (!lit_int(n, &v)) return false;
        *out = (double)v;
        return true;
    }
    return false;
}

static bool lit_bool(const ast_node_t *n, bool *out)
{
    if (!n || n->kind != AST_LITERAL) return false;
    if (n->op == TK_KW_TRUE) {
        *out = true;
        return true;
    }
    if (n->op == TK_KW_FALSE) {
        *out = false;
        return true;
    }
    return false;
}

static bool lit_str(const ast_node_t *n)
{
    return n && n->kind == AST_LITERAL &&
           (n->op == TK_STRING || n->op == TK_TRIPLE_STRING || n->op == TK_RAW_STRING) &&
           n->value.as.s != NULL;
}

static void fold_done(ast_node_t *n, token_kind_t op)
{
    n->kind = AST_LITERAL;
    n->op = op;
    n->a = NULL;
    n->b = NULL;
    n->synthetic = false;
}

static void fold_int(ast_node_t *n, long long v)
{
    n->value.kind = TV_INT;
    n->value.as.i = (uint64_t)v;
    fold_done(n, TK_INT);
}

static void fold_flt(ast_node_t *n, double v)
{
    n->value.kind = TV_FLOAT;
    n->value.as.f = v;
    fold_done(n, TK_FLOAT);
}

static void fold_bool(ast_node_t *n, bool v)
{
    n->value.kind = TV_BOOL;
    n->value.as.b = v;
    fold_done(n, v ? TK_KW_TRUE : TK_KW_FALSE);
}

static bool add_ok(long long a, long long b)
{
    if (b > 0) return a <= FOLD_I64_MAX - b;
    return a >= FOLD_I64_MIN - b;
}

static bool sub_ok(long long a, long long b)
{
    if (b > 0) return a >= FOLD_I64_MIN + b;
    return a <= FOLD_I64_MAX + b;
}

static bool mul_ok(long long a, long long b)
{
    if (a == 0 || b == 0) return true;
    if (a == -1) return b != FOLD_I64_MIN;
    if (b == -1) return a != FOLD_I64_MIN;
    if (a > 0) {
        if (b > 0) return a <= FOLD_I64_MAX / b;
        return b >= FOLD_I64_MIN / a;
    }
    if (b > 0) return a >= FOLD_I64_MIN / b;
    return a >= FOLD_I64_MAX / b;
}

static bool flt_finite(double v)
{
    return v == v && v <= 1.7976931348623157e308 && v >= -1.7976931348623157e308;
}

static bool fold_result_is_int(const ast_node_t *n)
{
    const char *ts = n->type_str;
    if (!ts) return false;
    return !strcmp(ts, "i8") || !strcmp(ts, "i16") || !strcmp(ts, "i32") ||
           !strcmp(ts, "i64") || !strcmp(ts, "u8") || !strcmp(ts, "u16") ||
           !strcmp(ts, "u32") || !strcmp(ts, "u64") || !strcmp(ts, "char");
}

static void fold_binary_int(sema_t *s, ast_node_t *n, long long a, long long b)
{
    switch (n->op) {
    case TK_PLUS:
        if (fold_result_is_int(n) && add_ok(a, b)) fold_int(n, a + b);
        return;
    case TK_MINUS:
        if (fold_result_is_int(n) && sub_ok(a, b)) fold_int(n, a - b);
        return;
    case TK_STAR:
        if (fold_result_is_int(n) && mul_ok(a, b)) fold_int(n, a * b);
        return;
    case TK_SLASH:
    case TK_PERCENT:
        if (b == 0) {
            SERR(s, 21, &n->span, "division by zero in constant expression");
            return;
        }
        if (!fold_result_is_int(n)) return;
        if (a == FOLD_I64_MIN && b == -1) return;
        fold_int(n, n->op == TK_SLASH ? a / b : a % b);
        return;
    case TK_EQ:
        fold_bool(n, a == b);
        return;
    case TK_NE:
        fold_bool(n, a != b);
        return;
    case TK_LT:
        fold_bool(n, a < b);
        return;
    case TK_GT:
        fold_bool(n, a > b);
        return;
    case TK_LE:
        fold_bool(n, a <= b);
        return;
    case TK_GE:
        fold_bool(n, a >= b);
        return;
    default:
        return;
    }
}

static void fold_binary_flt(ast_node_t *n, double a, double b)
{
    double r;
    switch (n->op) {
    case TK_PLUS:
        r = a + b;
        break;
    case TK_MINUS:
        r = a - b;
        break;
    case TK_STAR:
        r = a * b;
        break;
    case TK_SLASH:
        if (b == 0.0) return;
        r = a / b;
        break;
    case TK_EQ:
        fold_bool(n, a == b);
        return;
    case TK_NE:
        fold_bool(n, a != b);
        return;
    case TK_LT:
        fold_bool(n, a < b);
        return;
    case TK_GT:
        fold_bool(n, a > b);
        return;
    case TK_LE:
        fold_bool(n, a <= b);
        return;
    case TK_GE:
        fold_bool(n, a >= b);
        return;
    default:
        return;
    }
    if (!flt_finite(r)) return;
    if (!n->type_str || (strcmp(n->type_str, "f32") && strcmp(n->type_str, "f64")))
        return;
    fold_flt(n, r);
}

#define FOLD_STR_REPEAT_MAX_BYTES (1u << 20)

static void fold_binary_str_repeat(sema_t *s, ast_node_t *n, const ast_node_t *str_n,
                                   long long count)
{
    size_t sl = str_n->value.slen ? str_n->value.slen : strlen(str_n->value.as.s);
    if (count < 0) count = 0;
    uint64_t total = (uint64_t)sl * (uint64_t)count;
    if (total > FOLD_STR_REPEAT_MAX_BYTES) return;
    char *buf = (char *)arena_alloc(s->a, (size_t)total + 1);
    size_t i = 0;
    for (; i < (size_t)count; i++)
        memcpy(buf + i * sl, str_n->value.as.s, sl);
    buf[total] = '\0';
    n->value.kind = TV_STRING;
    n->value.as.s = buf;
    n->value.slen = (size_t)total;
    fold_done(n, TK_STRING);
}

static void fold_binary_str(sema_t *s, ast_node_t *n)
{
    size_t la = n->a->value.slen ? n->a->value.slen : strlen(n->a->value.as.s);
    size_t lb = n->b->value.slen ? n->b->value.slen : strlen(n->b->value.as.s);
    char *buf = (char *)arena_alloc(s->a, la + lb + 1);
    memcpy(buf, n->a->value.as.s, la);
    memcpy(buf + la, n->b->value.as.s, lb);
    buf[la + lb] = '\0';
    n->value.kind = TV_STRING;
    n->value.as.s = buf;
    n->value.slen = la + lb;
    fold_done(n, TK_STRING);
}

static void fold_binary(sema_t *s, ast_node_t *n)
{
    long long ia, ib;
    double fa, fb;
    bool ba, bb;
    if (n->op == TK_AND || n->op == TK_OR) {
        if (lit_bool(n->a, &ba) && lit_bool(n->b, &bb))
            fold_bool(n, n->op == TK_AND ? (ba && bb) : (ba || bb));
        return;
    }
    if (lit_int(n->a, &ia) && lit_int(n->b, &ib)) {
        fold_binary_int(s, n, ia, ib);
        return;
    }
    if (lit_flt(n->a, &fa) && lit_flt(n->b, &fb) &&
        (n->a->op == TK_FLOAT || n->b->op == TK_FLOAT)) {
        fold_binary_flt(n, fa, fb);
        return;
    }
    if (lit_bool(n->a, &ba) && lit_bool(n->b, &bb)) {
        if (n->op == TK_EQ)
            fold_bool(n, ba == bb);
        else if (n->op == TK_NE)
            fold_bool(n, ba != bb);
        return;
    }
    if (n->op == TK_PLUS && lit_str(n->a) && lit_str(n->b) && n->type_str &&
        !strcmp(n->type_str, "str")) {
        fold_binary_str(s, n);
        return;
    }
    if (n->op == TK_STAR && n->type_str && !strcmp(n->type_str, "str")) {
        if (lit_str(n->a) && lit_int(n->b, &ia))
            fold_binary_str_repeat(s, n, n->a, ia);
        else if (lit_str(n->b) && lit_int(n->a, &ia))
            fold_binary_str_repeat(s, n, n->b, ia);
    }
}

static void fold_unary(ast_node_t *n)
{
    long long iv;
    double fv;
    bool bv;
    if (n->op == TK_MINUS) {
        if (lit_int(n->a, &iv) && iv != FOLD_I64_MIN && fold_result_is_int(n)) {
            fold_int(n, -iv);
            return;
        }
        if (n->a && n->a->kind == AST_LITERAL && n->a->op == TK_FLOAT) {
            fv = n->a->value.as.f;
            fold_flt(n, -fv);
        }
        return;
    }
    if (n->op == TK_NOT && lit_bool(n->a, &bv)) fold_bool(n, !bv);
}

void sema_fold_expr(sema_t *s, ast_node_t *n)
{
    if (!n) return;
    if (n->kind == AST_BINARY)
        fold_binary(s, n);
    else if (n->kind == AST_UNARY)
        fold_unary(n);
}
