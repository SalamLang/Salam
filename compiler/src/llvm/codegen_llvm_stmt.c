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
    ll_emit_defers(ll);
    if (ll->is_main) {
        if (value)
            ll_emit_term(ll, "ret i32 %s", ll_conv(ll, ll_expr(ll, value), "i32"));
        else
            ll_emit_term(ll, "ret i32 0");
        return;
    }
    if (!ll->ret_ts || !strcmp(ll->ret_ts, "void")) {
        ll_emit_term(ll, "ret void");
        return;
    }
    const char *v = ll_conv(ll, ll_expr(ll, value), ll->ret_ts);
    ll_emit_term(ll, "ret %s %s", ll_ty(ll, ll->ret_ts), v);
}

static void ll_vardecl(ll_t *ll, ast_node_t *n)
{
    const char *ts = n->type_str ? n->type_str : "i32";
    const char *ptr = ll_fmt(ll, "%%v.%s.%d", n->name, ll->tmp++);
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

static void ll_repeat(ll_t *ll, ast_node_t *n)
{
    const char *initv, *boundv, *stepv;
    const char *cmpop;
    if (n->c) {
        initv = ll_conv(ll, ll_expr(ll, n->a), "i64");
        boundv = ll_conv(ll, ll_expr(ll, n->c), "i64");
        stepv = n->d ? ll_conv(ll, ll_expr(ll, n->d), "i64") : "1";
        cmpop = "sle";
    } else {
        initv = "0";
        boundv = ll_conv(ll, ll_expr(ll, n->a), "i64");
        stepv = "1";
        cmpop = "slt";
    }
    const char *ctr = ll_new_tmp(ll);
    ll_emit_alloca(ll, "%s = alloca i64", ctr);
    ll_emit(ll, "store i64 %s, ptr %s", initv, ctr);
    const char *condL = ll_new_lbl(ll, "rcond");
    const char *bodyL = ll_new_lbl(ll, "rbody");
    const char *stepL = ll_new_lbl(ll, "rstep");
    const char *endL = ll_new_lbl(ll, "rend");
    ll_emit_term(ll, "br label %%%s", condL);
    ll_emit_label(ll, condL);
    const char *cv = ll_new_tmp(ll);
    ll_emit(ll, "%s = load i64, ptr %s", cv, ctr);
    const char *cmp = ll_new_tmp(ll);
    ll_emit(ll, "%s = icmp %s i64 %s, %s", cmp, cmpop, cv, boundv);
    ll_emit_term(ll, "br i1 %s, label %%%s, label %%%s", cmp, bodyL, endL);
    ll_emit_label(ll, bodyL);
    if (ll->nloop >= 64) {
        ll_error(ll, n, "loop nesting too deep");
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
    case AST_RETURN:
        ll_emit_return(ll, n->a);
        break;
    case AST_IF:
        ll_if(ll, n);
        break;
    case AST_WHILE:
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
