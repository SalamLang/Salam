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

static void ll_emit_defers(ll_t *ll)
{
    {
        size_t i = ll->defers.len;
        for (; i > 0; i--)
            ll_stmt(ll, (ast_node_t *)ll->defers.data[i - 1]);
    }
}

void ll_emit_return(ll_t *ll, ast_node_t *value)
{
    /* The return expression must be fully evaluated (SSA value materialized)
     * before deferred statements run, since a defer may free/mutate a
     * variable the return expression reads (e.g. `defer v.free() ... ret
     * f(v)`). Emitting defers first would run that cleanup before the
     * value is even computed. */
    if (ll->match_result_ptr) {
        if (value) {
            const char *v = ll_conv(ll, ll_expr(ll, value), ll->match_result_ts);
            ll_emit(ll, "store %s %s, ptr %s", ll_ty(ll, ll->match_result_ts), v,
                    ll->match_result_ptr);
        }
        ll_emit_term(ll, "br label %%%s", ll->match_merge_block);
        return;
    }
    if (ll->is_main) {
        const char *v = value ? ll_conv(ll, ll_expr(ll, value), "i32") : NULL;
        ll_emit_defers(ll);
        if (value)
            ll_emit_term(ll, "ret i32 %s", v);
        else
            ll_emit_term(ll, "ret i32 0");
        return;
    }
    if (!ll->ret_ts || !strcmp(ll->ret_ts, "void")) {
        ll_emit_defers(ll);
        ll_emit_term(ll, "ret void");
        return;
    }
    const char *v = ll_conv(ll, ll_expr(ll, value), ll->ret_ts);
    ll_emit_defers(ll);
    ll_emit_term(ll, "ret %s %s", ll_ty(ll, ll->ret_ts), v);
}

static void ll_vardecl(ll_t *ll, ast_node_t *n)
{
    const char *ts = n->type_str ? n->type_str : "i32";
    const char *ptr = ll_fmt(ll, "%%v.%s.%d", ll_safe_name(ll, n->name), ll->tmp++);
    ll_emit_alloca(ll, "%s = alloca %s", ptr, ll_ty(ll, ts));
    ll_local_add(ll, n->name, ptr, ts);
    if (n->a) {
        const char *v = ll_conv(ll, ll_expr(ll, n->a), ts);
        ll_emit(ll, "store %s %s, ptr %s", ll_ty(ll, ts), v, ptr);
    }
}

static token_kind_t ll_compound_base(token_kind_t k)
{
    switch (k) {
    case TK_PLUS_EQ:
        return TK_PLUS;
    case TK_MINUS_EQ:
        return TK_MINUS;
    case TK_STAR_EQ:
        return TK_STAR;
    case TK_SLASH_EQ:
        return TK_SLASH;
    case TK_PERCENT_EQ:
        return TK_PERCENT;
    case TK_POWER_EQ:
        return TK_POWER;
    default:
        return TK_EOF;
    }
}

static void ll_assign(ll_t *ll, ast_node_t *n)
{
    ast_node_t *tgt = n->a;
    if (!tgt || (tgt->kind != AST_IDENTIFIER && tgt->kind != AST_MEMBER &&
                 tgt->kind != AST_INDEX)) {
        ll_error(ll, n, "assignment to a non-lvalue");
        return;
    }

    if (n->op == TK_ASSIGN && tgt->kind == AST_INDEX && ll_index_set(ll, tgt, n->b))
        return;
    ll_addr_t a = ll_addr_of(ll, tgt);
    const char *ts = a.ts;
    const char *rhs;
    if (n->op == TK_ASSIGN) {
        rhs = ll_conv(ll, ll_expr(ll, n->b), ts);
    } else {
        token_kind_t base = ll_compound_base(n->op);
        if (base == TK_EOF) {
            ll_error(ll, n, "compound assignment operator");
            return;
        }

        ast_node_t bin = *n;
        bin.kind = AST_BINARY;
        bin.op = base;
        bin.a = tgt;
        bin.b = n->b;
        bin.type_str = ts;
        rhs = ll_conv(ll, ll_binary(ll, &bin), ts);
    }
    ll_emit(ll, "store %s %s, ptr %s", ll_ty(ll, ts), rhs, a.ptr);
}

static void ll_stmts_scoped(ll_t *ll, ast_node_t *block)
{
    size_t mark = ll->locals.len;
    {
        size_t i = 0;
        for (; i < block->list.len; i++)
            ll_stmt(ll, (ast_node_t *)block->list.data[i]);
    }
    ll->locals.len = mark;
}

static void ll_if(ll_t *ll, ast_node_t *n)
{
    const char *cond = ll_as_i1(ll, ll_expr(ll, n->a));
    const char *thenL = ll_new_lbl(ll, "then");
    const char *endL = ll_new_lbl(ll, "endif");
    const char *elseL = n->c ? ll_new_lbl(ll, "else") : endL;
    ll_emit_term(ll, "br i1 %s, label %%%s, label %%%s", cond, thenL, elseL);
    ll_emit_label(ll, thenL);
    ll_stmts_scoped(ll, n->b);
    ll_emit_term(ll, "br label %%%s", endL);
    if (n->c) {
        ll_emit_label(ll, elseL);
        if (n->c->kind == AST_IF)
            ll_stmt(ll, n->c);
        else
            ll_stmts_scoped(ll, n->c);
        ll_emit_term(ll, "br label %%%s", endL);
    }
    ll_emit_label(ll, endL);
}

static void ll_match_stmt(ll_t *ll, ast_node_t *n)
{
    llv_t subj;
    bool is_variant;
    const char *endL;
    if (n->list.len == 0) {
        ll_expr(ll, n->a);
        return;
    }
    subj = ll_expr(ll, n->a);
    is_variant = subj.ts && !strncmp(subj.ts, "Variant<", 8);
    endL = ll_new_lbl(ll, "mend");
    {
        size_t i = 0;
        for (; i < n->list.len; i++) {
            ast_node_t *arm = (ast_node_t *)n->list.data[i];
            const char *cond = ll_match_arm_cond(ll, arm, subj, is_variant);
            const char *bodyL = ll_new_lbl(ll, "marm");
            const char *nextL = (i + 1 < n->list.len) ? ll_new_lbl(ll, "mnext") : endL;
            ll_emit_term(ll, "br i1 %s, label %%%s, label %%%s", cond, bodyL, nextL);
            ll_emit_label(ll, bodyL);
            ll_stmts_scoped(ll, arm->b);
            ll_emit_term(ll, "br label %%%s", endL);
            ll_emit_label(ll, nextL);
        }
    }
}

static void ll_while(ll_t *ll, ast_node_t *n)
{
    const char *condL = ll_new_lbl(ll, "wcond");
    const char *bodyL = ll_new_lbl(ll, "wbody");
    const char *endL = ll_new_lbl(ll, "wend");
    ll_emit_term(ll, "br label %%%s", condL);
    ll_emit_label(ll, condL);
    ll_emit_term(ll, "br i1 %s, label %%%s, label %%%s", ll_as_i1(ll, ll_expr(ll, n->a)),
                 bodyL, endL);
    ll_emit_label(ll, bodyL);
    if (ll->nloop >= 64) {
        ll_error(ll, n, "loop nesting too deep");
        return;
    }
    ll->brk[ll->nloop] = endL;
    ll->cont[ll->nloop] = condL;
    ll->nloop++;
    ll_stmts_scoped(ll, n->b);
    ll->nloop--;
    ll_emit_term(ll, "br label %%%s", condL);
    ll_emit_label(ll, endL);
}

static void ll_for(ll_t *ll, ast_node_t *n)
{
    size_t mark = ll->locals.len;
    if (n->a) ll_stmt(ll, n->a);
    const char *condL = ll_new_lbl(ll, "fcond");
    const char *bodyL = ll_new_lbl(ll, "fbody");
    const char *stepL = ll_new_lbl(ll, "fstep");
    const char *endL = ll_new_lbl(ll, "fend");
    ll_emit_term(ll, "br label %%%s", condL);
    ll_emit_label(ll, condL);
    if (n->b)
        ll_emit_term(ll, "br i1 %s, label %%%s, label %%%s",
                     ll_as_i1(ll, ll_expr(ll, n->b)), bodyL, endL);
    else
        ll_emit_term(ll, "br label %%%s", bodyL);
    ll_emit_label(ll, bodyL);
    if (ll->nloop >= 64) {
        ll_error(ll, n, "loop nesting too deep");
        ll->locals.len = mark;
        return;
    }
    ll->brk[ll->nloop] = endL;
    ll->cont[ll->nloop] = stepL;
    ll->nloop++;
    ll_stmts_scoped(ll, n->d);
    ll->nloop--;
    ll_emit_term(ll, "br label %%%s", stepL);
    ll_emit_label(ll, stepL);
    if (n->c) ll_stmt(ll, n->c);
    ll_emit_term(ll, "br label %%%s", condL);
    ll_emit_label(ll, endL);
    ll->locals.len = mark;
}

static bool ll_const_i64(ast_node_t *n, long long *out)
{
    if (!n || n->kind != AST_LITERAL || n->op != TK_INT) return false;
    *out = (long long)n->value.as.i;
    return true;
}

static void ll_repeat(ll_t *ll, ast_node_t *n)
{
    const char *initv, *boundv, *stepv;
    const char *dir = NULL;
    bool static_dir = false, static_up = false;
    if (n->c) {
        long long ci = 0, cb = 0, cs = 0;
        if (ll_const_i64(n->a, &ci) && ll_const_i64(n->c, &cb)) {
            static_dir = true;
            static_up = ci <= cb;
            initv = ll_fmt(ll, "%lld", ci);
            boundv = ll_fmt(ll, "%lld", cb);
            if (!n->d) {
                stepv = static_up ? "1" : "-1";
            } else if (ll_const_i64(n->d, &cs)) {
                stepv = ll_fmt(ll, "%lld", static_up ? cs : -cs);
            } else {
                const char *raw = ll_conv(ll, ll_expr(ll, n->d), "i64");
                if (static_up) {
                    stepv = raw;
                } else {
                    stepv = ll_new_tmp(ll);
                    ll_emit(ll, "%s = sub i64 0, %s", stepv, raw);
                }
            }
        } else {
            initv = ll_conv(ll, ll_expr(ll, n->a), "i64");
            boundv = ll_conv(ll, ll_expr(ll, n->c), "i64");
            const char *raw = n->d ? ll_conv(ll, ll_expr(ll, n->d), "i64") : "1";
            const char *neg = ll_new_tmp(ll);
            dir = ll_new_tmp(ll);
            ll_emit(ll, "%s = icmp sle i64 %s, %s", dir, initv, boundv);
            ll_emit(ll, "%s = sub i64 0, %s", neg, raw);
            stepv = ll_new_tmp(ll);
            ll_emit(ll, "%s = select i1 %s, i64 %s, i64 %s", stepv, dir, raw, neg);
        }
    } else {
        initv = "0";
        boundv = ll_conv(ll, ll_expr(ll, n->a), "i64");
        stepv = "1";
    }
    const char *ctr = ll_new_tmp(ll);
    ll_emit_alloca(ll, "%s = alloca i64", ctr);
    ll_emit(ll, "store i64 %s, ptr %s", initv, ctr);
    size_t mark = ll->locals.len;
    const char *bindp = NULL;
    bool iv_is64 = n->type_str && !strcmp(n->type_str, "i64");
    const char *ivty = iv_is64 ? "i64" : "i32";
    if (n->name) {
        bindp = ll_fmt(ll, "%%v.%s.%d", ll_safe_name(ll, n->name), ll->tmp++);
        ll_emit_alloca(ll, "%s = alloca %s", bindp, ivty);
        ll_local_add(ll, n->name, bindp, ivty);
    }
    const char *condL = ll_new_lbl(ll, "rcond");
    const char *bodyL = ll_new_lbl(ll, "rbody");
    const char *stepL = ll_new_lbl(ll, "rstep");
    const char *endL = ll_new_lbl(ll, "rend");
    ll_emit_term(ll, "br label %%%s", condL);
    ll_emit_label(ll, condL);
    const char *cv = ll_new_tmp(ll);
    ll_emit(ll, "%s = load i64, ptr %s", cv, ctr);
    const char *cmp;
    if (n->c && static_dir) {
        cmp = ll_new_tmp(ll);
        ll_emit(ll, "%s = icmp %s i64 %s, %s", cmp, static_up ? "sle" : "sge", cv,
                boundv);
    } else if (n->c) {
        const char *le = ll_new_tmp(ll);
        const char *ge = ll_new_tmp(ll);
        ll_emit(ll, "%s = icmp sle i64 %s, %s", le, cv, boundv);
        ll_emit(ll, "%s = icmp sge i64 %s, %s", ge, cv, boundv);
        cmp = ll_new_tmp(ll);
        ll_emit(ll, "%s = select i1 %s, i1 %s, i1 %s", cmp, dir, le, ge);
    } else {
        cmp = ll_new_tmp(ll);
        ll_emit(ll, "%s = icmp slt i64 %s, %s", cmp, cv, boundv);
    }
    ll_emit_term(ll, "br i1 %s, label %%%s, label %%%s", cmp, bodyL, endL);
    ll_emit_label(ll, bodyL);
    if (bindp) {
        if (iv_is64) {
            ll_emit(ll, "store i64 %s, ptr %s", cv, bindp);
        } else {
            const char *cvt = ll_new_tmp(ll);
            ll_emit(ll, "%s = trunc i64 %s to i32", cvt, cv);
            ll_emit(ll, "store i32 %s, ptr %s", cvt, bindp);
        }
    }
    if (ll->nloop >= 64) {
        ll_error(ll, n, "loop nesting too deep");
        ll->locals.len = mark;
        return;
    }
    ll->brk[ll->nloop] = endL;
    ll->cont[ll->nloop] = stepL;
    ll->nloop++;
    ll_stmts_scoped(ll, n->b);
    ll->nloop--;
    ll_emit_term(ll, "br label %%%s", stepL);
    ll_emit_label(ll, stepL);
    const char *cv2 = ll_new_tmp(ll);
    ll_emit(ll, "%s = load i64, ptr %s", cv2, ctr);
    const char *inc = ll_new_tmp(ll);
    ll_emit(ll, "%s = add i64 %s, %s", inc, cv2, stepv);
    ll_emit(ll, "store i64 %s, ptr %s", inc, ctr);
    ll_emit_term(ll, "br label %%%s", condL);
    ll_emit_label(ll, endL);
    ll->locals.len = mark;
}

void ll_stmt(ll_t *ll, ast_node_t *n)
{
    if (!n || ll->term) return;

    if (ll->debug && n->span.begin.line)
        ll->cur_dbg = ll_debug_location(ll, n->span.begin.line, n->span.begin.col);
    switch (n->kind) {
    case AST_BLOCK:
        ll_block(ll, n);
        break;
    case AST_VAR_DECL:
        ll_vardecl(ll, n);
        break;
    case AST_CONST_DECL:
        ll_vardecl(ll, n);
        break;
    case AST_ASSIGN:
        ll_assign(ll, n);
        break;
    case AST_EXPR_STMT:
        ll_expr(ll, n->a);
        break;
    case AST_INCDEC:
        ll_expr(ll, n);
        break;
    case AST_RETURN:
        ll_emit_return(ll, n->a);
        break;
    case AST_IF:
        ll_if(ll, n);
        break;
    case AST_MATCH:
        ll_match_stmt(ll, n);
        break;
    case AST_UNTIL:
        ll_while(ll, n);
        break;
    case AST_FOR:
        ll_for(ll, n);
        break;
    case AST_REPEAT:
        ll_repeat(ll, n);
        break;
    case AST_DEFER:
        vec_push(ll->a, &ll->defers, n->a);
        break;
    case AST_BREAK:
        if (ll->nloop) ll_emit_term(ll, "br label %%%s", ll->brk[ll->nloop - 1]);
        break;
    case AST_CONTINUE:
        if (ll->nloop) ll_emit_term(ll, "br label %%%s", ll->cont[ll->nloop - 1]);
        break;
    default:
        ll_error(ll, n, "%s statement", ast_kind_name(n->kind));
        break;
    }
}

void ll_block(ll_t *ll, ast_node_t *block)
{
    ll_stmts_scoped(ll, block);
}

void ll_block_top(ll_t *ll, ast_node_t *block)
{
    size_t i = 0;
    for (; i < block->list.len; i++)
        ll_stmt(ll, (ast_node_t *)block->list.data[i]);
}
