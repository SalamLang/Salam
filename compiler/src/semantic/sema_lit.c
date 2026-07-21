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

static type_t *ty(sema_t *s, type_kind_t k)
{
    return sema_ty(s, k);
}

static type_t *err_ty(sema_t *s)
{
    return sema_err_ty(s);
}

static type_t *decorate(sema_t *s, ast_node_t *n, type_t *t)
{
    return sema_decorate(s, n, t);
}

type_t *check_struct_lit(sema_t *s, ast_node_t *n)
{
    if (n->name) {
        const char *orig = n->name;
        n->name = intrinsic_type_canon(local_canon(s, n->name, &n->span));
        sema_check_intrinsic_type_lang(s, orig, &n->span);
    }
    type_t *exp = s->expected;
    s->expected = NULL;
    if (exp && exp->kind == TY_STRUCT && n->name &&
        (generic_template(s, n->name, SYM_STRUCT) || !scope_lookup(s->global, n->name)))
        n->name = exp->name;
    if (!n->name) {
        SERR(s, 1, &n->span, "cannot infer the struct type of this literal");
        return decorate(s, n, err_ty(s));
    }
    symbol_t *ssym = scope_lookup(s->global, n->name);
    if ((!ssym || ssym->kind != SYM_STRUCT) && n->list.len == 0) {
        symbol_t *tmpl = generic_template(s, n->name, SYM_STRUCT);
        if (!tmpl) {
            sema_load_prelude(s);
            if (s->prelude) {
                symbol_t *psym = scope_lookup_local(s->prelude, n->name);
                if (psym && psym->kind == SYM_STRUCT && psym->decl &&
                    psym->decl->typarams.len > 0)
                    tmpl = psym;
            }
        }
        if (tmpl) {
            char targs[128];
            size_t o = 0;
            {
                size_t i = 0;
                for (; i < tmpl->decl->typarams.len && o + 1 < sizeof(targs); i++) {
                    if (i > 0 && o + 2 < sizeof(targs)) {
                        targs[o++] = ',';
                        targs[o++] = ' ';
                    }
                    const char *pn = (const char *)tmpl->decl->typarams.data[i];
                    size_t pl = strlen(pn);
                    if (o + pl >= sizeof(targs)) pl = sizeof(targs) - o - 1;
                    memcpy(targs + o, pn, pl);
                    o += pl;
                }
            }
            targs[o] = 0;
            SERR(s, 71, &n->span,
                 "cannot infer the type arguments of '%s'; write '%s {} as %s<%s>'",
                 n->name, n->name, n->name, targs);
            return decorate(s, n, err_ty(s));
        }
    }
    if (!ssym || ssym->kind != SYM_STRUCT) {
        SERR(s, 1, &n->span, "unknown struct '%s'", n->name);
        return decorate(s, n, err_ty(s));
    }

    {
        size_t i = 0;
        for (; i < n->list.len; i++) {
            ast_node_t *fi = (ast_node_t *)n->list.data[i];
            fi->name = scope_member_canon(s, ssym->members, fi->name, &fi->span);
            symbol_t *f = scope_lookup_local(ssym->members, fi->name);
            type_t *vt =
                (f && f->kind == SYM_FIELD && stamp_empty_intrinsic(s, fi->a, f->type))
                    ? f->type
                    : sema_check_expr(s, fi->a);
            if (!f || f->kind != SYM_FIELD) {
                SERR(s, 16, &fi->span, "struct '%s' has no field '%s'", ssym->name,
                     fi->name);
            } else if (!type_assignable(f->type, vt)) {
                SERR(s, 2, &fi->span, "field '%s': cannot assign '%s' to '%s'", fi->name,
                     type_to_string(s->tc, vt), type_to_string(s->tc, f->type));
            } else if (f->type && f->type->kind == TY_DYN) {
                fi->a = coerce_to_dyn(s, f->type, fi->a, vt);
            } else if (f->type && f->type->kind == TY_VARIANT && vt &&
                       vt->kind != TY_VARIANT) {
                int tag = type_variant_tag(f->type, vt);
                if (tag >= 0) fi->a = coerce_to_variant(s, f->type, fi->a, tag);
            }
            if (f) decorate(s, fi, f->type);
        }
    }

    {
        size_t i = 0;
        for (; i < ssym->members->symbols.len; i++) {
            symbol_t *f = (symbol_t *)ssym->members->symbols.data[i];
            if (f->kind != SYM_FIELD) continue;
            if (f->decl && f->decl->a) continue;
            bool provided = false;
            {
                size_t j = 0;
                for (; j < n->list.len; j++) {
                    ast_node_t *fi = (ast_node_t *)n->list.data[j];
                    if (fi->name && strcmp(fi->name, f->name) == 0) {
                        provided = true;
                        break;
                    }
                }
            }
            if (!provided)
                SERR(s, 28, &n->span, "struct literal '%s' requires field '%s'",
                     ssym->name, f->name);
        }
    }
    return decorate(s, n, ssym->type);
}

type_t *check_array_lit(sema_t *s, ast_node_t *n)
{
    type_t *exp = s->expected;
    s->expected = NULL;
    type_t *exp_elem = (exp && exp->kind == TY_ARRAY) ? exp->elem : NULL;
    if (n->list.len == 0) {
        if (exp_elem) return decorate(s, n, type_array(s->tc, exp_elem, 0));
        SERR(s, 52, &n->span,
             "cannot infer the type of an empty array; write 'items := [] as Type[]'");
        return decorate(s, n, err_ty(s));
    }

    if (exp_elem && exp_elem->kind == TY_DYN) {
        {
            size_t i = 0;
            for (; i < n->list.len; i++) {
                type_t *t = sema_check_expr(s, (ast_node_t *)n->list.data[i]);
                n->list.data[i] =
                    coerce_to_dyn(s, exp_elem, (ast_node_t *)n->list.data[i], t);
            }
        }
        return decorate(s, n, type_array(s->tc, exp_elem, n->list.len));
    }
    type_t *elem = sema_check_expr(s, (ast_node_t *)n->list.data[0]);
    {
        size_t i = 1;
        for (; i < n->list.len; i++) {
            type_t *t = sema_check_expr(s, (ast_node_t *)n->list.data[i]);
            if (type_equiv(elem, t)) continue;
            type_t *c = type_common_arith(s->tc, elem, t);
            if (c)
                elem = c;
            else
                SERR(s, 56, &n->span, "array elements have incompatible types");
        }
    }
    return decorate(s, n, type_array(s->tc, elem, n->list.len));
}

bool defined_within(scope_t *start, scope_t *boundary, const char *name)
{
    {
        scope_t *p = start;
        for (; p; p = p->parent) {
            if (scope_lookup_local(p, name)) return true;
            if (p == boundary) break;
        }
    }
    return false;
}

void record_capture(sema_t *s, ast_node_t *id, type_t *t)
{
    const char *cts = type_to_string(s->tc, t);
    {
        lambda_ctx_t *L = s->lam;
        for (; L; L = L->prev) {
            if (defined_within(s->cur, L->boundary, id->name)) break;
            ast_node_t *lam = L->node;
            bool seen = false;
            {
                size_t i = 0;
                for (; i < lam->captures.len; i++)
                    if (!strcmp(((ast_node_t *)lam->captures.data[i])->name, id->name)) {
                        seen = true;
                        break;
                    }
            }
            if (seen) continue;
            ast_node_t *cap = ast_new(s->a, AST_IDENTIFIER, &id->span);
            cap->name = id->name;
            cap->type_str = cts;
            vec_push(s->a, &lam->captures, cap);
        }
    }
}

type_t *check_lambda(sema_t *s, ast_node_t *n)
{
    scope_t *sc = scope_new(s->a, SCOPE_FUNC, s->cur);
    scope_t *saved = s->cur;
    func_sig_t *saved_func = s->cur_func;
    type_t *saved_self = s->self_type;
    int saved_mad = s->match_arm_depth;
    type_t *saved_myexp = s->match_yield_expected;
    vec_t *saved_mycol = s->match_yield_collect;
    lambda_ctx_t lctx = {sc, n, s->lam};
    s->cur = sc;
    s->self_type = NULL;
    s->lam = &lctx;
    s->match_arm_depth = 0;
    s->match_yield_expected = NULL;
    s->match_yield_collect = NULL;
    n->captures.len = 0;
    vec_t ptypes;
    vec_init(&ptypes);
    {
        size_t i = 0;
        for (; i < n->list.len; i++) {
            ast_node_t *param = (ast_node_t *)n->list.data[i];
            symbol_t *ps = symbol_new(s->a, SYM_PARAM, param->name);
            ps->type = sema_resolve_type(s, param->type);
            ps->is_ref = param->is_ref;
            param->type_str = type_to_string(s->tc, ps->type);
            if (scope_define(s->a, sc, ps))
                SERR(s, 1, &param->span, "duplicate parameter '%s'", param->name);
            vec_push(s->a, &ptypes, ps->type);
        }
    }

    func_sig_t sig;
    memset(&sig, 0, sizeof(sig));
    sig.params = ptypes;
    sig.ret = ty(s, TY_VOID);
    sig.infer_ret = true;
    s->cur_func = &sig;
    sc->func = &sig;
    if (n->a) sema_check_block(s, n->a);
    type_t *ret = sig.ret;
    s->cur = saved;
    s->cur_func = saved_func;
    s->self_type = saved_self;
    s->lam = lctx.prev;
    s->match_arm_depth = saved_mad;
    s->match_yield_expected = saved_myexp;
    s->match_yield_collect = saved_mycol;
    return decorate(s, n, type_func(s->tc, ret, &ptypes));
}
