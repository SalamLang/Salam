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
#include "core/sal_format.h"
#include "semantic/sema_internal.h"

symbol_t *generic_template(sema_t *s, const char *name, sym_kind_t kind)
{
    symbol_t *sym = name ? scope_lookup(s->cur, name) : NULL;
    if (sym && sym->kind == kind && sym->decl && sym->decl->typarams.len > 0) return sym;
    return NULL;
}

static void g_sanitize(const char *ts, char *out, size_t cap)
{
    size_t o = 0;
    { const char *p = ts; for (; *p && o + 6 < cap; p++) {
        char c = *p;
        if (c == '*')      { memcpy(out + o, "_ptr", 4); o += 4; }
        else if (c == '[') { memcpy(out + o, "_arr", 4); o += 4; }
        else if (c == '<' || c == ',') out[o++] = '_';
        else if (c == '>' || c == ' ' || c == ']') {  }
        else out[o++] = c;
    } }
    out[o] = 0;
}

static void g_subst(arena_t *a, ast_node_t *n, vec_t *params, vec_t *args)
{
    if (!n) return;
    g_subst(a, n->type, params, args);
    g_subst(a, n->a, params, args); g_subst(a, n->b, params, args);
    g_subst(a, n->c, params, args); g_subst(a, n->d, params, args);
    { size_t i = 0; for (; i < n->list.len; i++) g_subst(a, (ast_node_t *)n->list.data[i], params, args); }
    { size_t i = 0; for (; i < n->dims.len; i++) g_subst(a, (ast_node_t *)n->dims.data[i], params, args); }
    if (n->kind == AST_TYPE && n->name) {
        { size_t i = 0; for (; i < params->len; i++) {
            if (strcmp(n->name, (const char *)params->data[i]) != 0) continue;
            ast_node_t *cc = ast_clone(a, (const ast_node_t *)args->data[i]);
            bool ptr = n->is_pointer || cc->is_pointer;
            n->name = cc->name;
            n->list = cc->list;
            n->is_pointer = ptr;
            n->is_dyn = cc->is_dyn;   
            vec_t merged; vec_init(&merged);
            { size_t k = 0; for (; k < cc->dims.len; k++) vec_push(a, &merged, cc->dims.data[k]); }
            { size_t k = 0; for (; k < n->dims.len; k++)  vec_push(a, &merged, n->dims.data[k]); }
            n->dims = merged;
            break;
        } }
    }
}

static const char *g_instance_name(sema_t *s, const char *base, vec_t *argtypes)
{
    char buf[512]; size_t o = 0;
    o = sal_catf(buf, sizeof buf, o, "%s", base);
    { size_t i = 0; for (; i < argtypes->len && o < sizeof buf - 130; i++) {
        char san[128];
        g_sanitize(type_to_string(s->tc, (type_t *)argtypes->data[i]), san, sizeof san);
        o = sal_catf(buf, sizeof buf, o, "_%s", san);
    } }
    return arena_strdup(s->a, buf);
}

static ast_node_t *g_type_to_ast(sema_t *s, type_t *t, const src_span_t *span)
{
    ast_node_t *n = ast_new(s->a, AST_TYPE, span);
    if (!t) { n->name = "void"; return n; }
    if (t->kind == TY_PTR && t->pointee) {
        n = g_type_to_ast(s, t->pointee, span);
        n->is_pointer = true;
        return n;
    }
    if (t->kind == TY_VEC) { n->name = "Vector"; ast_add(s->a, n, g_type_to_ast(s, t->elem, span)); return n; }
    if (t->kind == TY_MAP) {
        n->name = "HashMap";
        ast_add(s->a, n, g_type_to_ast(s, t->key, span));
        ast_add(s->a, n, g_type_to_ast(s, t->elem, span));
        return n;
    }
    n->name = arena_strdup(s->a, type_to_string(s->tc, t));
    return n;
}

static bool iface_satisfied(sema_t *s, type_t *concrete, const char *iface_name,
                            const char *tparam, const src_span_t *span)
{
    symbol_t *iface = scope_lookup(s->global, iface_name);
    if (!iface || iface->kind != SYM_INTERFACE) {
        SERR(s, 1, span, "bound '%s' on type parameter '%s' is not an interface", iface_name, tparam);
        return false;
    }
    
    scope_t *members = NULL;
    if (concrete && concrete->kind == TY_STRUCT && concrete->decl)
        members = ((symbol_t *)concrete->decl)->members;
    else if (concrete) {
        symbol_t *owner = scope_lookup(s->global, impl_owner_key(s->a, type_to_string(s->tc, concrete)));
        if (owner) members = owner->members;
    }
    if (!members) {
        SERR(s, 2, span, "'%s' does not satisfy '%s': no struct definition or `impl` block provides its methods",
             type_to_string(s->tc, concrete), iface_name);
        return false;
    }
    bool ok = true;
    { size_t i = 0; for (; i < iface->members->symbols.len; i++) {
        symbol_t *im = (symbol_t *)iface->members->symbols.data[i];
        if (im->kind != SYM_METHOD || im->overloads.len == 0) continue;
        func_sig_t *want = (func_sig_t *)im->overloads.data[0];
        symbol_t *sm = scope_lookup_local(members, im->name);
        if (!sm || sm->kind != SYM_METHOD) {
            SERR(s, 2, span, "'%s' does not implement interface '%s': missing method '%s'",
                 type_to_string(s->tc, concrete), iface_name, im->name);
            ok = false; continue;
        }
        bool sig_ok = false;
        { size_t k = 0; for (; k < sm->overloads.len; k++) {
            func_sig_t *have = (func_sig_t *)sm->overloads.data[k];
            if (have->params.len != want->params.len) continue;
            bool pm = true;
            { size_t p = 0; for (; p < want->params.len; p++)
                if (!type_equiv((type_t *)have->params.data[p], (type_t *)want->params.data[p])) { pm = false; break; } }
            if (pm && type_equiv(have->ret, want->ret)) { sig_ok = true; break; }
        } }
        if (!sig_ok) {
            SERR(s, 2, span, "'%s' implements method '%s' with a signature that does not match interface '%s'",
                 type_to_string(s->tc, concrete), im->name, iface_name);
            ok = false;
        }
    } }
    return ok;
}

static void g_check_bounds(sema_t *s, ast_node_t *tmpl, vec_t *argtypes, const src_span_t *span)
{
    { size_t i = 0; for (; i < tmpl->typarams.len && i < argtypes->len; i++) {
        const char *bound = i < tmpl->typaram_bounds.len
                          ? (const char *)tmpl->typaram_bounds.data[i] : NULL;
        if (bound)
            iface_satisfied(s, (type_t *)argtypes->data[i], bound,
                            (const char *)tmpl->typarams.data[i], span);
    } }
}

ast_node_t *coerce_to_dyn(sema_t *s, type_t *expected, ast_node_t *expr, type_t *etype)
{
    if (!expected || expected->kind != TY_DYN || !expr) return expr;
    if (!etype || etype->kind != TY_STRUCT) return expr;   
    iface_satisfied(s, etype, expected->name, expected->name, &expr->span);
    ast_node_t *box = ast_new(s->a, AST_CAST, &expr->span);
    box->a = expr;
    box->synthetic = true;
    box->type_str = type_to_string(s->tc, expected);   
    return box;
}

void coerce_args_to_dyn(sema_t *s, ast_node_t *call, vec_t *argtypes, func_sig_t *sig)
{
    if (!sig) return;
    { size_t i = 0; for (; i < call->list.len && i < sig->params.len && i < argtypes->len; i++) {
        type_t *pt = (type_t *)sig->params.data[i];
        if (pt && pt->kind == TY_DYN)
            call->list.data[i] = coerce_to_dyn(s, pt, (ast_node_t *)call->list.data[i],
                                               (type_t *)argtypes->data[i]);
    } }
}

symbol_t *g_instantiate_struct(sema_t *s, ast_node_t *tmpl, vec_t *targ_nodes, const src_span_t *span)
{
    if (targ_nodes->len != tmpl->typarams.len) {
        SERR(s, 1, span, "generic '%s' expects %zu type argument(s), got %zu",
             tmpl->name, tmpl->typarams.len, targ_nodes->len);
        return NULL;
    }
    vec_t argtypes; vec_init(&argtypes);
    { size_t i = 0; for (; i < targ_nodes->len; i++)
        vec_push(s->a, &argtypes, sema_resolve_type(s, (ast_node_t *)targ_nodes->data[i])); }
    const char *iname = g_instance_name(s, tmpl->name, &argtypes);
    symbol_t *existing = scope_lookup_local(s->global, iname);
    if (existing) return existing;
    g_check_bounds(s, tmpl, &argtypes, span);   
    ast_node_t *inst = ast_clone(s->a, tmpl);
    inst->name = iname;
    inst->synthetic = true;
    g_subst(s->a, inst, &tmpl->typarams, targ_nodes);
    inst->typarams.len = 0;
    vec_push(s->a, &s->program->list, inst);
    symbol_t *sym = symbol_new(s->a, SYM_STRUCT, iname);
    sym->decl = inst;
    sym->type = type_struct(s->tc, sym, iname);
    
    sym->members = scope_new(s->a, SCOPE_STRUCT, s->global);
    sym->members->label = iname;
    
    sym->members->aux = s->gen_pkg;
    sym->home = s->gen_pkg;
    sym->generic_base = tmpl->name;          
    vec_init(&sym->generic_args);
    { size_t i = 0; for (; i < argtypes.len; i++)
        vec_push(s->a, &sym->generic_args, argtypes.data[i]); }
    scope_define(s->a, s->global, sym);
    { size_t j = 0; for (; j < inst->list.len; j++) {
        ast_node_t *m = (ast_node_t *)inst->list.data[j];
        if (m->kind == AST_FIELD) {
            symbol_t *f = symbol_new(s->a, SYM_FIELD, m->name);
            f->type = sema_resolve_type(s, m->type);
            f->decl = m;
            scope_define(s->a, sym->members, f);
        } else if (m->kind == AST_FUNC_DEF) {
            symbol_t *mm = get_or_make_func(s, sym->members, m->name, SYM_METHOD);
            vec_push(s->a, &mm->overloads, build_sig(s, m, sym));
            mm->type = sym->type;
        }
    } }
    vec_push(s->a, &s->pending, inst);
    return sym;
}

static symbol_t *g_instantiate_func(sema_t *s, ast_node_t *tmpl, vec_t *targ_nodes, const src_span_t *span)
{
    if (targ_nodes->len != tmpl->typarams.len) {
        SERR(s, 1, span, "generic '%s' expects %zu type argument(s), got %zu",
             tmpl->name, tmpl->typarams.len, targ_nodes->len);
        return NULL;
    }
    vec_t argtypes; vec_init(&argtypes);
    { size_t i = 0; for (; i < targ_nodes->len; i++)
        vec_push(s->a, &argtypes, sema_resolve_type(s, (ast_node_t *)targ_nodes->data[i])); }
    const char *iname = g_instance_name(s, tmpl->name, &argtypes);
    symbol_t *existing = scope_lookup_local(s->global, iname);
    if (existing) return existing;
    g_check_bounds(s, tmpl, &argtypes, span);   
    ast_node_t *inst = ast_clone(s->a, tmpl);
    inst->name = iname;
    inst->synthetic = true;
    g_subst(s->a, inst, &tmpl->typarams, targ_nodes);
    inst->typarams.len = 0;
    vec_push(s->a, &s->program->list, inst);
    symbol_t *fsym = get_or_make_func(s, s->global, iname, SYM_FUNC);
    /* Remember the template's package so the instance body can resolve sibling
     * and self calls (other free functions of the same package) when it is
     * checked later in the importer's scope. Mirrors g_instantiate_struct. */
    if (!fsym->home) fsym->home = s->gen_pkg;
    vec_push(s->a, &fsym->overloads, build_sig(s, inst, NULL));
    vec_push(s->a, &s->pending, inst);
    return fsym;
}

static type_t *g_unify(ast_node_t *pty, type_t *at, const char *T)
{
    if (!pty || !at || !pty->name) return NULL;
    const char *pn = intrinsic_type_canon(pty->name);   
    if (strcmp(pty->name, T) == 0 && pty->dims.len == 0) {
        if (pty->is_pointer)
            return (at->kind == TY_PTR) ? at->pointee : NULL;
        return at;
    }
    if (strcmp(pn, "Vector") == 0 && at->kind == TY_VEC && pty->list.len >= 1)
        return g_unify((ast_node_t *)pty->list.data[0], at->elem, T);
    if (strcmp(pn, "HashMap") == 0 && at->kind == TY_MAP) {
        type_t *u = NULL;
        if (pty->list.len >= 1) u = g_unify((ast_node_t *)pty->list.data[0], at->key, T);
        if (!u && pty->list.len >= 2) u = g_unify((ast_node_t *)pty->list.data[1], at->elem, T);
        return u;
    }
    
    if (pty->list.len > 0 && at->kind == TY_STRUCT && at->decl) {
        symbol_t *isym = (symbol_t *)at->decl;
        if (isym->generic_base && strcmp(isym->generic_base, pty->name) == 0) {
            size_t k = pty->list.len < isym->generic_args.len ? pty->list.len : isym->generic_args.len;
            { size_t i = 0; for (; i < k; i++) {
                type_t *u = g_unify((ast_node_t *)pty->list.data[i],
                                    (type_t *)isym->generic_args.data[i], T);
                if (u) return u;
            } }
        }
    }
    return NULL;
}

symbol_t *g_infer_call(sema_t *s, symbol_t *tsym, vec_t *argtypes,
                       const src_span_t *span, type_t *expected)
{
    ast_node_t *tmpl = tsym->decl;
    vec_t targ_nodes; vec_init(&targ_nodes);
    { size_t t = 0; for (; t < tmpl->typarams.len; t++) {
        const char *T = (const char *)tmpl->typarams.data[t];
        type_t *bound = NULL;
        { size_t pi = 0; for (; pi < tmpl->list.len && pi < argtypes->len; pi++) {
            ast_node_t *p = (ast_node_t *)tmpl->list.data[pi];
            if (p->kind != AST_PARAM) continue;
            type_t *u = g_unify(p->type, (type_t *)argtypes->data[pi], T);
            if (u) { bound = u; break; }
        } }
        /* When a type parameter cannot be inferred from the arguments (e.g. a
         * nullary constructor like None<T>()), fall back to unifying the
         * function's declared return type against the expected type supplied
         * by the call context (assignment target or enclosing return). */
        if (!bound && expected && tmpl->type)
            bound = g_unify(tmpl->type, expected, T);
        if (!bound) {
            SERR(s, 1, span, "cannot infer type parameter '%s' of generic function '%s'", T, tmpl->name);
            return NULL;
        }
        vec_push(s->a, &targ_nodes, g_type_to_ast(s, bound, span));
    } }
    return g_instantiate_func(s, tmpl, &targ_nodes, span);
}
