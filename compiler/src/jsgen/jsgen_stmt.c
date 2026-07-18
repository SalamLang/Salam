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

#include "jsgen/jsgen_internal.h"

void jsg_emit_defers(jg_t *g)
{
    {
        size_t i = g->cg.fn_defers.len;
        for (; i > 0; i--)
            jsg_stmt(g, (ast_node_t *)g->cg.fn_defers.data[i - 1]);
    }
}

static const char *jsg_vardecl_inline(jg_t *g, ast_node_t *n)
{
    cg_t *cg = &g->cg;
    const char *kw = n->is_mut ? "let" : "const";
    const char *init = n->a ? jsg_expr(g, n->a) : jsg_zero(g, n->type_str);
    const char *nm = jsg_local(g, n->name, n->is_mut);
    return cg_fmt(cg, "%s %s = %s", kw, nm, init);
}

static const char *jsg_compound(jg_t *g, ast_node_t *n)
{
    cg_t *cg = &g->cg;
    ast_node_t *rhs = n->b;
    if (n->op != TK_ASSIGN || !n->a || n->a->kind != AST_IDENTIFIER) return NULL;
    if (!rhs || rhs->kind != AST_BINARY) return NULL;
    if (rhs->op != TK_PLUS && rhs->op != TK_MINUS && rhs->op != TK_STAR) return NULL;
    if (!rhs->a || rhs->a->kind != AST_IDENTIFIER || !rhs->a->name ||
        strcmp(rhs->a->name, n->a->name) != 0)
        return NULL;
    if (local_known(cg, n->a->name) && !jsg_local_is_mut(g, n->a->name)) return NULL;
    {
        const char *lv = jsg_expr(g, n->a);
        bool is_int = n->a->type_str && cg_is_int_typestr(n->a->type_str);
        if (is_int && (rhs->op == TK_PLUS || rhs->op == TK_MINUS) && rhs->b &&
            rhs->b->kind == AST_LITERAL && rhs->b->op == TK_INT &&
            (long long)rhs->b->value.as.i == 1)
            return cg_fmt(cg, "%s%s", lv, rhs->op == TK_PLUS ? "++" : "--");
        return cg_fmt(cg, "%s %s= %s", lv, cg_op(rhs->op),
                      jsg_expr_p(g, rhs->b, JSP_LAMBDA));
    }
}

static const char *jsg_simple_inline(jg_t *g, ast_node_t *n)
{
    if (n->kind == AST_VAR_DECL) return jsg_vardecl_inline(g, n);
    if (n->kind == AST_ASSIGN) {
        cg_t *cg = &g->cg;
        const char *c = jsg_compound(g, n);
        if (c) return c;
        if (n->op == TK_POWER_EQ)
            return cg_fmt(cg, "%s = Math.pow(%s, %s)", jsg_expr(g, n->a),
                          jsg_expr(g, n->a), jsg_expr(g, n->b));
        return cg_fmt(cg, "%s %s %s", jsg_expr(g, n->a), cg_op(n->op), jsg_expr(g, n->b));
    }
    if (n->kind == AST_EXPR_STMT) return jsg_expr(g, n->a);
    return jsg_expr(g, n);
}

static const char *jsg_stmt_expr(jg_t *g, ast_node_t *e)
{
    const char *s = jsg_expr(g, e);
    size_t len = strlen(s);
    if (len >= 2 && s[0] == '(' && s[len - 1] == ')' && s[1] != '{') {
        int depth = 0;
        char quote = 0;
        bool whole = true;
        size_t i = 0;
        for (; i < len; i++) {
            char c = s[i];
            if (quote) {
                if (c == '\\')
                    i++;
                else if (c == quote)
                    quote = 0;
                continue;
            }
            if (c == '"' || c == '\'' || c == '`') {
                quote = c;
            } else if (c == '(') {
                depth++;
            } else if (c == ')') {
                depth--;
                if (depth == 0 && i != len - 1) {
                    whole = false;
                    break;
                }
            }
        }
        if (whole && !quote) return arena_strndup(g->cg.a, s + 1, len - 2);
    }
    return s;
}

static void jsg_stmt_list(jg_t *g, ast_node_t *block)
{
    size_t m = g->cg.locals.len;
    {
        size_t i = 0;
        for (; i < block->list.len; i++)
            jsg_stmt(g, (ast_node_t *)block->list.data[i]);
    }
    jsg_scope_reset(g, m);
}

static bool jsg_int_lit(ast_node_t *n, long long *out)
{
    if (!n) return false;
    if (n->kind == AST_LITERAL && n->op == TK_INT) {
        *out = (long long)n->value.as.i;
        return true;
    }
    if (n->kind == AST_UNARY && n->op == TK_MINUS && n->a && n->a->kind == AST_LITERAL &&
        n->a->op == TK_INT) {
        *out = -(long long)n->a->value.as.i;
        return true;
    }
    return false;
}

static bool jsg_bound_inline_ok(jg_t *g, ast_node_t *n)
{
    if (!n) return false;
    if (n->kind == AST_LITERAL) return true;
    if (n->kind == AST_IDENTIFIER && n->name) {
        if (local_known(&g->cg, n->name)) return !jsg_local_is_mut(g, n->name);
        {
            symbol_t *s = scope_lookup(g->cg.sem->global, n->name);
            return s && s->kind == SYM_CONST;
        }
    }
    return false;
}

static const char *jsg_loop_var(jg_t *g, ast_node_t *n)
{
    if (n->name) return jsg_local(g, n->name, false);
    {
        static const char *const bases[] = {"i", "j", "k"};
        size_t b = 0;
        for (; b < 3; b++)
            if (!local_known(&g->cg, bases[b])) {
                const char *lv = jsg_fresh(g, bases[b]);
                jsg_local(g, lv, false);
                return lv;
            }
        {
            const char *lv = jsg_fresh(g, "i");
            jsg_local(g, lv, false);
            return lv;
        }
    }
}

static void jsg_repeat(jg_t *g, ast_node_t *n)
{
    cg_t *cg = &g->cg;
    size_t m = cg->locals.len;
    const char *lv = jsg_loop_var(g, n);
    if (!n->c) {
        bool is_int = cg_is_int_typestr(n->a ? n->a->type_str : NULL);
        const char *bound = jsg_expr_p(g, n->a, 11);
        if (!is_int) bound = cg_fmt(cg, "Math.trunc(%s)", jsg_expr(g, n->a));
        if (jsg_bound_inline_ok(g, n->a)) {
            cg_line(cg, "for (let %s = 0; %s < %s; %s++) {", lv, lv, bound, lv);
        } else {
            const char *cnt = jsg_fresh(g, "count");
            cg_line(cg, "for (let %s = 0, %s = %s; %s < %s; %s++) {", lv, cnt, bound, lv,
                    cnt, lv);
        }
    } else {
        long long from, to, step = 1;
        bool folded = jsg_int_lit(n->a, &from) && jsg_int_lit(n->c, &to) &&
                      (!n->d || jsg_int_lit(n->d, &step));
        if (folded) {
            bool up = from <= to;
            long long mag = step < 0 ? -step : step;
            if (mag == 1)
                cg_line(cg, "for (let %s = %lld; %s %s %lld; %s%s) {", lv, from, lv,
                        up ? "<=" : ">=", to, lv, up ? "++" : "--");
            else
                cg_line(cg, "for (let %s = %lld; %s %s %lld; %s %s= %lld) {", lv, from,
                        lv, up ? "<=" : ">=", to, lv, up ? "+" : "-", mag);
        } else {
            const char *endv = jsg_fresh(g, "end");
            const char *stepv = jsg_fresh(g, "step");
            bool ai = cg_is_int_typestr(n->a ? n->a->type_str : NULL);
            bool ci = cg_is_int_typestr(n->c ? n->c->type_str : NULL);
            const char *from_s =
                ai ? jsg_expr(g, n->a) : cg_fmt(cg, "Math.trunc(%s)", jsg_expr(g, n->a));
            const char *to_s =
                ci ? jsg_expr(g, n->c) : cg_fmt(cg, "Math.trunc(%s)", jsg_expr(g, n->c));
            const char *step_s =
                n->d ? cg_fmt(cg, "Math.trunc(%s)", jsg_expr(g, n->d)) : "1";
            cg_line(cg,
                    "for (let %s = %s, %s = %s, %s = (%s <= %s ? 1 : -1) * %s; "
                    "%s > 0 ? %s <= %s : %s >= %s; %s += %s) {",
                    lv, from_s, endv, to_s, stepv, lv, endv, step_s, stepv, lv, endv, lv,
                    endv, lv, stepv);
        }
    }
    cg->indent++;
    {
        size_t i = 0;
        for (; i < n->b->list.len; i++)
            jsg_stmt(g, (ast_node_t *)n->b->list.data[i]);
    }
    cg->indent--;
    cg_line(cg, "}");
    jsg_scope_reset(g, m);
}

void jsg_stmt(jg_t *g, ast_node_t *n)
{
    cg_t *cg = &g->cg;
    if (!n) return;
    switch (n->kind) {
    case AST_BLOCK:
        jsg_block(g, n);
        break;
    case AST_VAR_DECL:
        cg_line(cg, "%s;", jsg_vardecl_inline(g, n));
        break;
    case AST_CONST_DECL: {
        const char *init = jsg_expr(g, n->a);
        cg_line(cg, "const %s = %s;", jsg_local(g, n->name, false), init);
        break;
    }
    case AST_ASSIGN: {
        if (n->op == TK_ASSIGN && n->a && n->a->kind == AST_INDEX && n->a->a &&
            n->a->a->type_str) {
            char sname[96];
            symbol_t *ssym = cg_struct_of(cg, n->a->a->type_str, sname, sizeof sname);
            if (ssym) {
                symbol_t *m = scope_lookup_local(ssym->members, "operator_index_set");
                if (m && m->kind == SYM_METHOD) {
                    func_sig_t *sig = pick_op_overload(cg, m, 2);
                    if (sig) {
                        const char *spkg = ssym->pkgname ? ssym->pkgname : cg->pkg;
                        const char *fn =
                            jsg_fn_name(g, spkg, ssym->name, "operator_index_set", m, sig,
                                        false, false);
                        cg_line(cg, "%s(%s, %s, %s);", fn, jsg_expr(g, n->a->a),
                                jsg_expr(g, n->a->b), jsg_expr(g, n->b));
                        break;
                    }
                }
            }
        }
        if (n->op != TK_ASSIGN && n->a && n->a->type_str) {
            token_kind_t base_op = cg_compound_base(n->op);
            if (base_op != TK_EOF) {
                char sname[96];
                symbol_t *ssym = cg_struct_of(cg, n->a->type_str, sname, sizeof sname);
                if (ssym) {
                    const char *mname = cg_op_method(base_op);
                    if (mname) {
                        const char *call =
                            jsg_emit_op_call(g, n->a, ssym, sname, mname, n->b);
                        if (call) {
                            cg_line(cg, "%s = %s;", jsg_expr(g, n->a), call);
                            break;
                        }
                    }
                }
            }
        }
        if (n->op == TK_POWER_EQ) {
            cg_line(cg, "%s = Math.pow(%s, %s);", jsg_expr(g, n->a), jsg_expr(g, n->a),
                    jsg_expr(g, n->b));
            break;
        }
        if (n->op == TK_SLASH_EQ && n->a && cg_is_int_typestr(n->a->type_str)) {
            cg_line(cg, "%s = Math.trunc(%s / %s);", jsg_expr(g, n->a),
                    jsg_expr_p(g, n->a, 13), jsg_expr_p(g, n->b, 14));
            break;
        }
        {
            const char *c = jsg_compound(g, n);
            if (c) {
                cg_line(cg, "%s;", c);
                break;
            }
        }
        cg_line(cg, "%s %s %s;", jsg_expr(g, n->a), cg_op(n->op), jsg_expr(g, n->b));
        break;
    }
    case AST_EXPR_STMT:
        cg_line(cg, "%s;", jsg_stmt_expr(g, n->a));
        break;
    case AST_DEFER:
        vec_push(cg->a, &cg->fn_defers, n->a);
        break;
    case AST_RETURN:
        jsg_emit_defers(g);
        if (n->a)
            cg_line(cg, "return %s;", jsg_expr(g, n->a));
        else
            cg_line(cg, "return;");
        break;
    case AST_BREAK:
        cg_line(cg, "break;");
        break;
    case AST_CONTINUE:
        cg_line(cg, "continue;");
        break;
    case AST_IF:
        cg_line(cg, "if (%s) {", jsg_expr(g, n->a));
        cg->indent++;
        jsg_stmt_list(g, n->b);
        cg->indent--;
        if (n->c && n->c->kind == AST_IF) {
            cg_indent(cg);
            sb_puts(cg->c, "} else ");
            sb_puts(cg->c, cg_fmt(cg, "if (%s) {\n", jsg_expr(g, n->c->a)));
            cg->indent++;
            jsg_stmt_list(g, n->c->b);
            cg->indent--;
            if (n->c->c) {
                cg_line(cg, "} else {");
                cg->indent++;
                jsg_stmt(g, n->c->c);
                cg->indent--;
            }
            cg_line(cg, "}");
        } else if (n->c) {
            cg_line(cg, "} else {");
            cg->indent++;
            jsg_stmt_list(g, n->c);
            cg->indent--;
            cg_line(cg, "}");
        } else {
            cg_line(cg, "}");
        }
        break;
    case AST_UNTIL:
        cg_line(cg, "while (%s) {", jsg_expr(g, n->a));
        cg->indent++;
        jsg_stmt_list(g, n->b);
        cg->indent--;
        cg_line(cg, "}");
        break;
    case AST_REPEAT:
        jsg_repeat(g, n);
        break;
    case AST_EACH: {
        size_t m = cg->locals.len;
        if (n->c && n->c->name) {
            const char *it = jsg_expr_p(g, n->a, JSP_MEMBER);
            cg_line(cg, "for (const [%s, %s] of %s.entries()) {",
                    jsg_local(g, n->c->name, false), jsg_local(g, n->name, false), it);
        } else {
            const char *it = jsg_expr(g, n->a);
            cg_line(cg, "for (const %s of %s) {", jsg_local(g, n->name, false), it);
        }
        cg->indent++;
        {
            size_t i = 0;
            for (; i < n->b->list.len; i++)
                jsg_stmt(g, (ast_node_t *)n->b->list.data[i]);
        }
        cg->indent--;
        cg_line(cg, "}");
        jsg_scope_reset(g, m);
        break;
    }
    case AST_FOR: {
        size_t mark = cg->locals.len;
        const char *init = n->a ? jsg_simple_inline(g, n->a) : "";
        const char *cond = n->b ? jsg_expr(g, n->b) : "";
        const char *post = n->c ? jsg_simple_inline(g, n->c) : "";
        cg_line(cg, "for (%s; %s; %s) {", init, cond, post);
        cg->indent++;
        {
            size_t i = 0;
            for (; i < n->d->list.len; i++)
                jsg_stmt(g, (ast_node_t *)n->d->list.data[i]);
        }
        cg->indent--;
        cg_line(cg, "}");
        jsg_scope_reset(g, mark);
        break;
    }
    default:
        break;
    }
}

void jsg_block(jg_t *g, ast_node_t *block)
{
    jsg_stmt_list(g, block);
}

void jsg_function(jg_t *g, ast_node_t *fn, symbol_t *owner)
{
    cg_t *cg = &g->cg;
    symbol_t *fsym = owner ? scope_lookup_local(owner->members, fn->name)
                           : scope_lookup_local(cg->sem->global, fn->name);
    func_sig_t *sig = sig_of_decl(fsym, fn);
    if (!sig) return;
    {
        bool ti = owner && owner->kind == SYM_TYPEIMPL;
        const char *ti_ts = ti ? type_to_string(cg->sem->tc, owner->type) : NULL;
        bool is_instance = (owner && owner->generic_base) || fn->synthetic;
        const char *opkg = (owner && owner->pkgname) ? owner->pkgname : cg->pkg;
        const char *name;
        if (fn->is_extern)
            name = cg_cident(cg, fn->name);
        else if (ti)
            name =
                (fsym && fsym->overloads.len > 1)
                    ? cg_mangle_ti(cg, ti_ts, fn->name, &sig->params)
                    : jsg_ident(g, cg_fmt(cg, "%s_%s", cg_cident(cg, ti_ts), fn->name));
        else
            name = jsg_fn_name(g, opkg, owner ? owner->name : NULL, fn->name, fsym, sig,
                               false, is_instance);
        g->last_fn_name = name;
        if (!owner && cg->entry && strcmp(fn->name, cg->entry) == 0)
            g->entry_mangled = name;
        cg->cur_struct = owner;
        cg->cur_fn_home = fsym ? fsym->home : NULL;
        jsg_scope_reset(g, 0);
        g->fn_used_names.len = 0;
        {
            vec_t saved_defers = cg->fn_defers;
            sb_t params;
            vec_init(&cg->fn_defers);
            sb_init(&params);
            if (owner) {
                sb_puts(&params, "__self");
                jsg_local(g, "__self", false);
            }
            {
                size_t i = 0;
                for (; i < fn->list.len; i++) {
                    ast_node_t *p = (ast_node_t *)fn->list.data[i];
                    if (p->is_ref)
                        LOG_W(cg->log, PH_CODEGEN,
                              "js backend: reference parameter '%s' of '%s' is passed "
                              "by value",
                              p->name, fn->name);
                    if (owner || i) sb_puts(&params, ", ");
                    sb_puts(&params, jsg_local(g, p->name, false));
                }
            }
            cg_line(cg, "function %s(%s) {", name, sb_cstr(&params));
            sb_free(&params);
            cg->indent++;
            if (fn->a) jsg_block(g, fn->a);
            jsg_emit_defers(g);
            cg->indent--;
            cg_line(cg, "}");
            cg->fn_defers = saved_defers;
        }
        cg->cur_struct = NULL;
        cg->cur_fn_home = NULL;
    }
}
