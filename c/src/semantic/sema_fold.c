/*
 * Salam Programming Language (2024-2026)
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
    case TK_AMP:
        if (fold_result_is_int(n)) fold_int(n, (long long)((uint64_t)a & (uint64_t)b));
        return;
    case TK_PIPE:
        if (fold_result_is_int(n)) fold_int(n, (long long)((uint64_t)a | (uint64_t)b));
        return;
    case TK_CARET:
        if (fold_result_is_int(n)) fold_int(n, (long long)((uint64_t)a ^ (uint64_t)b));
        return;
    case TK_SHL:
        if (fold_result_is_int(n) && b >= 0 && b < 64)
            fold_int(n, (long long)((uint64_t)a << b));
        return;
    case TK_SHR:
        if (fold_result_is_int(n) && b >= 0 && b < 64) fold_int(n, a >> b);
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
    if (n->op == TK_TILDE && lit_int(n->a, &iv) && fold_result_is_int(n))
        fold_int(n, (long long)(~(uint64_t)iv));
}

void sema_fold_expr(sema_t *s, ast_node_t *n)
{
    if (!n) return;
    if (n->kind == AST_BINARY)
        fold_binary(s, n);
    else if (n->kind == AST_UNARY)
        fold_unary(n);
}

/*
 * Constant-expression evaluation.
 *
 * sema_fold_expr above rewrites a node into a literal, and it only fires
 * when the operands already *are* literals. That is enough for `1 > 2` but
 * it stops at a named `const`, so the dead-branch checks that read the
 * folded node saw an identifier and gave up. sema_const_eval answers the
 * same question without rewriting anything: it walks the expression, looks
 * consts up in the scope chain, and evaluates their initializers too.
 *
 * It is a query, so it reports nothing and marks nothing used - a check
 * that merely asks whether a loop is dead must not change what else the
 * program diagnoses.
 *
 * Soundness rule: every arithmetic result must fit the node's own type, or
 * the answer is "unknown". A u8 expression that wraps at run time is not
 * folded here, because saying "always false" about a wrapped value that is
 * actually true would turn a diagnostic into a false accusation. Missing a
 * dead loop is fine; inventing one is not.
 */

#define CONST_EVAL_MAX_DEPTH 32

static const char *cv_type_str(const ast_node_t *n)
{
    if (n->type_str) return n->type_str;
    return n->type ? n->type->name : NULL;
}

/* cv_int_range - the value range of an integer type_str. u64/size are
 * capped at i64 max because lit_int already refuses anything above it. */
static bool cv_int_range(const char *ts, long long *lo, long long *hi)
{
    if (!ts) return false;
    if (!strcmp(ts, "i8")) {
        *lo = -128LL;
        *hi = 127LL;
        return true;
    }
    if (!strcmp(ts, "i16")) {
        *lo = -32768LL;
        *hi = 32767LL;
        return true;
    }
    if (!strcmp(ts, "i32")) {
        *lo = -2147483648LL;
        *hi = 2147483647LL;
        return true;
    }
    if (!strcmp(ts, "i64")) {
        *lo = FOLD_I64_MIN;
        *hi = FOLD_I64_MAX;
        return true;
    }
    if (!strcmp(ts, "u8")) {
        *lo = 0LL;
        *hi = 255LL;
        return true;
    }
    if (!strcmp(ts, "u16")) {
        *lo = 0LL;
        *hi = 65535LL;
        return true;
    }
    if (!strcmp(ts, "u32")) {
        *lo = 0LL;
        *hi = 4294967295LL;
        return true;
    }
    if (!strcmp(ts, "u64") || !strcmp(ts, "size")) {
        *lo = 0LL;
        *hi = FOLD_I64_MAX;
        return true;
    }
    return false;
}

static bool cv_fits(const char *ts, long long v)
{
    long long lo, hi;
    if (!cv_int_range(ts, &lo, &hi)) return false;
    return v >= lo && v <= hi;
}

static bool cv_is_float_ts(const char *ts)
{
    return ts && !strcmp(ts, "f64");
}

static double cv_as_flt(const const_val_t *v)
{
    return v->kind == CV_FLOAT ? v->f : (double)v->i;
}

static void cv_set_int(const_val_t *out, long long v)
{
    out->kind = CV_INT;
    out->i = v;
    out->f = 0.0;
    out->b = false;
}

static void cv_set_flt(const_val_t *out, double v)
{
    out->kind = CV_FLOAT;
    out->i = 0;
    out->f = v;
    out->b = false;
}

static void cv_set_bool(const_val_t *out, bool v)
{
    out->kind = CV_BOOL;
    out->i = 0;
    out->f = 0.0;
    out->b = v;
}

static bool const_eval(sema_t *s, ast_node_t *n, const_val_t *out, int depth);

static bool const_binary_int(ast_node_t *n, long long a, long long b, const_val_t *out)
{
    long long r;
    switch (n->op) {
    case TK_EQ:
        cv_set_bool(out, a == b);
        return true;
    case TK_NE:
        cv_set_bool(out, a != b);
        return true;
    case TK_LT:
        cv_set_bool(out, a < b);
        return true;
    case TK_GT:
        cv_set_bool(out, a > b);
        return true;
    case TK_LE:
        cv_set_bool(out, a <= b);
        return true;
    case TK_GE:
        cv_set_bool(out, a >= b);
        return true;
    case TK_PLUS:
        if (!add_ok(a, b)) return false;
        r = a + b;
        break;
    case TK_MINUS:
        if (!sub_ok(a, b)) return false;
        r = a - b;
        break;
    case TK_STAR:
        if (!mul_ok(a, b)) return false;
        r = a * b;
        break;
    case TK_SLASH:
    case TK_PERCENT:
        if (b == 0 || (a == FOLD_I64_MIN && b == -1)) return false;
        r = (n->op == TK_SLASH) ? a / b : a % b;
        break;
    case TK_AMP:
        r = (long long)((uint64_t)a & (uint64_t)b);
        break;
    case TK_PIPE:
        r = (long long)((uint64_t)a | (uint64_t)b);
        break;
    case TK_CARET:
        r = (long long)((uint64_t)a ^ (uint64_t)b);
        break;
    case TK_SHL:
        if (b < 0 || b >= 64) return false;
        r = (long long)((uint64_t)a << b);
        break;
    case TK_SHR:
        if (b < 0 || b >= 64) return false;
        r = a >> b;
        break;
    default:
        return false;
    }
    if (!cv_fits(cv_type_str(n), r)) return false;
    cv_set_int(out, r);
    return true;
}

static bool const_binary_flt(ast_node_t *n, double a, double b, const_val_t *out)
{
    double r;
    switch (n->op) {
    case TK_EQ:
        cv_set_bool(out, a == b);
        return true;
    case TK_NE:
        cv_set_bool(out, a != b);
        return true;
    case TK_LT:
        cv_set_bool(out, a < b);
        return true;
    case TK_GT:
        cv_set_bool(out, a > b);
        return true;
    case TK_LE:
        cv_set_bool(out, a <= b);
        return true;
    case TK_GE:
        cv_set_bool(out, a >= b);
        return true;
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
        if (b == 0.0) return false;
        r = a / b;
        break;
    default:
        return false;
    }
    /* f32 arithmetic rounds narrower than this double math, so only f64
     * results are exact enough to claim as constants. */
    if (!flt_finite(r) || !cv_is_float_ts(cv_type_str(n))) return false;
    cv_set_flt(out, r);
    return true;
}

static bool const_binary(sema_t *s, ast_node_t *n, const_val_t *out, int depth)
{
    const_val_t va, vb;
    if (!n->a || !n->b) return false;

    /* Short-circuit: `false && f()` is false however unknown f() is. */
    if (n->op == TK_AND || n->op == TK_OR) {
        if (!const_eval(s, n->a, &va, depth) || va.kind != CV_BOOL) return false;
        if (n->op == TK_AND && !va.b) {
            cv_set_bool(out, false);
            return true;
        }
        if (n->op == TK_OR && va.b) {
            cv_set_bool(out, true);
            return true;
        }
        if (!const_eval(s, n->b, &vb, depth) || vb.kind != CV_BOOL) return false;
        cv_set_bool(out, vb.b);
        return true;
    }

    if (!const_eval(s, n->a, &va, depth) || !const_eval(s, n->b, &vb, depth))
        return false;
    if (va.kind == CV_BOOL || vb.kind == CV_BOOL) {
        if (va.kind != CV_BOOL || vb.kind != CV_BOOL) return false;
        if (n->op == TK_EQ) {
            cv_set_bool(out, va.b == vb.b);
            return true;
        }
        if (n->op == TK_NE) {
            cv_set_bool(out, va.b != vb.b);
            return true;
        }
        return false;
    }
    if (va.kind == CV_INT && vb.kind == CV_INT)
        return const_binary_int(n, va.i, vb.i, out);
    return const_binary_flt(n, cv_as_flt(&va), cv_as_flt(&vb), out);
}

static bool const_unary(sema_t *s, ast_node_t *n, const_val_t *out, int depth)
{
    const_val_t v;
    if (!n->a || !const_eval(s, n->a, &v, depth)) return false;
    if (n->op == TK_NOT) {
        if (v.kind != CV_BOOL) return false;
        cv_set_bool(out, !v.b);
        return true;
    }
    if (n->op == TK_MINUS) {
        if (v.kind == CV_FLOAT) {
            cv_set_flt(out, -v.f);
            return true;
        }
        if (v.kind != CV_INT || v.i == FOLD_I64_MIN) return false;
        if (!cv_fits(cv_type_str(n), -v.i)) return false;
        cv_set_int(out, -v.i);
        return true;
    }
    if (n->op == TK_TILDE) {
        long long r;
        if (v.kind != CV_INT) return false;
        r = (long long)(~(uint64_t)v.i);
        if (!cv_fits(cv_type_str(n), r)) return false;
        cv_set_int(out, r);
        return true;
    }
    return false;
}

static bool const_cast(sema_t *s, ast_node_t *n, const_val_t *out, int depth)
{
    const_val_t v;
    const char *ts;
    if (!n->a || !const_eval(s, n->a, &v, depth)) return false;
    ts = cv_type_str(n);
    if (!ts) return false;
    if (!strcmp(ts, "bool")) {
        if (v.kind != CV_BOOL) return false;
        cv_set_bool(out, v.b);
        return true;
    }
    if (cv_is_float_ts(ts)) {
        if (v.kind == CV_BOOL) return false;
        cv_set_flt(out, cv_as_flt(&v));
        return true;
    }
    if (v.kind == CV_BOOL) return false;
    if (v.kind == CV_FLOAT) {
        /* A cast that would wrap or round is not a value we can claim. */
        if (v.f != (double)(long long)v.f) return false;
        if (!cv_fits(ts, (long long)v.f)) return false;
        cv_set_int(out, (long long)v.f);
        return true;
    }
    if (!cv_fits(ts, v.i)) return false;
    cv_set_int(out, v.i);
    return true;
}

/* const_eval_sym - the value a `const` binding was declared with. */
static bool const_eval_sym(sema_t *s, symbol_t *sym, const_val_t *out, int depth)
{
    if (!sym || sym->kind != SYM_CONST || sym->is_mut) return false;
    if (sym->decl && sym->decl->a && const_eval(s, sym->decl->a, out, depth)) return true;
    if (sym->has_ival) {
        cv_set_int(out, sym->ival);
        return true;
    }
    return false;
}

/* const_eval_member - a `pkg.NAME` reference to an exported const. */
static bool const_eval_member(sema_t *s, ast_node_t *n, const_val_t *out, int depth)
{
    symbol_t *pk, *cs;
    if (!n->a || n->a->kind != AST_IDENTIFIER || !n->name) return false;
    pk = scope_lookup(s->cur, n->a->name);
    if (!pk || pk->kind != SYM_PACKAGE || !pk->members) return false;
    cs = scope_lookup_local(pk->members, n->name);
    if (!cs || !cs->is_pub) return false;
    return const_eval_sym(s, cs, out, depth);
}

static bool const_eval(sema_t *s, ast_node_t *n, const_val_t *out, int depth)
{
    if (!n || depth >= CONST_EVAL_MAX_DEPTH) return false;
    depth++;
    switch (n->kind) {
    case AST_LITERAL:
        if (n->op == TK_KW_TRUE) {
            cv_set_bool(out, true);
            return true;
        }
        if (n->op == TK_KW_FALSE) {
            cv_set_bool(out, false);
            return true;
        }
        if (n->op == TK_INT) {
            long long v;
            if (!lit_int(n, &v)) return false;
            cv_set_int(out, v);
            return true;
        }
        if (n->op == TK_FLOAT) {
            cv_set_flt(out, n->value.as.f);
            return true;
        }
        return false;
    case AST_IDENTIFIER:
        return const_eval_sym(s, scope_lookup(s->cur, n->name), out, depth);
    case AST_MEMBER:
        return const_eval_member(s, n, out, depth);
    case AST_UNARY:
        return const_unary(s, n, out, depth);
    case AST_BINARY:
        return const_binary(s, n, out, depth);
    case AST_CAST:
        return const_cast(s, n, out, depth);
    case AST_TERNARY: {
        const_val_t c;
        if (!const_eval(s, n->a, &c, depth) || c.kind != CV_BOOL) return false;
        return const_eval(s, c.b ? n->b : n->c, out, depth);
    }
    default:
        return false;
    }
}

bool sema_const_eval(sema_t *s, ast_node_t *n, const_val_t *out)
{
    return const_eval(s, n, out, 0);
}

bool sema_const_bool(sema_t *s, ast_node_t *n, bool *out)
{
    const_val_t v;
    if (!const_eval(s, n, &v, 0) || v.kind != CV_BOOL) return false;
    *out = v.b;
    return true;
}
