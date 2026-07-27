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
#include "parser/parser_internal.h"

static void parse_array_dims(parser_t *p, ast_node_t *n)
{
    while (p_match(p, TK_LBRACKET)) {
        p_skip_terminators(p);

        if (p_at(p, TK_COLON)) {
            p_advance(p);
            p_skip_terminators(p);
            p_expect(p, TK_RBRACKET, "']' to close slice type 'T[:]'");
            n->is_slice = true;
            return;
        }
        ast_node_t *dim = NULL;
        if (!p_at(p, TK_RBRACKET)) dim = parse_expr(p);
        p_skip_terminators(p);
        p_expect(p, TK_RBRACKET, "']' in array type");
        vec_push(p->a, &n->dims, dim);
    }
}

static void parse_ptr_suffix(parser_t *p, ast_node_t *n)
{
    if (p_match(p, TK_STAR)) n->is_pointer = true;
}

static ast_node_t *parse_type_dyn(parser_t *p, ast_node_t *n)
{
    p_advance(p);
    n->is_dyn = true;
    n->name = p_peek(p)->lexeme;
    p_advance(p);
    parse_array_dims(p, n);
    parse_ptr_suffix(p, n);
    p_fin(p, n);
    return n;
}

static ast_node_t *parse_type_func(parser_t *p, ast_node_t *n)
{
    n->name = "func";
    p_advance(p);
    p_expect(p, TK_LPAREN, "'(' in function type");
    p_comma_list(p, n, TK_RPAREN, parse_type);
    p_expect(p, TK_RPAREN, "')' in function type");
    if (p_at(p, TK_IDENT) || p_at(p, TK_KW_FUNC)) n->type = parse_type(p);
    parse_ptr_suffix(p, n);
    p_fin(p, n);
    return n;
}

static ast_node_t *parse_type_named(parser_t *p, ast_node_t *n)
{
    n->name = p_name(p, "expected type name");
    if (!n->name) return n;

    if (p_at(p, TK_DOT) && p_peek2(p)->kind == TK_IDENT) {
        p_advance(p);
        const char *pkg = n->name, *ty = p_peek(p)->lexeme;
        p_advance(p);
        size_t ln = strlen(pkg) + strlen(ty) + 2;
        char *q = (char *)arena_alloc(p->a, ln);
        sal_snprintf(q, ln, "%s.%s", pkg, ty);
        n->name = q;
    }

    if (p_at(p, TK_LT)) {
        p_advance(p);
        ast_add(p->a, n, parse_type(p));
        while (p_match(p, TK_COMMA))
            ast_add(p->a, n, parse_type(p));
        p_close_angle(p, "'>' to close type arguments");
    }
    parse_array_dims(p, n);
    parse_ptr_suffix(p, n);
    p_fin(p, n);
    return n;
}

ast_node_t *parse_type(parser_t *p)
{
    P_RULE(p, "type");

    if (!p_recurse_enter(p, "type nested too deeply")) return p_mk(p, AST_TYPE);
    ast_node_t *n = p_mk(p, AST_TYPE);
    if (p_at(p, TK_KW_EXTERN) && p_peek2(p)->kind == TK_KW_FUNC) {
        p_advance(p);
        n = parse_type_func(p, n);
        n->is_extern = true;
    } else if (p_at(p, TK_IDENT) && strcmp(p_peek(p)->lexeme, "dyn") == 0 &&
               p_peek2(p)->kind == TK_IDENT)
        n = parse_type_dyn(p, n);
    else if (p_at(p, TK_KW_FUNC))
        n = parse_type_func(p, n);
    else
        n = parse_type_named(p, n);
    p_recurse_leave(p);
    return n;
}

ast_node_t *parse_type_anno(parser_t *p)
{
    p_expect(p, TK_COLON, "':' before type");
    return parse_type(p);
}
