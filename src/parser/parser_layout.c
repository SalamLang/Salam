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

static ast_node_t *layout_element(parser_t *p);
static ast_node_t *layout_attribute(parser_t *p);
static void layout_body(parser_t *p, ast_node_t *parent)
{
    while (!p_at(p, TK_KW_END) && !p_at_eof(p)) {
        p_skip_terminators(p);
        if (p_at(p, TK_KW_END) || p_at_eof(p)) break;
        if (p_at(p, TK_LAYOUT_ELEMENT)) {
            ast_add(p->a, parent, layout_element(p));
        } else if (p_at(p, TK_LAYOUT_ATTR_NAME)) {
            ast_add(p->a, parent, layout_attribute(p));
        } else {
            p_error(p, "unexpected token inside layout block");
            if (!p_at_eof(p)) p_advance(p);
        }
        if (p->panic) p_sync(p);
    }
}

static ast_node_t *layout_element(parser_t *p)
{
    P_RULE(p, "layout_element");
    ast_node_t *n = ast_new(p->a, AST_LAYOUT_ELEMENT, &p_peek(p)->span);
    
    if (!p_recurse_enter(p, "layout nested too deeply")) return n;
    n->name = p_peek(p)->lexeme;
    p_advance(p);                                   
    p_expect(p, TK_COLON, "':' after layout element name");
    p_skip_terminators(p);
    layout_body(p, n);
    p_expect(p, TK_KW_END, "'end' to close layout element");
    n->span.end = p_prev(p)->span.end;
    p_skip_terminators(p);
    p_recurse_leave(p);
    return n;
}

static ast_node_t *layout_attribute(parser_t *p)
{
    P_RULE(p, "layout_attribute");
    ast_node_t *n = ast_new(p->a, AST_LAYOUT_ATTR, &p_peek(p)->span);
    n->name = p_peek(p)->lexeme;
    p_advance(p);                                   
    p_expect(p, TK_ASSIGN, "'=' after layout attribute name");
    n->a = parse_expr(p);                           
    n->span.end = p_prev(p)->span.end;
    p_skip_terminators(p);
    return n;
}

ast_node_t *parse_layout_block(parser_t *p)
{
    P_RULE(p, "layout_block");
    ast_node_t *n = ast_new(p->a, AST_LAYOUT_BLOCK, &p_peek(p)->span);
    p_advance(p);                                   
    p_expect(p, TK_COLON, "':' after 'layout'");
    p_skip_terminators(p);
    layout_body(p, n);
    p_expect(p, TK_KW_END, "'end' to close layout block");
    n->span.end = p_prev(p)->span.end;
    return n;
}

ast_node_t *parse_component(parser_t *p)
{
    P_RULE(p, "component");
    ast_node_t *n = ast_new(p->a, AST_LAYOUT_COMPONENT, &p_peek(p)->span);
    p_advance(p);                                   
    n->name = p_name(p, "expected component name");
    if (p_match(p, TK_LPAREN)) {                     
        if (!p_at(p, TK_RPAREN)) {
            do {
                ast_node_t *param = p_mk(p, AST_PARAM);
                param->name = p_name(p, "expected component parameter name");
                if (param->name && p_match(p, TK_ASSIGN))
                    param->a = parse_expr(p);        
                p_fin(p, param);
                if (param->name) ast_add(p->a, n, param);
            } while (p_match(p, TK_COMMA));
        }
        p_expect(p, TK_RPAREN, "')' after component parameters");
    }
    p_expect(p, TK_COLON, "':' after component header");
    p_skip_terminators(p);
    ast_node_t *body = ast_new(p->a, AST_BLOCK, &p_peek(p)->span);
    layout_body(p, body);
    p_expect(p, TK_KW_END, "'end' to close component");
    body->span.end = p_prev(p)->span.end;
    n->a = body;
    n->span.end = p_prev(p)->span.end;
    return n;
}
