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

static bool tk_is_arith(token_kind_t k)
{ return k==TK_PLUS||k==TK_MINUS||k==TK_STAR||k==TK_SLASH||k==TK_PERCENT; }

bool sema_type_is_stringable(type_t *t)
{ return t->kind==TY_STR || t->kind==TY_BOOL || t->kind==TY_CHAR || type_is_numeric(t); }

static bool tk_is_cmp(token_kind_t k)
{ return k==TK_LT||k==TK_GT||k==TK_LE||k==TK_GE; }

static bool tk_is_eq(token_kind_t k)
{ return k==TK_EQ||k==TK_NE; }

const char *sema_op_method(token_kind_t k)
{
    switch (k) {
        case TK_PLUS:    return "operator_add";
        case TK_MINUS:   return "operator_sub";
        case TK_STAR:    return "operator_mul";
        case TK_SLASH:   return "operator_div";
        case TK_PERCENT: return "operator_mod";
        case TK_POWER:   return "operator_pow";
        case TK_EQ:      return "operator_eq";
        case TK_NE:      return "operator_ne";
        case TK_LT:      return "operator_lt";
        case TK_GT:      return "operator_gt";
        case TK_LE:      return "operator_le";
        case TK_GE:      return "operator_ge";
        default:         return NULL;
    }
}

token_kind_t sema_compound_base(token_kind_t k)
{
    switch (k) {
        case TK_PLUS_EQ:    return TK_PLUS;
        case TK_MINUS_EQ:   return TK_MINUS;
        case TK_STAR_EQ:    return TK_STAR;
        case TK_SLASH_EQ:   return TK_SLASH;
        case TK_PERCENT_EQ: return TK_PERCENT;
        default:            return TK_EOF;
    }
}

type_t *sema_try_op_overload(sema_t *s, ast_node_t *n, symbol_t *ssym,
                             const char *mname, type_t *rhs_type)
{
    symbol_t *m = scope_lookup_local(ssym->members, mname);
    if (!m || m->kind != SYM_METHOD) return NULL;
    vec_t at; vec_init(&at);
    if (rhs_type) vec_push(s->a, &at, rhs_type);
    func_sig_t *sig = resolve_overload(s, m, &at, &n->span, mname);
    return sig ? sig->ret : NULL;
}

static type_t *check_binary(sema_t *s, ast_node_t *n)
{
    type_t *l = sema_check_expr(s, n->a);
    type_t *r = sema_check_expr(s, n->b);
    token_kind_t op = n->op;
    if (type_is_error(l) || type_is_error(r)) return decorate(s, n, err_ty(s));
    
    {
        symbol_t *ssym = struct_sym_of(l);
        if (ssym) {
            if (op == TK_NE) {
                symbol_t *m_ne = scope_lookup_local(ssym->members, "operator_ne");
                if (!m_ne || m_ne->kind != SYM_METHOD) {
                    type_t *ret = sema_try_op_overload(s, n, ssym, "operator_eq", r);
                    if (ret) {
                        n->synthetic = true;  
                        return decorate(s, n, ty(s, TY_BOOL));
                    }
                }
            }
            const char *mname = sema_op_method(op);
            if (mname) {
                type_t *ret = sema_try_op_overload(s, n, ssym, mname, r);
                if (ret) return decorate(s, n, ret);
            }
        }
    }
    if (op == TK_POWER) {
        if (!type_is_numeric(l) || !type_is_numeric(r))
            SERR(s, 21, &n->span, "operator '**' requires numeric operands");
        return decorate(s, n, ty(s, TY_F64));
    }
    if (op == TK_AND || op == TK_OR) {
        if (l->kind != TY_BOOL || r->kind != TY_BOOL)
            SERR(s, 21, &n->span, "logical operator requires bool operands");
        return decorate(s, n, ty(s, TY_BOOL));
    }
    if (tk_is_arith(op)) {
        
        if (op == TK_PLUS && (l->kind == TY_STR || r->kind == TY_STR)) {
            if (sema_type_is_stringable(l) && sema_type_is_stringable(r))
                return decorate(s, n, ty(s, TY_STR));
            SERR(s, 21, &n->span,
                 "operator '+' on a string requires the other operand to be a string, number, bool, or char (got '%s' and '%s')",
                 type_to_string(s->tc, l), type_to_string(s->tc, r));
            return decorate(s, n, err_ty(s));
        }
        type_t *c = type_common_arith(s->tc, l, r);
        if (!c) { SERR(s, 21, &n->span, "operator cannot be applied to '%s' and '%s'",
                       type_to_string(s->tc, l), type_to_string(s->tc, r));
                  return decorate(s, n, err_ty(s)); }
        if (op == TK_PERCENT && !type_is_integer(c))
            SERR(s, 21, &n->span, "operator '%%' requires integer operands");
        return decorate(s, n, c);
    }
    if (tk_is_cmp(op) || tk_is_eq(op)) {
        bool ok = false;
        if (type_is_numeric(l) && type_is_numeric(r) && type_common_arith(s->tc, l, r)) ok = true;
        else if (type_equiv(l, r)) ok = true;     
        else if (tk_is_eq(op) &&                  
                 ((l->kind == TY_NULL && r->kind == TY_PTR) ||
                  (r->kind == TY_NULL && l->kind == TY_PTR))) ok = true;
        if (!ok)
            SERR(s, 21, &n->span, "cannot compare '%s' and '%s'",
                 type_to_string(s->tc, l), type_to_string(s->tc, r));
        return decorate(s, n, ty(s, TY_BOOL));
    }
    return decorate(s, n, err_ty(s));
}

static type_t *check_member(sema_t *s, ast_node_t *n)
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

type_t *sema_check_expr(sema_t *s, ast_node_t *n)
{
    if (!n) return ty(s, TY_VOID);
    switch (n->kind) {
        case AST_LITERAL: {
            type_t *t;
            switch (n->op) {
                case TK_INT: {
                    
                    uint64_t u = n->value.as.i;
                    if      (u <= 2147483647ULL)          t = ty(s, TY_I32);
                    else if (u <= 9223372036854775807ULL) t = ty(s, TY_I64);
                    else                                  t = ty(s, TY_U64);
                    break;
                }
                case TK_FLOAT:         t = ty(s, TY_F64); break;
                case TK_STRING:
                case TK_TRIPLE_STRING:
                case TK_RAW_STRING:    t = ty(s, TY_STR);  break;
                case TK_CHAR:          t = ty(s, TY_CHAR); break;
                case TK_UTF8_CHAR:     t = ty(s, TY_STR);  break;
                case TK_KW_TRUE:
                case TK_KW_FALSE:      t = ty(s, TY_BOOL); break;
                case TK_KW_NULL:       t = ty(s, TY_NULL); break;
                default:               t = err_ty(s); break;
            }
            return decorate(s, n, t);
        }
        case AST_IDENTIFIER: {
            symbol_t *sym = scope_lookup(s->cur, n->name);
            if (!sym) {   
                const char *c = local_canon(s, n->name);
                if (c != n->name) { n->name = c; sym = scope_lookup(s->cur, n->name); }
            }
            if (!sym) { SERR(s, 1, &n->span, "unknown identifier '%s'", n->name);
                        return decorate(s, n, err_ty(s)); }
            if (sym->kind == SYM_FUNC || sym->kind == SYM_METHOD) {
                SERR(s, 1, &n->span, "function '%s' used as a value", n->name);
                return decorate(s, n, err_ty(s));
            }
            if (sym->kind == SYM_STRUCT || sym->kind == SYM_ENUM) {
                SERR(s, 1, &n->span, "type '%s' used as a value", n->name);
                return decorate(s, n, err_ty(s));
            }
            sym->used = true;   
            
            if (s->lam && (sym->kind == SYM_VAR || sym->kind == SYM_PARAM) &&
                !defined_within(s->cur, s->lam->boundary, n->name) &&
                !scope_lookup_local(s->global, n->name))
                record_capture(s, n, sym->type);
            if (sym->kind == SYM_PARAM && sym->is_ref) n->is_ref = true;
            return decorate(s, n, sym->type);
        }
        case AST_THIS: {
            if (!s->self_type) { SERR(s, 14, &n->span, "'this' used outside a method");
                                 return decorate(s, n, err_ty(s)); }
            return decorate(s, n, s->self_type);
        }
        case AST_BINARY: return check_binary(s, n);
        case AST_UNARY: {
            type_t *o = sema_check_expr(s, n->a);
            
            {
                symbol_t *ssym = struct_sym_of(o);
                if (ssym) {
                    const char *mname = (n->op == TK_MINUS) ? "operator_sub"
                                      : (n->op == TK_NOT)   ? "operator_not" : NULL;
                    if (mname) {
                        type_t *ret = sema_try_op_overload(s, n, ssym, mname, NULL);
                        if (ret) return decorate(s, n, ret);
                    }
                }
            }
            if (n->op == TK_NOT) {
                if (o->kind != TY_BOOL && !type_is_error(o))
                    SERR(s, 21, &n->span, "operator '!' requires a bool operand");
                return decorate(s, n, ty(s, TY_BOOL));
            }
            if (!type_is_numeric(o) && !type_is_error(o))
                SERR(s, 21, &n->span, "unary '-' requires a numeric operand");
            return decorate(s, n, o);
        }
        case AST_CAST: {
            type_t *o = sema_check_expr(s, n->a);
            type_t *target = sema_resolve_type(s, n->type);
            if (!type_castable(target, o))
                SERR(s, 23, &n->span, "cannot cast '%s' to '%s'",
                     type_to_string(s->tc, o), type_to_string(s->tc, target));
            return decorate(s, n, target);
        }
        case AST_CALL:        return check_call(s, n);
        case AST_MEMBER:      return check_member(s, n);
        case AST_INDEX: {
            type_t *arr = sema_check_expr(s, n->a);
            type_t *idx = sema_check_expr(s, n->b);
            
            {
                symbol_t *ssym = struct_sym_of(arr);
                if (ssym) {
                    type_t *ret = sema_try_op_overload(s, n, ssym, "operator_index", idx);
                    if (ret) return decorate(s, n, ret);
                }
            }
            if (!type_is_error(idx) && !type_is_integer(idx))
                SERR(s, 21, &n->span, "array index must be an integer, got '%s'",
                     type_to_string(s->tc, idx));
            if (arr->kind == TY_ARRAY) return decorate(s, n, arr->elem);
            if (arr->kind == TY_SLICE) return decorate(s, n, arr->elem);
            if (arr->kind == TY_PTR)   return decorate(s, n, arr->pointee);
            if (!type_is_error(arr))
                SERR(s, 21, &n->span, "cannot index non-array type '%s'",
                     type_to_string(s->tc, arr));
            return decorate(s, n, err_ty(s));
        }
        case AST_SLICE: {
            type_t *base = sema_check_expr(s, n->a);
            if (n->b) {
                type_t *lo = sema_check_expr(s, n->b);
                if (!type_is_error(lo) && !type_is_integer(lo))
                    SERR(s, 21, &n->span, "slice bound must be an integer, got '%s'",
                         type_to_string(s->tc, lo));
            }
            if (n->c) {
                type_t *hi = sema_check_expr(s, n->c);
                if (!type_is_error(hi) && !type_is_integer(hi))
                    SERR(s, 21, &n->span, "slice bound must be an integer, got '%s'",
                         type_to_string(s->tc, hi));
            }
            if (base->kind == TY_ARRAY || base->kind == TY_SLICE)
                return decorate(s, n, type_slice(s->tc, base->elem));
            if (!type_is_error(base))
                SERR(s, 21, &n->span, "cannot slice non-array type '%s'",
                     type_to_string(s->tc, base));
            return decorate(s, n, err_ty(s));
        }
        case AST_ARRAY_LIT:   return check_array_lit(s, n);
        case AST_STRUCT_LIT:  return check_struct_lit(s, n);
        case AST_LAMBDA:      return check_lambda(s, n);
        default:
            return decorate(s, n, err_ty(s));
    }
}
