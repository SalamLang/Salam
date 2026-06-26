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
         strcmp(n->type->name, "\xD8\xAE\xD9\x88\xD8\xAF\xDA\xA9\xD8\xA7\xD8\xB1") == 0))
        n->type = NULL;
}

static bool at_type_anno(const parser_t *p)
{
    return p_at(p, TK_COLON) || p_at(p, TK_IDENT) || p_at(p, TK_KW_FUNC);
}

ast_node_t *parse_var_decl(parser_t *p)
{
    P_RULE(p, "var_decl");
    ast_node_t *n = p_mk(p, AST_VAR_DECL);
    n->is_mut = p_match(p, TK_KW_MUT);              
    n->name = parse_decl_name(p);
    if (!n->name) n->name = "<error>";
    if (at_type_anno(p)) n->type = parse_type_anno(p);
    demote_auto_type(n);
    if (p_match(p, TK_ASSIGN)) n->a = parse_expr(p);
    p_fin(p, n);
    return n;
}

ast_node_t *parse_bare_var_decl(parser_t *p)
{
    P_RULE(p, "bare_var_decl");
    ast_node_t *n = p_mk(p, AST_VAR_DECL);
    n->is_mut = false;
    n->name = parse_decl_name(p);                   
    n->type = parse_type_anno(p);                   
    demote_auto_type(n);
    if (p_match(p, TK_ASSIGN)) n->a = parse_expr(p);
    p_fin(p, n);
    return n;
}
