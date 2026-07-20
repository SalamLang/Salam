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

static ast_node_t *parse_lambda(parser_t *p);
static ast_node_t *parse_array_lit(parser_t *p);
static ast_node_t *parse_struct_lit(parser_t *p);
static bool lambda_ahead(const parser_t *p)
{
    if (!p_at(p, TK_LPAREN)) return false;
    size_t i = p->pos;
    int depth = 0;
    for (; i < p->count; i++) {
        token_kind_t k = token_stream_at(p->toks, i)->kind;
        if (k == TK_LPAREN)
            depth++;
        else if (k == TK_RPAREN) {
            if (--depth == 0) {
                i++;
                break;
            }
        } else if (k == TK_EOF)
            return false;
    }
    return i < p->count && token_stream_at(p->toks, i)->kind == TK_FAT_ARROW;
}

static const char *builtin_callable_name(token_kind_t k)
{
    switch (k) {
    case TK_KW_PRINT:
        return "print";
    case TK_KW_PRINTLN:
        return "println";
    case TK_KW_PRINTERR:
        return "printerr";
    case TK_KW_PRINTERRLN:
        return "printerrln";
    case TK_KW_INPUT:
        return "input";
    default:
        return NULL;
    }
}

ast_node_t *parse_primary(parser_t *p)
{
    const token_t *t = p_peek(p);
    switch (t->kind) {
    case TK_INT:
    case TK_FLOAT:
    case TK_STRING:
    case TK_TRIPLE_STRING:
    case TK_RAW_STRING:
    case TK_CHAR:
    case TK_UTF8_CHAR:
    case TK_KW_TRUE:
    case TK_KW_FALSE:
    case TK_KW_NULL: {
        ast_node_t *n = p_mk(p, AST_LITERAL);
        n->op = t->kind;
        n->value = t->value;
        n->name = t->lexeme;
        p_advance(p);
        p_fin(p, n);
        return n;
    }
    case TK_KW_THIS: {
        ast_node_t *n = p_mk(p, AST_THIS);
        p_advance(p);
        p_fin(p, n);
        return n;
    }
    case TK_KW_PRINT:
    case TK_KW_PRINTLN:
    case TK_KW_PRINTERR:
    case TK_KW_PRINTERRLN:
    case TK_KW_INPUT: {
        ast_node_t *n = p_mk(p, AST_IDENTIFIER);
        n->name = builtin_callable_name(t->kind);
        p_advance(p);
        p_fin(p, n);
        return n;
    }
    case TK_LPAREN:
        if (lambda_ahead(p)) return parse_lambda(p);
        {
            p_advance(p);
            ast_node_t *e = parse_expr(p);
            p_expect(p, TK_RPAREN, "')'");
            return e;
        }
    case TK_LBRACKET:
        return parse_array_lit(p);
    case TK_KW_MATCH:
        return parse_match(p);
    case TK_AMP: {
        ast_node_t *n = p_mk(p, AST_FUNC_ADDR);
        p_advance(p);
        n->name = p_name(p, "expected a function name after '&'");
        p_fin(p, n);
        return n;
    }
    case TK_IDENT: {
        if (p_peek2(p)->kind == TK_LBRACE && !p->no_struct_lit)
            return parse_struct_lit(p);

        ast_node_t *n = p_mk(p, AST_IDENTIFIER);
        n->name = p_munch_value_name(p);
        p_fin(p, n);
        return n;
    }
    default: {
        p_error(p, "expected an expression");
        ast_node_t *n = p_mk(p, AST_IDENTIFIER);
        n->name = "<error>";
        if (!p_at_eof(p)) p_advance(p);
        p_fin(p, n);
        return n;
    }
    }
}

static ast_node_t *parse_lambda(parser_t *p)
{
    ast_node_t *n = p_mk(p, AST_LAMBDA);
    p_expect(p, TK_LPAREN, "'(' to open lambda parameters");
    if (!p_at(p, TK_RPAREN)) {
        do {
            ast_node_t *param = p_mk(p, AST_PARAM);
            param->name = p_name(p, "expected parameter name");
            if (!param->name) break;
            param->type = parse_type_anno(p);
            p_fin(p, param);
            ast_add(p->a, n, param);
        } while (p_match(p, TK_COMMA));
    }
    p_expect(p, TK_RPAREN, "')' after lambda parameters");
    p_expect(p, TK_FAT_ARROW, "'=>' in lambda");
    if (p_at(p, TK_COLON)) {
        n->a = parse_block(p);
    } else {
        ast_node_t *e = parse_expr(p);
        ast_node_t *blk = ast_new(p->a, AST_BLOCK, e ? &e->span : &p_peek(p)->span);
        ast_node_t *ret = ast_new(p->a, AST_RETURN, e ? &e->span : &p_peek(p)->span);
        ret->a = e;
        ast_add(p->a, blk, ret);
        n->a = blk;
    }
    p_fin(p, n);
    return n;
}

static ast_node_t *parse_array_lit(parser_t *p)
{
    ast_node_t *n = p_mk(p, AST_ARRAY_LIT);
    p_advance(p);

    p_skip_terminators(p);
    while (!p_at(p, TK_RBRACKET) && !p_at_eof(p)) {
        ast_add(p->a, n, parse_expr(p));
        p_match(p, TK_COMMA);
        p_skip_terminators(p);
    }
    p_expect(p, TK_RBRACKET, "']' to close array literal");
    p_fin(p, n);
    return n;
}

static ast_node_t *parse_struct_lit(parser_t *p)
{
    ast_node_t *n = p_mk(p, AST_STRUCT_LIT);
    n->name = p_peek(p)->lexeme;
    p_advance(p);
    p_expect(p, TK_LBRACE, "'{' in struct literal");

    p_skip_terminators(p);
    while (!p_at(p, TK_RBRACE) && !p_at_eof(p)) {
        ast_node_t *fi = p_mk(p, AST_FIELD_INIT);
        fi->name = p_at(p, TK_IDENT)
                       ? p_munch_name(p)
                       : p_member_name(p, "expected field name in struct literal");
        if (!fi->name) break;
        p_expect(p, TK_ASSIGN, "'=' in field initializer");
        fi->a = parse_expr(p);
        p_fin(p, fi);
        ast_add(p->a, n, fi);
        p_match(p, TK_COMMA);
        p_skip_terminators(p);
    }
    p_expect(p, TK_RBRACE, "'}' to close struct literal");
    p_fin(p, n);
    return n;
}
