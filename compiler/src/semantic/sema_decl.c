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
#include "i18n/i18n.h"

static type_t *ty(sema_t *s, type_kind_t k)
{
    return sema_ty(s, k);
}

func_sig_t *build_sig(sema_t *s, ast_node_t *fn, symbol_t *owner)
{
    func_sig_t *sig = (func_sig_t *)arena_alloc(s->a, sizeof(*sig));
    memset(sig, 0, sizeof(*sig));
    vec_init(&sig->params);
    sig->ret = fn->type ? sema_resolve_type(s, fn->type) : ty(s, TY_VOID);
    sig->decl = fn;
    sig->owner = owner;
    sig->variadic = fn->is_variadic;
    size_t required = 0;
    bool seen_default = false;
    {
        size_t i = 0;
        for (; i < fn->list.len; i++) {
            ast_node_t *param = (ast_node_t *)fn->list.data[i];
            type_t *pt = sema_resolve_type(s, param->type);
            vec_push(s->a, &sig->params, pt);
            if (param->a)
                seen_default = true;
            else if (!seen_default)
                required++;
        }
    }
    sig->required = required;
    sig->mangled = mangle_func(s->a, owner ? owner->name : NULL, fn->name, &sig->params);
    return sig;
}

symbol_t *get_or_make_func(sema_t *s, scope_t *sc, const char *name, sym_kind_t kind)
{
    symbol_t *sym = scope_lookup_local(sc, name);
    if (sym && (sym->kind == SYM_FUNC || sym->kind == SYM_METHOD)) return sym;
    sym = symbol_new(s->a, kind, name);
    scope_define(s->a, sc, sym);
    return sym;
}

static void check_link(sema_t *s, ast_node_t *d)
{
    const char *spec =
        (d->value.kind == TV_STRING && d->value.as.s) ? d->value.as.s : NULL;
    if (!spec || !spec[0]) {
        SERR(s, 57, &d->span, "link directive needs a library name or path");
        return;
    }
    const char *kind = d->name;
    if (kind && !strcmp(kind, "framework")) return;
    if (spec[0] == '-') return;
    if (!strpbrk(spec, "/\\.")) return;
    bool absolute = spec[0] == '/' || spec[0] == '\\' ||
                    (isalpha((unsigned char)spec[0]) && spec[1] == ':');
    const char *full = spec;
    if (!absolute && s->dir && s->dir[0]) {
        size_t n = strlen(s->dir) + 1 + strlen(spec) + 1;
        char *buf = (char *)arena_alloc(s->a, n);
        sal_snprintf(buf, n, "%s/%s", s->dir, spec);
        full = buf;
    }
    FILE *f = fopen(full, "rb");
    if (f) {
        fclose(f);
        return;
    }
    if (full != spec)
        SERR(s, 57, &d->span, "linked library not found: '%s' (resolved to '%s')", spec,
             full);
    else
        SERR(s, 57, &d->span, "linked library not found: '%s'", spec);
}

void sema_collect(sema_t *s, ast_node_t *program)
{
    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (d->synthetic) continue;
            if (d->kind == AST_STRUCT_DEF) {
                symbol_t *sym = symbol_new(s->a, SYM_STRUCT, d->name);
                sym->decl = d;
                sym->is_pub = d->is_pub;

                const char *c_type_name = d->name;
                if (s->pkg && s->pkg[0] && strcmp(s->pkg, "main") != 0) {
                    char _ctn[512];
                    sal_snprintf(_ctn, sizeof(_ctn), "%s_%s", s->pkg, d->name);
                    c_type_name = arena_strdup(s->a, _ctn);
                }
                sym->type = type_struct(s->tc, sym, c_type_name);
                sym->members = scope_new(s->a, SCOPE_STRUCT, s->global);
                sym->members->label = d->name;
                sym->home = s->global;
                if (scope_define(s->a, s->global, sym))
                    SERR(s, 1, &d->span, "redefinition of '%s'", d->name);
            } else if (d->kind == AST_ENUM_DEF) {
                symbol_t *sym = symbol_new(s->a, SYM_ENUM, d->name);
                sym->decl = d;
                sym->is_pub = d->is_pub;
                const char *c_enum_name = d->name;
                if (s->pkg && s->pkg[0] && strcmp(s->pkg, "main") != 0) {
                    char _cen[512];
                    sal_snprintf(_cen, sizeof(_cen), "%s_%s", s->pkg, d->name);
                    c_enum_name = arena_strdup(s->a, _cen);
                }
                sym->type = type_enum(s->tc, sym, c_enum_name);
                sym->members = scope_new(s->a, SCOPE_STRUCT, s->global);
                if (scope_define(s->a, s->global, sym))
                    SERR(s, 1, &d->span, "redefinition of '%s'", d->name);
            } else if (d->kind == AST_TYPE_ALIAS) {
                symbol_t *sym = symbol_new(s->a, SYM_ALIAS, d->name);
                sym->decl = d;
                sym->is_pub = d->is_pub;
                scope_define(s->a, s->global, sym);
            } else if (d->kind == AST_INTERFACE_DEF) {
                symbol_t *sym = symbol_new(s->a, SYM_INTERFACE, d->name);
                sym->decl = d;
                sym->is_pub = d->is_pub;
                sym->members = scope_new(s->a, SCOPE_STRUCT, s->global);
                sym->members->label = d->name;
                if (scope_define(s->a, s->global, sym))
                    SERR(s, 1, &d->span, "redefinition of '%s'", d->name);
            }
        }
    }

    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (d->synthetic) continue;
            switch (d->kind) {
            case AST_LINK:
                check_link(s, d);
                break;
            case AST_TYPE_ALIAS: {
                symbol_t *sym = scope_lookup_local(s->global, d->name);
                if (sym) sym->type = sema_resolve_type(s, d->type);
                break;
            }
            case AST_STRUCT_DEF: {
                if (d->list.len == 0)
                    SERR(s, 58, &d->span,
                         "empty struct '%s' (declare at least one field or method)",
                         d->name);
                if (d->typarams.len > 0) break;
                symbol_t *sym = scope_lookup_local(s->global, d->name);
                {
                    size_t j = 0;
                    for (; j < d->list.len; j++) {
                        ast_node_t *m = (ast_node_t *)d->list.data[j];
                        if (m->kind == AST_FIELD) {
                            symbol_t *f = symbol_new(s->a, SYM_FIELD, m->name);
                            f->type = sema_resolve_type(s, m->type);
                            f->decl = m;
                            if (f->type && f->type->kind == TY_DYN)
                                SERR(s, 2, &m->span,
                                     "field '%s': a `dyn` value cannot yet be a struct "
                                     "field; "
                                     "make '%s' generic (`<T: %s>`) or store the data "
                                     "another way",
                                     m->name, d->name,
                                     f->type->name ? f->type->name : "Iface");
                            if (scope_define(s->a, sym->members, f))
                                SERR(s, 27, &m->span,
                                     "duplicate field '%s' in struct '%s'", m->name,
                                     d->name);
                        } else if (m->kind == AST_FUNC_DEF) {
                            symbol_t *mm =
                                get_or_make_func(s, sym->members, m->name, SYM_METHOD);
                            if (!mm->decl) mm->decl = m;
                            vec_push(s->a, &mm->overloads, build_sig(s, m, sym));
                            mm->type = sym->type;
                        }
                    }
                }
                break;
            }
            case AST_INTERFACE_DEF: {
                if (d->list.len == 0)
                    SERR(s, 58, &d->span,
                         "empty interface '%s' (declare at least one method)", d->name);
                symbol_t *sym = scope_lookup_local(s->global, d->name);
                if (!sym) break;
                {
                    size_t j = 0;
                    for (; j < d->list.len; j++) {
                        ast_node_t *m = (ast_node_t *)d->list.data[j];
                        if (m->kind != AST_FUNC_DEF) continue;
                        symbol_t *mm =
                            get_or_make_func(s, sym->members, m->name, SYM_METHOD);
                        if (!mm->decl) mm->decl = m;
                        vec_push(s->a, &mm->overloads, build_sig(s, m, sym));
                    }
                }
                break;
            }
            case AST_IMPL_DEF: {
                type_t *tt = sema_resolve_type(s, d->type);
                if (!tt || tt->kind == TY_ERROR) break;
                const char *ts = type_to_string(s->tc, tt);
                d->type->type_str = ts;
                const char *key = impl_owner_key(s->a, ts);
                symbol_t *owner = scope_lookup_local(s->global, key);
                if (!owner) {
                    owner = symbol_new(s->a, SYM_TYPEIMPL, key);
                    owner->type = tt;
                    owner->members = scope_new(s->a, SCOPE_STRUCT, s->global);
                    owner->members->label = ts;
                    scope_define(s->a, s->global, owner);
                }
                symbol_t *iface = scope_lookup(s->global, d->name);
                if (!iface || iface->kind != SYM_INTERFACE)
                    SERR(s, 1, &d->span, "'%s' in `impl ... for ...` is not an interface",
                         d->name);
                {
                    size_t j = 0;
                    for (; j < d->list.len; j++) {
                        ast_node_t *m = (ast_node_t *)d->list.data[j];
                        if (m->kind != AST_FUNC_DEF) continue;
                        symbol_t *mm =
                            get_or_make_func(s, owner->members, m->name, SYM_METHOD);
                        if (!mm->decl) mm->decl = m;
                        vec_push(s->a, &mm->overloads, build_sig(s, m, owner));
                        mm->type = owner->type;
                    }
                }
                break;
            }
            case AST_ENUM_DEF: {
                if (d->list.len == 0)
                    SERR(s, 58, &d->span, "empty enum '%s' (declare at least one member)",
                         d->name);
                symbol_t *sym = scope_lookup_local(s->global, d->name);
                long long next = 0;
                {
                    size_t j = 0;
                    for (; j < d->list.len; j++) {
                        ast_node_t *m = (ast_node_t *)d->list.data[j];
                        symbol_t *em = symbol_new(s->a, SYM_ENUM_MEMBER, m->name);
                        if (m->a && m->a->kind == AST_LITERAL &&
                            m->a->value.kind == TV_INT)
                            next = (long long)m->a->value.as.i;
                        em->enum_value = next++;
                        em->type = sym->type;
                        if (scope_define(s->a, sym->members, em))
                            SERR(s, 1, &m->span, "duplicate enum member '%s'", m->name);
                    }
                }
                break;
            }
            case AST_FUNC_DEF: {
                symbol_t *fsym = get_or_make_func(s, s->global, d->name, SYM_FUNC);
                if (!fsym->decl) fsym->decl = d;
                if (d->is_pub) fsym->is_pub = true;
                if (d->typarams.len > 0) break;
                vec_push(s->a, &fsym->overloads, build_sig(s, d, NULL));
                break;
            }
            case AST_VAR_DECL:
            case AST_CONST_DECL: {
                symbol_t *g = symbol_new(
                    s->a, d->kind == AST_CONST_DECL ? SYM_CONST : SYM_VAR, d->name);
                g->type = d->type ? sema_resolve_type(s, d->type) : NULL;
                g->is_mut = d->is_mut;
                g->is_pub = d->is_pub;
                g->decl = d;
                if (d->kind == AST_CONST_DECL && d->a && d->a->kind == AST_LITERAL &&
                    d->a->value.kind == TV_INT) {
                    g->has_ival = true;
                    g->ival = (long long)d->a->value.as.i;
                }
                if (scope_define(s->a, s->global, g))
                    SERR(s, 1, &d->span, "redefinition of global '%s'", d->name);
                break;
            }
            default:
                break;
            }
        }
    }
}

static func_sig_t *find_sig(symbol_t *fsym, ast_node_t *decl)
{
    {
        size_t i = 0;
        for (; i < fsym->overloads.len; i++) {
            func_sig_t *sig = (func_sig_t *)fsym->overloads.data[i];
            if (sig->decl == decl) return sig;
        }
    }
    return NULL;
}

static void check_function(sema_t *s, ast_node_t *fn, symbol_t *owner, func_sig_t *sig)
{
    scope_t *home = NULL;
    if (!owner) {
        symbol_t *fs = scope_lookup_local(s->global, fn->name);
        if (fs && fs->home) home = fs->home;
    }
    scope_t *parent = owner ? owner->members : (home ? home : s->global);
    scope_t *sc = scope_new(s->a, SCOPE_FUNC, parent);
    sc->label = fn->name;
    sc->func = sig;
    scope_t *saved = s->cur;
    type_t *saved_self = s->self_type;
    func_sig_t *saved_func = s->cur_func;
    scope_t *saved_gp = s->gen_pkg;
    s->cur = sc;
    s->cur_func = sig;
    if (home) s->gen_pkg = home;
    {
        bool seen_default = false;
        size_t i = 0;
        for (; i < fn->list.len; i++) {
            ast_node_t *param = (ast_node_t *)fn->list.data[i];
            if (!param->a) {
                if (seen_default)
                    SERR(s, 64, &param->span,
                         "parameter '%s' without a default value cannot follow a "
                         "parameter with one",
                         param->name);
                continue;
            }
            seen_default = true;
            if (fn->is_extern)
                SERR(s, 64, &param->span,
                     "extern parameter '%s' cannot have a default value", param->name);
            type_t *pt = (sig && i < sig->params.len) ? (type_t *)sig->params.data[i]
                                                      : sema_resolve_type(s, param->type);
            if (param->a->kind == AST_STRUCT_LIT && pt && pt->kind == TY_STRUCT)
                s->expected = pt;
            else if (param->a->kind == AST_LITERAL && pt)
                s->expected = pt;
            type_t *dt = sema_check_expr(s, param->a);
            if (pt && dt && !type_assignable(pt, dt))
                SERR(s, 2, &param->span,
                     "default for parameter '%s': cannot assign '%s' to '%s'",
                     param->name, type_to_string(s->tc, dt), type_to_string(s->tc, pt));
        }
    }
    if (owner) {
        s->self_type =
            (owner->kind == SYM_TYPEIMPL) ? owner->type : type_ptr(s->tc, owner->type);
        sc->self_type = s->self_type;
        symbol_t *th = symbol_new(s->a, SYM_PARAM, "this");
        th->type = s->self_type;
        scope_define(s->a, sc, th);
    } else {
        s->self_type = NULL;
    }
    {
        size_t i = 0;
        for (; i < fn->list.len; i++) {
            ast_node_t *param = (ast_node_t *)fn->list.data[i];
            symbol_t *ps = symbol_new(s->a, SYM_PARAM, param->name);

            ps->type = (sig && i < sig->params.len) ? (type_t *)sig->params.data[i]
                                                    : sema_resolve_type(s, param->type);
            ps->is_mut = false;
            ps->is_ref = param->is_ref;
            ps->decl = param;
            if (scope_define(s->a, sc, ps))
                SERR(s, 1, &param->span, "duplicate parameter '%s'", param->name);
        }
    }
    LOG_D(s->log, PH_SEMANTIC, "enter function '%s'%s", fn->name,
          owner ? i18n_tr(" (method)") : "");

    if (fn->a && fn->a->kind == AST_BLOCK && fn->a->list.len == 0)
        SERR(s, 61, &fn->span,
             "empty function '%s' (its body must contain at least one statement)",
             fn->name);
    if (fn->a) sema_check_block(s, fn->a);
    LOG_D(s->log, PH_SEMANTIC, "exit function '%s'", fn->name);

    if (fn->a) {
        {
            size_t i = 0;
            for (; i < sc->symbols.len; i++) {
                symbol_t *p = (symbol_t *)sc->symbols.data[i];
                if (p->kind == SYM_PARAM && !p->used && p->decl && p->name &&
                    p->name[0] != '_' && strcmp(p->name, "this") != 0)
                    SERR(s, 62, &p->decl->span,
                         "unused parameter '%s' (prefix with '_' if intentional)",
                         p->name);
            }
        }
    }
    s->cur = saved;
    s->self_type = saved_self;
    s->cur_func = saved_func;
    s->gen_pkg = saved_gp;
}

static void check_toplevel(sema_t *s, ast_node_t *d)
{
    switch (d->kind) {
    case AST_FUNC_DEF: {
        symbol_t *fsym = scope_lookup_local(s->global, d->name);
        check_function(s, d, NULL, find_sig(fsym, d));
        break;
    }
    case AST_STRUCT_DEF: {
        symbol_t *ssym = scope_lookup_local(s->global, d->name);
        if (!ssym) break;
        {
            size_t j = 0;
            for (; j < d->list.len; j++) {
                ast_node_t *m = (ast_node_t *)d->list.data[j];
                if (m->kind == AST_FUNC_DEF) {
                    symbol_t *msym = scope_lookup_local(ssym->members, m->name);
                    check_function(s, m, ssym, find_sig(msym, m));
                } else if (m->kind == AST_FIELD && m->a) {
                    symbol_t *f = scope_lookup_local(ssym->members, m->name);
                    type_t *vt;
                    if (f && stamp_empty_intrinsic(s, m->a, f->type)) {
                        vt = f->type;
                    } else {
                        if (f && m->a->kind == AST_STRUCT_LIT && f->type &&
                            f->type->kind == TY_STRUCT)
                            s->expected = f->type;
                        else if (f && m->a->kind == AST_LITERAL)
                            s->expected = f->type;
                        vt = sema_check_expr(s, m->a);
                    }
                    if (f && !type_assignable(f->type, vt))
                        SERR(s, 2, &m->span,
                             "default for field '%s': cannot assign '%s' to '%s'",
                             m->name, type_to_string(s->tc, vt),
                             type_to_string(s->tc, f->type));
                }
            }
        }
        break;
    }
    case AST_IMPL_DEF: {
        type_t *tt = sema_resolve_type(s, d->type);
        if (!tt) break;
        symbol_t *owner = scope_lookup_local(
            s->global, impl_owner_key(s->a, type_to_string(s->tc, tt)));
        if (!owner) break;
        {
            size_t j = 0;
            for (; j < d->list.len; j++) {
                ast_node_t *m = (ast_node_t *)d->list.data[j];
                if (m->kind != AST_FUNC_DEF) continue;
                symbol_t *msym = scope_lookup_local(owner->members, m->name);
                check_function(s, m, owner, find_sig(msym, m));
            }
        }
        break;
    }
    case AST_VAR_DECL:
    case AST_CONST_DECL: {
        symbol_t *g = scope_lookup_local(s->global, d->name);
        type_t *t = sema_check_var_decl(s, d);
        if (g && !g->type) g->type = t;
        break;
    }
    case AST_LAYOUT_BLOCK:
        sema_check_layout(s, d, NULL);
        break;
    default:
        break;
    }
}

/*
 * Enforce that every primitive type name a file writes matches that file's own
 * language: a Persian source uses Persian type names (صحیح۳۲, رشته, ...) and an
 * English source uses English ones. Only the module's own authored annotations
 * are visited (synthetic generic instances and imported packages are checked in
 * their own language elsewhere), so interop with the English stdlib is fine.
 */
static void lint_lang_types(sema_t *s, ast_node_t *n)
{
    if (!n) return;
    if (n->kind == AST_TYPE && n->name && type_prim_kind_from_name(n->name, NULL) >= 0 &&
        type_prim_kind_from_name(n->name, s->lang) < 0) {
        bool fa = s->lang && s->lang[0] == 'f';
        SERR(s, 1, &n->span,
             fa ? i18n_tr("type name '%s' must be Persian in a Persian file")
                : i18n_tr("type name '%s' must be English in an English file"),
             n->name);
    }
    lint_lang_types(s, n->type);
    lint_lang_types(s, n->a);
    lint_lang_types(s, n->b);
    lint_lang_types(s, n->c);
    lint_lang_types(s, n->d);
    {
        size_t i = 0;
        for (; i < n->list.len; i++)
            lint_lang_types(s, (ast_node_t *)n->list.data[i]);
    }
    {
        size_t i = 0;
        for (; i < n->dims.len; i++)
            lint_lang_types(s, (ast_node_t *)n->dims.data[i]);
    }
}

void sema_check_pass(sema_t *s, ast_node_t *program)
{
    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (d->synthetic) continue;
            lint_lang_types(s, d);
        }
    }
    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (d->synthetic || d->typarams.len > 0) continue;
            check_toplevel(s, d);
        }
    }

    {
        size_t i = 0;
        for (; i < s->pending.len; i++)
            check_toplevel(s, (ast_node_t *)s->pending.data[i]);
    }
}
