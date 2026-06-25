#include "core/prelude.h"
#include "semantic/sema_internal.h"

static type_t *ty(sema_t *s, type_kind_t k) { return sema_ty(s, k); }

static type_t *err_ty(sema_t *s)            { return sema_err_ty(s); }

static type_t *decorate(sema_t *s, ast_node_t *n, type_t *t) { return sema_decorate(s, n, t); }

type_t *check_member(sema_t *s, ast_node_t *n)
{
    if (n->a && n->a->kind == AST_IDENTIFIER) {
        symbol_t *pk = scope_lookup(s->cur, n->a->name);
        if (pk && pk->kind == SYM_PACKAGE) {
            n->name = pkg_member_canon(pk, n->name);   
            symbol_t *m = scope_lookup_local(pk->members, n->name);
            if (!m) { SERR(s, 16, &n->span, "package '%s' has no exported member '%s'", pk->name, n->name);
                      return decorate(s, n, err_ty(s)); }
            if (!m->is_pub) {
                SERR(s, 17, &n->span, "'%s' is not exported by package '%s' (mark it 'pub')",
                     n->name, pk->name);
                return decorate(s, n, err_ty(s));
            }
            if (m->kind == SYM_FUNC) { SERR(s, 17, &n->span, "function '%s.%s' used as a value (call it)", pk->name, n->name);
                                       return decorate(s, n, err_ty(s)); }
            decorate(s, n->a, pk->type);
            return decorate(s, n, m->type);
        }
    }
    
    if (n->a && n->a->kind == AST_IDENTIFIER) {
        symbol_t *sy = scope_lookup(s->cur, n->a->name);
        if (sy && sy->kind == SYM_ENUM) {
            decorate(s, n->a, sy->type);
            symbol_t *m = sy->members ? scope_lookup_local(sy->members, n->name) : NULL;
            if (!m) SERR(s, 16, &n->span, "enum '%s' has no member '%s'", sy->name, n->name);
            return decorate(s, n, sy->type);
        }
    }
    type_t *objt = sema_check_expr(s, n->a);
    if (type_is_error(objt)) return decorate(s, n, err_ty(s));
    symbol_t *ssym = struct_sym_of(objt);
    if (!ssym) {
        SERR(s, 29, &n->span, "member access on non-struct type '%s'", type_to_string(s->tc, objt));
        return decorate(s, n, err_ty(s));
    }
    n->name = scope_member_canon(ssym->members, n->name);   
    symbol_t *f = scope_lookup_local(ssym->members, n->name);
    if (!f) { SERR(s, 16, &n->span, "no field '%s' in struct '%s'", n->name, ssym->name);
              return decorate(s, n, err_ty(s)); }
    if (f->kind == SYM_METHOD) {
        SERR(s, 17, &n->span, "method '%s' used as a value (call it)", n->name);
        return decorate(s, n, err_ty(s));
    }
    return decorate(s, n, f->type);
}

type_t *check_method_call(sema_t *s, ast_node_t *n, vec_t *argtypes)
{
    ast_node_t *callee = n->a;
    type_t *objt = sema_check_expr(s, callee->a);
    if (type_is_error(objt)) return decorate(s, n, err_ty(s));
    
    if (callee->name &&
        (objt->kind == TY_MAP || objt->kind == TY_MAP_ITER ||
         objt->kind == TY_VEC || objt->kind == TY_STR ||
         (objt->kind == TY_PTR && objt->pointee && objt->pointee->kind == TY_FILE)))
        callee->name = intrinsic_method_canon(callee->name);
    
    if (objt->kind == TY_MAP) {
        const char *m = callee->name;
        type_t *K = objt->key, *V = objt->elem;
        decorate(s, callee, objt);
        if (!strcmp(m,"put"))    { if (argtypes->len!=2) SERR(s,12,&n->span,"put(key,value) takes 2 args"); return decorate(s,n,ty(s,TY_VOID)); }
        if (!strcmp(m,"get"))    { if (argtypes->len!=1) SERR(s,12,&n->span,"get(key) takes 1 arg"); return decorate(s,n,V); }
        if (!strcmp(m,"has"))    { if (argtypes->len!=1) SERR(s,12,&n->span,"has(key) takes 1 arg"); return decorate(s,n,ty(s,TY_BOOL)); }
        if (!strcmp(m,"remove")) { if (argtypes->len!=1) SERR(s,12,&n->span,"remove(key) takes 1 arg"); return decorate(s,n,ty(s,TY_BOOL)); }
        if (!strcmp(m,"size"))   return decorate(s,n,ty(s,TY_I32));
        if (!strcmp(m,"iter"))   return decorate(s,n,type_map_iter(s->tc,K,V));
        SERR(s,17,&n->span,"HashMap has no method '%s'",m);
        return decorate(s,n,err_ty(s));
    }
    if (objt->kind == TY_MAP_ITER) {
        const char *m = callee->name;
        decorate(s, callee, objt);
        if (!strcmp(m,"has_next")) return decorate(s,n,ty(s,TY_BOOL));
        if (!strcmp(m,"key"))      return decorate(s,n,objt->key);
        if (!strcmp(m,"value"))    return decorate(s,n,objt->elem);
        if (!strcmp(m,"next"))     return decorate(s,n,ty(s,TY_VOID));
        SERR(s,17,&n->span,"MapIter has no method '%s'",m);
        return decorate(s,n,err_ty(s));
    }
    
    if (objt->kind == TY_VEC) {
        const char *m = callee->name;
        type_t *E = objt->elem;
        decorate(s, callee, objt);
        
        if (E && E->kind == TY_DYN && (!strcmp(m,"push") || !strcmp(m,"set"))) {
            size_t vi = !strcmp(m,"push") ? 0 : 1;     
            if (vi < n->list.len && vi < argtypes->len)
                n->list.data[vi] = coerce_to_dyn(s, E, (ast_node_t *)n->list.data[vi],
                                                 (type_t *)argtypes->data[vi]);
        }
        if (!strcmp(m,"push")) { if (argtypes->len!=1) SERR(s,12,&n->span,"push(value) takes 1 arg"); return decorate(s,n,ty(s,TY_VOID)); }
        if (!strcmp(m,"pop"))  return decorate(s,n,E);
        if (!strcmp(m,"get"))  { if (argtypes->len!=1) SERR(s,12,&n->span,"get(index) takes 1 arg"); return decorate(s,n,type_ptr(s->tc,E)); }
        if (!strcmp(m,"set"))  { if (argtypes->len!=2) SERR(s,12,&n->span,"set(index,value) takes 2 args"); return decorate(s,n,ty(s,TY_VOID)); }
        if (!strcmp(m,"len"))  return decorate(s,n,ty(s,TY_I32));
        if (!strcmp(m,"cap"))  return decorate(s,n,ty(s,TY_I32));
        if (!strcmp(m,"free")) return decorate(s,n,ty(s,TY_VOID));
        SERR(s,17,&n->span,"Vector has no method '%s'",m);
        return decorate(s,n,err_ty(s));
    }
    
    if (objt->kind == TY_PTR && objt->pointee && objt->pointee->kind == TY_FILE) {
        const char *m = callee->name;
        decorate(s, callee, objt);
        if (!strcmp(m,"read"))     { if (argtypes->len!=1) SERR(s,12,&n->span,"read(size) takes 1 arg"); return decorate(s,n,ty(s,TY_STR)); }
        if (!strcmp(m,"readline")) return decorate(s,n,ty(s,TY_STR));
        if (!strcmp(m,"write"))    { if (argtypes->len!=1) SERR(s,12,&n->span,"write(data) takes 1 arg"); return decorate(s,n,ty(s,TY_I32)); }
        if (!strcmp(m,"seek"))     { if (argtypes->len!=2) SERR(s,12,&n->span,"seek(offset,whence) takes 2 args"); return decorate(s,n,ty(s,TY_I32)); }
        if (!strcmp(m,"close"))    return decorate(s,n,ty(s,TY_VOID));
        SERR(s,17,&n->span,"File has no method '%s'",m);
        return decorate(s,n,err_ty(s));
    }
    
    if (objt->kind == TY_STR) {
        const char *m = callee->name;
        decorate(s, callee, objt);
        if (!strcmp(m,"len"))      return decorate(s,n,ty(s,TY_I32));
        if (!strcmp(m,"concat"))   { if (argtypes->len!=1) SERR(s,12,&n->span,"concat(other) takes 1 arg"); return decorate(s,n,ty(s,TY_STR)); }
        if (!strcmp(m,"substr"))   { if (argtypes->len!=2) SERR(s,12,&n->span,"substr(start,len) takes 2 args"); return decorate(s,n,ty(s,TY_STR)); }
        if (!strcmp(m,"find"))     { if (argtypes->len!=1) SERR(s,12,&n->span,"find(sub) takes 1 arg"); return decorate(s,n,ty(s,TY_I32)); }
        if (!strcmp(m,"split"))    { if (argtypes->len!=1) SERR(s,12,&n->span,"split(delim) takes 1 arg"); return decorate(s,n,sema_vec_str(s, &n->span)); }
        if (!strcmp(m,"trim"))     return decorate(s,n,ty(s,TY_STR));
        if (!strcmp(m,"to_int"))   return decorate(s,n,ty(s,TY_I32));
        if (!strcmp(m,"to_float")) return decorate(s,n,ty(s,TY_F64));
        SERR(s,17,&n->span,"str has no method '%s'",m);
        return decorate(s,n,err_ty(s));
    }
    
    {
        type_t *dynt = objt->kind == TY_DYN ? objt
                     : (objt->kind == TY_PTR && objt->pointee && objt->pointee->kind == TY_DYN)
                       ? objt->pointee : NULL;
        if (dynt && dynt->decl) {
            symbol_t *iface = (symbol_t *)dynt->decl;
            decorate(s, callee, objt);
            if (!strcmp(callee->name, "free")) return decorate(s, n, ty(s, TY_VOID));
            callee->name = scope_member_canon(iface->members, callee->name);
            symbol_t *m = scope_lookup_local(iface->members, callee->name);
            if (!m || m->kind != SYM_METHOD) {
                SERR(s, 17, &n->span, "interface '%s' has no method '%s'", iface->name, callee->name);
                return decorate(s, n, err_ty(s));
            }
            func_sig_t *sig = resolve_overload(s, m, argtypes, &n->span, callee->name);
            coerce_args_to_dyn(s, n, argtypes, sig);
            return decorate(s, n, sig ? sig->ret : err_ty(s));
        }
    }
    symbol_t *ssym = struct_sym_of(objt);
    if (!ssym) {
        
        symbol_t *owner = scope_lookup(s->global, impl_owner_key(s->a, type_to_string(s->tc, objt)));
        if (owner) {
            symbol_t *im = scope_lookup_local(owner->members, callee->name);
            if (im && im->kind == SYM_METHOD) {
                func_sig_t *sig = resolve_overload(s, im, argtypes, &n->span, callee->name);
                decorate(s, callee, im->type);
                return decorate(s, n, sig ? sig->ret : err_ty(s));
            }
        }
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
        decorate(s, callee, ft);
        if (argtypes->len != ft->params.len)
            SERR(s, 12, &n->span, "function value takes %zu argument(s), got %zu",
                 ft->params.len, argtypes->len);
        else for (size_t i = 0; i < argtypes->len; i++)
            if (!type_assignable((type_t *)ft->params.data[i], (type_t *)argtypes->data[i]))
                SERR(s, 2, &n->span, "argument %zu: cannot pass '%s' to '%s'", i + 1,
                     type_to_string(s->tc, (type_t *)argtypes->data[i]),
                     type_to_string(s->tc, (type_t *)ft->params.data[i]));
        return decorate(s, n, ft->elem);
    }
    if (!m || m->kind != SYM_METHOD) {
        SERR(s, 17, &n->span, "no method '%s' for type '%s'", callee->name, ssym->name);
        return decorate(s, n, err_ty(s));
    }
    func_sig_t *sig = resolve_overload(s, m, argtypes, &n->span, callee->name);
    coerce_args_to_dyn(s, n, argtypes, sig);   
    decorate(s, callee, m->type);
    return decorate(s, n, sig ? sig->ret : err_ty(s));
}
