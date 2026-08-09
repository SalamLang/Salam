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

#include "llvm/codegen_llvm_internal.h"
#include "core/sal_format.h"
#include "codegen/print_fmt.h"
#include "semantic/builtins.h"
#include "i18n/i18n.h"

/*
 * No 'nsw'/'nuw': salam defines signed and unsigned overflow alike as
 * two's-complement wrap (SALAM-TYPES.md 4.1), while 'nsw' tells LLVM that
 * signed overflow is poison - which let `(100 as i8) + (100 as i8)` fold to
 * 200 instead of wrapping to -56.
 */
static const char *ll_arith_op(token_kind_t k, bool isflt, bool issigned)
{
    switch (k) {
    case TK_PLUS:
        return isflt ? "fadd" : "add";
    case TK_MINUS:
        return isflt ? "fsub" : "sub";
    case TK_STAR:
        return isflt ? "fmul" : "mul";
    case TK_SLASH:
        return isflt ? "fdiv" : (issigned ? "sdiv" : "udiv");
    case TK_PERCENT:
        return isflt ? "frem" : (issigned ? "srem" : "urem");
    case TK_AMP:
        return "and";
    case TK_PIPE:
        return "or";
    case TK_CARET:
        return "xor";
    case TK_SHL:
        return "shl";
    case TK_SHR:
        return issigned ? "ashr" : "lshr";
    default:
        return NULL;
    }
}

static const char *ll_cmp_pred(token_kind_t k, bool isflt, bool issigned)
{
    switch (k) {
    case TK_EQ:
        return isflt ? "oeq" : "eq";
    case TK_NE:
        /* `une`, not `one`: `!=` has to be true when either side is NaN,
         * the way C's `!=` is. `one` is the ordered form and answers false
         * for NaN, which made math.IsNaN() (`ret x != x`) always false on
         * the LLVM backend while the C backend got it right. */
        return isflt ? "une" : "ne";
    case TK_LT:
        return isflt ? "olt" : (issigned ? "slt" : "ult");
    case TK_GT:
        return isflt ? "ogt" : (issigned ? "sgt" : "ugt");
    case TK_LE:
        return isflt ? "ole" : (issigned ? "sle" : "ule");
    case TK_GE:
        return isflt ? "oge" : (issigned ? "sge" : "uge");
    default:
        return NULL;
    }
}

static bool ll_is_cmp(token_kind_t k)
{
    return k == TK_EQ || k == TK_NE || k == TK_LT || k == TK_GT || k == TK_LE ||
           k == TK_GE;
}

static const char *ll_to_ptr(ll_t *ll, llv_t v)
{
    if (ll_is_ptr_ts(v.ts) || ll_is_str(v.ts) || !strcmp(v.ts, "null")) return v.ref;
    const char *r = ll_new_tmp(ll);
    ll_emit(ll, "%s = inttoptr %s %s to ptr", r, ll_ty(ll, v.ts), v.ref);
    return r;
}

static const char *ll_str_operand(ll_t *ll, ast_node_t *n)
{
    const char *ts = n->type_str;
    if (ts && ll_is_str(ts)) return ll_expr(ll, n).ref;
    llv_t v = ll_expr(ll, n);
    const char *r = ll_new_tmp(ll);
    if (ll_is_bool(ts)) {
        const char *t = ll_strconst(ll, "true"), *f = ll_strconst(ll, "false");
        ll_emit(ll, "%s = select i1 %s, ptr %s, ptr %s", r, v.ref, t, f);
    } else if (ts && !strcmp(ts, "char")) {
        ll_need(ll, LL_H_CHARSTR);
        ll_emit(ll, "%s = call ptr @salam_ll_charstr(i32 %s)", r, ll_conv(ll, v, "i32"));
    } else if (ll_is_float(ts)) {
        ll_need(ll, LL_H_F64STR);
        ll_emit(ll, "%s = call ptr @salam_ll_f64str(double %s)", r,
                ll_conv(ll, v, "f64"));
    } else if (ll_is_int(ts)) {
        if (ll_is_signed(ts)) {
            ll_need(ll, LL_H_I64STR);
            ll_emit(ll, "%s = call ptr @salam_ll_i64str(i64 %s)", r,
                    ll_conv(ll, v, "i64"));
        } else {
            ll_need(ll, LL_H_U64STR);
            ll_emit(ll, "%s = call ptr @salam_ll_u64str(i64 %s)", r,
                    ll_conv(ll, v, "u64"));
        }
    } else {
        ll_error(ll, n, "string concatenation with an unsupported operand type '%s'",
                 ts ? ts : "?");
        return v.ref;
    }
    return r;
}

static bool ll_binary_string(ll_t *ll, ast_node_t *n, token_kind_t op, llv_t *out)
{
    bool as = n->a->type_str && ll_is_str(n->a->type_str);
    bool bs = n->b->type_str && ll_is_str(n->b->type_str);
    if (op == TK_PLUS && (as || bs)) {
        const char *L = ll_str_operand(ll, n->a), *R = ll_str_operand(ll, n->b);
        const char *r = ll_new_tmp(ll);
        ll_need(ll, LL_H_STRCAT);
        ll_emit(ll, "%s = call ptr @salam_ll_strcat(ptr %s, ptr %s)", r, L, R);
        *out = (llv_t){r, "str"};
        return true;
    }
    if (op == TK_STAR && (as || bs)) {
        ast_node_t *sop = as ? n->a : n->b;
        ast_node_t *nop = as ? n->b : n->a;
        const char *S = ll_expr(ll, sop).ref;
        const char *N = ll_conv(ll, ll_expr(ll, nop), "i32");
        const char *r = ll_new_tmp(ll);
        ll_need(ll, LL_H_REPEAT);
        ll_emit(ll, "%s = call ptr @salam_ll_repeat(ptr %s, i32 %s)", r, S, N);
        *out = (llv_t){r, "str"};
        return true;
    }
    if (ll_is_cmp(op) && as && bs) {
        llv_t L = ll_expr(ll, n->a), R = ll_expr(ll, n->b);
        const char *c = ll_new_tmp(ll), *r = ll_new_tmp(ll);
        ll_emit(ll, "%s = call i32 @strcmp(ptr %s, ptr %s)", c, L.ref, R.ref);
        ll_emit(ll, "%s = icmp %s i32 %s, 0", r, ll_cmp_pred(op, false, true), c);
        *out = (llv_t){r, "bool"};
        return true;
    }
    return false;
}

static llv_t ll_binary_pow(ll_t *ll, ast_node_t *n)
{
    const char *l = ll_conv(ll, ll_expr(ll, n->a), "f64");
    const char *rr = ll_conv(ll, ll_expr(ll, n->b), "f64");
    const char *r = ll_new_tmp(ll);
    ll_emit(ll, "%s = call double @llvm.pow.f64(double %s, double %s)", r, l, rr);
    if (n->type_str && !ll_is_float(n->type_str))
        return (llv_t){ll_conv(ll, (llv_t){r, "f64"}, n->type_str), n->type_str};
    return (llv_t){r, n->type_str ? n->type_str : "f64"};
}

static const char *ll_op_method_name(token_kind_t k)
{
    switch (k) {
    case TK_PLUS:
        return "operator_add";
    case TK_MINUS:
        return "operator_sub";
    case TK_STAR:
        return "operator_mul";
    case TK_SLASH:
        return "operator_div";
    case TK_PERCENT:
        return "operator_mod";
    case TK_POWER:
        return "operator_pow";
    case TK_EQ:
        return "operator_eq";
    case TK_NE:
        return "operator_ne";
    case TK_LT:
        return "operator_lt";
    case TK_GT:
        return "operator_gt";
    case TK_LE:
        return "operator_le";
    case TK_GE:
        return "operator_ge";
    default:
        return NULL;
    }
}

/*
 * The scope a callee's body must be emitted under: the one that declares
 * its owning type, not whichever module happens to be calling. Falls back
 * to the current scope for a type that cannot be placed.
 */
static scope_t *ll_owner_scope(ll_t *ll, symbol_t *owner)
{
    scope_t *sc = ll_scope_of(ll, owner);
    return sc ? sc : ll->pkg_scope;
}

static symbol_t *ll_op_struct(ll_t *ll, const char *ts, const char **sname)
{
    if (!ts) return NULL;
    *sname = ll_is_ptr_ts(ts) ? arena_strndup(ll->a, ts, strlen(ts) - 1) : ts;
    symbol_t *ss = ll_struct_sym(ll, *sname);
    /* Mangle by the struct symbol's own name, never by the spelling at the
     * use site. ll_ensure_fn emits the body through ll_function(.., ss),
     * which mangles with ss->name, so a receiver whose type_str carries the
     * package ("excel.FileMeta") would call @salam_excel__FileMeta_free
     * while the definition landed as @salam_FileMeta_free. */
    if (ss && ss->name) *sname = ss->name;
    return ss;
}

static func_sig_t *ll_pick_arity(symbol_t *ms, size_t want)
{
    {
        size_t i = 0;
        for (; i < ms->overloads.len; i++) {
            func_sig_t *sig = (func_sig_t *)ms->overloads.data[i];
            if (sig->params.len == want) return sig;
        }
    }
    return NULL;
}

static bool ll_op_call(ll_t *ll, ast_node_t *recv, const char *sname, symbol_t *ss,
                       const char *mname, ast_node_t *rhs, llv_t *out)
{
    symbol_t *ms = scope_lookup_local(ss->members, mname);
    if (!ms || ms->kind != SYM_METHOD) return false;
    func_sig_t *sig = ll_pick_arity(ms, rhs ? 1 : 0);
    if (!sig) return false;
    /*
     * Operator methods reached this way (v[i] lowering to
     * operator_index, etc.) emitted the call without ever requesting the
     * callee's body, so a monomorphized instance nothing else referenced -
     * Vector<diag.Diag>'s operator_index - was called but never defined,
     * and the module failed to parse. Every other call site pairs its
     * emit with an ensure_fn; this one was missing it.
     */
    ll_ensure_fn(ll, sig->decl, ss, ll_owner_scope(ll, ss));
    const char *recvref =
        ll_is_ptr_ts(recv->type_str) ? ll_expr(ll, recv).ref : ll_addr_of(ll, recv).ptr;
    sb_t ab;
    sb_init(&ab);
    sb_puts(&ab, ll_fmt(ll, "ptr %s", recvref));
    if (rhs) {
        const char *pts = sig->params.len
                              ? type_to_string(ll->sem->tc, (type_t *)sig->params.data[0])
                              : NULL;
        llv_t rv = ll_expr(ll, rhs);
        if (!pts) pts = rv.ts;
        sb_puts(&ab, ll_fmt(ll, ", %s %s", ll_ty(ll, pts), ll_conv(ll, rv, pts)));
    }
    const char *args = arena_strdup(ll->a, sb_cstr(&ab));
    sb_free(&ab);
    const char *rts = type_to_string(ll->sem->tc, sig->ret);
    const char *fname = ll_mangle(ll, sname, mname, sig);
    if (rts && !strcmp(rts, "void")) {
        ll_emit(ll, "call void @%s(%s)", fname, args);
        *out = (llv_t){"0", "void"};
        return true;
    }
    const char *r = ll_new_tmp(ll);
    ll_emit(ll, "%s = call %s @%s(%s)", r, ll_ty(ll, rts), fname, args);
    *out = (llv_t){r, rts};
    return true;
}

bool ll_index_set(ll_t *ll, ast_node_t *idx, ast_node_t *value)
{
    if (!idx->a || !idx->a->type_str) return false;
    const char *sname;
    symbol_t *ss = ll_op_struct(ll, idx->a->type_str, &sname);
    if (!ss) return false;
    symbol_t *ms = scope_lookup_local(ss->members, "operator_index_set");
    if (!ms || ms->kind != SYM_METHOD) return false;
    func_sig_t *sig = ll_pick_arity(ms, 2);
    if (!sig) return false;
    /* Same missing pairing as ll_op_call: emit the call, request the body. */
    ll_ensure_fn(ll, sig->decl, ss, ll_owner_scope(ll, ss));
    const char *recv = ll_is_ptr_ts(idx->a->type_str) ? ll_expr(ll, idx->a).ref
                                                      : ll_addr_of(ll, idx->a).ptr;
    const char *p0 = type_to_string(ll->sem->tc, (type_t *)sig->params.data[0]);
    const char *p1 = type_to_string(ll->sem->tc, (type_t *)sig->params.data[1]);
    const char *iv = ll_conv(ll, ll_expr(ll, idx->b), p0);
    const char *vv = ll_conv(ll, ll_expr(ll, value), p1);
    const char *fname = ll_mangle(ll, sname, "operator_index_set", sig);
    ll_emit(ll, "call void @%s(ptr %s, %s %s, %s %s)", fname, recv, ll_ty(ll, p0), iv,
            ll_ty(ll, p1), vv);
    return true;
}

llv_t ll_binary(ll_t *ll, ast_node_t *n)
{
    token_kind_t op = n->op;

    const char *sname;
    symbol_t *oss = n->a ? ll_op_struct(ll, n->a->type_str, &sname) : NULL;
    if (oss) {
        bool neg = (op == TK_NE && n->synthetic);
        const char *mname = neg ? "operator_eq" : ll_op_method_name(op);
        llv_t out;
        if (mname && ll_op_call(ll, n->a, sname, oss, mname, n->b, &out)) {
            if (!neg) return out;
            const char *r = ll_new_tmp(ll);
            ll_emit(ll, "%s = xor i1 %s, true", r, out.ref);
            return (llv_t){r, "bool"};
        }
    }

    llv_t handled;
    if (ll_binary_string(ll, n, op, &handled)) return handled;

    if (op == TK_AND || op == TK_OR) {
        bool is_and = (op == TK_AND);
        const char *rhsL = ll_new_lbl(ll, "sc_rhs");
        const char *scL = ll_new_lbl(ll, "sc_short");
        const char *joinL = ll_new_lbl(ll, "sc_join");
        const char *endL = ll_new_lbl(ll, "sc_end");
        const char *la = ll_as_i1(ll, ll_expr(ll, n->a));
        if (is_and)
            ll_emit_term(ll, "br i1 %s, label %%%s, label %%%s", la, rhsL, scL);
        else
            ll_emit_term(ll, "br i1 %s, label %%%s, label %%%s", la, scL, rhsL);
        ll_emit_label(ll, scL);
        ll_emit_term(ll, "br label %%%s", endL);
        ll_emit_label(ll, rhsL);
        const char *lb = ll_as_i1(ll, ll_expr(ll, n->b));
        ll_emit_term(ll, "br label %%%s", joinL);
        ll_emit_label(ll, joinL);
        ll_emit_term(ll, "br label %%%s", endL);
        ll_emit_label(ll, endL);
        {
            const char *r = ll_new_tmp(ll);
            ll_emit(ll, "%s = phi i1 [ %s, %%%s ], [ %s, %%%s ]", r,
                    is_and ? "false" : "true", scL, lb, joinL);
            return (llv_t){r, "bool"};
        }
    }
    if (op == TK_POWER) return ll_binary_pow(ll, n);
    llv_t L = ll_expr(ll, n->a), R = ll_expr(ll, n->b);
    if (ll_is_cmp(op)) {
        bool lp = ll_is_ptr_ts(L.ts) || !strcmp(L.ts, "null");
        bool rp = ll_is_ptr_ts(R.ts) || !strcmp(R.ts, "null");
        if (lp || rp) {
            const char *lc = ll_to_ptr(ll, L), *rc = ll_to_ptr(ll, R);
            const char *r = ll_new_tmp(ll);
            ll_emit(ll, "%s = icmp %s ptr %s, %s", r, ll_cmp_pred(op, false, false), lc,
                    rc);
            return (llv_t){r, "bool"};
        }
        const char *common = ll_common(ll, L.ts, R.ts);
        const char *lc = ll_conv(ll, L, common), *rc = ll_conv(ll, R, common);
        bool flt = ll_is_float(common);
        const char *r = ll_new_tmp(ll);
        ll_emit(ll, "%s = %s %s %s %s, %s", r, flt ? "fcmp" : "icmp",
                ll_cmp_pred(op, flt, ll_is_signed(common)), ll_ty(ll, common), lc, rc);
        return (llv_t){r, "bool"};
    }

    const char *rt = n->type_str ? n->type_str : ll_common(ll, L.ts, R.ts);
    const char *lc = ll_conv(ll, L, rt), *rc = ll_conv(ll, R, rt);
    bool flt = ll_is_float(rt);
    const char *o = ll_arith_op(op, flt, ll_is_signed(rt));
    if (!o) {
        ll_error(ll, n, "binary operator");
        return ll_poison(rt);
    }
    const char *r = ll_new_tmp(ll);
    ll_emit(ll, "%s = %s %s %s, %s", r, o, ll_ty(ll, rt), lc, rc);
    return (llv_t){r, rt};
}

static llv_t ll_unary(ll_t *ll, ast_node_t *n)
{
    if (n->a && (n->op == TK_MINUS || n->op == TK_NOT)) {
        const char *sname;
        symbol_t *oss = ll_op_struct(ll, n->a->type_str, &sname);
        if (oss) {
            const char *mname = n->op == TK_MINUS ? "operator_sub" : "operator_not";
            llv_t out;
            if (ll_op_call(ll, n->a, sname, oss, mname, NULL, &out)) return out;
        }
    }
    if (n->op == TK_NOT) {
        const char *v = ll_as_i1(ll, ll_expr(ll, n->a));
        const char *r = ll_new_tmp(ll);
        ll_emit(ll, "%s = xor i1 %s, true", r, v);
        return (llv_t){r, "bool"};
    }
    if (n->op == TK_MINUS) {
        llv_t v = ll_expr(ll, n->a);
        const char *rt = n->type_str ? n->type_str : v.ts;
        const char *cv = ll_conv(ll, v, rt);
        const char *r = ll_new_tmp(ll);
        if (ll_is_float(rt))
            ll_emit(ll, "%s = fneg %s %s", r, ll_ty(ll, rt), cv);
        else
            ll_emit(ll, "%s = sub %s 0, %s", r, ll_ty(ll, rt), cv);
        return (llv_t){r, rt};
    }
    if (n->op == TK_TILDE) {
        llv_t v = ll_expr(ll, n->a);
        const char *rt = n->type_str ? n->type_str : v.ts;
        const char *cv = ll_conv(ll, v, rt);
        const char *r = ll_new_tmp(ll);
        ll_emit(ll, "%s = xor %s %s, -1", r, ll_ty(ll, rt), cv);
        return (llv_t){r, rt};
    }
    ll_error(ll, n, "unary operator");
    return ll_poison(n->type_str);
}

static llv_t ll_incdec(ll_t *ll, ast_node_t *n)
{
    ast_node_t *tgt = n->a;
    if (!tgt || (tgt->kind != AST_IDENTIFIER && tgt->kind != AST_MEMBER &&
                 tgt->kind != AST_INDEX)) {
        ll_error(ll, n, "operand of '++'/'--' is not assignable");
        return ll_poison(n->type_str);
    }
    ll_addr_t a = ll_addr_of(ll, tgt);
    const char *ts = a.ts;
    const char *oldv = ll_new_tmp(ll);
    ll_emit(ll, "%s = load %s, ptr %s", oldv, ll_ty(ll, ts), a.ptr);
    bool flt = ll_is_float(ts);
    const char *step = flt ? "1.0" : "1";
    const char *op =
        ll_arith_op(n->op == TK_PLUS_PLUS ? TK_PLUS : TK_MINUS, flt, ll_is_signed(ts));
    const char *newv = ll_new_tmp(ll);
    ll_emit(ll, "%s = %s %s %s, %s", newv, op, ll_ty(ll, ts), oldv, step);
    ll_emit(ll, "store %s %s, ptr %s", ll_ty(ll, ts), newv, a.ptr);
    return (llv_t){n->is_prefix ? newv : oldv, ts};
}

static void ll_lower_print(ll_t *ll, ast_node_t *n, bool nl, int err)
{
    vec_t segs;
    vec_init(&segs);
    pf_build(ll->a, n, nl, &segs);
    if (segs.len == 0) return;

    bool buffered = ll->single_threaded && !err;

    if (buffered) {
        bool all_lit = true;
        {
            size_t i = 0;
            for (; i < segs.len; i++)
                if (((pf_seg_t *)segs.data[i])->kind != PF_LIT) {
                    all_lit = false;
                    break;
                }
        }
        if (all_lit) {
            sb_t raw;
            sb_init(&raw);
            {
                size_t i = 0;
                for (; i < segs.len; i++)
                    sb_puts(&raw, ((pf_seg_t *)segs.data[i])->text);
            }
            size_t rawlen = raw.len;
            const char *lit = ll_strconst(ll, sb_cstr(&raw));
            sb_free(&raw);
            if (rawlen) {
                ll_need(ll, LL_H_OUTBUF);
                ll_emit(ll, "call void @salam_out_write(ptr %s, i64 %zu)", lit, rawlen);
            }
            return;
        }
    }

    sb_t fmt;
    sb_init(&fmt);
    sb_t args;
    sb_init(&args);
    {
        size_t i = 0;
        for (; i < segs.len; i++) {
            pf_seg_t *s = (pf_seg_t *)segs.data[i];
            if (s->kind == PF_LIT) {
                {
                    const char *p = s->text;
                    for (; *p; p++) {
                        if (*p == '%') sb_putc(&fmt, '%');
                        sb_putc(&fmt, *p);
                    }
                }
                continue;
            }
            llv_t v = ll_expr(ll, s->expr);
            sb_puts(&fmt, pf_spec(s->kind));
            switch (s->kind) {
            case PF_STR:
                sb_puts(&args, ll_fmt(ll, ", ptr %s", v.ref));
                break;
            case PF_BOOL: {
                const char *t = ll_strconst(ll, "true"), *f = ll_strconst(ll, "false");
                const char *sel = ll_new_tmp(ll);
                ll_emit(ll, "%s = select i1 %s, ptr %s, ptr %s", sel, v.ref, t, f);
                sb_puts(&args, ll_fmt(ll, ", ptr %s", sel));
                break;
            }
            case PF_CHAR:
                sb_puts(&args, ll_fmt(ll, ", i32 %s", ll_conv(ll, v, "i32")));
                break;
            case PF_F64:
                sb_puts(&args, ll_fmt(ll, ", double %s", ll_conv(ll, v, "f64")));
                break;
            case PF_I32:
                sb_puts(&args, ll_fmt(ll, ", i32 %s", ll_conv(ll, v, "i32")));
                break;
            case PF_U32:
                sb_puts(&args, ll_fmt(ll, ", i32 %s", ll_conv(ll, v, "u32")));
                break;
            case PF_I64:
                sb_puts(&args, ll_fmt(ll, ", i64 %s", v.ref));
                break;
            case PF_U64:
                sb_puts(&args, ll_fmt(ll, ", i64 %s", v.ref));
                break;
            case PF_SIZE:
                sb_puts(&args, ll_fmt(ll, ", %s %s", ll->usize, ll_conv(ll, v, "size")));
                break;
            default:
                break;
            }
        }
    }
    const char *f = ll_strconst(ll, sb_cstr(&fmt));
    if (ll->single_threaded) {
        ll_need(ll, LL_H_OUTBUF);
        ll_emit(ll, "call void @salam_out_flush()");
    }
    const char *t = ll_new_tmp(ll);
    if (err && ll_target_is_windows(ll->triple)) {
        ll_emit(ll, "%s = call i32 (ptr, ...) @printf(ptr %s%s)", t, f, sb_cstr(&args));
    } else if (err) {
        ll_emit(ll, "%s = call i32 (i32, ptr, ...) @dprintf(i32 2, ptr %s%s)", t, f,
                sb_cstr(&args));
    } else {
        ll_emit(ll, "%s = call i32 (ptr, ...) @printf(ptr %s%s)", t, f, sb_cstr(&args));
        if (buffered) {
            /*
             * fflush(NULL) flushes every open output stream, which is all
             * this needs: push the printf line out to fd 1 before the next
             * @salam_out_write bypasses stdio. Reaching it through the
             * `stdout` global instead would need the platform's spelling of
             * that symbol - Darwin calls it `__stdoutp`, so `@stdout` was
             * left unresolved there and took the whole module down with it,
             * which is why every formatted print failed on macOS while
             * literal-only ones worked.
             */
            const char *t2 = ll_new_tmp(ll);
            ll_emit(ll, "%s = call i32 @fflush(ptr null)", t2);
        }
    }
    sb_free(&fmt);
    sb_free(&args);
}

static bool ll_param_is_ref(func_sig_t *sig, size_t i)
{
    return sig && sig->decl && i < sig->decl->list.len &&
           ((ast_node_t *)sig->decl->list.data[i])->is_ref;
}

static void ll_emit_arg(ll_t *ll, sb_t *ab, ast_node_t *arg, func_sig_t *sig, size_t i)
{
    if (ll_param_is_ref(sig, i)) {
        ll_addr_t a = ll_addr_of(ll, arg);
        sb_puts(ab, ll_fmt(ll, "ptr %s", a.ptr));
        return;
    }
    llv_t v = ll_expr(ll, arg);
    const char *pts = i < sig->params.len
                          ? type_to_string(ll->sem->tc, (type_t *)sig->params.data[i])
                          : v.ts;
    sb_puts(ab, ll_fmt(ll, "%s %s", ll_ty(ll, pts), ll_conv(ll, v, pts)));
}

static void ll_fill_defaults(ll_t *ll, sb_t *ab, ast_node_t *n, func_sig_t *sig,
                             bool first)
{
    if (!sig || !sig->decl) return;
    size_t np = sig->decl->list.len;
    {
        size_t i = n->list.len;
        for (; i < np; i++) {
            ast_node_t *param = (ast_node_t *)sig->decl->list.data[i];
            if (!param->a) continue;
            if (!first) sb_puts(ab, ", ");
            ll_emit_arg(ll, ab, param->a, sig, i);
            first = false;
        }
    }
}

static llv_t ll_call_user(ll_t *ll, ast_node_t *n, const char *nm)
{
    symbol_t *fsym = NULL;
    /*
     * The enclosing package's own function outranks whatever the global
     * scope holds under that name, exactly as ordinary scoping demands.
     * ll_sym searches globally first, which got this wrong twice: std/time
     * is `package time` and also declares `extern func time(tp: void*)`, so
     * a bare `time(null)` inside it resolved to SYM_PACKAGE and was reported
     * as an unknown function; and std/net/http's own `send(method, url,
     * headers, body): Response` lost to the libc `send()` extern that
     * std/net/internal/rawsock declares, which silently lowered four string
     * arguments through ptrtoint into a socket call.
     */
    if (ll->pkg_scope) {
        symbol_t *local = scope_lookup_local(ll->pkg_scope, nm);
        if (local && local->kind == SYM_FUNC) fsym = local;
    }
    if (!fsym) fsym = ll_sym(ll, nm);
    if (!fsym || fsym->kind != SYM_FUNC) {
        ll_error(ll, n, "call to unknown/unsupported function '%s'", nm);
        return ll_poison(n->type_str);
    }
    func_sig_t *sig = ll_pick_overload(ll, fsym, n);
    ll_ensure_fn(ll, sig->decl, NULL, ll->pkg_scope);
    sb_t ab;
    sb_init(&ab);
    {
        size_t i = 0;
        for (; i < n->list.len; i++) {
            if (i) sb_puts(&ab, ", ");
            ll_emit_arg(ll, &ab, (ast_node_t *)n->list.data[i], sig, i);
        }
    }
    ll_fill_defaults(ll, &ab, n, sig, n->list.len == 0);
    const char *args = arena_strdup(ll->a, sb_cstr(&ab));
    sb_free(&ab);
    const char *rts = type_to_string(ll->sem->tc, sig->ret);

    bool is_ext = sig->decl && sig->decl->is_extern;
    const char *fname = is_ext ? nm : ll_mangle(ll, NULL, nm, sig);
    const char *callty = "";
    if (is_ext && sig->variadic) {
        sb_t sp;
        sb_init(&sp);
        sb_putc(&sp, '(');
        {
            size_t i = 0;
            for (; i < sig->params.len; i++) {
                if (i) sb_puts(&sp, ", ");
                sb_puts(&sp, ll_ty(ll, type_to_string(ll->sem->tc,
                                                      (type_t *)sig->params.data[i])));
            }
        }
        sb_puts(&sp, sig->params.len ? ", ...)" : "...)");
        callty = ll_fmt(ll, " %s", arena_strdup(ll->a, sb_cstr(&sp)));
        sb_free(&sp);
    }
    if (rts && !strcmp(rts, "void")) {
        ll_emit(ll, "call void%s @%s(%s)", callty, fname, args);
        return (llv_t){"0", "void"};
    }
    const char *r = ll_new_tmp(ll);
    ll_emit(ll, "%s = call %s%s @%s(%s)", r, ll_ty(ll, rts), callty, fname, args);
    return (llv_t){r, rts};
}

static llv_t ll_len_of(ll_t *ll, ast_node_t *n, ast_node_t *arg)
{
    const char *ts = arg->type_str;
    if (ll_is_slice_ts(ts)) {
        llv_t v = ll_expr(ll, arg);
        const char *l = ll_new_tmp(ll);
        ll_emit(ll, "%s = extractvalue { ptr, i64 } %s, 1", l, v.ref);
        return (llv_t){ll_conv(ll, (llv_t){l, "i64"}, "i32"), "i32"};
    }
    if (ts && strchr(ts, '[')) return (llv_t){ll_fmt(ll, "%ld", ll_array_dim(ts)), "i32"};
    if (!ts || !ll_is_str(ts)) {
        ll_error(ll, n, "len() of an unsupported type");
        return ll_poison("i32");
    }
    {
        long klen = ast_str_lit_len(arg);
        if (klen >= 0) return (llv_t){ll_fmt(ll, "%ld", klen), "i32"};
    }
    llv_t v = ll_expr(ll, arg);
    const char *l = ll_new_tmp(ll);
    ll_emit(ll, "%s = call %s @strlen(ptr %s)", l, ll->usize, v.ref);
    return (llv_t){ll_usize_to_i32(ll, l), "i32"};
}

static llv_t ll_call_len(ll_t *ll, ast_node_t *n)
{
    return ll_len_of(ll, n, (ast_node_t *)n->list.data[0]);
}

static llv_t ll_emit_call(ll_t *ll, ast_node_t *n, func_sig_t *sig, const char *lead,
                          const char *fname, const char *rts)
{
    sb_t ab;
    sb_init(&ab);
    bool first = !lead || !lead[0];
    if (!first) sb_puts(&ab, lead);
    {
        size_t i = 0;
        for (; i < n->list.len; i++) {
            if (!first) sb_puts(&ab, ", ");
            ll_emit_arg(ll, &ab, (ast_node_t *)n->list.data[i], sig, i);
            first = false;
        }
    }
    ll_fill_defaults(ll, &ab, n, sig, first);
    const char *args = arena_strdup(ll->a, sb_cstr(&ab));
    sb_free(&ab);
    if (rts && !strcmp(rts, "void")) {
        ll_emit(ll, "call void @%s(%s)", fname, args);
        return (llv_t){"0", "void"};
    }
    const char *r = ll_new_tmp(ll);
    ll_emit(ll, "%s = call %s @%s(%s)", r, ll_ty(ll, rts), fname, args);
    return (llv_t){r, rts};
}

/* Defined below, but needed by ll_call_method's func-typed-field case. */
static llv_t ll_call_indirect(ll_t *ll, ast_node_t *n, ast_node_t *callee);
static bool ll_is_func_ts(const char *ts);

/*
 * Locate a `salam_*` runtime builtin by name in any package sema has
 * loaded. These are not symbols that have to come from a prebuilt runtime
 * archive: the stdlib declares them as `extern:` blocks that *do* carry a
 * Salam body (std/fs/fs.salam's salam_file_read, std/text/text.salam's
 * salam_str_split, ...), which ll_toplevel already emits - only the
 * dispatch from the surface syntax (`f.read(n)`, `s.split(d)`) down to
 * them was missing. Mirrors how the C backend names the same runtime
 * entry points in codegen_call.c's call_file/call_str.
 */
static func_sig_t *ll_runtime_fn(ll_t *ll, const char *name, symbol_t **owner)
{
    size_t p = 0;
    for (; p < ll->sem->packages.len; p++) {
        symbol_t *pk = (symbol_t *)ll->sem->packages.data[p];
        symbol_t *fs;
        if (!pk || pk->kind != SYM_PACKAGE || !pk->members) continue;
        fs = scope_lookup_local(pk->members, name);
        if (!fs || fs->kind != SYM_FUNC || fs->overloads.len == 0) continue;
        *owner = pk;
        return (func_sig_t *)fs->overloads.data[0];
    }
    return NULL;
}

/*
 * Emit a call to a runtime builtin resolved by ll_runtime_fn. `recv`, when
 * non-NULL, is lowered as the leading argument - the runtime entry points
 * for method-shaped builtins take their receiver as parameter 0, so the
 * call's own argument list lines up with sig->params starting at index 1.
 * That offset is why this does not go through ll_emit_call, which pairs
 * n->list.data[i] with sig->params.data[i].
 */
static bool ll_call_runtime(ll_t *ll, ast_node_t *n, const char *rtname, ast_node_t *recv,
                            llv_t *out)
{
    symbol_t *pk = NULL;
    func_sig_t *sig = ll_runtime_fn(ll, rtname, &pk);
    size_t base, i;
    sb_t ab;
    const char *args, *rts, *r;
    if (!sig || !sig->decl) return false;
    ll_touch_pkg_named(ll, pk->pkgname);
    ll_ensure_fn(ll, sig->decl, NULL, pk->members);
    sb_init(&ab);
    base = 0;
    if (recv) {
        llv_t rv = ll_expr(ll, recv);
        const char *pts = sig->params.len
                              ? type_to_string(ll->sem->tc, (type_t *)sig->params.data[0])
                              : rv.ts;
        sb_puts(&ab, ll_fmt(ll, "%s %s", ll_ty(ll, pts), ll_conv(ll, rv, pts)));
        base = 1;
    }
    for (i = 0; i < n->list.len; i++) {
        llv_t v = ll_expr(ll, (ast_node_t *)n->list.data[i]);
        const char *pts =
            (i + base) < sig->params.len
                ? type_to_string(ll->sem->tc, (type_t *)sig->params.data[i + base])
                : v.ts;
        if (base || i) sb_puts(&ab, ", ");
        sb_puts(&ab, ll_fmt(ll, "%s %s", ll_ty(ll, pts), ll_conv(ll, v, pts)));
    }
    args = arena_strdup(ll->a, sb_cstr(&ab));
    sb_free(&ab);
    rts = type_to_string(ll->sem->tc, sig->ret);
    if (rts && !strcmp(rts, "void")) {
        ll_emit(ll, "call void @%s(%s)", rtname, args);
        *out = (llv_t){"0", "void"};
        return true;
    }
    r = ll_new_tmp(ll);
    ll_emit(ll, "%s = call %s @%s(%s)", r, ll_ty(ll, rts), rtname, args);
    *out = (llv_t){r, rts};
    return true;
}

/*
 * Builtins that hand back a freshly allocated `const char**` plus an
 * out-param element count, which the surface language sees as a
 * Vector<str>. Vector<T> is { data: T*, _len: int, _cap: int } (see
 * std/collections/vector.salam), so cap is filled with the same count as
 * len - the buffer is exactly sized and never grown in place, matching
 * what the C backend's call_ident/call_str build for the same runtimes.
 * `recv` is the receiver for method-shaped builtins (s.split(d)) and NULL
 * for free functions (listdir(p), args()).
 */
static bool ll_call_vec_str(ll_t *ll, ast_node_t *n, const char *rtname, ast_node_t *recv,
                            llv_t *out)
{
    symbol_t *pk = NULL;
    func_sig_t *sig = ll_runtime_fn(ll, rtname, &pk);
    const char *vts = n->type_str ? n->type_str : "Vector<str>";
    const char *vty, *cnt, *data, *ln, *v0, *v1, *v2;
    sb_t ab;
    size_t i;
    if (!sig || !sig->decl) return false;
    ll_touch_pkg_named(ll, pk->pkgname);
    ll_ensure_fn(ll, sig->decl, NULL, pk->members);
    vty = ll_ty(ll, vts);
    cnt = ll_new_tmp(ll);
    ll_emit_alloca(ll, "%s = alloca i32", cnt);
    sb_init(&ab);
    if (recv) sb_puts(&ab, ll_fmt(ll, "ptr %s", ll_expr(ll, recv).ref));
    for (i = 0; i < n->list.len; i++) {
        llv_t v = ll_expr(ll, (ast_node_t *)n->list.data[i]);
        if (recv || i) sb_puts(&ab, ", ");
        sb_puts(&ab, ll_fmt(ll, "ptr %s", v.ref));
    }
    if (recv || n->list.len) sb_puts(&ab, ", ");
    sb_puts(&ab, ll_fmt(ll, "ptr %s", cnt));
    data = ll_new_tmp(ll);
    ll_emit(ll, "%s = call ptr @%s(%s)", data, rtname, arena_strdup(ll->a, sb_cstr(&ab)));
    sb_free(&ab);
    ln = ll_new_tmp(ll);
    ll_emit(ll, "%s = load i32, ptr %s", ln, cnt);
    v0 = ll_new_tmp(ll);
    v1 = ll_new_tmp(ll);
    v2 = ll_new_tmp(ll);
    ll_emit(ll, "%s = insertvalue %s undef, ptr %s, 0", v0, vty, data);
    ll_emit(ll, "%s = insertvalue %s %s, i32 %s, 1", v1, vty, v0, ln);
    ll_emit(ll, "%s = insertvalue %s %s, i32 %s, 2", v2, vty, v1, ln);
    *out = (llv_t){v2, vts};
    return true;
}

/* File* methods - the C backend's call_file() counterpart. */
static bool ll_call_file(ll_t *ll, ast_node_t *n, ast_node_t *obj, const char *m,
                         llv_t *out)
{
    static const struct {
        const char *method;
        const char *runtime;
    } map[] = {{"read", "salam_file_read"},
               {"readline", "salam_file_readline"},
               {"write", "salam_file_write"},
               {"seek", "salam_file_seek"},
               {"close", "salam_file_close"}};
    size_t i = 0;
    for (; i < sizeof map / sizeof map[0]; i++)
        if (!strcmp(m, map[i].method))
            return ll_call_runtime(ll, n, map[i].runtime, obj, out);
    return false;
}

/*
 * `pkg.NAME` where NAME is a package-level const/var or an enum member of
 * an enum the package exports - the non-call half of package-qualified
 * access, which previously fell through to ll_member_addr and reported the
 * package identifier itself as an unknown address.
 *
 * Touching the package first is what makes this work: ll_touch_pkg emits
 * that package's globals and registers them in ll->globals, so the
 * ll_global_find below can only succeed afterwards.
 */
static bool ll_pkg_value(ll_t *ll, ast_node_t *n, symbol_t *pk, llv_t *out)
{
    symbol_t *m;
    lvar_t *g;
    const char *r;
    if (!pk->members) return false;
    m = scope_lookup_local(pk->members, n->name);
    if (!m) return false;
    if (m->kind == SYM_ENUM_MEMBER) {
        *out = (llv_t){ll_fmt(ll, "%lld", (long long)m->enum_value), "i32"};
        return true;
    }
    if (m->kind != SYM_CONST && m->kind != SYM_VAR) return false;
    ll_touch_pkg_named(ll, pk->pkgname);
    g = ll_global_find(ll, n->name);
    if (!g) return false;
    r = ll_new_tmp(ll);
    ll_emit(ll, "%s = load %s, ptr %s", r, ll_ty(ll, g->ts), g->ptr);
    *out = (llv_t){r, g->ts};
    return true;
}

static llv_t ll_call_pkg(ll_t *ll, ast_node_t *n, symbol_t *pk, const char *fname_)
{
    ll_touch_pkg_named(ll, pk->pkgname);
    symbol_t *fs = scope_lookup_local(pk->members, fname_);
    if (!fs || fs->kind != SYM_FUNC || fs->overloads.len == 0) {
        ll_error(ll, n, "package function '%s' not found", fname_);
        return ll_poison(n->type_str);
    }
    func_sig_t *sig = ll_pick_overload(ll, fs, n);
    ll_ensure_fn(ll, sig->decl, NULL, pk->members);
    bool is_ext = sig->decl && sig->decl->is_extern;
    /* pk->pkgname, not the caller's package: this is the one call path that
     * resolves through another package's member scope. */
    const char *fname = is_ext ? fname_
                               : ll_mangle_in(ll, pk->pkgname ? pk->pkgname : pk->name,
                                              NULL, fname_, sig);
    return ll_emit_call(ll, n, sig, "", fname, type_to_string(ll->sem->tc, sig->ret));
}

static bool ll_call_str(ll_t *ll, ast_node_t *n, ast_node_t *obj, const char *m,
                        llv_t *out)
{
    size_t na = n->list.len;
    if (!strcmp(m, "len") && na == 0) {
        long klen = ast_str_lit_len(obj);
        if (klen >= 0) {
            *out = (llv_t){ll_fmt(ll, "%ld", klen), "i32"};
            return true;
        }
    }
    /* Before `recv` is materialized below: ll_call_vec_str lowers the
     * receiver itself, and evaluating `obj` twice would duplicate any side
     * effects in it. */
    if (!strcmp(m, "split") && na == 1)
        return ll_call_vec_str(ll, n, "salam_str_split", obj, out);
    const char *recv = ll_expr(ll, obj).ref;
    const char *r;
    if (!strcmp(m, "len")) {
        const char *l = ll_new_tmp(ll);
        ll_emit(ll, "%s = call %s @strlen(ptr %s)", l, ll->usize, recv);
        *out = (llv_t){ll_usize_to_i32(ll, l), "i32"};
        return true;
    }
    if (!strcmp(m, "concat") && na == 1) {
        const char *a = ll_expr(ll, (ast_node_t *)n->list.data[0]).ref;
        r = ll_new_tmp(ll);
        ll_need(ll, LL_H_STRCAT);
        ll_emit(ll, "%s = call ptr @salam_ll_strcat(ptr %s, ptr %s)", r, recv, a);
        *out = (llv_t){r, "str"};
        return true;
    }
    if (!strcmp(m, "substr") && na == 2) {
        const char *s = ll_conv(ll, ll_expr(ll, (ast_node_t *)n->list.data[0]), "i32");
        const char *l = ll_conv(ll, ll_expr(ll, (ast_node_t *)n->list.data[1]), "i32");
        r = ll_new_tmp(ll);
        ll_need(ll, LL_H_SUBSTR);
        ll_emit(ll, "%s = call ptr @salam_ll_substr(ptr %s, i32 %s, i32 %s)", r, recv, s,
                l);
        *out = (llv_t){r, "str"};
        return true;
    }
    if (!strcmp(m, "trim")) {
        r = ll_new_tmp(ll);
        ll_need(ll, LL_H_TRIM);
        ll_emit(ll, "%s = call ptr @salam_ll_trim(ptr %s)", r, recv);
        *out = (llv_t){r, "str"};
        return true;
    }
    if (!strcmp(m, "to_int")) {
        const char *x = ll_new_tmp(ll);
        r = ll_new_tmp(ll);
        ll_emit(ll, "%s = call i64 @strtol(ptr %s, ptr null, i32 10)", x, recv);
        ll_emit(ll, "%s = trunc i64 %s to i32", r, x);
        *out = (llv_t){r, "i32"};
        return true;
    }
    if (!strcmp(m, "to_float")) {
        r = ll_new_tmp(ll);
        ll_emit(ll, "%s = call double @strtod(ptr %s, ptr null)", r, recv);
        *out = (llv_t){r, "f64"};
        return true;
    }
    if ((!strcmp(m, "find") || !strcmp(m, "search") || !strcmp(m, "indexOf")) &&
        na == 1) {
        const char *a = ll_expr(ll, (ast_node_t *)n->list.data[0]).ref;
        const char *p = ll_new_tmp(ll), *pi = ll_new_tmp(ll), *ri = ll_new_tmp(ll);
        const char *d = ll_new_tmp(ll), *o = ll_new_tmp(ll), *nz = ll_new_tmp(ll);
        ll_emit(ll, "%s = call ptr @strstr(ptr %s, ptr %s)", p, recv, a);
        ll_emit(ll, "%s = ptrtoint ptr %s to i64", pi, p);
        ll_emit(ll, "%s = ptrtoint ptr %s to i64", ri, recv);
        ll_emit(ll, "%s = sub i64 %s, %s", d, pi, ri);
        ll_emit(ll, "%s = trunc i64 %s to i32", o, d);
        ll_emit(ll, "%s = icmp eq ptr %s, null", nz, p);
        r = ll_new_tmp(ll);
        ll_emit(ll, "%s = select i1 %s, i32 -1, i32 %s", r, nz, o);
        *out = (llv_t){r, "i32"};
        return true;
    }
    if (!strcmp(m, "lower")) {
        r = ll_new_tmp(ll);
        ll_need(ll, LL_H_LOWER);
        ll_emit(ll, "%s = call ptr @salam_ll_lower(ptr %s)", r, recv);
        *out = (llv_t){r, "str"};
        return true;
    }
    if (!strcmp(m, "upper")) {
        r = ll_new_tmp(ll);
        ll_need(ll, LL_H_UPPER);
        ll_emit(ll, "%s = call ptr @salam_ll_upper(ptr %s)", r, recv);
        *out = (llv_t){r, "str"};
        return true;
    }
    if (!strcmp(m, "repeat") && na == 1) {
        const char *cnt = ll_conv(ll, ll_expr(ll, (ast_node_t *)n->list.data[0]), "i32");
        r = ll_new_tmp(ll);
        ll_need(ll, LL_H_REPEAT);
        ll_emit(ll, "%s = call ptr @salam_ll_repeat(ptr %s, i32 %s)", r, recv, cnt);
        *out = (llv_t){r, "str"};
        return true;
    }
    return false;
}

static llv_t ll_call_dyn(ll_t *ll, ast_node_t *n, ast_node_t *obj, const char *iface,
                         const char *mname)
{
    char ib[160];
    size_t k = 0;
    {
        const char *p = iface;
        for (; *p && *p != '*' && *p != '[' && k < sizeof ib - 1; p++)
            ib[k++] = *p;
    }
    ib[k] = 0;
    symbol_t *isym = ll_sym(ll, ib);
    if (!isym || isym->kind != SYM_INTERFACE) {
        ll_error(ll, n, "dynamic call on non-interface '%s'", ib);
        return ll_poison(n->type_str);
    }
    /*
     * `d.free()` on an interface value releases the *box*, not a method on
     * the interface - so no vtable slot could ever match it and the lookup
     * below would report "interface 'Shape' has no method 'free'". Same
     * lowering the C backend gives it in call_dyn().
     */
    if (!strcmp(mname, "free") && n->list.len == 0) {
        llv_t dv = ll_expr(ll, obj);
        /* Same `dyn X*` auto-deref the dispatch path below needs: Vector.get()
         * returns the address of the slot, and extractvalue wants the %dyn. */
        if (ll_is_ptr_ts(dv.ts)) {
            const char *ld = ll_new_tmp(ll);
            ll_emit(ll, "%s = load %%dyn, ptr %s", ld, dv.ref);
            dv.ref = ld;
        }
        const char *data = ll_new_tmp(ll);
        ll_emit(ll, "%s = extractvalue %%dyn %s, 0", data, dv.ref);
        ll_emit(ll, "call void @free(ptr %s)", data);
        return (llv_t){"0", "void"};
    }
    int idx = 0, slot = -1;
    func_sig_t *msig = NULL;
    {
        size_t m = 0;
        for (; m < isym->members->symbols.len; m++) {
            symbol_t *im = (symbol_t *)isym->members->symbols.data[m];
            if (im->kind != SYM_METHOD || !im->overloads.len) continue;
            if (!strcmp(im->name, mname)) {
                slot = idx;
                msig = (func_sig_t *)im->overloads.data[0];
                break;
            }
            idx++;
        }
    }
    if (slot < 0) {
        ll_error(ll, n, "interface '%s' has no method '%s'", ib, mname);
        return ll_poison(n->type_str);
    }

    llv_t dv = ll_expr(ll, obj);
    /*
     * `dyn X*` - a pointer TO an interface value, which is what Vector.get()
     * hands back. Auto-deref before dispatching: extractvalue needs the %dyn
     * itself, not the address of the slot holding it. This used to appear to
     * work only because ll_ty("dyn X*") wrongly answered "%dyn", which broke
     * the data-pointer side of the same vector instead.
     */
    if (ll_is_ptr_ts(dv.ts)) {
        const char *ld = ll_new_tmp(ll);
        ll_emit(ll, "%s = load %%dyn, ptr %s", ld, dv.ref);
        dv.ref = ld;
    }
    const char *data = ll_new_tmp(ll), *vt = ll_new_tmp(ll), *sl = ll_new_tmp(ll),
               *fn = ll_new_tmp(ll);
    ll_emit(ll, "%s = extractvalue %%dyn %s, 0", data, dv.ref);
    ll_emit(ll, "%s = extractvalue %%dyn %s, 1", vt, dv.ref);
    ll_emit(ll, "%s = getelementptr inbounds ptr, ptr %s, i64 %d", sl, vt, slot);
    ll_emit(ll, "%s = load ptr, ptr %s", fn, sl);

    sb_t ab;
    sb_init(&ab);
    sb_puts(&ab, ll_fmt(ll, "ptr %s", data));
    {
        size_t i = 0;
        for (; i < n->list.len; i++) {
            llv_t v = ll_expr(ll, (ast_node_t *)n->list.data[i]);
            const char *pt =
                msig && i < msig->params.len
                    ? type_to_string(ll->sem->tc, (type_t *)msig->params.data[i])
                    : v.ts;
            sb_puts(&ab, ll_fmt(ll, ", %s %s", ll_ty(ll, pt), ll_conv(ll, v, pt)));
        }
    }
    const char *args = arena_strdup(ll->a, sb_cstr(&ab));
    sb_free(&ab);
    const char *rts = type_to_string(ll->sem->tc, msig->ret);
    if (rts && !strcmp(rts, "void")) {
        ll_emit(ll, "call void %s(%s)", fn, args);
        return (llv_t){"0", "void"};
    }
    const char *r = ll_new_tmp(ll);
    ll_emit(ll, "%s = call %s %s(%s)", r, ll_ty(ll, rts), fn, args);
    return (llv_t){r, rts};
}

static llv_t ll_call_method(ll_t *ll, ast_node_t *n, ast_node_t *callee)
{
    ast_node_t *obj = callee->a;
    const char *mname = callee->name;
    const char *ots = obj->type_str ? obj->type_str : "";

    /* Trailing '*'s are not part of the interface name (see ll_call_dyn's
     * auto-deref): "dyn Shape*" dispatches on Shape, not on "Shape*". */
    if (!strncmp(ots, "dyn ", 4)) {
        const char *ib = ots + 4;
        size_t ibn = strlen(ib);
        while (ibn && ib[ibn - 1] == '*')
            ibn--;
        return ll_call_dyn(ll, n, obj, arena_strndup(ll->a, ib, ibn), mname);
    }

    if (!strcmp(ots, "str")) {
        llv_t o;
        if (ll_call_str(ll, n, obj, mname, &o)) return o;
    }

    if (!strcmp(ots, "File*")) {
        llv_t o;
        if (ll_call_file(ll, n, obj, mname, &o)) return o;
    }

    if (!strcmp(mname, "len") && (ll_is_slice_ts(ots) || (ots && strchr(ots, '['))))
        return ll_len_of(ll, n, obj);

    /*
     * Package-qualified call (`str.Equals(...)`). Gated on the receiver
     * having no value type of its own: ll_sym searches the global scope and
     * then *every loaded package*, so a function-local whose name matches a
     * package's - `mut map := Vector {} as Vector<int>` in jsgen.salam,
     * against std/map's `package map` - resolved to the package and its
     * `map.free()` became "package function 'free' not found". A package
     * identifier carries no value type, so requiring an empty/<null> type
     * separates the two cleanly.
     */
    if (obj->kind == AST_IDENTIFIER && (!ots[0] || !strcmp(ots, "<null>"))) {
        symbol_t *pk = ll_sym(ll, obj->name);
        if (pk && pk->kind == SYM_PACKAGE) return ll_call_pkg(ll, n, pk, mname);
        /*
         * ll_sym searches the global scope first, so a package whose name
         * is also an extern function's - std/time is `package time` and
         * declares `extern func time(...)` - can resolve to the function
         * instead, leaving `time.FormatDate(...)` reported as a method on
         * type '<null>'. Whether that happens depends on which other
         * packages a program pulls in, which is why it only showed up in
         * std/excel and not in a two-line test.
         *
         * Only consulted when the receiver is not a value in scope, so a
         * local that legitimately shadows a package name still wins.
         */
        {
            size_t p = 0;
            for (; p < ll->sem->packages.len; p++) {
                symbol_t *cand = (symbol_t *)ll->sem->packages.data[p];
                if (!cand || cand->kind != SYM_PACKAGE || !cand->pkgname) continue;
                if (!strcmp(cand->pkgname, obj->name))
                    return ll_call_pkg(ll, n, cand, mname);
            }
        }
    }

    bool isptr = ll_is_ptr_ts(ots);
    const char *sname = isptr ? arena_strndup(ll->a, ots, strlen(ots) - 1) : ots;
    symbol_t *ss = ll_struct_sym(ll, sname);
    symbol_t *ms = ss ? scope_lookup_local(ss->members, mname) : NULL;
    if (ms && ms->kind == SYM_METHOD && ms->overloads.len) {
        func_sig_t *sig = ll_pick_overload(ll, ms, n);
        ll_ensure_fn(ll, sig->decl, ss, ll_owner_scope(ll, ss));
        const char *recv = isptr ? ll_expr(ll, obj).ref : ll_addr_of(ll, obj).ptr;
        /* ss->name, not sname: see ll_op_struct. A package-qualified
         * receiver type would otherwise call a symbol nothing defines. */
        return ll_emit_call(ll, n, sig, ll_fmt(ll, "ptr %s", recv),
                            ll_mangle(ll, ss->name ? ss->name : sname, mname, sig),
                            type_to_string(ll->sem->tc, sig->ret));
    }

    symbol_t *impl = ll_sym(ll, impl_owner_key(ll->a, ots));
    if (impl && impl->kind == SYM_TYPEIMPL) {
        symbol_t *im = scope_lookup_local(impl->members, mname);
        if (im && im->kind == SYM_METHOD && im->overloads.len) {
            func_sig_t *sig = ll_pick_overload(ll, im, n);
            ll_ensure_fn(ll, sig->decl, impl, ll_owner_scope(ll, impl));
            llv_t rv = ll_expr(ll, obj);
            return ll_emit_call(ll, n, sig, ll_fmt(ll, "%s %s", ll_ty(ll, ots), rv.ref),
                                ll_mangle_ti(ll, ots, mname, sig),
                                type_to_string(ll->sem->tc, sig->ret));
        }
    }

    /*
     * `o.fn(args)` where `fn` is a *field* holding a function value, not a
     * method - so no SYM_METHOD lookup above could ever match it. Same
     * lowering as any other indirect call; ll_call_indirect re-evaluates
     * the callee to get the closure pointer, which for a member expression
     * is the field load.
     */
    if (ss && ll_is_func_ts(callee->type_str)) return ll_call_indirect(ll, n, callee);

    ll_error(ll, n, "method '%s' on type '%s' (or overloaded/builtin method)", mname,
             ots);
    return ll_poison(n->type_str);
}

static llv_t ll_call_indirect(ll_t *ll, ast_node_t *n, ast_node_t *callee)
{
    const char *fts = callee->type_str;
    llv_t env = ll_expr(ll, callee);
    const char *fn = ll_new_tmp(ll);
    ll_emit(ll, "%s = load ptr, ptr %s", fn, env.ref);
    const char *rts = ll_func_ret(ll, fts);
    vec_t pts;
    ll_func_params(ll, fts, &pts);
    sb_t ab;
    sb_init(&ab);
    sb_puts(&ab, ll_fmt(ll, "ptr %s", env.ref));
    {
        size_t i = 0;
        for (; i < n->list.len; i++) {
            llv_t v = ll_expr(ll, (ast_node_t *)n->list.data[i]);
            const char *pt = i < pts.len ? (const char *)pts.data[i] : v.ts;
            sb_puts(&ab, ll_fmt(ll, ", %s %s", ll_ty(ll, pt), ll_conv(ll, v, pt)));
        }
    }
    const char *args = arena_strdup(ll->a, sb_cstr(&ab));
    sb_free(&ab);
    if (rts && !strcmp(rts, "void")) {
        ll_emit(ll, "call void %s(%s)", fn, args);
        return (llv_t){"0", "void"};
    }
    const char *r = ll_new_tmp(ll);
    ll_emit(ll, "%s = call %s %s(%s)", r, ll_ty(ll, rts), fn, args);
    return (llv_t){r, rts};
}

static bool ll_is_func_ts(const char *ts)
{
    return ts && !strncmp(ts, "func(", 5);
}

/*
 * Call through a raw C function pointer - `externfunc(...)`, produced by
 * `x as extern func (i32, i32) i32` and by COM vtable slot casts in
 * std/webview. Unlike a `func(...)` value, which is a closure (env pointer
 * whose first word is the code pointer, so ll_call_indirect loads through
 * it and passes the env as argument 0), an externfunc IS the code pointer
 * and takes no hidden argument. The C backend has kept these apart since
 * cg_call's first line; this is the LLVM side of that split.
 */
static llv_t ll_call_raw_ptr(ll_t *ll, ast_node_t *n, ast_node_t *callee)
{
    const char *fts = callee->type_str;
    llv_t fv = ll_expr(ll, callee);
    const char *fn = ll_new_tmp(ll);
    const char *rts = ll_func_ret(ll, fts);
    vec_t pts;
    sb_t ab;
    const char *args;
    /*
     * The callee only needs an inttoptr when it really is an integer. A raw
     * function pointer is already a ptr under opaque pointers, and
     * `inttoptr ptr ... to ptr` is not a legal cast at all - `raw as extern
     * func (...)` produced exactly that and the module failed to parse.
     */
    if (!strcmp(ll_ty(ll, fv.ts), "ptr"))
        fn = fv.ref;
    else
        ll_emit(ll, "%s = inttoptr %s %s to ptr", fn, ll_ty(ll, fv.ts), fv.ref);
    ll_func_params(ll, fts, &pts);
    sb_init(&ab);
    {
        size_t i = 0;
        for (; i < n->list.len; i++) {
            llv_t v = ll_expr(ll, (ast_node_t *)n->list.data[i]);
            const char *pt = i < pts.len ? (const char *)pts.data[i] : v.ts;
            if (i) sb_puts(&ab, ", ");
            sb_puts(&ab, ll_fmt(ll, "%s %s", ll_ty(ll, pt), ll_conv(ll, v, pt)));
        }
    }
    args = arena_strdup(ll->a, sb_cstr(&ab));
    sb_free(&ab);
    if (rts && !strcmp(rts, "void")) {
        ll_emit(ll, "call void %s(%s)", fn, args);
        return (llv_t){"0", "void"};
    }
    {
        const char *r = ll_new_tmp(ll);
        ll_emit(ll, "%s = call %s %s(%s)", r, ll_ty(ll, rts), fn, args);
        return (llv_t){r, rts};
    }
}

static bool ll_call_intrinsic(ll_t *ll, ast_node_t *n, const char *nm, llv_t *out)
{
    size_t na = n->list.len;
    ast_node_t *a0 = na > 0 ? (ast_node_t *)n->list.data[0] : NULL;
    const char *r;
    if (!strcmp(nm, "hash") && na == 1) {
        r = ll_new_tmp(ll);
        if (ll_is_str(a0->type_str)) {
            ll_need(ll, LL_H_STRHASH);
            ll_emit(ll, "%s = call i64 @salam_ll_strhash(ptr %s)", r,
                    ll_expr(ll, a0).ref);
        } else {
            ll_need(ll, LL_H_INTHASH);
            ll_emit(ll, "%s = call i64 @salam_ll_inthash(i64 %s)", r,
                    ll_conv(ll, ll_expr(ll, a0), "u64"));
        }
        *out = (llv_t){r, "u64"};
        return true;
    }
    if (!strcmp(nm, "char_from_code") && na == 1) {
        r = ll_new_tmp(ll);
        ll_need(ll, LL_H_CHARSTR);
        ll_emit(ll, "%s = call ptr @salam_ll_charstr(i32 %s)", r,
                ll_conv(ll, ll_expr(ll, a0), "i32"));
        *out = (llv_t){r, "str"};
        return true;
    }
    /* char_code(s) is the inverse of char_from_code: the unsigned value of
     * the first byte. Loaded directly rather than through a runtime call,
     * matching the C backend's `(int32_t)(unsigned char)(s)[0]`. */
    if (!strcmp(nm, "char_code") && na == 1) {
        const char *b = ll_new_tmp(ll);
        r = ll_new_tmp(ll);
        ll_emit(ll, "%s = load i8, ptr %s", b, ll_expr(ll, a0).ref);
        ll_emit(ll, "%s = zext i8 %s to i32", r, b);
        *out = (llv_t){r, "i32"};
        return true;
    }
    if (!strcmp(nm, "args") && na == 0)
        return ll_call_vec_str(ll, n, "salam_args", NULL, out);
    if (!strcmp(nm, "listdir") && na == 1)
        return ll_call_vec_str(ll, n, "salam_os_listdir", NULL, out);
    if (!strcmp(nm, "input") && na == 0)
        return ll_call_runtime(ll, n, "salam_input", NULL, out);
    /* open(path, mode) -> std/fs's salam_open, the File* constructor. */
    if (!strcmp(nm, "open") && na == 2)
        return ll_call_runtime(ll, n, "salam_open", NULL, out);
    if (!strcmp(nm, "lang") && na == 0) {
        *out = (llv_t){ll_strconst(ll, i18n_lang()), "str"};
        return true;
    }
    /*
     * funcptr(f)/spawn(f) take a *function name*, not a value, so the
     * argument is lowered to the mangled symbol's address rather than
     * through ll_expr. spawn additionally hands that address to the
     * thread runtime. Mirrors call_ident() in the C backend.
     */
    if ((!strcmp(nm, "funcptr") || !strcmp(nm, "spawn")) && na == 1 &&
        a0->kind == AST_IDENTIFIER) {
        symbol_t *fs = ll_sym(ll, a0->name);
        func_sig_t *fsig = (fs && fs->kind == SYM_FUNC && fs->overloads.len)
                               ? (func_sig_t *)fs->overloads.data[0]
                               : NULL;
        const char *sym;
        if (!fsig || !fsig->decl) return false;
        ll_ensure_fn(ll, fsig->decl, NULL, ll->pkg_scope);
        sym = fsig->decl->is_extern ? a0->name : ll_mangle(ll, NULL, a0->name, fsig);
        r = ll_new_tmp(ll);
        ll_emit(ll, "%s = ptrtoint ptr @%s to i64", r, sym);
        if (!strcmp(nm, "funcptr")) {
            *out = (llv_t){r, "i64"};
            return true;
        }
        {
            symbol_t *pk = NULL;
            func_sig_t *sp = ll_runtime_fn(ll, "salam_thread_spawn", &pk);
            const char *h;
            if (!sp || !sp->decl) return false;
            ll_touch_pkg_named(ll, pk->pkgname);
            ll_ensure_fn(ll, sp->decl, NULL, pk->members);
            h = ll_new_tmp(ll);
            ll_emit(ll, "%s = inttoptr i64 %s to ptr", h, r);
            r = ll_new_tmp(ll);
            ll_emit(ll, "%s = call i64 @salam_thread_spawn(ptr %s)", r, h);
            *out = (llv_t){r, "i64"};
            return true;
        }
    }
    /* callhandler(fp, arg) - an indirect call through an integer-encoded
     * function pointer, the shape the layout/webview callbacks use. */
    if (!strcmp(nm, "callhandler") && na == 2) {
        ast_node_t *a1 = (ast_node_t *)n->list.data[1];
        const char *fp = ll_new_tmp(ll);
        ll_emit(ll, "%s = inttoptr i64 %s to ptr", fp,
                ll_conv(ll, ll_expr(ll, a0), "i64"));
        ll_emit(ll, "call void %s(i64 %s)", fp, ll_conv(ll, ll_expr(ll, a1), "i64"));
        *out = (llv_t){"0", "void"};
        return true;
    }
    /*
     * Table-driven builtins (join -> salam_thread_join, strcmp, ...) - the
     * same k_builtins table the C backend falls back to in call_ident(),
     * so a new entry there reaches both backends without a second edit.
     */
    {
        const salam_builtin_t *bi = salam_builtin_lookup(nm);
        if (bi && ll_call_runtime(ll, n, bi->runtime, NULL, out)) return true;
    }
    return false;
}

static llv_t ll_call(ll_t *ll, ast_node_t *n)
{
    ast_node_t *callee = n->a;
    /* Checked before every other form, exactly as cg_call does: a raw C
     * function pointer is callable whatever expression shape produced it. */
    if (callee && ll_is_extern_fn_ts(callee->type_str))
        return ll_call_raw_ptr(ll, n, callee);
    if (callee && callee->kind == AST_MEMBER) return ll_call_method(ll, n, callee);
    if (callee && callee->kind == AST_IDENTIFIER) {
        const char *nm = callee->name;
        if (!strcmp(nm, "print")) {
            ll_lower_print(ll, n, false, 0);
            return (llv_t){"0", "void"};
        }
        if (!strcmp(nm, "println")) {
            ll_lower_print(ll, n, true, 0);
            return (llv_t){"0", "void"};
        }
        if (!strcmp(nm, "printerr")) {
            ll_lower_print(ll, n, false, 1);
            return (llv_t){"0", "void"};
        }
        if (!strcmp(nm, "printerrln")) {
            ll_lower_print(ll, n, true, 1);
            return (llv_t){"0", "void"};
        }
        if (!strcmp(nm, "len") && n->list.len == 1) return ll_call_len(ll, n);
        if (!strcmp(nm, "sizeof") && n->list.len == 1) {
            const char *t = ((ast_node_t *)n->list.data[0])->type_str;
            const char *szp = ll_new_tmp(ll), *sz = ll_new_tmp(ll);
            ll_emit(ll, "%s = getelementptr %s, ptr null, i32 1", szp,
                    ll_ty(ll, t ? t : "i32"));
            ll_emit(ll, "%s = ptrtoint ptr %s to %s", sz, szp, ll->usize);
            return (llv_t){sz, "size"};
        }
        symbol_t *fsym = ll_sym(ll, nm);
        if (fsym && fsym->kind == SYM_FUNC) return ll_call_user(ll, n, nm);

        if (!fsym) {
            llv_t o;
            if (ll_call_intrinsic(ll, n, nm, &o)) return o;
        }
        if (ll_is_func_ts(callee->type_str)) return ll_call_indirect(ll, n, callee);
        return ll_call_user(ll, n, nm);
    }
    if (callee && ll_is_func_ts(callee->type_str)) return ll_call_indirect(ll, n, callee);
    ll_error(ll, n, "method / package / indirect call");
    return ll_poison(n->type_str);
}

static ll_addr_t ll_member_addr(ll_t *ll, ast_node_t *n);
static ll_addr_t ll_index_addr(ll_t *ll, ast_node_t *n);
ll_addr_t ll_addr_of(ll_t *ll, ast_node_t *n)
{
    switch (n->kind) {
    case AST_IDENTIFIER: {
        lvar_t *v = ll_local_find(ll, n->name);
        if (v) return (ll_addr_t){v->ptr, v->ts};

        if (ll->cur_lambda) {
            vec_t *caps = &ll->cur_lambda->captures;
            {
                size_t i = 0;
                for (; i < caps->len; i++) {
                    ast_node_t *c = (ast_node_t *)caps->data[i];
                    if (!strcmp(c->name, n->name)) {
                        const char *r = ll_new_tmp(ll);
                        ll_emit(ll,
                                "%s = getelementptr inbounds %s, ptr %s, i32 0, i32 %zu",
                                r, ll->env_ty, ll->env_ref, i + 1);
                        return (ll_addr_t){r, c->type_str};
                    }
                }
            }
        }
        if (ll->self_ts) {
            symbol_t *ss = ll_struct_sym(ll, ll->self_ts), *fs = NULL;
            int idx = ss ? ll_field_index(ss, n->name, &fs) : -1;
            if (idx >= 0) {
                const char *fts = type_to_string(ll->sem->tc, fs->type);
                const char *r = ll_new_tmp(ll);
                ll_emit(ll, "%s = getelementptr inbounds %s, ptr %s, i32 0, i32 %d", r,
                        ll_struct_ltype(ll, ll->self_ts), ll->this_ref, idx);
                return (ll_addr_t){r, fts};
            }
        }
        lvar_t *g = ll_global_find(ll, n->name);
        if (g) return (ll_addr_t){g->ptr, g->ts};
        /*
         * A package-level global reached from a function of that same
         * package that nothing has touched yet - std/core's `mut _argc`,
         * read by its own salam_args(), when the only entry point into
         * core was an ensure_fn from elsewhere. ll_touch_pkg emits the
         * package's globals, so the lookup can only succeed after it.
         */
        {
            size_t p = 0;
            for (; p < ll->sem->packages.len; p++) {
                symbol_t *pk = (symbol_t *)ll->sem->packages.data[p];
                symbol_t *gv;
                if (!pk || pk->kind != SYM_PACKAGE || !pk->members) continue;
                gv = scope_lookup_local(pk->members, n->name);
                if (!gv || (gv->kind != SYM_VAR && gv->kind != SYM_CONST)) continue;
                ll_touch_pkg(ll, pk);
                g = ll_global_find(ll, n->name);
                if (g) return (ll_addr_t){g->ptr, g->ts};
            }
        }
        /*
         * Same problem, one scope further out: a package's own top-level
         * `pub const` is registered in that package's sema scope, not in
         * this one's members scope and not in ll->sem->global, so neither
         * the walk above nor a global lookup can see it - there is simply no
         * index from the name back to the declaring package. Touching
         * packages until the global materializes is the only lookup
         * available. It is bounded, idempotent, and reached only on the path
         * that would otherwise be a hard error, and after the first sweep
         * every package is already touched.
         *
         * Whether this path was needed used to depend on the platform, which
         * is why it survived so long: std/net/internal/rawsock declares a
         * `_rawsock_wsa_init` global inside `if SALAM_OS_WINDOWS`, and
         * emitting it got the package touched early, so `AF_INET` and
         * `SOCK_STREAM` resolved by luck. On Linux that global is condcomp'd
         * away and every socket program failed to compile.
         */
        {
            size_t p = 0;
            for (; p < ll->sem->packages.len; p++) {
                symbol_t *pk = (symbol_t *)ll->sem->packages.data[p];
                if (!pk || pk->kind != SYM_PACKAGE || !pk->decl) continue;
                /*
                 * ll_emit_globals rather than ll_touch_pkg: a package can be
                 * recorded in pkg_touched while these globals were never
                 * emitted, so the touch would short-circuit and change
                 * nothing. Emitting straight from the package's AST is what
                 * actually resolves the name, and it is safe to repeat now
                 * that ll_emit_globals skips names it has already emitted.
                 */
                ll_emit_globals(ll, pk->decl);
                g = ll_global_find(ll, n->name);
                if (g) return (ll_addr_t){g->ptr, g->ts};
            }
        }
        ll_error(ll, n, "address of an unknown identifier '%s'", n->name);
        return (ll_addr_t){"null", n->type_str ? n->type_str : "i32"};
    }
    case AST_MEMBER:
        return ll_member_addr(ll, n);
    case AST_INDEX:
        return ll_index_addr(ll, n);
    default: {
        llv_t v = ll_expr(ll, n);
        const char *p = ll_fmt(ll, "%%agg.%d", ll->tmp++);
        ll_emit_alloca(ll, "%s = alloca %s", p, ll_ty(ll, v.ts));
        ll_emit(ll, "store %s %s, ptr %s", ll_ty(ll, v.ts), v.ref, p);
        return (ll_addr_t){p, v.ts};
    }
    }
}

static ll_addr_t ll_member_addr(ll_t *ll, ast_node_t *n)
{
    const char *ots = n->a->type_str ? n->a->type_str : "";
    const char *base, *sname;
    if (ll_is_ptr_ts(ots)) {
        base = ll_expr(ll, n->a).ref;
        sname = arena_strndup(ll->a, ots, strlen(ots) - 1);
    } else {
        ll_addr_t b = ll_addr_of(ll, n->a);
        base = b.ptr;
        sname = ots;
    }
    symbol_t *ss = ll_struct_sym(ll, sname), *fs = NULL;
    int idx = ss ? ll_field_index(ss, n->name, &fs) : -1;
    if (idx < 0) {
        ll_error(ll, n, "member '%s' of non-struct/unknown type '%s'", n->name, sname);
        return (ll_addr_t){"null", n->type_str ? n->type_str : "i32"};
    }
    const char *fts = type_to_string(ll->sem->tc, fs->type);
    const char *r = ll_new_tmp(ll);
    ll_emit(ll, "%s = getelementptr inbounds %s, ptr %s, i32 0, i32 %d", r,
            ll_struct_ltype(ll, sname), base, idx);
    return (ll_addr_t){r, fts};
}

static ll_addr_t ll_index_addr(ll_t *ll, ast_node_t *n)
{
    const char *ots = n->a->type_str ? n->a->type_str : "";
    if (ll_is_slice_ts(ots)) {
        const char *ets = ll_slice_elem(ll, ots);
        llv_t sv = ll_expr(ll, n->a);
        const char *data = ll_new_tmp(ll);
        ll_emit(ll, "%s = extractvalue { ptr, i64 } %s, 0", data, sv.ref);
        const char *idx = ll_conv(ll, ll_expr(ll, n->b), "i64");
        const char *r = ll_new_tmp(ll);
        ll_emit(ll, "%s = getelementptr inbounds %s, ptr %s, i64 %s", r, ll_ty(ll, ets),
                data, idx);
        return (ll_addr_t){r, ets};
    }
    /*
     * `s[i]` on a str. A str IS the pointer, so this indexes the value, not
     * the slot holding it - the array path below took the variable's address
     * and emitted `getelementptr ptr, ptr %v.s, i64 0, i64 %i`, which is not
     * valid IR at all ("invalid getelementptr indices": ptr is not an
     * aggregate, so the leading 0 has nothing to step through). Element type
     * comes from sema, which types str[i] as TY_CHAR.
     */
    if (!strcmp(ots, "str")) {
        const char *base = ll_expr(ll, n->a).ref;
        const char *ets = n->type_str ? n->type_str : "char";
        const char *idx = ll_conv(ll, ll_expr(ll, n->b), "i64");
        const char *r = ll_new_tmp(ll);
        ll_emit(ll, "%s = getelementptr inbounds %s, ptr %s, i64 %s", r, ll_ty(ll, ets),
                base, idx);
        return (ll_addr_t){r, ets};
    }
    if (ll_is_ptr_ts(ots)) {
        const char *base = ll_expr(ll, n->a).ref;
        const char *ets = arena_strndup(ll->a, ots, strlen(ots) - 1);
        const char *idx = ll_conv(ll, ll_expr(ll, n->b), "i64");
        const char *r = ll_new_tmp(ll);
        ll_emit(ll, "%s = getelementptr inbounds %s, ptr %s, i64 %s", r, ll_ty(ll, ets),
                base, idx);
        return (ll_addr_t){r, ets};
    }
    ll_addr_t b = ll_addr_of(ll, n->a);
    const char *ets = ll_array_elem(ll, ots);
    const char *idx = ll_conv(ll, ll_expr(ll, n->b), "i64");
    const char *r = ll_new_tmp(ll);
    ll_emit(ll, "%s = getelementptr inbounds %s, ptr %s, i64 0, i64 %s", r,
            ll_ty(ll, ots), b.ptr, idx);
    return (ll_addr_t){r, ets};
}

static llv_t ll_load_addr(ll_t *ll, ast_node_t *n)
{
    ll_addr_t a = ll_addr_of(ll, n);
    const char *r = ll_new_tmp(ll);
    ll_emit(ll, "%s = load %s, ptr %s", r, ll_ty(ll, a.ts), a.ptr);
    return (llv_t){r, a.ts};
}

static llv_t ll_struct_lit(ll_t *ll, ast_node_t *n)
{
    symbol_t *ss = ll_struct_sym(ll, n->type_str ? n->type_str : n->name);
    if (!ss) {
        ll_error(ll, n, "struct literal of unknown/unsupported type '%s'",
                 n->name ? n->name : "?");
        return ll_poison(n->type_str);
    }
    const char *sname = n->type_str ? n->type_str : n->name;
    const char *sty = ll_struct_ltype(ll, sname);
    const char *cur = "zeroinitializer";
    int idx = 0;
    {
        size_t i = 0;
        for (; i < ss->members->symbols.len; i++) {
            symbol_t *f = (symbol_t *)ss->members->symbols.data[i];
            if (f->kind != SYM_FIELD) continue;
            const char *fts = type_to_string(ll->sem->tc, f->type);
            ast_node_t *prov = NULL;
            {
                size_t j = 0;
                for (; j < n->list.len; j++) {
                    ast_node_t *fi = (ast_node_t *)n->list.data[j];
                    if (fi->name && !strcmp(fi->name, f->name)) {
                        prov = fi;
                        break;
                    }
                }
            }
            const char *val =
                prov ? ll_conv(ll, ll_expr(ll, prov->a), fts)
                     : (f->decl && f->decl->a ? ll_conv(ll, ll_expr(ll, f->decl->a), fts)
                                              : ll_zero(fts));
            const char *r = ll_new_tmp(ll);
            ll_emit(ll, "%s = insertvalue %s %s, %s %s, %d", r, sty, cur, ll_ty(ll, fts),
                    val, idx);
            cur = r;
            idx++;
        }
    }
    return (llv_t){idx ? cur : "zeroinitializer", sname};
}

static llv_t ll_array_lit(ll_t *ll, ast_node_t *n)
{
    const char *ats = n->type_str;
    if (!ats || !strchr(ats, '[')) {
        ll_error(ll, n, "array literal without an array type");
        return ll_poison(ats);
    }
    const char *ety = ll_array_elem(ll, ats);
    const char *aty = ll_ty(ll, ats), *lety = ll_ty(ll, ety);
    const char *cur = "zeroinitializer";
    {
        size_t i = 0;
        for (; i < n->list.len; i++) {
            const char *v = ll_conv(ll, ll_expr(ll, (ast_node_t *)n->list.data[i]), ety);
            const char *r = ll_new_tmp(ll);
            ll_emit(ll, "%s = insertvalue %s %s, %s %s, %zu", r, aty, cur, lety, v, i);
            cur = r;
        }
    }
    return (llv_t){n->list.len ? cur : "zeroinitializer", ats};
}

static llv_t ll_slice_expr(ll_t *ll, ast_node_t *n)
{
    const char *sts = n->type_str;
    const char *ets = ll_slice_elem(ll, sts);
    const char *let = ll_ty(ll, ets);
    const char *bts = n->a->type_str ? n->a->type_str : "";

    const char *lo = n->b ? ll_conv(ll, ll_expr(ll, n->b), "i64") : "0";
    const char *data, *blen;

    if (ll_is_slice_ts(bts)) {
        llv_t bv = ll_expr(ll, n->a);
        const char *bp = ll_new_tmp(ll);
        ll_emit(ll, "%s = extractvalue { ptr, i64 } %s, 0", bp, bv.ref);
        blen = ll_new_tmp(ll);
        ll_emit(ll, "%s = extractvalue { ptr, i64 } %s, 1", blen, bv.ref);
        data = ll_new_tmp(ll);
        ll_emit(ll, "%s = getelementptr inbounds %s, ptr %s, i64 %s", data, let, bp, lo);
    } else {
        ll_addr_t b = ll_addr_of(ll, n->a);
        blen = ll_fmt(ll, "%ld", ll_array_dim(bts));
        data = ll_new_tmp(ll);
        ll_emit(ll, "%s = getelementptr inbounds %s, ptr %s, i64 0, i64 %s", data,
                ll_ty(ll, bts), b.ptr, lo);
    }

    const char *hi = n->c ? ll_conv(ll, ll_expr(ll, n->c), "i64") : blen;
    const char *len = ll_new_tmp(ll);
    ll_emit(ll, "%s = sub i64 %s, %s", len, hi, lo);

    const char *a0 = ll_new_tmp(ll), *a1 = ll_new_tmp(ll);
    ll_emit(ll, "%s = insertvalue { ptr, i64 } zeroinitializer, ptr %s, 0", a0, data);
    ll_emit(ll, "%s = insertvalue { ptr, i64 } %s, i64 %s, 1", a1, a0, len);
    return (llv_t){a1, sts};
}

static llv_t ll_lambda_value(ll_t *ll, ast_node_t *n)
{
    ll_emit_lambda(ll, n);
    const char *name = n->name;
    const char *envty = ll_fmt(ll, "%%lam.%s.env", name + strlen("salam_lambda_"));
    size_t ncap = n->captures.len;
    if (ncap == 0) return (llv_t){ll_fmt(ll, "@%s.env", name), n->type_str};

    const char *szp = ll_new_tmp(ll), *sz = ll_new_tmp(ll), *env = ll_new_tmp(ll);
    ll_emit(ll, "%s = getelementptr %s, ptr null, i32 1", szp, envty);
    ll_emit(ll, "%s = ptrtoint ptr %s to i64", sz, szp);
    ll_emit(ll, "%s = call ptr @malloc(i64 %s)", env, sz);
    const char *f0 = ll_new_tmp(ll);
    ll_emit(ll, "%s = getelementptr inbounds %s, ptr %s, i32 0, i32 0", f0, envty, env);
    ll_emit(ll, "store ptr @%s, ptr %s", name, f0);
    {
        size_t i = 0;
        for (; i < ncap; i++) {
            ast_node_t *c = (ast_node_t *)n->captures.data[i];
            llv_t cv = ll_expr(ll, c);
            const char *fp = ll_new_tmp(ll);
            ll_emit(ll, "%s = getelementptr inbounds %s, ptr %s, i32 0, i32 %zu", fp,
                    envty, env, i + 1);
            ll_emit(ll, "store %s %s, ptr %s", ll_ty(ll, c->type_str),
                    ll_conv(ll, cv, c->type_str), fp);
        }
    }
    return (llv_t){env, n->type_str};
}

static llv_t ll_literal(ll_t *ll, ast_node_t *n)
{
    switch (n->op) {
    case TK_INT: {
        const char *ts = n->type_str ? n->type_str : "i32";
        return (llv_t){ll_fmt(ll, "%lld", (long long)n->value.as.i), ts};
    }
    case TK_FLOAT: {
        char buf[64];
        sal_snprintf(buf, sizeof buf, "%.17g", n->value.as.f);
        return (llv_t){ll_fp_text(ll, buf), n->type_str ? n->type_str : "f64"};
    }
    case TK_STRING:
    case TK_TRIPLE_STRING:
    case TK_RAW_STRING:
    case TK_UTF8_CHAR:
        return (llv_t){ll_strconst(ll, n->value.as.s ? n->value.as.s : ""), "str"};
    case TK_CHAR:
        return (llv_t){ll_fmt(ll, "%lld", (long long)n->value.as.i), "char"};
    case TK_KW_TRUE:
        return (llv_t){"true", "bool"};
    case TK_KW_FALSE:
        return (llv_t){"false", "bool"};
    case TK_KW_NULL:
        return (llv_t){"null", n->type_str ? n->type_str : "i32*"};
    default:
        return (llv_t){"0", "i32"};
    }
}

static const char *ll_match_pat_eq(ll_t *ll, llv_t subj, ast_node_t *pat_head)
{
    llv_t pv = ll_expr(ll, pat_head);
    if (ll_is_str(subj.ts)) {
        const char *c = ll_new_tmp(ll);
        const char *r = ll_new_tmp(ll);
        ll_emit(ll, "%s = call i32 @strcmp(ptr %s, ptr %s)", c, subj.ref, pv.ref);
        ll_emit(ll, "%s = icmp eq i32 %s, 0", r, c);
        return r;
    }
    {
        const char *conv = ll_conv(ll, pv, subj.ts);
        const char *r = ll_new_tmp(ll);
        if (ll_is_float(subj.ts))
            ll_emit(ll, "%s = fcmp oeq %s %s, %s", r, ll_ty(ll, subj.ts), subj.ref, conv);
        else
            ll_emit(ll, "%s = icmp eq %s %s, %s", r, ll_ty(ll, subj.ts), subj.ref, conv);
        return r;
    }
}

static const char *ll_match_variant_tag_cond(ll_t *ll, llv_t subj, int tag)
{
    const char *t = ll_new_tmp(ll);
    const char *r = ll_new_tmp(ll);
    ll_emit(ll, "%s = extractvalue %s %s, 0", t, ll_ty(ll, subj.ts), subj.ref);
    ll_emit(ll, "%s = icmp eq i32 %s, %d", r, t, tag);
    return r;
}

const char *ll_match_arm_cond(ll_t *ll, ast_node_t *arm, llv_t subj, bool is_variant)
{
    if (arm->op == TK_KW_ELSE) return "true";
    {
        const char *acc = NULL;
        size_t i = 0;
        for (; i < arm->list.len; i++) {
            ast_node_t *pat = (ast_node_t *)arm->list.data[i];
            const char *c =
                is_variant ? ll_match_variant_tag_cond(ll, subj, (int)pat->value.as.i)
                           : ll_match_pat_eq(ll, subj, pat->a);
            if (!acc) {
                acc = c;
                continue;
            }
            {
                const char *r = ll_new_tmp(ll);
                ll_emit(ll, "%s = or i1 %s, %s", r, acc, c);
                acc = r;
            }
        }
        return acc ? acc : "false";
    }
}

static llv_t ll_match_expr(ll_t *ll, ast_node_t *n)
{
    llv_t subj = ll_expr(ll, n->a);
    bool is_variant = subj.ts && !strncmp(subj.ts, "Variant<", 8);
    const char *rts = n->type_str ? n->type_str : "i32";
    const char *resptr = ll_fmt(ll, "%%mres.%d", ll->tmp++);
    const char *endL = ll_new_lbl(ll, "mend");
    const char *fallbackL = ll_new_lbl(ll, "mtrap");
    const char *saved_ptr = ll->match_result_ptr;
    const char *saved_ts = ll->match_result_ts;
    const char *saved_merge = ll->match_merge_block;
    llv_t out;
    ll_emit_alloca(ll, "%s = alloca %s", resptr, ll_ty(ll, rts));
    ll->match_result_ptr = resptr;
    ll->match_result_ts = rts;
    ll->match_merge_block = endL;
    {
        size_t i = 0;
        for (; i < n->list.len; i++) {
            ast_node_t *arm = (ast_node_t *)n->list.data[i];
            const char *cond = ll_match_arm_cond(ll, arm, subj, is_variant);
            const char *bodyL = ll_new_lbl(ll, "marm");
            const char *nextL =
                (i + 1 < n->list.len) ? ll_new_lbl(ll, "mnext") : fallbackL;
            ll_emit_term(ll, "br i1 %s, label %%%s, label %%%s", cond, bodyL, nextL);
            ll_emit_label(ll, bodyL);
            ll_stmt(ll, arm->b);
            ll_emit_term(ll, "br label %%%s", endL);
            ll_emit_label(ll, nextL);
        }
    }
    if (n->list.len == 0) {
        ll_emit_term(ll, "br label %%%s", fallbackL);
        ll_emit_label(ll, fallbackL);
    }
    ll_emit(ll, "call void @llvm.trap()");
    ll_emit_term(ll, "unreachable");
    ll_emit_label(ll, endL);
    ll->match_result_ptr = saved_ptr;
    ll->match_result_ts = saved_ts;
    ll->match_merge_block = saved_merge;
    {
        const char *r = ll_new_tmp(ll);
        ll_emit(ll, "%s = load %s, ptr %s", r, ll_ty(ll, rts), resptr);
        out = (llv_t){r, rts};
    }
    return out;
}

llv_t ll_expr(ll_t *ll, ast_node_t *n)
{
    if (!n) return (llv_t){"0", "i32"};
    switch (n->kind) {
    case AST_LITERAL:
        return ll_literal(ll, n);
    case AST_IDENTIFIER: {
        ll_addr_t a = ll_addr_of(ll, n);
        const char *r = ll_new_tmp(ll);
        ll_emit(ll, "%s = load %s, ptr %s", r, ll_ty(ll, a.ts), a.ptr);
        return (llv_t){r, a.ts};
    }
    case AST_FUNC_ADDR: {
        symbol_t *fsym = ll_sym(ll, n->name);
        if (!fsym || fsym->kind != SYM_FUNC || fsym->overloads.len != 1) {
            ll_error(ll, n, "cannot take the address of '%s'", n->name);
            return ll_poison(n->type_str ? n->type_str : "ptr");
        }
        func_sig_t *sig = (func_sig_t *)fsym->overloads.data[0];
        bool is_ext = sig->decl && sig->decl->is_extern;
        if (!is_ext) ll_ensure_fn(ll, sig->decl, NULL, ll->pkg_scope);
        const char *fname = is_ext ? n->name : ll_mangle(ll, NULL, n->name, sig);
        return (llv_t){ll_fmt(ll, "@%s", fname), "void*"};
    }
    case AST_THIS: {
        if (!ll->this_ref) {
            ll_error(ll, n, "'this' outside a method");
            return ll_poison(n->type_str);
        }
        if (ll->self_byval) {
            const char *r = ll_new_tmp(ll);
            ll_emit(ll, "%s = load %s, ptr %s", r, ll_ty(ll, ll->self_ts), ll->this_ref);
            return (llv_t){r, ll->self_ts};
        }
        return (llv_t){ll->this_ref, ll_fmt(ll, "%s*", ll->self_ts)};
    }
    case AST_BINARY:
        return ll_binary(ll, n);
    case AST_MATCH:
        return ll_match_expr(ll, n);
    case AST_VARIANT_BOX: {
        llv_t v = ll_expr(ll, n->a);
        return (llv_t){ll_box_variant(ll, v, (int)n->value.as.i, n->type_str),
                       n->type_str};
    }
    case AST_VARIANT_UNWRAP: {
        llv_t v = ll_expr(ll, n->a);
        return ll_unwrap_variant(ll, v, n->type_str ? n->type_str : "i32");
    }
    case AST_TERNARY: {
        const char *rt = n->type_str ? n->type_str : "i32";
        const char *cond = ll_as_i1(ll, ll_expr(ll, n->a));
        const char *thenL = ll_new_lbl(ll, "tern_then");
        const char *elseL = ll_new_lbl(ll, "tern_else");
        const char *tjoinL = ll_new_lbl(ll, "tern_tjoin");
        const char *fjoinL = ll_new_lbl(ll, "tern_fjoin");
        const char *endL = ll_new_lbl(ll, "tern_end");
        ll_emit_term(ll, "br i1 %s, label %%%s, label %%%s", cond, thenL, elseL);
        ll_emit_label(ll, thenL);
        const char *tv = ll_conv(ll, ll_expr(ll, n->b), rt);
        ll_emit_term(ll, "br label %%%s", tjoinL);
        ll_emit_label(ll, tjoinL);
        ll_emit_term(ll, "br label %%%s", endL);
        ll_emit_label(ll, elseL);
        const char *fv = ll_conv(ll, ll_expr(ll, n->c), rt);
        ll_emit_term(ll, "br label %%%s", fjoinL);
        ll_emit_label(ll, fjoinL);
        ll_emit_term(ll, "br label %%%s", endL);
        ll_emit_label(ll, endL);
        const char *r = ll_new_tmp(ll);
        ll_emit(ll, "%s = phi %s [ %s, %%%s ], [ %s, %%%s ]", r, ll_ty(ll, rt), tv,
                tjoinL, fv, fjoinL);
        return (llv_t){r, rt};
    }
    case AST_UNARY:
        return ll_unary(ll, n);
    case AST_INCDEC:
        return ll_incdec(ll, n);
    case AST_CAST: {
        llv_t v = ll_expr(ll, n->a);
        const char *to = n->type && n->type->type_str
                             ? n->type->type_str
                             : (n->type_str ? n->type_str : v.ts);
        return (llv_t){ll_conv(ll, v, to), to};
    }
    case AST_CALL:
        return ll_call(ll, n);
    case AST_MEMBER: {
        if (n->a && n->a->kind == AST_IDENTIFIER) {
            symbol_t *e = ll_enum_sym(ll, n->a->name);
            if (e) {
                symbol_t *m = scope_lookup_local(e->members, n->name);
                if (m && m->kind == SYM_ENUM_MEMBER)
                    return (llv_t){ll_fmt(ll, "%lld", (long long)m->enum_value), "i32"};
            }
            {
                llv_t pv;
                symbol_t *pk = ll_sym(ll, n->a->name);
                if (pk && pk->kind == SYM_PACKAGE && ll_pkg_value(ll, n, pk, &pv))
                    return pv;
            }
        }
        return ll_load_addr(ll, n);
    }
    case AST_INDEX: {
        if (n->a && n->a->type_str) {
            const char *sname;
            symbol_t *oss = ll_op_struct(ll, n->a->type_str, &sname);
            llv_t out;
            if (oss && ll_op_call(ll, n->a, sname, oss, "operator_index", n->b, &out))
                return out;
        }
        return ll_load_addr(ll, n);
    }
    case AST_STRUCT_LIT:
        return ll_struct_lit(ll, n);
    case AST_ARRAY_LIT:
        return ll_array_lit(ll, n);
    case AST_SLICE:
        return ll_slice_expr(ll, n);
    case AST_LAMBDA:
        return ll_lambda_value(ll, n);
    case AST_ASSIGN:
        /*
         * Assignment used as an expression, which is how `a = b = c`
         * parses: the inner `b = c` is the right operand of the outer
         * assignment. Perform the store, then yield the value now in the
         * target - reloading rather than reusing the stored value so a
         * narrowing target ("a: i8 = 300") produces what a subsequent read
         * would, which is also what the C backend's `(a = b)` gives.
         */
        ll_assign(ll, n);
        {
            ll_addr_t a = ll_addr_of(ll, n->a);
            const char *r = ll_new_tmp(ll);
            ll_emit(ll, "%s = load %s, ptr %s", r, ll_ty(ll, a.ts), a.ptr);
            return (llv_t){r, a.ts};
        }
    default:
        ll_error(ll, n, "%s expression", ast_kind_name(n->kind));
        return ll_poison(n->type_str);
    }
}
