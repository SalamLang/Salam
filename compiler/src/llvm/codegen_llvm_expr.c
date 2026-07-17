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

#include "llvm/codegen_llvm_internal.h"
#include "core/sal_format.h"
#include "codegen/print_fmt.h"

static const char *ll_arith_op(token_kind_t k, bool isflt, bool issigned)
{
    switch (k) {
    case TK_PLUS:
        return isflt ? "fadd" : (issigned ? "add nsw" : "add");
    case TK_MINUS:
        return isflt ? "fsub" : (issigned ? "sub nsw" : "sub");
    case TK_STAR:
        return isflt ? "fmul" : (issigned ? "mul nsw" : "mul");
    case TK_SLASH:
        return isflt ? "fdiv" : (issigned ? "sdiv" : "udiv");
    case TK_PERCENT:
        return isflt ? "frem" : (issigned ? "srem" : "urem");
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
        return isflt ? "one" : "ne";
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

static symbol_t *ll_op_struct(ll_t *ll, const char *ts, const char **sname)
{
    if (!ts) return NULL;
    *sname = ll_is_ptr_ts(ts) ? arena_strndup(ll->a, ts, strlen(ts) - 1) : ts;
    return ll_struct_sym(ll, *sname);
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
        const char *common = ll_common(L.ts, R.ts);
        const char *lc = ll_conv(ll, L, common), *rc = ll_conv(ll, R, common);
        bool flt = ll_is_float(common);
        const char *r = ll_new_tmp(ll);
        ll_emit(ll, "%s = %s %s %s %s, %s", r, flt ? "fcmp" : "icmp",
                ll_cmp_pred(op, flt, ll_is_signed(common)), ll_ty(ll, common), lc, rc);
        return (llv_t){r, "bool"};
    }

    const char *rt = n->type_str ? n->type_str : ll_common(L.ts, R.ts);
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
            ll_emit(ll, "%s = sub%s %s 0, %s", r, ll_is_signed(rt) ? " nsw" : "",
                    ll_ty(ll, rt), cv);
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
            default:
                break;
            }
        }
    }
    const char *f = ll_strconst(ll, sb_cstr(&fmt));
    const char *t = ll_new_tmp(ll);
    if (err)
        ll_emit(ll, "%s = call i32 (i32, ptr, ...) @dprintf(i32 2, ptr %s%s)", t, f,
                sb_cstr(&args));
    else
        ll_emit(ll, "%s = call i32 (ptr, ...) @printf(ptr %s%s)", t, f, sb_cstr(&args));
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
    symbol_t *fsym = ll_sym(ll, nm);
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

static llv_t ll_call_len(ll_t *ll, ast_node_t *n)
{
    ast_node_t *arg = (ast_node_t *)n->list.data[0];
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
    llv_t v = ll_expr(ll, arg);
    const char *l = ll_new_tmp(ll);
    ll_emit(ll, "%s = call %s @strlen(ptr %s)", l, ll->usize, v.ref);
    return (llv_t){ll_usize_to_i32(ll, l), "i32"};
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

static llv_t ll_call_pkg(ll_t *ll, ast_node_t *n, symbol_t *pk, const char *fname_)
{
    symbol_t *fs = scope_lookup_local(pk->members, fname_);
    if (!fs || fs->kind != SYM_FUNC || fs->overloads.len == 0) {
        ll_error(ll, n, "package function '%s' not found", fname_);
        return ll_poison(n->type_str);
    }
    func_sig_t *sig = ll_pick_overload(ll, fs, n);
    ll_ensure_fn(ll, sig->decl, NULL, pk->members);
    bool is_ext = sig->decl && sig->decl->is_extern;
    const char *fname = is_ext ? fname_ : ll_mangle(ll, NULL, fname_, sig);
    return ll_emit_call(ll, n, sig, "", fname, type_to_string(ll->sem->tc, sig->ret));
}

static bool ll_call_str(ll_t *ll, ast_node_t *n, ast_node_t *obj, const char *m,
                        llv_t *out)
{
    size_t na = n->list.len;
    const char *recv = ll_expr(ll, obj).ref;
    const char *r;
    if (!strcmp(m, "len") || !strcmp(m, "length")) {
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
    if (!strcmp(m, "find") && na == 1) {
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

    if (!strncmp(ots, "dyn ", 4)) return ll_call_dyn(ll, n, obj, ots + 4, mname);

    if (!strcmp(ots, "str")) {
        llv_t o;
        if (ll_call_str(ll, n, obj, mname, &o)) return o;
    }

    if (obj->kind == AST_IDENTIFIER) {
        symbol_t *pk = ll_sym(ll, obj->name);
        if (pk && pk->kind == SYM_PACKAGE) return ll_call_pkg(ll, n, pk, mname);
    }

    bool isptr = ll_is_ptr_ts(ots);
    const char *sname = isptr ? arena_strndup(ll->a, ots, strlen(ots) - 1) : ots;
    symbol_t *ss = ll_struct_sym(ll, sname);
    symbol_t *ms = ss ? scope_lookup_local(ss->members, mname) : NULL;
    if (ms && ms->kind == SYM_METHOD && ms->overloads.len) {
        func_sig_t *sig = ll_pick_overload(ll, ms, n);
        ll_ensure_fn(ll, sig->decl, ss, ll->pkg_scope);
        const char *recv = isptr ? ll_expr(ll, obj).ref : ll_addr_of(ll, obj).ptr;
        return ll_emit_call(ll, n, sig, ll_fmt(ll, "ptr %s", recv),
                            ll_mangle(ll, sname, mname, sig),
                            type_to_string(ll->sem->tc, sig->ret));
    }

    symbol_t *impl = ll_sym(ll, impl_owner_key(ll->a, ots));
    if (impl && impl->kind == SYM_TYPEIMPL) {
        symbol_t *im = scope_lookup_local(impl->members, mname);
        if (im && im->kind == SYM_METHOD && im->overloads.len) {
            func_sig_t *sig = ll_pick_overload(ll, im, n);
            ll_ensure_fn(ll, sig->decl, impl, ll->pkg_scope);
            llv_t rv = ll_expr(ll, obj);
            return ll_emit_call(ll, n, sig, ll_fmt(ll, "%s %s", ll_ty(ll, ots), rv.ref),
                                ll_mangle_ti(ll, ots, mname, sig),
                                type_to_string(ll->sem->tc, sig->ret));
        }
    }

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
    return false;
}

static llv_t ll_call(ll_t *ll, ast_node_t *n)
{
    ast_node_t *callee = n->a;
    if (callee && callee->kind == AST_MEMBER) return ll_call_method(ll, n, callee);
    if (callee && callee->kind == AST_IDENTIFIER) {
        const char *nm = callee->name;
        if (!strcmp(nm, "print") || !strcmp(nm, "_")) {
            ll_lower_print(ll, n, false, 0);
            return (llv_t){"0", "void"};
        }
        if (!strcmp(nm, "println") || !strcmp(nm, "__")) {
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
            ll_emit(ll, "%s = ptrtoint ptr %s to i64", sz, szp);
            return (llv_t){sz, "u64"};
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
        if (!strpbrk(buf, ".eEnN"))
            sal_snprintf(buf, sizeof buf, "%.17g.0", n->value.as.f);
        return (llv_t){arena_strdup(ll->a, buf), n->type_str ? n->type_str : "f64"};
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
    default:
        ll_error(ll, n, "%s expression", ast_kind_name(n->kind));
        return ll_poison(n->type_str);
    }
}
