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

#include "codegen/codegen_internal.h"

func_sig_t *sig_of_decl(symbol_t *fsym, ast_node_t *decl)
{
    if (!fsym) return NULL;
    {
        size_t i = 0;
        for (; i < fsym->overloads.len; i++) {
            func_sig_t *sig = (func_sig_t *)fsym->overloads.data[i];
            if (sig->decl == decl) return sig;
        }
    }
    return NULL;
}

func_sig_t *pick_overload(cg_t *cg, symbol_t *fsym, ast_node_t *call)
{
    func_sig_t *fallback = NULL;
    {
        size_t i = 0;
        for (; i < fsym->overloads.len; i++) {
            func_sig_t *sig = (func_sig_t *)fsym->overloads.data[i];
            if (!fallback) fallback = sig;
            if (call->list.len < sig->required) continue;
            if (!sig->variadic && call->list.len > sig->params.len) continue;
            size_t nfix =
                call->list.len < sig->params.len ? call->list.len : sig->params.len;
            bool ok = true;
            {
                size_t j = 0;
                for (; j < nfix; j++) {
                    ast_node_t *arg = (ast_node_t *)call->list.data[j];
                    const char *pt =
                        type_to_string(cg->sem->tc, (type_t *)sig->params.data[j]);
                    if (!arg->type_str || strcmp(arg->type_str, pt) != 0) {
                        ok = false;
                        break;
                    }
                }
            }
            if (ok) return sig;
        }
    }
    return fallback;
}

symbol_t *struct_by_name(cg_t *cg, const char *name)
{
    symbol_t *s = scope_lookup(cg->sem->global, name);
    return (s && s->kind == SYM_STRUCT) ? s : NULL;
}

bool cg_addressable(const ast_node_t *n)
{
    return n->kind == AST_IDENTIFIER || n->kind == AST_MEMBER || n->kind == AST_INDEX ||
           n->kind == AST_THIS || n->kind == AST_STRUCT_LIT;
}

symbol_t *cg_struct_of(cg_t *cg, const char *ts, char *sname, size_t cap)
{
    bool ptr;
    vec_t dims;
    parse_typestr(ts ? ts : "", sname, cap, &ptr, &dims, cg->a);
    return struct_by_name(cg, sname);
}

const char *cg_op_method(token_kind_t k)
{
    switch (k) {
    case TK_PLUS:
        return "operator_add";
    case TK_MINUS:
        return "operator_sub";
    case TK_STAR:
        return "operator_mul";
    case TK_SLASH:
        return "operator_div";
    case TK_PERCENT:
        return "operator_mod";
    case TK_POWER:
        return "operator_pow";
    case TK_EQ:
        return "operator_eq";
    case TK_NE:
        return "operator_ne";
    case TK_LT:
        return "operator_lt";
    case TK_GT:
        return "operator_gt";
    case TK_LE:
        return "operator_le";
    case TK_GE:
        return "operator_ge";
    default:
        return NULL;
    }
}

token_kind_t cg_compound_base(token_kind_t k)
{
    switch (k) {
    case TK_PLUS_EQ:
        return TK_PLUS;
    case TK_MINUS_EQ:
        return TK_MINUS;
    case TK_STAR_EQ:
        return TK_STAR;
    case TK_SLASH_EQ:
        return TK_SLASH;
    case TK_PERCENT_EQ:
        return TK_PERCENT;
    default:
        return TK_EOF;
    }
}

func_sig_t *pick_op_overload(cg_t *cg, symbol_t *msym, size_t want_nparams)
{
    (void)cg;
    func_sig_t *fallback = NULL;
    {
        size_t i = 0;
        for (; i < msym->overloads.len; i++) {
            func_sig_t *sig = (func_sig_t *)msym->overloads.data[i];
            if (sig->params.len == want_nparams) {
                if (!fallback) fallback = sig;
            }
        }
    }
    return fallback;
}
