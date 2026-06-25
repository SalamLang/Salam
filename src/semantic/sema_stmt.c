#include "core/prelude.h"
#include "semantic/sema_internal.h"

static type_t *ty(sema_t *s, type_kind_t k) { return sema_ty(s, k); }

static type_t *err_ty(sema_t *s)            { return sema_err_ty(s); }

static bool lvalue_mutable(sema_t *s, ast_node_t *n, bool *is_lvalue)
{
    *is_lvalue = false;
    if (!n) return false;
    if (n->kind == AST_IDENTIFIER) {
        symbol_t *sym = scope_lookup(s->cur, n->name);
        *is_lvalue = (sym != NULL);
        if (!sym) return false;
        if (sym->is_ref) return true;   
        return sym->is_mut;
    }
    if (n->kind == AST_THIS) { *is_lvalue = true; return true; }
    if (n->kind == AST_MEMBER || n->kind == AST_INDEX) {
        bool inner_lv;
        bool m = lvalue_mutable(s, n->a, &inner_lv);
        *is_lvalue = inner_lv;
        return m;
    }
    return false;
}

static void define_local(sema_t *s, ast_node_t *decl, sym_kind_t kind, type_t *t)
{
    symbol_t *sym = symbol_new(s->a, kind, decl->name);
    sym->type = t;
    sym->is_mut = decl->is_mut;
    sym->decl = decl;
    if (scope_define(s->a, s->cur, sym))
        SERR(s, 1, &decl->span, "redefinition of '%s'", decl->name);
}
type_t *sema_check_var_decl(sema_t *s, ast_node_t *n)
{
    type_t *declared = n->type ? sema_resolve_type(s, n->type) : NULL;
    
    if (n->a && n->a->kind == AST_STRUCT_LIT && n->a->name)
        n->a->name = intrinsic_type_canon(n->a->name);
    
    if (declared && declared->kind == TY_MAP && n->a &&
        n->a->kind == AST_STRUCT_LIT && n->a->name && strcmp(n->a->name, "HashMap") == 0) {
        n->a->type_str = type_to_string(s->tc, declared);
        n->type_str = n->a->type_str;
        return declared;
    }
    
    if (declared && declared->kind == TY_VEC && n->a &&
        n->a->kind == AST_STRUCT_LIT && n->a->name && strcmp(n->a->name, "Vector") == 0) {
        n->a->type_str = type_to_string(s->tc, declared);
        n->type_str = n->a->type_str;
        return declared;
    }
    
    
    if (declared && declared->kind == TY_STRUCT && n->a &&
        n->a->kind == AST_STRUCT_LIT && n->a->name &&
        (generic_template(s, n->a->name, SYM_STRUCT) || !scope_lookup(s->global, n->a->name))) {
        n->a->name = declared->name;
    }
    
    if (declared && declared->kind == TY_ARRAY && declared->elem &&
        declared->elem->kind == TY_DYN && n->a && n->a->kind == AST_ARRAY_LIT)
        s->expected = declared;
    type_t *initt    = n->a    ? sema_check_expr(s, n->a)      : NULL;
    type_t *t;
    if (declared && initt) {
        if (!type_assignable(declared, initt))
            SERR(s, 2, &n->span, "cannot assign '%s' to '%s' in declaration of '%s'",
                 type_to_string(s->tc, initt), type_to_string(s->tc, declared), n->name);
        if (declared->kind == TY_DYN) n->a = coerce_to_dyn(s, declared, n->a, initt);  
        t = declared;
    } else if (declared) {
        t = declared;
    } else if (initt) {
        t = initt;
    } else {
        SERR(s, 1, &n->span, "cannot infer type of '%s' (no type or initializer)", n->name);
        t = err_ty(s);
    }
    n->type_str = type_to_string(s->tc, t);
    return t;
}

static void check_stmt(sema_t *s, ast_node_t *n)
{
    if (!n) return;
    switch (n->kind) {
        case AST_BLOCK: sema_check_block(s, n); break;
        case AST_VAR_DECL: {
            type_t *t = sema_check_var_decl(s, n);
            define_local(s, n, n->is_mut ? SYM_VAR : SYM_VAR, t);
            break;
        }
        case AST_CONST_DECL: {
            type_t *t = sema_check_var_decl(s, n);
            symbol_t *sym = symbol_new(s->a, SYM_CONST, n->name);
            sym->type = t; sym->is_mut = false; sym->decl = n;
            if (n->a && n->a->kind == AST_LITERAL && n->a->value.kind == TV_INT) {
                sym->has_ival = true; sym->ival = (long long)n->a->value.as.i;
            }
            if (scope_define(s->a, s->cur, sym))
                SERR(s, 1, &n->span, "redefinition of '%s'", n->name);
            break;
        }
        case AST_ASSIGN: {
            type_t *tt = sema_check_expr(s, n->a);
            type_t *vt = sema_check_expr(s, n->b);
            bool is_lvalue;
            bool mut = lvalue_mutable(s, n->a, &is_lvalue);
            if (!is_lvalue)
                SERR(s, 13, &n->span, "assignment target is not assignable");
            else if (!mut)
                SERR(s, 13, &n->span, "cannot assign to immutable target");
            
            if (n->op == TK_ASSIGN && n->a && n->a->kind == AST_INDEX &&
                n->a->a && n->a->a->type_str) {
                symbol_t *ssym = NULL;
                {
                    type_t *obst = n->a->a->type_str ? type_prim(s->tc, TY_ERROR) : NULL;
                    (void)obst;
                    symbol_t *tmp = scope_lookup(s->cur, n->a->a->type_str);
                    if (!tmp) {
                        
                        size_t bl = strlen(n->a->a->type_str);
                        if (bl > 1 && n->a->a->type_str[bl-1] == '*') {
                            char *base = arena_strndup(s->a, n->a->a->type_str, bl-1);
                            tmp = scope_lookup(s->cur, base);
                        }
                    }
                    if (tmp && tmp->kind == SYM_STRUCT) ssym = tmp;
                }
                if (ssym) {
                    symbol_t *m = scope_lookup_local(ssym->members, "operator_index_set");
                    if (m && m->kind == SYM_METHOD)
                        break;  
                }
            }
            
            if (n->op != TK_ASSIGN && tt && tt->kind == TY_STRUCT) {
                token_kind_t base_op = sema_compound_base(n->op);
                const char *mname = sema_op_method(base_op);
                if (mname) {
                    symbol_t *ssym = struct_sym_of(tt);
                    if (ssym) {
                        type_t *ret = sema_try_op_overload(s, n, ssym, mname, vt);
                        if (ret) break;  
                    }
                }
            }
            
            if (n->op == TK_PLUS_EQ && tt && tt->kind == TY_STR && sema_type_is_stringable(vt))
                break;
            if (!type_assignable(tt, vt))
                SERR(s, 2, &n->span, "cannot assign '%s' to '%s'",
                     type_to_string(s->tc, vt), type_to_string(s->tc, tt));
            break;
        }
        case AST_IF: {
            type_t *c = sema_check_expr(s, n->a);
            if (c->kind != TY_BOOL && !type_is_error(c))
                SERR(s, 21, &n->a->span, "if condition must be bool, got '%s'", type_to_string(s->tc, c));
            if (n->b && n->b->kind == AST_BLOCK && n->b->list.len == 0)
                SERR(s, 60, &n->b->span, "empty 'if' branch (it must contain at least one statement)");
            check_stmt(s, n->b);
            if (n->c) {
                
                if (n->c->kind == AST_BLOCK && n->c->list.len == 0)
                    SERR(s, 60, &n->c->span, "empty 'else' branch (it must contain at least one statement)");
                check_stmt(s, n->c);
            }
            break;
        }
        case AST_WHILE: {
            type_t *c = sema_check_expr(s, n->a);
            if (c->kind != TY_BOOL && !type_is_error(c))
                SERR(s, 21, &n->a->span, "while condition must be bool, got '%s'", type_to_string(s->tc, c));
            s->loop_depth++;
            check_stmt(s, n->b);
            s->loop_depth--;
            break;
        }
        case AST_REPEAT: {
            type_t *c = sema_check_expr(s, n->a);
            if (!type_is_numeric(c) && !type_is_error(c))
                SERR(s, 63, &n->a->span,
                     "repeat count must be a number, got '%s'", type_to_string(s->tc, c));
            if (n->c) {                                  
                type_t *e = sema_check_expr(s, n->c);
                if (!type_is_numeric(e) && !type_is_error(e))
                    SERR(s, 63, &n->c->span,
                         "repeat range bound must be a number, got '%s'", type_to_string(s->tc, e));
            }
            if (n->d) {                                  
                type_t *st = sema_check_expr(s, n->d);
                if (!type_is_numeric(st) && !type_is_error(st))
                    SERR(s, 63, &n->d->span,
                         "repeat step must be a number, got '%s'", type_to_string(s->tc, st));
            }
            s->loop_depth++;
            check_stmt(s, n->b);
            s->loop_depth--;
            break;
        }
        case AST_FOR: {
            scope_t *sc = scope_new(s->a, SCOPE_BLOCK, s->cur);
            scope_t *saved = s->cur; s->cur = sc;
            if (n->a) check_stmt(s, n->a);
            if (n->b) {
                type_t *c = sema_check_expr(s, n->b);
                if (c->kind != TY_BOOL && !type_is_error(c))
                    SERR(s, 21, &n->b->span, "for condition must be bool, got '%s'", type_to_string(s->tc, c));
            }
            if (n->c) check_stmt(s, n->c);
            s->loop_depth++;
            check_stmt(s, n->d);
            s->loop_depth--;
            s->cur = saved;
            break;
        }
        case AST_RETURN: {
            if (s->cur_func && s->cur_func->infer_ret) {
                type_t *vt = n->a ? sema_check_expr(s, n->a) : ty(s, TY_VOID);
                s->cur_func->ret = vt;
                s->cur_func->infer_ret = false;
                break;
            }
            type_t *ret = s->cur_func ? s->cur_func->ret : ty(s, TY_VOID);
            if (n->a) {
                s->expected = ret;
                type_t *vt = sema_check_expr(s, n->a);
                s->expected = NULL;
                if (ret->kind == TY_VOID)
                    SERR(s, 49, &n->span, "cannot return a value from a void function");
                else if (!type_assignable(ret, vt))
                    SERR(s, 2, &n->span, "return type mismatch: expected '%s', got '%s'",
                         type_to_string(s->tc, ret), type_to_string(s->tc, vt));
                else if (ret->kind == TY_DYN) n->a = coerce_to_dyn(s, ret, n->a, vt);  
            } else if (ret->kind != TY_VOID) {
                SERR(s, 48, &n->span, "missing return value; function returns '%s'",
                     type_to_string(s->tc, ret));
            }
            break;
        }
        case AST_DEFER:
            if (n->a) check_stmt(s, n->a);
            break;
        case AST_BREAK:
        case AST_CONTINUE:
            if (s->loop_depth == 0)
                SERR(s, 37, &n->span, "'%s' outside of a loop",
                     n->kind == AST_BREAK ? "break" : "continue");
            break;
        case AST_EXPR_STMT: sema_check_expr(s, n->a); break;
        default: break;
    }
}

void sema_check_block(sema_t *s, ast_node_t *block)
{
    scope_t *sc = scope_new(s->a, SCOPE_BLOCK, s->cur);
    scope_t *saved = s->cur; s->cur = sc;
    LOG_D(s->log, PH_SEMANTIC, "enter block scope (%zu stmts)", block->list.len);
    { size_t i = 0; for (; i < block->list.len; i++)
        check_stmt(s, (ast_node_t *)block->list.data[i]); }
    
    { size_t i = 0; for (; i < sc->symbols.len; i++) {
        symbol_t *v = (symbol_t *)sc->symbols.data[i];
        if (v->kind == SYM_VAR && !v->used && v->decl && v->name && v->name[0] != '_')
            SERR(s, 59, &v->decl->span, "unused variable '%s' (prefix with '_' if intentional)", v->name);
    } }
    s->cur = saved;
    LOG_D(s->log, PH_SEMANTIC, "exit block scope");
}
