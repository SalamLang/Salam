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
#include "parser/parser_internal.h"

static void demote_auto_type(ast_node_t *n)
{
    if (n->type && n->type->name && !n->type->is_pointer && !n->type->is_dyn &&
        n->type->list.len == 0 && n->type->dims.len == 0 &&
        (strcmp(n->type->name, "auto") == 0 ||
         strcmp(n->type->name, "\xD8\xAE\xD9\x88\xD8\xAF\xDA\xA9\xD8\xA7\xD8\xB1") == 0 ||
         strcmp(n->type->name, "\xD8\xAA\xD9\x84\xD9\x82\xD8\xA7\xD8\xA6\xD9\x8A") == 0))
        n->type = NULL;
}

static void parse_decl_tail(parser_t *p, ast_node_t *n)
{
    if (p_match(p, TK_COLON_ASSIGN)) {
        n->a = parse_expr(p);
        return;
    }
    if (p_at(p, TK_COLON)) {
        n->type = parse_type_anno(p);
        demote_auto_type(n);
        if (p_at(p, TK_ASSIGN)) {
            p_error(p, "declarations with a type annotation were removed; use "
                       "'name := value' (or 'name := value as Type')");
            p_advance(p);
            n->a = parse_expr(p);
        }
        return;
    }
    if (p_at(p, TK_ASSIGN)) {
        p_error(p, "use ':=' to declare a new variable ('name := value'); "
                   "'=' only assigns to an existing one");
        p_advance(p);
        n->a = parse_expr(p);
        return;
    }
    p_error(p, "expected ':=' and an initializer, or ': Type' for an "
               "uninitialized declaration");
}

ast_node_t *parse_var_decl(parser_t *p)
{
    P_RULE(p, "var_decl");
    ast_node_t *n = p_mk(p, AST_VAR_DECL);
    n->is_mut = p_match(p, TK_KW_MUT);
    n->name = parse_decl_name(p);
    if (!n->name) n->name = "<error>";
    parse_decl_tail(p, n);
    p_fin(p, n);
    return n;
}

ast_node_t *parse_bare_var_decl(parser_t *p)
{
    P_RULE(p, "bare_var_decl");
    ast_node_t *n = p_mk(p, AST_VAR_DECL);
    n->is_mut = false;
    n->name = parse_decl_name(p);
    if (!n->name) n->name = "<error>";
    parse_decl_tail(p, n);
    p_fin(p, n);
    return n;
}
