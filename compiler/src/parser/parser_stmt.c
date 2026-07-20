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

static void fill_block_body(parser_t *p, ast_node_t *n);
static ast_node_t *parse_print_stmt(parser_t *p);
static ast_node_t *parse_statement(parser_t *p);
static ast_node_t *parse_if(parser_t *p);
static ast_node_t *parse_if_tail(parser_t *p);
static ast_node_t *parse_until(parser_t *p);
static ast_node_t *parse_repeat(parser_t *p);
static ast_node_t *parse_each(parser_t *p);
static ast_node_t *parse_return(parser_t *p);
static ast_node_t *parse_defer(parser_t *p);

ast_node_t *parse_cond_expr(parser_t *p)
{
    bool saved = p->no_struct_lit;
    p->no_struct_lit = true;
    ast_node_t *e = parse_expr(p);
    p->no_struct_lit = saved;
    return e;
}

static void fill_block_body(parser_t *p, ast_node_t *n)
{
    p_skip_terminators(p);
    while (!p_at(p, TK_KW_END) && !p_at(p, TK_KW_ELSE) && !p_at_eof(p)) {
        p_skip_terminators(p);
        if (p_at(p, TK_KW_END) || p_at(p, TK_KW_ELSE) || p_at_eof(p)) break;
        ast_node_t *s = parse_statement(p);
        if (s) ast_add(p->a, n, s);
        if (p->panic) p_sync(p);
    }
}

ast_node_t *parse_block(parser_t *p)
{
    P_RULE(p, "block");
    if (!p_recurse_enter(p, "block nested too deeply")) return p_mk(p, AST_BLOCK);
    ast_node_t *n = p_mk(p, AST_BLOCK);
    p_expect(p, TK_COLON, "':' to open block");
    fill_block_body(p, n);
    p_expect(p, TK_KW_END, "'end' to close block");
    p_fin(p, n);
    p_recurse_leave(p);
    return n;
}

static ast_node_t *parse_statement(parser_t *p)
{
    P_RULE(p, "statement");
    switch (p_peek(p)->kind) {
    case TK_KW_PRINT:
    case TK_KW_PRINTLN:
    case TK_KW_PRINTERR:
    case TK_KW_PRINTERRLN:
        return parse_print_stmt(p);
    case TK_KW_MUT: {
        ast_node_t *n = parse_var_decl(p);
        p_term(p);
        return n;
    }
    case TK_KW_CONST:
        return parse_const(p);
    case TK_KW_IF:
        return parse_if(p);
    case TK_KW_UNTIL:
        return parse_until(p);
    case TK_KW_EACH:
        return parse_each(p);
    case TK_KW_REPEAT:
        return parse_repeat(p);
    case TK_KW_RET:
        return parse_return(p);
    case TK_KW_DEFER:
        return parse_defer(p);
    case TK_KW_BREAK: {
        ast_node_t *n = p_mk(p, AST_BREAK);
        p_advance(p);
        p_fin(p, n);
        p_term(p);
        return n;
    }
    case TK_KW_CONTINUE: {
        ast_node_t *n = p_mk(p, AST_CONTINUE);
        p_advance(p);
        p_fin(p, n);
        p_term(p);
        return n;
    }
    case TK_IDENT: {
        size_t m = p_ident_run_len(p);
        token_kind_t after = p_peekn(p, m)->kind;
        bool is_decl = (after == TK_COLON || after == TK_COLON_ASSIGN);
        if (is_decl) {
            ast_node_t *n = parse_bare_var_decl(p);
            p_term(p);
            return n;
        }
        SALAM_FALLTHROUGH;
    }
    default: {
        ast_node_t *e = parse_expr(p);
        ast_node_t *stmt;
        if (e && e->kind == AST_ASSIGN) {
            stmt = e;
        } else {
            stmt = ast_new(p->a, AST_EXPR_STMT, e ? &e->span : &p_peek(p)->span);
            stmt->a = e;
            if (e) stmt->span = e->span;
        }
        p_term(p);
        return stmt;
    }
    }
}

/* True when a '(' right after the print keyword closes only once the whole
 * statement ends, e.g. `print (a, b)` or `print (2)` -- the call-style wrap
 * this statement must reject in favor of the bare `print a, b` form. A '('
 * that is part of a larger expression, e.g. `print (a + b) / 2`, is fine. */
static bool print_args_fully_parenthesized(parser_t *p)
{
    if (!p_at(p, TK_LPAREN)) return false;
    size_t depth = 0, idx = 0;
    for (;;) {
        token_kind_t k = p_peekn(p, idx)->kind;
        if (k == TK_EOF) return false;
        if (k == TK_LPAREN) {
            depth++;
        } else if (k == TK_RPAREN) {
            depth--;
            if (depth == 0) break;
        }
        idx++;
    }
    token_kind_t after = p_peekn(p, idx + 1)->kind;
    return after == TK_STMT_END || after == TK_KW_END || after == TK_RBRACE ||
           after == TK_EOF;
}

static ast_node_t *parse_print_stmt(parser_t *p)
{
    token_kind_t k = p_peek(p)->kind;
    ast_node_t *call = p_mk(p, AST_CALL);
    ast_node_t *callee = p_mk(p, AST_IDENTIFIER);
    callee->name = (k == TK_KW_PRINT)      ? "print"
                   : (k == TK_KW_PRINTLN)  ? "println"
                   : (k == TK_KW_PRINTERR) ? "printerr"
                                           : "printerrln";
    call->a = callee;
    p_advance(p);
    p_fin(p, callee);
    if (print_args_fully_parenthesized(p))
        p_error(p, "print arguments must not be wrapped in parentheses; remove them");
    if (!p_at(p, TK_STMT_END) && !p_at(p, TK_KW_END) && !p_at(p, TK_RBRACE) &&
        !p_at_eof(p)) {
        do {
            ast_add(p->a, call, parse_expr(p));
        } while (p_match(p, TK_COMMA));
    }
    p_fin(p, call);
    ast_node_t *stmt = ast_new(p->a, AST_EXPR_STMT, &call->span);
    stmt->a = call;
    p_term(p);
    return stmt;
}

static ast_node_t *parse_return(parser_t *p)
{
    P_RULE(p, "return_stmt");
    ast_node_t *n = p_mk(p, AST_RETURN);
    p_advance(p);
    if (!p_at(p, TK_STMT_END) && !p_at(p, TK_KW_END) && !p_at(p, TK_KW_ELSE) &&
        !p_at_eof(p)) {
        n->a = parse_expr(p);
    }
    p_fin(p, n);
    p_term(p);
    return n;
}

static ast_node_t *parse_defer(parser_t *p)
{
    P_RULE(p, "defer_stmt");
    if (!p_recurse_enter(p, "statement nested too deeply")) return p_mk(p, AST_DEFER);
    ast_node_t *n = p_mk(p, AST_DEFER);
    p_advance(p);
    n->a = parse_statement(p);
    p_fin(p, n);
    p_recurse_leave(p);
    return n;
}

static ast_node_t *parse_if_tail(parser_t *p)
{
    if (!p_recurse_enter(p, "if/else chain nested too deeply")) return p_mk(p, AST_IF);
    ast_node_t *n = p_mk(p, AST_IF);
    n->a = parse_cond_expr(p);
    n->b = p_mk(p, AST_BLOCK);
    p_expect(p, TK_COLON, "':' after if condition");
    fill_block_body(p, n->b);
    p_fin(p, n->b);
    p_skip_terminators(p);
    if (p_match(p, TK_KW_ELSE)) {
        p_match(p, TK_KW_IF);
        if (!p_at(p, TK_COLON)) {
            n->c = parse_if_tail(p);
        } else {
            n->c = p_mk(p, AST_BLOCK);
            p_expect(p, TK_COLON, "':' after else");
            fill_block_body(p, n->c);
            p_expect(p, TK_KW_END, "'end' to close if");
            p_fin(p, n->c);
        }
    } else {
        p_expect(p, TK_KW_END, "'end' to close if");
    }
    p_fin(p, n);
    p_recurse_leave(p);
    return n;
}

static ast_node_t *parse_if(parser_t *p)
{
    P_RULE(p, "if_stmt");
    p_advance(p);
    return parse_if_tail(p);
}

static ast_node_t *parse_until(parser_t *p)
{
    P_RULE(p, "until_stmt");
    ast_node_t *n = p_mk(p, AST_UNTIL);
    p_advance(p);
    n->a = parse_cond_expr(p);
    n->b = parse_block(p);
    p_fin(p, n);
    return n;
}

static ast_node_t *parse_repeat(parser_t *p)
{
    P_RULE(p, "repeat_stmt");
    ast_node_t *n = p_mk(p, AST_REPEAT);
    p_advance(p);
    n->a = parse_cond_expr(p);
    if (p_at(p, TK_KW_TO)) {
        p_advance(p);
        n->c = parse_cond_expr(p);
    }
    if (p_at(p, TK_KW_STEP)) {
        p_advance(p);
        if (!n->c) p_error(p, "'by' in a repeat loop needs a 'to' bound before it");
        n->d = parse_cond_expr(p);
    }
    if (p_match(p, TK_KW_WITH))
        n->name = p_name(p, "expected an index variable name after 'with'");
    n->b = parse_block(p);
    p_fin(p, n);
    return n;
}

static ast_node_t *parse_each(parser_t *p)
{
    P_RULE(p, "each_stmt");
    ast_node_t *n = p_mk(p, AST_EACH);
    p_advance(p);
    if (p_match(p, TK_LPAREN)) {
        ast_node_t *k = p_mk(p, AST_IDENTIFIER);
        k->name = p_name(p, "expected a binding name in 'each (key, value)'");
        p_fin(p, k);
        n->c = k;
        p_expect(p, TK_COMMA, "',' between the two 'each' binding names");
        n->name = p_name(p, "expected the value binding name in 'each (key, value)'");
        p_expect(p, TK_RPAREN, "')' after the 'each' binding names");
    } else {
        n->name = p_name(p, "expected a binding name after 'each'");
    }
    p_expect(p, TK_KW_IN, "'in' before the collection in an 'each' loop");
    n->a = parse_cond_expr(p);
    n->b = parse_block(p);
    p_fin(p, n);
    return n;
}
