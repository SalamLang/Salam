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
#include "core/sal_format.h"
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

static size_t resolve_array_dim(sema_t *s, ast_node_t *dim)
{
    if (!dim) return 0;

    if (dim->kind == AST_LITERAL && dim->value.kind == TV_INT)
        return (size_t)dim->value.as.i;

    if (dim->kind == AST_IDENTIFIER) {
        symbol_t *cs = scope_lookup(s->cur, dim->name);
        if (cs && cs->kind == SYM_CONST && cs->has_ival) return (size_t)cs->ival;
        SERR(s, 11, &dim->span, "array size must be a constant integer");
        return 0;
    }

    if (dim->kind == AST_MEMBER && dim->a && dim->a->kind == AST_IDENTIFIER) {
        symbol_t *pk = scope_lookup(s->cur, dim->a->name);
        if (pk && pk->kind == SYM_PACKAGE) {
            const char *mname = pkg_member_canon(s, pk, dim->name, &dim->span);
            symbol_t *cs = scope_lookup_local(pk->members, mname);
            if (!cs) {
                SERR(s, 16, &dim->span, "package '%s' has no exported member '%s'",
                     pk->name, mname);
                return 0;
            }
            if (!cs->is_pub) {
                SERR(s, 17, &dim->span,
                     "'%s' is not exported by package '%s' (mark it 'pub')", mname,
                     pk->name);
                return 0;
            }
            if (cs->kind == SYM_CONST && cs->has_ival) return (size_t)cs->ival;
        }
        SERR(s, 11, &dim->span, "array size must be a constant integer");
        return 0;
    }

    SERR(s, 11, &dim->span, "array size must be a constant integer");
    return 0;
}

/*
 * Wrap a resolved base in whatever the type node's suffixes asked for, in the
 * one order that makes both spellings mean what they read as:
 *   Edge*[6]  ->  elem_ptr_depth, so the star binds first: array of pointers
 *   Edge[6]*  ->  ptr_depth, so the star binds last: pointer to the array
 * Every branch of sema_resolve_type routes through here; branches that used to
 * apply only is_pointer silently dropped the dims of a type like
 * `pkg.Type[3]`.
 */
static type_t *apply_type_suffixes(sema_t *s, ast_node_t *tnode, type_t *base)
{
    {
        int k = tnode->elem_ptr_depth;
        for (; k > 0; k--)
            base = type_ptr(s->tc, base);
    }
    {
        size_t i = tnode->dims.len;
        for (; i-- > 0;) {
            ast_node_t *dim = (ast_node_t *)tnode->dims.data[i];
            base = type_array(s->tc, base, resolve_array_dim(s, dim));
        }
    }
    if (tnode->is_slice) base = type_slice(s->tc, base);
    {
        /* is_pointer without a depth is a node the generic machinery or a
         * desugaring built by hand; treat it as one level. */
        int k = tnode->ptr_depth ? tnode->ptr_depth : (tnode->is_pointer ? 1 : 0);
        for (; k > 0; k--)
            base = type_ptr(s->tc, base);
    }
    return base;
}

type_t *sema_resolve_type(sema_t *s, ast_node_t *tnode)
{
    if (!tnode) return ty(s, TY_VOID);
    /* A type the generic machinery resolved in the caller's scope and is
     * carrying into a package's scope, where the name would not resolve. See
     * ast_node_t::sema_type. */
    if (tnode->sema_type) return (type_t *)tnode->sema_type;
    type_t *base;

    if (tnode->is_dyn) {
        const char *why = NULL;
        symbol_t *iface = sema_lookup_iface(s, tnode->name, &tnode->span, &why);
        if (!iface) {
            if (why)
                SERR(s, 1, &tnode->span, "'%s' after 'dyn': %s", tnode->name, why);
            else
                SERR(s, 1, &tnode->span,
                     "'%s' is not an interface (required after 'dyn')", tnode->name);
            return err_ty(s);
        }
        /* Name the type after the interface itself, never after the spelling
         * used at this site, so "Connection" and "db.Connection" agree. */
        const char *iname = iface->name;
        if (iface->pkgname && iface->pkgname[0] && strcmp(iface->pkgname, "main") != 0) {
            char buf[512];
            sal_snprintf(buf, sizeof buf, "%s_%s", iface->pkgname, iface->name);
            iname = arena_strdup(s->a, buf);
        }
        {
            type_t *dt = type_dyn(s->tc, iface, iname);
            return apply_type_suffixes(s, tnode, dt);
        }
    }

    const char *dot = tnode->name ? strchr(tnode->name, '.') : NULL;
    if (dot) {
        char pname[64];
        size_t pl = (size_t)(dot - tnode->name);
        if (pl >= sizeof(pname)) pl = sizeof(pname) - 1;
        memcpy(pname, tnode->name, pl);
        pname[pl] = 0;
        const char *tname = dot + 1;
        symbol_t *pk2 = scope_lookup(s->cur, pname);
        if (!pk2 || pk2->kind != SYM_PACKAGE) {
            SERR(s, 1, &tnode->span, "unknown package '%s' in type '%s'", pname,
                 tnode->name);
            return err_ty(s);
        }
        tname = pkg_member_canon(s, pk2, tname, &tnode->span);
        symbol_t *tsym = scope_lookup_local(pk2->members, tname);
        if (!tsym || (tsym->kind != SYM_STRUCT && tsym->kind != SYM_ENUM &&
                      tsym->kind != SYM_ALIAS)) {
            SERR(s, 1, &tnode->span, "package '%s' has no exported type '%s'", pname,
                 tname);
            return err_ty(s);
        }
        if (!tsym->is_pub) {
            SERR(s, 17, &tnode->span,
                 "'%s' is not exported by package '%s' (mark it 'pub')", tname, pname);
            return err_ty(s);
        }

        if (tsym->kind == SYM_STRUCT && tsym->decl && tsym->decl->typarams.len > 0) {
            if (tnode->list.len == 0) {
                SERR(s, 1, &tnode->span, "generic type '%s' requires type arguments",
                     tnode->name);
                return err_ty(s);
            }
            scope_t *save_cur = s->cur;
            scope_t *save_gp = s->gen_pkg;
            s->cur = pk2->members;
            s->gen_pkg = pk2->members;
            symbol_t *inst =
                g_instantiate_struct(s, tsym->decl, &tnode->list, &tnode->span);
            s->cur = save_cur;
            s->gen_pkg = save_gp;
            base = inst ? inst->type : err_ty(s);
            return apply_type_suffixes(s, tnode, base);
        }
        base = tsym->type;
        return apply_type_suffixes(s, tnode, base);
    }

    if (tnode->name && type_prim_kind_from_name(tnode->name, NULL) < 0) {
        const char *orig = tnode->name;
        tnode->name = intrinsic_type_canon(local_canon(s, tnode->name, &tnode->span));
        if (!tnode->synthetic) sema_check_intrinsic_type_lang(s, orig, &tnode->span);
    }

    if (tnode->name && strcmp(tnode->name, "func") == 0) {
        vec_t ptypes;
        vec_init(&ptypes);
        {
            size_t i = 0;
            for (; i < tnode->list.len; i++)
                vec_push(s->a, &ptypes,
                         sema_resolve_type(s, (ast_node_t *)tnode->list.data[i]));
        }
        type_t *ret = tnode->type ? sema_resolve_type(s, tnode->type) : ty(s, TY_VOID);
        base = type_func(s->tc, ret, &ptypes);
        if (tnode->is_extern) base->length = 1;
        base = apply_type_suffixes(s, tnode, base);
        decorate(s, tnode, base);
        return base;
    }
    int pk = tnode->name ? type_prim_kind_from_name(tnode->name, NULL) : -1;

    if (tnode->sema_base_type) {
        /* `T*`/`T[N]` in a template the generic machinery substituted into.
         * The name would not resolve here; the base already did, in the scope
         * that named it. See ast_node_t::sema_base_type. */
        base = (type_t *)tnode->sema_base_type;
    } else if (tnode->name && strcmp(tnode->name, "File") == 0) {
        base = type_file(s->tc);
    } else if (tnode->name && strcmp(tnode->name, "Variant") == 0) {
        if (tnode->list.len < 2) {
            SERR(s, 1, &tnode->span,
                 "'Variant' requires at least 2 type arguments (got %zu)",
                 tnode->list.len);
            base = err_ty(s);
        } else {
            type_t *members[64];
            size_t n = tnode->list.len;
            if (n > 64) n = 64;
            {
                size_t i = 0;
                for (; i < n; i++) {
                    members[i] = sema_resolve_type(s, (ast_node_t *)tnode->list.data[i]);
                    if (members[i] && members[i]->kind == TY_VARIANT)
                        SERR(s, 1, &tnode->span,
                             "'Variant' cannot contain another 'Variant' as a member");
                }
            }
            base = type_variant(s->tc, members, n);
        }
    } else if (pk >= 0) {
        base = ty(s, (type_kind_t)pk);
    } else if (generic_template(s, tnode->name, SYM_STRUCT)) {
        symbol_t *gt = generic_template(s, tnode->name, SYM_STRUCT);

        scope_t *save_gp = s->gen_pkg;
        if (gt->home) s->gen_pkg = gt->home;
        symbol_t *inst = g_instantiate_struct(s, gt->decl, &tnode->list, &tnode->span);
        s->gen_pkg = save_gp;
        base = inst ? inst->type : err_ty(s);
    } else {
        symbol_t *sym = tnode->name ? scope_lookup(s->cur, tnode->name) : NULL;
        scope_t *home = sym ? sym->home : NULL;

        if (!sym && tnode->name) {
            sema_load_prelude(s);
            if (s->prelude) {
                sym = scope_lookup_local(s->prelude, tnode->name);
                home = s->prelude;
            }
        }
        if (sym && sym->decl && sym->decl->typarams.len > 0) {
            if (tnode->list.len == 0) {
                SERR(s, 1, &tnode->span, "generic type '%s' requires type arguments",
                     tnode->name);
                base = err_ty(s);
            } else {
                scope_t *save_gp = s->gen_pkg;
                if (home) s->gen_pkg = home;
                symbol_t *inst =
                    g_instantiate_struct(s, sym->decl, &tnode->list, &tnode->span);
                s->gen_pkg = save_gp;
                base = inst ? inst->type : err_ty(s);
            }
        } else if (sym && (sym->kind == SYM_STRUCT || sym->kind == SYM_ENUM ||
                           sym->kind == SYM_ALIAS)) {
            base = sym->type;
        } else {
            SERR(s, 1, &tnode->span, "unknown type '%s'",
                 tnode->name ? tnode->name : "?");
            base = err_ty(s);
        }
    }

    base = apply_type_suffixes(s, tnode, base);
    decorate(s, tnode, base);
    return base;
}
