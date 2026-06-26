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

static type_t *ty(sema_t *s, type_kind_t k) { return sema_ty(s, k); }

static type_t *err_ty(sema_t *s)            { return sema_err_ty(s); }

static type_t *decorate(sema_t *s, ast_node_t *n, type_t *t) { return sema_decorate(s, n, t); }

type_t *sema_resolve_type(sema_t *s, ast_node_t *tnode)
{
    if (!tnode) return ty(s, TY_VOID);
    type_t *base;
    
    if (tnode->is_dyn) {
        symbol_t *iface = scope_lookup(s->cur, tnode->name);
        if (!iface || iface->kind != SYM_INTERFACE) {
            SERR(s, 1, &tnode->span, "'%s' is not an interface (required after 'dyn')", tnode->name);
            return err_ty(s);
        }
        type_t *dt = type_dyn(s->tc, iface, tnode->name);
        { size_t i = tnode->dims.len; for (; i-- > 0; ) {       
            ast_node_t *dim = (ast_node_t *)tnode->dims.data[i];
            size_t len = 0;
            if (dim && dim->kind == AST_LITERAL && dim->value.kind == TV_INT) len = (size_t)dim->value.as.i;
            else if (dim && dim->kind == AST_IDENTIFIER) {
                symbol_t *cs = scope_lookup(s->cur, dim->name);
                if (cs && cs->kind == SYM_CONST && cs->has_ival) len = (size_t)cs->ival;
                else SERR(s, 11, &dim->span, "array size must be a constant integer");
            } else if (dim) SERR(s, 11, &dim->span, "array size must be a constant integer");
            dt = type_array(s->tc, dt, len);
        } }
        if (tnode->is_pointer) dt = type_ptr(s->tc, dt);   
        return dt;
    }
    
    const char *dot = tnode->name ? strchr(tnode->name, '.') : NULL;
    if (dot) {
        char pname[64]; size_t pl = (size_t)(dot - tnode->name);
        if (pl >= sizeof(pname)) pl = sizeof(pname) - 1;
        memcpy(pname, tnode->name, pl); pname[pl] = 0;
        const char *tname = dot + 1;
        symbol_t *pk2 = scope_lookup(s->cur, pname);
        if (!pk2 || pk2->kind != SYM_PACKAGE) {
            SERR(s, 1, &tnode->span, "unknown package '%s' in type '%s'", pname, tnode->name);
            return err_ty(s);
        }
        tname = pkg_member_canon(pk2, tname);   
        symbol_t *tsym = scope_lookup_local(pk2->members, tname);
        if (!tsym || (tsym->kind != SYM_STRUCT && tsym->kind != SYM_ENUM && tsym->kind != SYM_ALIAS)) {
            SERR(s, 1, &tnode->span, "package '%s' has no exported type '%s'", pname, tname);
            return err_ty(s);
        }
        if (!tsym->is_pub) {
            SERR(s, 17, &tnode->span, "'%s' is not exported by package '%s' (mark it 'pub')", tname, pname);
            return err_ty(s);
        }
        
        if (tsym->kind == SYM_STRUCT && tsym->decl && tsym->decl->typarams.len > 0) {
            if (tnode->list.len == 0) {
                SERR(s, 1, &tnode->span, "generic type '%s' requires type arguments", tnode->name);
                return err_ty(s);
            }
            scope_t *save_cur = s->cur; scope_t *save_gp = s->gen_pkg;
            s->cur = pk2->members; s->gen_pkg = pk2->members;
            symbol_t *inst = g_instantiate_struct(s, tsym->decl, &tnode->list, &tnode->span);
            s->cur = save_cur; s->gen_pkg = save_gp;
            base = inst ? inst->type : err_ty(s);
            if (tnode->is_pointer) base = type_ptr(s->tc, base);
            return base;
        }
        base = tsym->type;
        if (tnode->is_pointer) base = type_ptr(s->tc, base);
        return base;
    }
    
    if (tnode->name) tnode->name = intrinsic_type_canon(local_canon(s, tnode->name));
    
    if (tnode->name && strcmp(tnode->name, "func") == 0) {
        vec_t ptypes; vec_init(&ptypes);
        { size_t i = 0; for (; i < tnode->list.len; i++)
            vec_push(s->a, &ptypes, sema_resolve_type(s, (ast_node_t *)tnode->list.data[i])); }
        type_t *ret = tnode->type ? sema_resolve_type(s, tnode->type) : ty(s, TY_VOID);
        base = type_func(s->tc, ret, &ptypes);
        if (tnode->is_pointer) base = type_ptr(s->tc, base);
        decorate(s, tnode, base);
        return base;
    }
    int pk = tnode->name ? type_prim_kind_from_name(tnode->name) : -1;
    
    if (tnode->name && strcmp(tnode->name, "File") == 0) {
        base = type_file(s->tc);
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
                SERR(s, 1, &tnode->span, "generic type '%s' requires type arguments", tnode->name);
                base = err_ty(s);
            } else {
                scope_t *save_gp = s->gen_pkg;
                if (home) s->gen_pkg = home;
                symbol_t *inst = g_instantiate_struct(s, sym->decl, &tnode->list, &tnode->span);
                s->gen_pkg = save_gp;
                base = inst ? inst->type : err_ty(s);
            }
        } else if (sym && (sym->kind == SYM_STRUCT || sym->kind == SYM_ENUM || sym->kind == SYM_ALIAS)) {
            base = sym->type;
        } else {
            SERR(s, 1, &tnode->span, "unknown type '%s'", tnode->name ? tnode->name : "?");
            base = err_ty(s);
        }
    }
    
    { size_t i = tnode->dims.len; for (; i-- > 0; ) {
        ast_node_t *dim = (ast_node_t *)tnode->dims.data[i];
        size_t len = 0;
        if (dim && dim->kind == AST_LITERAL && dim->value.kind == TV_INT) {
            len = (size_t)dim->value.as.i;
        } else if (dim && dim->kind == AST_IDENTIFIER) {     
            symbol_t *cs = scope_lookup(s->cur, dim->name);
            if (cs && cs->kind == SYM_CONST && cs->has_ival) len = (size_t)cs->ival;
            else SERR(s, 11, &dim->span, "array size must be a constant integer");
        } else if (dim) {
            SERR(s, 11, &dim->span, "array size must be a constant integer");
        }
        base = type_array(s->tc, base, len);
    } }
    if (tnode->is_pointer) base = type_ptr(s->tc, base);
    decorate(s, tnode, base);
    return base;
}
