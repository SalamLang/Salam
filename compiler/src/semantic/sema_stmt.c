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

static void check_each(sema_t *s, ast_node_t *n);

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
    if (lv.is_projection) {
        if (lv.root && (lv.root->is_mut || lv.root->is_ref)) lv.root->mutated = true;
        return LV_OK;
    }
    if (!lv.root) return LV_OK;
    if (lv.root->is_mut || lv.root->is_ref) {
        lv.root->mutated = true;
        return LV_OK;
    }
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
    if (!n->type && n->a && n->a->kind == AST_CAST && n->a->type &&
        (n->a->a->kind == AST_ARRAY_LIT || n->a->a->kind == AST_STRUCT_LIT ||
         n->a->a->kind == AST_LITERAL)) {
        n->type = n->a->type;
        n->a = n->a->a;
    }
    type_t *declared = n->type ? sema_resolve_type(s, n->type) : NULL;

    if (n->a && n->a->kind == AST_STRUCT_LIT && n->a->name) {
        const char *orig = n->a->name;
        n->a->name = intrinsic_type_canon(n->a->name);
        sema_check_intrinsic_type_lang(s, orig, &n->a->span);
    }

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

    if (declared && n->a &&
        (n->a->kind == AST_CALL || n->a->kind == AST_LITERAL ||
         n->a->kind == AST_ARRAY_LIT || n->a->kind == AST_TERNARY ||
         n->a->kind == AST_MATCH))
        s->expected = declared;
    type_t *initt = n->a ? sema_check_expr(s, n->a) : NULL;
    type_t *t;
    if (declared && initt) {
        if (!type_assignable(declared, initt))
            SERR(s, 2, &n->span, "cannot assign '%s' to '%s' in declaration of '%s'",
                 type_to_string(s->tc, initt), type_to_string(s->tc, declared), n->name);
        if (declared->kind == TY_DYN) n->a = coerce_to_dyn(s, declared, n->a, initt);
        if (declared->kind == TY_VARIANT && initt->kind != TY_VARIANT) {
            int tag = type_variant_tag(declared, initt);
            if (tag >= 0) n->a = coerce_to_variant(s, declared, n->a, tag);
        }
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

static ast_node_t *ea_node(sema_t *s, ast_kind_t k, const src_span_t *sp)
{
    ast_node_t *n = ast_new(s->a, k, sp);
    n->synthetic = true;
    return n;
}

static ast_node_t *ea_ident(sema_t *s, const char *name, const src_span_t *sp)
{
    ast_node_t *n = ea_node(s, AST_IDENTIFIER, sp);
    n->name = name;
    return n;
}

static ast_node_t *ea_int(sema_t *s, uint64_t v, const src_span_t *sp)
{
    ast_node_t *n = ea_node(s, AST_LITERAL, sp);
    n->op = TK_INT;
    n->value.kind = TV_INT;
    n->value.as.i = v;
    return n;
}

static ast_node_t *ea_decl(sema_t *s, const char *name, ast_node_t *init, bool is_mut,
                           const src_span_t *sp)
{
    ast_node_t *n = ea_node(s, AST_VAR_DECL, sp);
    n->name = name;
    n->a = init;
    n->is_mut = is_mut;
    return n;
}

static ast_node_t *ea_mcall(sema_t *s, const char *recv, const char *method,
                            ast_node_t *arg, const src_span_t *sp)
{
    ast_node_t *mem = ea_node(s, AST_MEMBER, sp);
    mem->a = ea_ident(s, recv, sp);
    mem->name = method;
    {
        ast_node_t *call = ea_node(s, AST_CALL, sp);
        call->a = mem;
        if (arg) ast_add(s->a, call, arg);
        return call;
    }
}

static ast_node_t *ea_index(sema_t *s, ast_node_t *base, ast_node_t *idx,
                            const src_span_t *sp)
{
    ast_node_t *n = ea_node(s, AST_INDEX, sp);
    n->a = base;
    n->b = idx;
    return n;
}

static ast_node_t *ea_binary(sema_t *s, token_kind_t op, ast_node_t *l, ast_node_t *r,
                             const src_span_t *sp)
{
    ast_node_t *n = ea_node(s, AST_BINARY, sp);
    n->op = op;
    n->a = l;
    n->b = r;
    return n;
}

static const char *ea_name(sema_t *s, const char *prefix, int id)
{
    char buf[32];
    sal_snprintf(buf, sizeof buf, "%s%d", prefix, id);
    return arena_strdup(s->a, buf);
}

static void each_finish(sema_t *s, ast_node_t *n, ast_node_t *seq_decl, ast_node_t *fr)
{
    n->kind = AST_BLOCK;
    n->name = NULL;
    n->a = NULL;
    n->b = NULL;
    n->c = NULL;
    n->d = NULL;
    n->synthetic = true;
    vec_init(&n->list);
    if (seq_decl) vec_push(s->a, &n->list, seq_decl);
    vec_push(s->a, &n->list, fr);
    sema_check_block(s, n);
}

static void check_each(sema_t *s, ast_node_t *n)
{
    src_span_t sp = n->span;
    type_t *it = sema_check_expr(s, n->a);
    if (n->a->kind == AST_ARRAY_LIT && n->a->list.len == 0)
        SERR(s, 68, &n->a->span,
             "'each' over an empty array literal: the loop body can never run");
    if (type_is_error(it)) return;
    {
        const char *keyname = n->c ? n->c->name : NULL;
        const char *valname = n->name;
        ast_node_t *body = n->b;
        int id = ++s->each_n;
        const char *seq;
        ast_node_t *seq_decl = NULL;
        bool vec_like = it->kind == TY_VEC;
        bool map_like = it->kind == TY_MAP;

        if (it->kind == TY_STRUCT && it->decl) {
            symbol_t *isym = (symbol_t *)it->decl;
            if (isym->generic_base && strcmp(isym->generic_base, "Vector") == 0)
                vec_like = true;
            if (isym->generic_base && strcmp(isym->generic_base, "HashMap") == 0)
                map_like = true;
        }

        if (!vec_like && !map_like && it->kind != TY_ARRAY && it->kind != TY_SLICE) {
            SERR(s, 65, &n->a->span, "each cannot iterate a value of type '%s'",
                 type_to_string(s->tc, it));
            return;
        }
        if (map_like && !keyname) {
            SERR(s, 65, &sp,
                 "iterating a HashMap needs two bindings: each (key, value) in map");
            return;
        }
        if ((it->kind == TY_ARRAY || it->kind == TY_SLICE) && it->elem &&
            it->elem->kind == TY_ARRAY) {
            SERR(s, 65, &sp,
                 "each over a multi-dimensional array is not supported; use repeat "
                 "with an index");
            return;
        }

        if (n->a->kind == AST_IDENTIFIER) {
            seq = n->a->name;
        } else {
            seq = ea_name(s, "__ea", id);
            seq_decl = ea_decl(s, seq, n->a, false, &sp);
        }

        if (map_like) {
            const char *itn = ea_name(s, "__ei", id);
            ast_node_t *fr = ea_node(s, AST_FOR, &sp);
            fr->a = ea_decl(s, itn, ea_mcall(s, seq, "iter", NULL, &sp), true, &sp);
            fr->b = ea_mcall(s, itn, "has_next", NULL, &sp);
            {
                ast_node_t *post = ea_node(s, AST_EXPR_STMT, &sp);
                post->a = ea_mcall(s, itn, "next", NULL, &sp);
                fr->c = post;
            }
            {
                ast_node_t *blk = ea_node(s, AST_BLOCK, &sp);
                ast_add(s->a, blk,
                        ea_decl(s, keyname, ea_mcall(s, itn, "key", NULL, &sp), false,
                                n->c ? &n->c->span : &sp));
                ast_add(s->a, blk,
                        ea_decl(s, valname, ea_mcall(s, itn, "value", NULL, &sp), false,
                                &sp));
                {
                    size_t i = 0;
                    for (; i < body->list.len; i++)
                        ast_add(s->a, blk, (ast_node_t *)body->list.data[i]);
                }
                fr->d = blk;
            }
            each_finish(s, n, seq_decl, fr);
            return;
        }

        {
            const char *idx = ea_name(s, "__ei", id);
            ast_node_t *fr = ea_node(s, AST_FOR, &sp);
            fr->a = ea_decl(s, idx, ea_int(s, 0, &sp), true, &sp);
            {
                ast_node_t *lenx;
                if (vec_like) {
                    lenx = ea_mcall(s, seq, "len", NULL, &sp);
                } else {
                    lenx = ea_node(s, AST_CALL, &sp);
                    lenx->a = ea_ident(s, "len", &sp);
                    ast_add(s->a, lenx, ea_ident(s, seq, &sp));
                }
                fr->b = ea_binary(s, TK_LT, ea_ident(s, idx, &sp), lenx, &sp);
            }
            {
                ast_node_t *asn = ea_node(s, AST_ASSIGN, &sp);
                asn->op = TK_ASSIGN;
                asn->a = ea_ident(s, idx, &sp);
                asn->b =
                    ea_binary(s, TK_PLUS, ea_ident(s, idx, &sp), ea_int(s, 1, &sp), &sp);
                fr->c = asn;
            }
            {
                ast_node_t *blk = ea_node(s, AST_BLOCK, &sp);
                if (keyname)
                    ast_add(s->a, blk,
                            ea_decl(s, keyname, ea_ident(s, idx, &sp), false,
                                    n->c ? &n->c->span : &sp));
                {
                    ast_node_t *elem;
                    if (vec_like)
                        elem = ea_index(
                            s, ea_mcall(s, seq, "get", ea_ident(s, idx, &sp), &sp),
                            ea_int(s, 0, &sp), &sp);
                    else
                        elem = ea_index(s, ea_ident(s, seq, &sp), ea_ident(s, idx, &sp),
                                        &sp);
                    ast_add(s->a, blk, ea_decl(s, valname, elem, false, &sp));
                }
                {
                    size_t i = 0;
                    for (; i < body->list.len; i++)
                        ast_add(s->a, blk, (ast_node_t *)body->list.data[i]);
                }
                fr->d = blk;
            }
            each_finish(s, n, seq_decl, fr);
        }
    }
}

static void check_stmt(sema_t *s, ast_node_t *n)
{
    if (!n) return;
    switch (n->kind) {
    case AST_BLOCK:
        sema_check_block(s, n);
        break;
    case AST_MATCH:
        sema_check_match(s, n, false, NULL);
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
        if (n->a && n->a->kind == AST_INDEX && n->a->a && n->a->a->type_str &&
            !strcmp(n->a->a->type_str, "str"))
            SERR(s, 13, &n->span,
                 "strings are immutable: cannot assign to an index of a 'str'");
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
        else if (op_valid && n->op == TK_ASSIGN && tt && tt->kind == TY_VARIANT && vt &&
                 vt->kind != TY_VARIANT) {
            int tag = type_variant_tag(tt, vt);
            if (tag >= 0) n->b = coerce_to_variant(s, tt, n->b, tag);
        }
        break;
    }
    case AST_IF: {
        type_t *c = sema_check_expr(s, n->a);
        if (c->kind != TY_BOOL && !type_is_error(c))
            SERR(s, 21, &n->a->span, "if condition must be bool, got '%s'",
                 type_to_string(s->tc, c));
        if (n->a->kind == AST_LITERAL && n->a->op == TK_KW_FALSE)
            SERR(s, 68, &n->a->span,
                 "'if' condition is always false: its body can never run");
        else if (n->a->kind == AST_LITERAL && n->a->op == TK_KW_TRUE) {
            if (n->c)
                SERR(s, 68, &n->a->span,
                     "'if' condition is always true: the 'else' branch can never run");
            else
                SERR(s, 68, &n->a->span,
                     "'if' condition is always true: remove the redundant 'if'");
        }
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
    case AST_UNTIL: {
        type_t *c = sema_check_expr(s, n->a);
        if (c->kind != TY_BOOL && !type_is_error(c))
            SERR(s, 21, &n->a->span, "until condition must be bool, got '%s'",
                 type_to_string(s->tc, c));
        if (n->a->kind == AST_LITERAL && n->a->op == TK_KW_FALSE)
            SERR(s, 68, &n->a->span,
                 "'until' condition is always false: the loop body can never run");
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
        if (!n->c) {
            if (n->a->kind == AST_LITERAL &&
                ((n->a->value.kind == TV_INT && n->a->value.as.i == 0) ||
                 (n->a->value.kind == TV_FLOAT && n->a->value.as.f == 0)))
                SERR(s, 68, &n->a->span,
                     "'repeat' count is the constant 0: the loop body can never run");
            else if ((n->a->kind == AST_UNARY && n->a->op == TK_MINUS && n->a->a &&
                      n->a->a->kind == AST_LITERAL) ||
                     (n->a->kind == AST_LITERAL && n->a->value.kind == TV_INT &&
                      n->a->type_str && n->a->type_str[0] != 'u' &&
                      (long long)n->a->value.as.i < 0))
                SERR(s, 68, &n->a->span,
                     "'repeat' count is a negative constant: the loop body can never "
                     "run");
        }
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
            if (n->d->kind == AST_LITERAL && n->d->value.kind == TV_INT &&
                n->d->value.as.i == 0)
                SERR(s, 63, &n->d->span,
                     "repeat step must be a positive number (the direction comes from "
                     "the bounds)");
            if (n->d->kind == AST_UNARY && n->d->op == TK_MINUS)
                SERR(s, 63, &n->d->span,
                     "repeat step must be a positive number (the direction comes from "
                     "the bounds)");
        }
        scope_t *saved = s->cur;
        if (n->name) {
            scope_t *sc = scope_new(s->a, SCOPE_BLOCK, s->cur);
            symbol_t *iv = symbol_new(s->a, SYM_VAR, n->name);
            iv->type = ty(s, TY_I32);
            iv->is_mut = false;
            iv->decl = n;
            scope_define(s->a, sc, iv);
            s->cur = sc;
        }
        s->loop_depth++;
        check_stmt(s, n->b);
        s->loop_depth--;
        s->cur = saved;
        break;
    }
    case AST_EACH:
        check_each(s, n);
        break;
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
        if (s->match_arm_depth > 0 && s->match_yield_collect) {
            type_t *saved_exp = s->expected;
            type_t *vt;
            match_yield_t *y;
            s->expected = s->match_yield_expected;
            vt = n->a ? sema_check_expr(s, n->a) : ty(s, TY_VOID);
            s->expected = saved_exp;
            y = (match_yield_t *)arena_alloc(s->a, sizeof(*y));
            y->ret = n;
            y->type = vt;
            vec_push(s->a, s->match_yield_collect, y);
            break;
        }
        if (s->cur_func && s->cur_func->decl && s->cur_func->decl->is_noret &&
            s->cur_func->decl->name)
            SERR(s, 12, &n->span,
                 "'noret' function '%s' cannot contain 'ret': it never returns",
                 s->cur_func->decl->name);
        if (s->cur_func && s->cur_func->infer_ret) {
            type_t *vt = n->a ? sema_check_expr(s, n->a) : ty(s, TY_VOID);
            s->cur_func->ret = vt;
            s->cur_func->ret_span = n->span;
            s->cur_func->ret_weak =
                n->a && n->a->kind == AST_LITERAL && n->a->op == TK_INT;
            s->cur_func->infer_ret = false;
            break;
        }
        type_t *ret = s->cur_func ? s->cur_func->ret : ty(s, TY_VOID);
        if (n->a) {
            s->expected = ret;
            type_t *vt = sema_check_expr(s, n->a);
            s->expected = NULL;
            if (s->cur_func && type_is_float(ret)) {
                if (type_is_integer(vt)) {
                    if (!s->cur_func->ret_int_seen) s->cur_func->ret_int_span = n->span;
                    s->cur_func->ret_int_seen = true;
                } else if (type_is_float(vt))
                    s->cur_func->ret_float_seen = true;
            }
            if (ret->kind == TY_VOID)
                SERR(s, 49, &n->span, "cannot return a value from a void function");
            else if (!type_assignable(ret, vt)) {
                bool inferred =
                    s->cur_func && s->cur_func->decl && !s->cur_func->decl->type;
                if (inferred && !s->lam && s->cur_func->ret_weak &&
                    type_is_numeric(ret) && type_is_numeric(vt)) {
                    s->cur_func->ret = vt;
                    s->cur_func->ret_span = n->span;
                    s->cur_func->ret_weak =
                        n->a->kind == AST_LITERAL && n->a->op == TK_INT;
                    s->cur_func->ret_widened = true;
                } else if (inferred)
                    SERR(s, 2, &n->span,
                         "conflicting inferred return types for '%s': an earlier "
                         "'ret' (line %u) produces '%s', this one produces '%s'; "
                         "annotate the function's return type explicitly",
                         s->cur_func->decl->name, s->cur_func->ret_span.begin.line,
                         type_to_string(s->tc, ret), type_to_string(s->tc, vt));
                else
                    SERR(s, 2, &n->span, "return type mismatch: expected '%s', got '%s'",
                         type_to_string(s->tc, ret), type_to_string(s->tc, vt));
            } else if (ret->kind == TY_DYN)
                n->a = coerce_to_dyn(s, ret, n->a, vt);
            else if (ret->kind == TY_VARIANT && vt->kind != TY_VARIANT) {
                int tag = type_variant_tag(ret, vt);
                if (tag >= 0) n->a = coerce_to_variant(s, ret, n->a, tag);
            }
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

static bool loopless_break(ast_node_t *n)
{
    if (!n) return false;
    switch (n->kind) {
    case AST_BREAK:
        return true;
    case AST_BLOCK: {
        size_t i = 0;
        for (; i < n->list.len; i++)
            if (loopless_break((ast_node_t *)n->list.data[i])) return true;
        return false;
    }
    case AST_IF:
        return loopless_break(n->b) || loopless_break(n->c);
    case AST_MATCH: {
        size_t i = 0;
        for (; i < n->list.len; i++) {
            ast_node_t *arm = (ast_node_t *)n->list.data[i];
            if (loopless_break(arm->b)) return true;
        }
        return false;
    }
    default:
        return false;
    }
}

bool sema_stmt_terminates(sema_t *s, ast_node_t *n)
{
    if (!n) return false;
    switch (n->kind) {
    case AST_RETURN:
    case AST_BREAK:
    case AST_CONTINUE:
        return true;
    case AST_BLOCK: {
        size_t i = 0;
        for (; i < n->list.len; i++)
            if (sema_stmt_terminates(s, (ast_node_t *)n->list.data[i])) return true;
        return false;
    }
    case AST_IF:
        return n->c && sema_stmt_terminates(s, n->b) && sema_stmt_terminates(s, n->c);
    case AST_UNTIL:
        return n->a && n->a->kind == AST_LITERAL && n->a->op == TK_KW_TRUE &&
               !loopless_break(n->b);
    case AST_MATCH: {
        bool has_else = false;
        size_t i = 0;
        if (n->list.len == 0) return false;
        for (; i < n->list.len; i++) {
            ast_node_t *arm = (ast_node_t *)n->list.data[i];
            if (arm->op == TK_KW_ELSE) has_else = true;
            if (!sema_stmt_terminates(s, arm->b)) return false;
        }
        return has_else;
    }
    case AST_EXPR_STMT: {
        ast_node_t *c = n->a;
        if (c && c->kind == AST_CALL && c->a && c->a->kind == AST_IDENTIFIER) {
            symbol_t *sym = scope_lookup(s->cur, c->a->name);
            if (sym && sym->kind == SYM_FUNC && sym->overloads.len == 1) {
                func_sig_t *sig = (func_sig_t *)sym->overloads.data[0];
                if (sig->decl && sig->decl->is_noret) return true;
            }
        }
        return false;
    }
    default:
        return false;
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
    if (!s->requal) {
        size_t i = 0;
        for (; i + 1 < block->list.len; i++) {
            if (sema_stmt_terminates(s, (ast_node_t *)block->list.data[i])) {
                ast_node_t *dead = (ast_node_t *)block->list.data[i + 1];
                SERR(s, 70, &dead->span,
                     "unreachable code: the previous statement always exits this "
                     "block");
                break;
            }
        }
    }

    {
        size_t i = 0;
        for (; i < sc->symbols.len; i++) {
            symbol_t *v = (symbol_t *)sc->symbols.data[i];
            if (v->kind == SYM_VAR && !v->used && v->decl && v->name && v->name[0] != '_')
                SERR(s, 59, &v->decl->span,
                     "unused variable '%s' (prefix with '_' if intentional)", v->name);
            else if (v->kind == SYM_VAR && v->is_mut && !v->mutated && v->decl &&
                     !v->decl->synthetic && v->name && v->name[0] != '_')
                SERR(s, 69, &v->decl->span,
                     "variable '%s' is declared 'mut' but never mutated; remove 'mut' "
                     "or declare it 'const'",
                     v->name);
        }
    }
    s->cur = saved;
    LOG_D(s->log, PH_SEMANTIC, "exit block scope");
}
