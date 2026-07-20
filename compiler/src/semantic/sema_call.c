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
#include "semantic/builtins.h"
#include "semantic/dce.h"

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

static func_sig_t *ro_found(sema_t *s, func_sig_t *sig, const src_span_t *span,
                            const char *what)
{
    if (sig && sig->decl && sig->decl->is_deprecated)
        SWARN(s, 13, span, "call to deprecated function '%s'", what);
    if (sig && sig->infer_ret) {
        if (sig->in_check)
            SERR(s, 12, span,
                 "cannot infer the return type of recursive function '%s'; annotate "
                 "its return type",
                 what);
        else
            sema_check_function_now(s, sig);
    }
    {
        ast_node_t *pf = sema_pure_fn(s);
        if (pf && sig && sig->decl && !sig->decl->is_pure)
            SERR(s, 12, span,
                 "'pure' function '%s' cannot call '%s': only 'pure' functions may be "
                 "called from a pure function",
                 pf->name, what);
    }
    return sig;
}

func_sig_t *resolve_overload(sema_t *s, symbol_t *fsym, vec_t *argtypes,
                             const src_span_t *span, const char *what)
{
    func_sig_t *match = NULL;
    size_t nmatch = 0;

    {
        size_t i = 0;
        for (; i < fsym->overloads.len; i++) {
            func_sig_t *sig = (func_sig_t *)fsym->overloads.data[i];
            if (argtypes->len < sig->required) continue;
            if (!sig->variadic && argtypes->len > sig->params.len) continue;
            size_t nfix =
                argtypes->len < sig->params.len ? argtypes->len : sig->params.len;
            bool ok = true;
            {
                size_t j = 0;
                for (; j < nfix; j++) {
                    if (!type_equiv((type_t *)sig->params.data[j],
                                    (type_t *)argtypes->data[j])) {
                        ok = false;
                        break;
                    }
                }
            }
            if (ok) {
                match = sig;
                nmatch++;
            }
        }
    }
    if (nmatch == 1) return ro_found(s, match, span, what);
    if (nmatch > 1) {
        SERR(s, 12, span, "ambiguous call to '%s' (%zu candidates)", what, nmatch);
        return NULL;
    }

    {
        size_t i = 0;
        for (; i < fsym->overloads.len; i++) {
            func_sig_t *sig = (func_sig_t *)fsym->overloads.data[i];
            if (argtypes->len < sig->required) continue;
            if (!sig->variadic && argtypes->len > sig->params.len) continue;
            size_t nfix =
                argtypes->len < sig->params.len ? argtypes->len : sig->params.len;
            bool ok = true;
            {
                size_t j = 0;
                for (; j < nfix; j++) {
                    if (!type_assignable((type_t *)sig->params.data[j],
                                         (type_t *)argtypes->data[j])) {
                        ok = false;
                        break;
                    }
                }
            }
            if (ok) {
                match = sig;
                nmatch++;
            }
        }
    }
    if (nmatch == 1) return ro_found(s, match, span, what);
    if (nmatch > 1) {
        SERR(s, 12, span, "ambiguous call to '%s' (%zu candidates)", what, nmatch);
        return NULL;
    }
    if (fsym->overloads.len == 1) {
        func_sig_t *sig = (func_sig_t *)fsym->overloads.data[0];
        if (argtypes->len < sig->required ||
            (!sig->variadic && argtypes->len > sig->params.len)) {
            if (sig->variadic)
                SERR(s, 12, span, "'%s' expects at least %zu argument(s), got %zu", what,
                     sig->required, argtypes->len);
            else if (sig->required == sig->params.len)
                SERR(s, 12, span, "'%s' expects %zu argument(s), got %zu", what,
                     sig->params.len, argtypes->len);
            else
                SERR(s, 12, span, "'%s' expects %zu to %zu argument(s), got %zu", what,
                     sig->required, sig->params.len, argtypes->len);
            return NULL;
        }
        size_t nfix = argtypes->len < sig->params.len ? argtypes->len : sig->params.len;
        {
            size_t j = 0;
            for (; j < nfix; j++) {
                if (!type_assignable((type_t *)sig->params.data[j],
                                     (type_t *)argtypes->data[j])) {
                    SERR(s, 12, span, "argument %zu: cannot pass '%s' to '%s'", j + 1,
                         type_to_string(s->tc, (type_t *)argtypes->data[j]),
                         type_to_string(s->tc, (type_t *)sig->params.data[j]));
                    return NULL;
                }
            }
        }
    }
    SERR(s, 12, span, "no matching overload for '%s' with %zu argument(s)", what,
         argtypes->len);
    return NULL;
}

static void mark_ref_args(sema_t *s, ast_node_t *call, func_sig_t *sig)
{
    if (!sig || !sig->decl) return;
    size_t i = 0;
    for (; i < call->list.len && i < sig->decl->list.len; i++) {
        ast_node_t *p = (ast_node_t *)sig->decl->list.data[i];
        ast_node_t *arg = (ast_node_t *)call->list.data[i];
        if (p->kind != AST_PARAM || !p->is_ref) continue;
        if (arg && arg->kind == AST_IDENTIFIER && arg->name) {
            symbol_t *sym = scope_lookup(s->cur, arg->name);
            if (sym) sym->mutated = true;
        }
    }
}

static type_t *try_impl_call(sema_t *s, ast_node_t *n, ast_node_t *callee, type_t *objt,
                             vec_t *argtypes)
{
    symbol_t *owner =
        scope_lookup(s->global, impl_owner_key(s->a, type_to_string(s->tc, objt)));
    if (!owner) return NULL;
    symbol_t *m = scope_lookup_local(owner->members, callee->name);
    if (!m || m->kind != SYM_METHOD) return NULL;
    func_sig_t *sig = resolve_overload(s, m, argtypes, &n->span, callee->name);
    mark_ref_args(s, n, sig);
    decorate(s, callee, m->type);
    return decorate(s, n, sig ? sig->ret : err_ty(s));
}

static void check_pure_recv_mut(sema_t *s, ast_node_t *n, const char *m)
{
    ast_node_t *pf = sema_pure_fn(s);
    if (pf)
        SERR(s, 12, &n->span,
             "'pure' function '%s' cannot call '%s': it modifies its receiver", pf->name,
             m);
}

static void check_pure_builtin(sema_t *s, ast_node_t *n, const char *nm)
{
    static const char *impure[] = {"print", "println",     "printerr", "printerrln",
                                   "input", "open",        "args",     "listdir",
                                   "spawn", "callhandler", "join",     NULL};
    ast_node_t *pf = sema_pure_fn(s);
    if (!pf || !nm) return;
    {
        size_t i = 0;
        for (; impure[i]; i++) {
            if (strcmp(nm, impure[i]) == 0) {
                SERR(s, 12, &n->span,
                     "'pure' function '%s' cannot call '%s': it has side effects",
                     pf->name, nm);
                return;
            }
        }
    }
}

static bool dce_caller_rooted(sema_t *s)
{
    if (!s->cur_func) return true;
    if (s->cur_func->owner) return true;
    if (!s->cur_func->decl) return true;
    if (s->cur_func->decl->typarams.len > 0) return true;
    if (s->cur_func->decl->is_extern) return true;
    if (s->cur_func->decl->synthetic) return true;
    return false;
}

static void dce_note(sema_t *s, const char *callee_pkg, const char *callee_fn)
{
    if (!dce_enabled()) return;
    if (dce_caller_rooted(s)) {
        dce_mark_root(callee_pkg, callee_fn);
        return;
    }
    dce_note_call(s->pkg, s->cur_func->decl->name, callee_pkg, callee_fn);
}

type_t *check_call(sema_t *s, ast_node_t *n)
{
    ast_node_t *callee = n->a;
    type_t *call_expected = s->expected;
    s->expected = NULL;

    if (callee && callee->kind == AST_IDENTIFIER) check_pure_builtin(s, n, callee->name);

    if (callee && callee->kind == AST_IDENTIFIER && strcmp(callee->name, "spawn") == 0) {
        if (n->list.len != 1 || ((ast_node_t *)n->list.data[0])->kind != AST_IDENTIFIER) {
            SERR(s, 12, &n->span, "spawn(func) takes a single function name");
            return decorate(s, n, ty(s, TY_I64));
        }
        ast_node_t *fn = (ast_node_t *)n->list.data[0];
        symbol_t *fsym = scope_lookup(s->cur, fn->name);
        if (!fsym || fsym->kind != SYM_FUNC) {
            SERR(s, 12, &n->span, "spawn argument '%s' is not a function", fn->name);
            return decorate(s, n, ty(s, TY_I64));
        }
        fsym->used = true;
        dce_mark_root(s->pkg, fn->name);
        bool ok = false;
        {
            size_t i = 0;
            for (; i < fsym->overloads.len; i++) {
                func_sig_t *sig = (func_sig_t *)fsym->overloads.data[i];
                if (sig->params.len == 0 && sig->ret && sig->ret->kind == TY_VOID)
                    ok = true;
            }
        }
        if (!ok)
            SERR(s, 12, &n->span,
                 "spawn requires a function taking no arguments and returning nothing");
        decorate(s, fn, ty(s, TY_VOID));
        decorate(s, callee, ty(s, TY_VOID));
        return decorate(s, n, ty(s, TY_I64));
    }

    if (callee && callee->kind == AST_IDENTIFIER &&
        strcmp(callee->name, "funcptr") == 0) {
        if (n->list.len != 1 || ((ast_node_t *)n->list.data[0])->kind != AST_IDENTIFIER) {
            SERR(s, 12, &n->span, "funcptr(func) takes a single function name");
            return decorate(s, n, ty(s, TY_I64));
        }
        ast_node_t *fn = (ast_node_t *)n->list.data[0];
        symbol_t *fsym = scope_lookup(s->cur, fn->name);
        if (!fsym || fsym->kind != SYM_FUNC) {
            SERR(s, 12, &n->span, "funcptr argument '%s' is not a function", fn->name);
            return decorate(s, n, ty(s, TY_I64));
        }
        fsym->used = true;
        dce_mark_root(s->pkg, fn->name);
        decorate(s, fn, ty(s, TY_VOID));
        decorate(s, callee, ty(s, TY_VOID));
        return decorate(s, n, ty(s, TY_I64));
    }

    if (callee && callee->kind == AST_IDENTIFIER &&
        strcmp(callee->name, "callhandler") == 0) {
        if (n->list.len != 2) {
            SERR(s, 12, &n->span,
                 "callhandler(fn, arg) takes a function pointer and an i64 argument");
            return decorate(s, n, ty(s, TY_VOID));
        }
        sema_check_expr(s, (ast_node_t *)n->list.data[0]);
        sema_check_expr(s, (ast_node_t *)n->list.data[1]);
        decorate(s, callee, ty(s, TY_VOID));
        return decorate(s, n, ty(s, TY_VOID));
    }

    if (callee && callee->kind == AST_IDENTIFIER && strcmp(callee->name, "sizeof") == 0) {
        if (n->list.len != 1 || ((ast_node_t *)n->list.data[0])->kind != AST_TYPE) {
            SERR(s, 12, &n->span, "sizeof(Type) takes a single type operand");
            return decorate(s, n, ty(s, TY_U64));
        }
        ast_node_t *op = (ast_node_t *)n->list.data[0];
        type_t *ot = sema_resolve_type(s, op);
        decorate(s, op, ot);
        decorate(s, callee, ty(s, TY_VOID));
        return decorate(s, n, ty(s, TY_U64));
    }

    vec_t argtypes;
    vec_init(&argtypes);
    {
        size_t i = 0;
        for (; i < n->list.len; i++)
            vec_push(s->a, &argtypes, sema_check_expr(s, (ast_node_t *)n->list.data[i]));
    }

    {
        type_t *ft = NULL;
        if (callee && callee->kind == AST_IDENTIFIER) {
            symbol_t *fv = scope_lookup(s->cur, callee->name);
            if (fv && fv->type && fv->type->kind == TY_FUNC &&
                (fv->kind == SYM_VAR || fv->kind == SYM_PARAM || fv->kind == SYM_CONST)) {
                ft = fv->type;
                fv->used = true;
            }
        } else if (callee && callee->kind != AST_MEMBER) {
            type_t *ct = sema_check_expr(s, callee);
            if (ct && ct->kind == TY_FUNC) ft = ct;
        }
        if (ft) {
            decorate(s, callee, ft);
            {
                ast_node_t *pf = sema_pure_fn(s);
                if (pf)
                    SERR(s, 12, &n->span,
                         "'pure' function '%s' cannot call a function value: its "
                         "purity cannot be verified",
                         pf->name);
            }
            if (argtypes.len != ft->params.len)
                SERR(s, 12, &n->span, "function value takes %zu argument(s), got %zu",
                     ft->params.len, argtypes.len);
            else {
                size_t i = 0;
                for (; i < argtypes.len; i++)
                    if (!type_assignable((type_t *)ft->params.data[i],
                                         (type_t *)argtypes.data[i]))
                        SERR(s, 2, &n->span, "argument %zu: cannot pass '%s' to '%s'",
                             i + 1, type_to_string(s->tc, (type_t *)argtypes.data[i]),
                             type_to_string(s->tc, (type_t *)ft->params.data[i]));
            }
            return decorate(s, n, ft->elem);
        }
    }

    if (callee && callee->kind == AST_IDENTIFIER) {
        const char *nm = callee->name;
        if (strcmp(nm, "print") == 0 || strcmp(nm, "println") == 0 ||
            strcmp(nm, "printerr") == 0 || strcmp(nm, "printerrln") == 0) {
            decorate(s, callee, ty(s, TY_VOID));
            return decorate(s, n, ty(s, TY_VOID));
        }
        if (strcmp(nm, "input") == 0) {
            decorate(s, callee, ty(s, TY_STR));
            return decorate(s, n, ty(s, TY_STR));
        }

        if (!strcmp(nm, "hash") && argtypes.len == 1) {
            decorate(s, callee, ty(s, TY_VOID));
            return decorate(s, n, ty(s, TY_U64));
        }

        if (!strcmp(nm, "open")) {
            if (argtypes.len != 2) SERR(s, 12, &n->span, "open(path, mode) takes 2 args");
            decorate(s, callee, ty(s, TY_VOID));
            return decorate(s, n, type_ptr(s->tc, type_file(s->tc)));
        }

        if (!strcmp(nm, "env")) {
            if (argtypes.len != 1) SERR(s, 12, &n->span, "env(name) takes 1 arg");
            decorate(s, callee, ty(s, TY_VOID));
            return decorate(s, n, ty(s, TY_STR));
        }
        if (!strcmp(nm, "args")) {
            if (argtypes.len != 0) SERR(s, 12, &n->span, "args() takes no arguments");
            decorate(s, callee, ty(s, TY_VOID));
            return decorate(s, n, sema_vec_str(s, &n->span));
        }
        if (!strcmp(nm, "lang")) {
            if (argtypes.len != 0) SERR(s, 12, &n->span, "lang() takes no arguments");
            decorate(s, callee, ty(s, TY_VOID));
            return decorate(s, n, ty(s, TY_STR));
        }

        if (!strcmp(nm, "listdir")) {
            if (argtypes.len != 1) SERR(s, 12, &n->span, "%s takes 1 argument", nm);
            decorate(s, callee, ty(s, TY_VOID));
            return decorate(s, n, sema_vec_str(s, &n->span));
        }
        symbol_t *sym = scope_lookup(s->cur, nm);
        if (!sym) {
            const char *c = local_canon(s, nm);
            if (c != nm) {
                nm = c;
                callee->name = c;
                sym = scope_lookup(s->cur, nm);
            }
        }
        if (!sym || (sym->kind != SYM_FUNC && sym->kind != SYM_METHOD)) {
            if (!strcmp(nm, "len") && argtypes.len == 1) {
                type_kind_t ak = ((type_t *)argtypes.data[0])->kind;
                if (ak == TY_ARRAY || ak == TY_SLICE || ak == TY_STR) {
                    decorate(s, callee, ty(s, TY_VOID));
                    return decorate(s, n, ty(s, TY_I32));
                }
            }

            if (!strcmp(nm, "char_code") && argtypes.len == 1 &&
                ((type_t *)argtypes.data[0])->kind == TY_STR) {
                decorate(s, callee, ty(s, TY_VOID));
                return decorate(s, n, ty(s, TY_I32));
            }
            const salam_builtin_t *b = salam_builtin_lookup(nm);
            if (b) {
                if (b->nargs >= 0 && (int)argtypes.len != b->nargs)
                    SERR(s, 12, &n->span, "builtin '%s' takes %d argument(s), got %zu",
                         nm, b->nargs, argtypes.len);

                if (strcmp(b->arg, "*") != 0) {
                    int ak = type_prim_kind_from_name(b->arg, NULL);
                    {
                        size_t i = 0;
                        for (; i < argtypes.len; i++)
                            if (!type_assignable(ty(s, (type_kind_t)ak),
                                                 (type_t *)argtypes.data[i]))
                                SERR(s, 2, &n->span,
                                     "builtin '%s' expects '%s', got '%s'", nm, b->arg,
                                     type_to_string(s->tc, (type_t *)argtypes.data[i]));
                    }
                }
                decorate(s, callee, ty(s, TY_VOID));
                return decorate(
                    s, n, ty(s, (type_kind_t)type_prim_kind_from_name(b->ret, NULL)));
            }
            SERR(s, 12, &n->span, "call to undefined function '%s'", nm);
            return decorate(s, n, err_ty(s));
        }

        sym->used = true;
        dce_note(s, sym->pkgname ? sym->pkgname : s->pkg, nm);
        if (sym->kind == SYM_FUNC && sym->decl && sym->decl->typarams.len > 0) {
            symbol_t *inst = g_infer_call(s, sym, &argtypes, &n->span, call_expected);
            if (!inst) return decorate(s, n, err_ty(s));
            sym = inst;
            callee->name = inst->name;
        }
        func_sig_t *sig = resolve_overload(s, sym, &argtypes, &n->span, nm);
        mark_ref_args(s, n, sig);
        coerce_args_to_dyn(s, n, &argtypes, sig);
        decorate(s, callee, sym->type);
        return decorate(s, n,
                        sig ? g_localize_instance(s, sig->ret, &n->span) : err_ty(s));
    }

    if (callee && callee->kind == AST_MEMBER && callee->a &&
        callee->a->kind == AST_IDENTIFIER) {
        symbol_t *pk = scope_lookup(s->cur, callee->a->name);
        if (pk && pk->kind == SYM_PACKAGE) {
            callee->name = pkg_member_canon(pk, callee->name);
            const char *fname = callee->name;
            symbol_t *fn = scope_lookup_local(pk->members, fname);
            if (!fn || fn->kind != SYM_FUNC) {
                SERR(s, 12, &n->span, "package '%s' has no exported function '%s'",
                     pk->name, fname);
                return decorate(s, n, err_ty(s));
            }
            if (!fn->is_pub) {
                SERR(s, 17, &n->span,
                     "'%s' is not exported by package '%s' (mark it 'pub')", fname,
                     pk->name);
                return decorate(s, n, err_ty(s));
            }
            fn->used = true;
            dce_note(s, pk->pkgname, fname);

            if (fn->decl && fn->decl->typarams.len > 0) {
                scope_t *save_cur = s->cur;
                scope_t *save_gp = s->gen_pkg;
                s->cur = pk->members;
                s->gen_pkg = pk->members;
                symbol_t *inst = g_infer_call(s, fn, &argtypes, &n->span, call_expected);
                s->cur = save_cur;
                s->gen_pkg = save_gp;
                if (!inst) return decorate(s, n, err_ty(s));
                callee->kind = AST_IDENTIFIER;
                callee->name = inst->name;
                func_sig_t *sig = resolve_overload(s, inst, &argtypes, &n->span, fname);
                mark_ref_args(s, n, sig);
                decorate(s, callee, inst->type);
                return decorate(
                    s, n, sig ? g_localize_instance(s, sig->ret, &n->span) : err_ty(s));
            }
            func_sig_t *sig = resolve_overload(s, fn, &argtypes, &n->span, fname);
            mark_ref_args(s, n, sig);
            decorate(s, callee->a, pk->type);
            decorate(s, callee, fn->type);
            return decorate(s, n,
                            sig ? g_localize_instance(s, sig->ret, &n->span) : err_ty(s));
        }
    }

    if (callee && callee->kind == AST_MEMBER) {
        type_t *objt = sema_check_expr(s, callee->a);
        if (type_is_error(objt)) return decorate(s, n, err_ty(s));
        {
            ast_node_t *root = callee->a;
            while (root && (root->kind == AST_MEMBER || root->kind == AST_INDEX))
                root = root->a;
            if (root && root->kind == AST_IDENTIFIER && root->name) {
                symbol_t *rsym = scope_lookup(s->cur, root->name);
                if (rsym) rsym->mutated = true;
            }
        }

        if (callee->name &&
            (objt->kind == TY_MAP || objt->kind == TY_MAP_ITER || objt->kind == TY_VEC ||
             objt->kind == TY_STR ||
             (objt->kind == TY_PTR && objt->pointee && objt->pointee->kind == TY_FILE)))
            callee->name = intrinsic_method_canon(callee->name);

        if (objt->kind == TY_MAP) {
            const char *m = callee->name;
            type_t *K = objt->key, *V = objt->elem;
            decorate(s, callee, objt);
            if (!strcmp(m, "put")) {
                check_pure_recv_mut(s, n, m);
                if (argtypes.len != 2)
                    SERR(s, 12, &n->span, "put(key,value) takes 2 args");
                return decorate(s, n, ty(s, TY_VOID));
            }
            if (!strcmp(m, "get")) {
                if (argtypes.len != 1) SERR(s, 12, &n->span, "get(key) takes 1 arg");
                return decorate(s, n, V);
            }
            if (!strcmp(m, "has")) {
                if (argtypes.len != 1) SERR(s, 12, &n->span, "has(key) takes 1 arg");
                return decorate(s, n, ty(s, TY_BOOL));
            }
            if (!strcmp(m, "remove")) {
                check_pure_recv_mut(s, n, m);
                if (argtypes.len != 1) SERR(s, 12, &n->span, "remove(key) takes 1 arg");
                return decorate(s, n, ty(s, TY_BOOL));
            }
            if (!strcmp(m, "size")) return decorate(s, n, ty(s, TY_I32));
            if (!strcmp(m, "iter")) return decorate(s, n, type_map_iter(s->tc, K, V));
            SERR(s, 17, &n->span, "HashMap has no method '%s'", m);
            return decorate(s, n, err_ty(s));
        }
        if (objt->kind == TY_MAP_ITER) {
            const char *m = callee->name;
            decorate(s, callee, objt);
            if (!strcmp(m, "has_next")) return decorate(s, n, ty(s, TY_BOOL));
            if (!strcmp(m, "key")) return decorate(s, n, objt->key);
            if (!strcmp(m, "value")) return decorate(s, n, objt->elem);
            if (!strcmp(m, "next")) return decorate(s, n, ty(s, TY_VOID));
            SERR(s, 17, &n->span, "MapIter has no method '%s'", m);
            return decorate(s, n, err_ty(s));
        }

        if (objt->kind == TY_VEC) {
            const char *m = callee->name;
            type_t *E = objt->elem;
            decorate(s, callee, objt);

            if (E && E->kind == TY_DYN && (!strcmp(m, "push") || !strcmp(m, "set"))) {
                size_t vi = !strcmp(m, "push") ? 0 : 1;
                if (vi < n->list.len && vi < argtypes.len)
                    n->list.data[vi] = coerce_to_dyn(s, E, (ast_node_t *)n->list.data[vi],
                                                     (type_t *)argtypes.data[vi]);
            }
            if (!strcmp(m, "push")) {
                check_pure_recv_mut(s, n, m);
                if (argtypes.len != 1) SERR(s, 12, &n->span, "push(value) takes 1 arg");
                return decorate(s, n, ty(s, TY_VOID));
            }
            if (!strcmp(m, "pop")) {
                check_pure_recv_mut(s, n, m);
                return decorate(s, n, E);
            }
            if (!strcmp(m, "get")) {
                if (argtypes.len != 1) SERR(s, 12, &n->span, "get(index) takes 1 arg");
                return decorate(s, n, type_ptr(s->tc, E));
            }
            if (!strcmp(m, "set")) {
                check_pure_recv_mut(s, n, m);
                if (argtypes.len != 2)
                    SERR(s, 12, &n->span, "set(index,value) takes 2 args");
                return decorate(s, n, ty(s, TY_VOID));
            }
            if (!strcmp(m, "len")) return decorate(s, n, ty(s, TY_I32));
            if (!strcmp(m, "cap")) return decorate(s, n, ty(s, TY_I32));
            if (!strcmp(m, "free")) {
                check_pure_recv_mut(s, n, m);
                return decorate(s, n, ty(s, TY_VOID));
            }
            SERR(s, 17, &n->span, "Vector has no method '%s'", m);
            return decorate(s, n, err_ty(s));
        }

        if (objt->kind == TY_PTR && objt->pointee && objt->pointee->kind == TY_FILE) {
            const char *m = callee->name;
            decorate(s, callee, objt);
            {
                ast_node_t *pf = sema_pure_fn(s);
                if (pf)
                    SERR(s, 12, &n->span,
                         "'pure' function '%s' cannot call '%s': it has side effects",
                         pf->name, m);
            }
            if (!strcmp(m, "read")) {
                if (argtypes.len != 1) SERR(s, 12, &n->span, "read(size) takes 1 arg");
                return decorate(s, n, ty(s, TY_STR));
            }
            if (!strcmp(m, "readline")) return decorate(s, n, ty(s, TY_STR));
            if (!strcmp(m, "write")) {
                if (argtypes.len != 1) SERR(s, 12, &n->span, "write(data) takes 1 arg");
                return decorate(s, n, ty(s, TY_I32));
            }
            if (!strcmp(m, "seek")) {
                if (argtypes.len != 2)
                    SERR(s, 12, &n->span, "seek(offset,whence) takes 2 args");
                return decorate(s, n, ty(s, TY_I32));
            }
            if (!strcmp(m, "close")) return decorate(s, n, ty(s, TY_VOID));
            SERR(s, 17, &n->span, "File has no method '%s'", m);
            return decorate(s, n, err_ty(s));
        }

        if (objt->kind == TY_STR) {
            const char *m = callee->name;
            decorate(s, callee, objt);
            if (!strcmp(m, "len")) return decorate(s, n, ty(s, TY_I32));
            if (!strcmp(m, "concat")) {
                if (argtypes.len != 1) SERR(s, 12, &n->span, "concat(other) takes 1 arg");
                return decorate(s, n, ty(s, TY_STR));
            }
            if (!strcmp(m, "substr")) {
                if (argtypes.len != 2)
                    SERR(s, 12, &n->span, "substr(start,len) takes 2 args");
                return decorate(s, n, ty(s, TY_STR));
            }
            if (!strcmp(m, "find")) {
                if (argtypes.len != 1) SERR(s, 12, &n->span, "find(sub) takes 1 arg");
                return decorate(s, n, ty(s, TY_I32));
            }
            if (!strcmp(m, "split")) {
                if (argtypes.len != 1) SERR(s, 12, &n->span, "split(delim) takes 1 arg");
                return decorate(s, n, sema_vec_str(s, &n->span));
            }
            if (!strcmp(m, "trim")) return decorate(s, n, ty(s, TY_STR));
            if (!strcmp(m, "to_int")) return decorate(s, n, ty(s, TY_I32));
            if (!strcmp(m, "to_float")) return decorate(s, n, ty(s, TY_F64));
            {
                type_t *r = try_impl_call(s, n, callee, objt, &argtypes);
                if (r) return r;
            }
            SERR(s, 17, &n->span, "str has no method '%s'", m);
            return decorate(s, n, err_ty(s));
        }

        {
            type_t *dynt =
                objt->kind == TY_DYN ? objt
                : (objt->kind == TY_PTR && objt->pointee && objt->pointee->kind == TY_DYN)
                    ? objt->pointee
                    : NULL;
            if (dynt && dynt->decl) {
                symbol_t *iface = (symbol_t *)dynt->decl;
                decorate(s, callee, objt);
                if (!strcmp(callee->name, "free")) return decorate(s, n, ty(s, TY_VOID));
                callee->name = scope_member_canon(iface->members, callee->name);
                symbol_t *m = scope_lookup_local(iface->members, callee->name);
                if (!m || m->kind != SYM_METHOD) {
                    SERR(s, 17, &n->span, "interface '%s' has no method '%s'",
                         iface->name, callee->name);
                    return decorate(s, n, err_ty(s));
                }
                func_sig_t *sig =
                    resolve_overload(s, m, &argtypes, &n->span, callee->name);
                mark_ref_args(s, n, sig);
                coerce_args_to_dyn(s, n, &argtypes, sig);
                return decorate(s, n, sig ? sig->ret : err_ty(s));
            }
        }
        symbol_t *ssym = struct_sym_of(objt);
        if (!ssym) {
            type_t *r = try_impl_call(s, n, callee, objt, &argtypes);
            if (r) return r;
            SERR(s, 29, &n->span, "method call on non-struct type '%s'",
                 type_to_string(s->tc, objt));
            return decorate(s, n, err_ty(s));
        }
        callee->name = scope_member_canon(ssym->members, callee->name);

        if (!scope_lookup_local(ssym->members, callee->name)) {
            const char *intr = intrinsic_method_canon(callee->name);
            if (intr != callee->name && scope_lookup_local(ssym->members, intr))
                callee->name = intr;
        }
        symbol_t *m = scope_lookup_local(ssym->members, callee->name);

        if (m && m->kind == SYM_FIELD && m->type && m->type->kind == TY_FUNC) {
            type_t *ft = m->type;
            if (!m->is_pub && struct_sym_of(s->self_type) != ssym)
                SERR(s, 17, &n->span,
                     "field '%s' is private in struct '%s' (mark it 'pub')", callee->name,
                     ssym->name);
            decorate(s, callee, ft);
            {
                ast_node_t *pf = sema_pure_fn(s);
                if (pf)
                    SERR(s, 12, &n->span,
                         "'pure' function '%s' cannot call a function value: its "
                         "purity cannot be verified",
                         pf->name);
            }
            if (argtypes.len != ft->params.len)
                SERR(s, 12, &n->span, "function value takes %zu argument(s), got %zu",
                     ft->params.len, argtypes.len);
            else {
                size_t i = 0;
                for (; i < argtypes.len; i++)
                    if (!type_assignable((type_t *)ft->params.data[i],
                                         (type_t *)argtypes.data[i]))
                        SERR(s, 2, &n->span, "argument %zu: cannot pass '%s' to '%s'",
                             i + 1, type_to_string(s->tc, (type_t *)argtypes.data[i]),
                             type_to_string(s->tc, (type_t *)ft->params.data[i]));
            }
            return decorate(s, n, ft->elem);
        }
        if (!m || m->kind != SYM_METHOD) {
            SERR(s, 17, &n->span, "no method '%s' for type '%s'", callee->name,
                 ssym->name);
            return decorate(s, n, err_ty(s));
        }
        if (!m->is_pub && struct_sym_of(s->self_type) != ssym)
            SERR(s, 17, &n->span, "method '%s' is private in struct '%s' (mark it 'pub')",
                 callee->name, ssym->name);
        func_sig_t *sig = resolve_overload(s, m, &argtypes, &n->span, callee->name);
        mark_ref_args(s, n, sig);
        coerce_args_to_dyn(s, n, &argtypes, sig);
        decorate(s, callee, m->type);
        return decorate(s, n,
                        sig ? g_localize_instance(s, sig->ret, &n->span) : err_ty(s));
    }
    sema_check_expr(s, callee);
    SERR(s, 12, &n->span, "expression is not callable");
    return decorate(s, n, err_ty(s));
}

bool stamp_empty_intrinsic(sema_t *s, ast_node_t *val, type_t *target)
{
    if (!val || !target || val->kind != AST_STRUCT_LIT || !val->name) return false;
    val->name = intrinsic_type_canon(val->name);
    if ((target->kind == TY_VEC && strcmp(val->name, "Vector") == 0) ||
        (target->kind == TY_MAP && strcmp(val->name, "HashMap") == 0)) {
        val->type_str = type_to_string(s->tc, target);
        decorate(s, val, target);
        return true;
    }
    return false;
}
