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

typedef struct {
    bool is_lvalue;
    bool is_projection;
    symbol_t *root;
} lvalue_info_t;

static void analyze_lvalue(sema_t *s, ast_node_t *n, lvalue_info_t *out)
{
    if (!n) return;
    if (n->kind == AST_IDENTIFIER) {
        symbol_t *sym = scope_lookup(s->cur, n->name);
        if (!sym) return;
        out->is_lvalue = true;
        out->root = sym;
        return;
    }
    if (n->kind == AST_THIS) {
        out->is_lvalue = true;
        return;
    }
    if (n->kind == AST_MEMBER || n->kind == AST_INDEX) {
        analyze_lvalue(s, n->a, out);
        if (out->is_lvalue) out->is_projection = true;
    }
}

lvalue_verdict_t sema_classify_write(sema_t *s, ast_node_t *n, symbol_t **root_out)
{
    lvalue_info_t lv = {0};
    analyze_lvalue(s, n, &lv);
    if (root_out) *root_out = lv.root;
    if (!lv.is_lvalue) return LV_NOT_LVALUE;
    if (lv.root && lv.root->kind == SYM_CONST) return LV_CONST;
    if (lv.is_projection) return LV_OK;
    if (!lv.root) return LV_OK;
    if (lv.root->is_mut || lv.root->is_ref) return LV_OK;
    return LV_IMMUTABLE;
}

bool sema_lvalue_mutable(sema_t *s, ast_node_t *n, bool *is_lvalue)
{
    symbol_t *root = NULL;
    lvalue_verdict_t v = sema_classify_write(s, n, &root);
    *is_lvalue = (v != LV_NOT_LVALUE);
    return v == LV_OK;
}

static bool pure_param_escapes(type_t *t)
{
    if (!t) return false;
    return t->kind == TY_PTR || t->kind == TY_VEC || t->kind == TY_MAP ||
           t->kind == TY_SLICE;
}

void sema_check_pure_write(sema_t *s, ast_node_t *target, const src_span_t *span)
{
    ast_node_t *fn = sema_pure_fn(s);
    if (!fn) return;
    ast_node_t *root = target;
    ast_node_t *below = NULL;
    while (root && (root->kind == AST_MEMBER || root->kind == AST_INDEX)) {
        below = root;
        root = root->a;
    }
    if (!root || root->kind != AST_IDENTIFIER || !root->name) return;
    scope_t *where = NULL;
    symbol_t *sym = scope_lookup_where(s->cur, root->name, &where);
    if (!sym) return;
    if (sym->kind == SYM_PACKAGE && below && below->kind == AST_MEMBER) {
        SERR(s, 13, span,
             "'pure' function '%s' cannot assign to global '%s': writing a global "
             "variable is a side effect",
             fn->name, below->name ? below->name : root->name);
        return;
    }
    if (sym->kind == SYM_VAR && where && where->kind == SCOPE_GLOBAL) {
        SERR(s, 13, span,
             "'pure' function '%s' cannot assign to global '%s': writing a global "
             "variable is a side effect",
             fn->name, root->name);
        return;
    }
    if (sym->kind == SYM_PARAM && target != root && pure_param_escapes(sym->type))
        SERR(s, 13, span,
             "'pure' function '%s' cannot write through parameter '%s': the write is "
             "visible to the caller",
             fn->name, root->name);
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

    if (declared && declared->kind == TY_MAP && n->a && n->a->kind == AST_STRUCT_LIT &&
        n->a->name && strcmp(n->a->name, "HashMap") == 0) {
        n->a->type_str = type_to_string(s->tc, declared);
        n->type_str = n->a->type_str;
        return declared;
    }

    if (declared && declared->kind == TY_VEC && n->a && n->a->kind == AST_STRUCT_LIT &&
        n->a->name && strcmp(n->a->name, "Vector") == 0) {
        n->a->type_str = type_to_string(s->tc, declared);
        n->type_str = n->a->type_str;
        return declared;
    }

    if (declared && declared->kind == TY_STRUCT && n->a && n->a->kind == AST_STRUCT_LIT &&
        n->a->name &&
        (generic_template(s, n->a->name, SYM_STRUCT) ||
         !scope_lookup(s->global, n->a->name))) {
        n->a->name = declared->name;
    }

    if (declared && declared->kind == TY_ARRAY && declared->elem &&
        declared->elem->kind == TY_DYN && n->a && n->a->kind == AST_ARRAY_LIT)
        s->expected = declared;
    else if (declared && n->a && (n->a->kind == AST_CALL || n->a->kind == AST_LITERAL))
        s->expected = declared;
    type_t *initt = n->a ? sema_check_expr(s, n->a) : NULL;
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
        SERR(s, 1, &n->span, "cannot infer type of '%s' (no type or initializer)",
             n->name);
        t = err_ty(s);
    }
    n->type_str = type_to_string(s->tc, t);
    return t;
}

static void check_stmt(sema_t *s, ast_node_t *n)
{
    if (!n) return;
    switch (n->kind) {
    case AST_BLOCK:
        sema_check_block(s, n);
        break;
    case AST_VAR_DECL: {
        type_t *t = sema_check_var_decl(s, n);
        define_local(s, n, n->is_mut ? SYM_VAR : SYM_VAR, t);
        break;
    }
    case AST_CONST_DECL: {
        type_t *t = sema_check_var_decl(s, n);
        symbol_t *sym = symbol_new(s->a, SYM_CONST, n->name);
        sym->type = t;
        sym->is_mut = false;
        sym->decl = n;
        if (n->a && n->a->kind == AST_LITERAL && n->a->value.kind == TV_INT) {
            sym->has_ival = true;
            sym->ival = (long long)n->a->value.as.i;
        }
        if (scope_define(s->a, s->cur, sym))
            SERR(s, 1, &n->span, "redefinition of '%s'", n->name);
        break;
    }
    case AST_ASSIGN: {
        type_t *tt = sema_check_expr(s, n->a);
        if (tt && n->b && n->b->kind == AST_LITERAL) s->expected = tt;
        type_t *vt = sema_check_expr(s, n->b);
        symbol_t *wroot = NULL;
        switch (sema_classify_write(s, n->a, &wroot)) {
        case LV_NOT_LVALUE:
            SERR(s, 13, &n->span, "assignment target is not assignable");
            break;
        case LV_CONST:
            SERR(s, 13, &n->span,
                 "cannot assign to '%s': a 'const' binding is fully immutable, "
                 "including its elements and fields",
                 wroot ? wroot->name : "target");
            break;
        case LV_IMMUTABLE:
            SERR(s, 13, &n->span,
                 "cannot reassign immutable variable '%s'; declare it 'mut' to "
                 "reassign it, or assign to its elements or fields instead",
                 wroot ? wroot->name : "target");
            break;
        case LV_OK:
            sema_check_pure_write(s, n->a, &n->span);
            break;
        }

        if (n->op == TK_ASSIGN && n->a && n->a->kind == AST_INDEX && n->a->a &&
            n->a->a->type_str) {
            symbol_t *ssym = NULL;
            {
                type_t *obst = n->a->a->type_str ? type_prim(s->tc, TY_ERROR) : NULL;
                (void)obst;
                symbol_t *tmp = scope_lookup(s->cur, n->a->a->type_str);
                if (!tmp) {
                    size_t bl = strlen(n->a->a->type_str);
                    if (bl > 1 && n->a->a->type_str[bl - 1] == '*') {
                        char *base = arena_strndup(s->a, n->a->a->type_str, bl - 1);
                        tmp = scope_lookup(s->cur, base);
                    }
                }
                if (tmp && tmp->kind == SYM_STRUCT) ssym = tmp;
            }
            if (ssym) {
                symbol_t *m = scope_lookup_local(ssym->members, "operator_index_set");
                if (m && m->kind == SYM_METHOD) break;
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

        if (n->op == TK_PLUS_EQ && tt && tt->kind == TY_STR &&
            sema_type_is_stringable(vt))
            break;

        bool op_valid = true;
        if (n->op != TK_ASSIGN && tt && !type_is_error(tt) && !type_is_error(vt)) {
            token_kind_t base_op = sema_compound_base(n->op);
            if (base_op == TK_POWER) {
                type_t *pow_res = type_prim(s->tc, TY_F64);
                if (!type_is_numeric(tt) || !type_is_numeric(vt)) {
                    SERR(s, 21, &n->span, "operator '**' requires numeric operands");
                    op_valid = false;
                } else if (!type_assignable(tt, pow_res)) {
                    SERR(s, 2, &n->span, "cannot assign '%s' to '%s'",
                         type_to_string(s->tc, pow_res), type_to_string(s->tc, tt));
                    op_valid = false;
                }
            } else if (base_op != TK_EOF) {
                type_t *c =
                    (tt->kind == TY_STRUCT) ? NULL : type_common_arith(s->tc, tt, vt);
                if (!c) {
                    SERR(s, 21, &n->span, "operator cannot be applied to '%s' and '%s'",
                         type_to_string(s->tc, tt), type_to_string(s->tc, vt));
                    op_valid = false;
                } else if (base_op == TK_PERCENT && !type_is_integer(c)) {
                    SERR(s, 21, &n->span, "operator '%%' requires integer operands");
                    op_valid = false;
                }
            }
        }
        if (op_valid && !type_assignable(tt, vt))
            SERR(s, 2, &n->span, "cannot assign '%s' to '%s'", type_to_string(s->tc, vt),
                 type_to_string(s->tc, tt));
        break;
    }
    case AST_IF: {
        type_t *c = sema_check_expr(s, n->a);
        if (c->kind != TY_BOOL && !type_is_error(c))
            SERR(s, 21, &n->a->span, "if condition must be bool, got '%s'",
                 type_to_string(s->tc, c));
        if (n->b && n->b->kind == AST_BLOCK && n->b->list.len == 0)
            SERR(s, 60, &n->b->span,
                 "empty 'if' branch (it must contain at least one statement)");
        check_stmt(s, n->b);
        if (n->c) {
            if (n->c->kind == AST_BLOCK && n->c->list.len == 0)
                SERR(s, 60, &n->c->span,
                     "empty 'else' branch (it must contain at least one statement)");
            check_stmt(s, n->c);
        }
        break;
    }
    case AST_WHILE: {
        type_t *c = sema_check_expr(s, n->a);
        if (c->kind != TY_BOOL && !type_is_error(c))
            SERR(s, 21, &n->a->span, "while condition must be bool, got '%s'",
                 type_to_string(s->tc, c));
        s->loop_depth++;
        check_stmt(s, n->b);
        s->loop_depth--;
        break;
    }
    case AST_REPEAT: {
        type_t *c = sema_check_expr(s, n->a);
        if (!type_is_numeric(c) && !type_is_error(c))
            SERR(s, 63, &n->a->span, "repeat count must be a number, got '%s'",
                 type_to_string(s->tc, c));
        if (n->c) {
            type_t *e = sema_check_expr(s, n->c);
            if (!type_is_numeric(e) && !type_is_error(e))
                SERR(s, 63, &n->c->span, "repeat range bound must be a number, got '%s'",
                     type_to_string(s->tc, e));
        }
        if (n->d) {
            type_t *st = sema_check_expr(s, n->d);
            if (!type_is_numeric(st) && !type_is_error(st))
                SERR(s, 63, &n->d->span, "repeat step must be a number, got '%s'",
                     type_to_string(s->tc, st));
        }
        s->loop_depth++;
        check_stmt(s, n->b);
        s->loop_depth--;
        break;
    }
    case AST_FOR: {
        scope_t *sc = scope_new(s->a, SCOPE_BLOCK, s->cur);
        scope_t *saved = s->cur;
        s->cur = sc;
        if (n->a) check_stmt(s, n->a);
        if (n->b) {
            type_t *c = sema_check_expr(s, n->b);
            if (c->kind != TY_BOOL && !type_is_error(c))
                SERR(s, 21, &n->b->span, "for condition must be bool, got '%s'",
                     type_to_string(s->tc, c));
        }
        if (n->c) check_stmt(s, n->c);
        s->loop_depth++;
        check_stmt(s, n->d);
        s->loop_depth--;
        s->cur = saved;
        break;
    }
    case AST_RETURN: {
        if (s->cur_func && s->cur_func->decl && s->cur_func->decl->is_noret &&
            s->cur_func->decl->name)
            SERR(s, 12, &n->span,
                 "'noret' function '%s' cannot contain 'ret': it never returns",
                 s->cur_func->decl->name);
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
            else if (ret->kind == TY_DYN)
                n->a = coerce_to_dyn(s, ret, n->a, vt);
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
    case AST_EXPR_STMT:
        sema_check_expr(s, n->a);
        break;
    case AST_INCDEC:
        sema_check_expr(s, n);
        break;
    default:
        break;
    }
}

void sema_check_block(sema_t *s, ast_node_t *block)
{
    scope_t *sc = scope_new(s->a, SCOPE_BLOCK, s->cur);
    scope_t *saved = s->cur;
    s->cur = sc;
    LOG_D(s->log, PH_SEMANTIC, "enter block scope (%zu stmts)", block->list.len);
    {
        size_t i = 0;
        for (; i < block->list.len; i++)
            check_stmt(s, (ast_node_t *)block->list.data[i]);
    }

    {
        size_t i = 0;
        for (; i < sc->symbols.len; i++) {
            symbol_t *v = (symbol_t *)sc->symbols.data[i];
            if (v->kind == SYM_VAR && !v->used && v->decl && v->name && v->name[0] != '_')
                SERR(s, 59, &v->decl->span,
                     "unused variable '%s' (prefix with '_' if intentional)", v->name);
        }
    }
    s->cur = saved;
    LOG_D(s->log, PH_SEMANTIC, "exit block scope");
}
