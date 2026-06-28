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

#define BP_LEFT(w)   ((int)((unsigned)(w) >> 16))
#define BP_RIGHT(w)  ((int)((unsigned)(w) & 0xFFFFu))
#define BP(l, r)     (((unsigned)(l) << 16) | (unsigned)(r))
static ast_node_t *parse_expr_bp(parser_t *p, int min_bp);
static ast_node_t *parse_nud(parser_t *p);
static ast_node_t *parse_led(parser_t *p, ast_node_t *lhs, token_kind_t op, unsigned bp);
static unsigned bp_of(token_kind_t k)
{
    switch (k) {
        
        case TK_ASSIGN:
        case TK_PLUS_EQ:  case TK_MINUS_EQ:
        case TK_STAR_EQ:  case TK_SLASH_EQ: case TK_PERCENT_EQ:
            return BP(10, 10);
        
        case TK_OR:               return BP(20, 21);
        
        case TK_AND:              return BP(30, 31);
        
        case TK_EQ:  case TK_NE:  return BP(40, 41);
        
        case TK_LT:  case TK_GT:
        case TK_LE:  case TK_GE:  return BP(50, 51);
        
        case TK_PLUS: case TK_MINUS: return BP(60, 61);
        
        case TK_STAR: case TK_SLASH: case TK_PERCENT: return BP(70, 71);
        
        case TK_KW_AS:            return BP(80, 81);
        
        case TK_POWER:            return BP(100, 100);
        
        case TK_LPAREN:
        case TK_LBRACKET:
        case TK_DOT:              return BP(110, 111);
        default:                   return 0;
    }
}

static ast_node_t *make_binary(parser_t *p, token_kind_t op, ast_node_t *l, ast_node_t *r)
{
    ast_node_t *n = ast_new(p->a, AST_BINARY, &l->span);
    n->op = op; n->a = l; n->b = r;
    if (r) n->span.end = r->span.end;
    return n;
}

static ast_node_t *parse_nud(parser_t *p)
{
    token_kind_t k = p_peek(p)->kind;
    
    if (k == TK_NOT || k == TK_MINUS) {
        ast_node_t *n = p_mk(p, AST_UNARY);
        n->op = p_advance(p)->kind;
        n->a = parse_expr_bp(p, 90);
        p_fin(p, n);
        return n;
    }
    return parse_primary(p);  
}

static ast_node_t *led_call(parser_t *p, ast_node_t *lhs)
{
    ast_node_t *call = ast_new(p->a, AST_CALL, &lhs->span);
    call->a = lhs;
    p_advance(p);                                  
    
    if (lhs->kind == AST_IDENTIFIER && lhs->name && !strcmp(lhs->name, "sizeof")) {
        ast_add(p->a, call, parse_type(p));
        p_expect(p, TK_RPAREN, "')' after sizeof type");
        p_fin(p, call);
        return call;
    }
    p_comma_list(p, call, TK_RPAREN, parse_expr); 
    p_expect(p, TK_RPAREN, "')' after arguments");
    p_fin(p, call);
    return call;
}

static ast_node_t *led_index(parser_t *p, ast_node_t *lhs)
{
    src_span_t span = lhs->span;
    p_advance(p);
    p_skip_terminators(p);

    ast_node_t *lo = NULL;
    if (!p_at(p, TK_COLON)) lo = parse_expr_bp(p, 0);
    p_skip_terminators(p);

    if (p_at(p, TK_COLON)) {
        p_advance(p);
        p_skip_terminators(p);
        ast_node_t *hi = NULL;
        if (!p_at(p, TK_RBRACKET)) hi = parse_expr_bp(p, 0);
        p_skip_terminators(p);
        p_expect(p, TK_RBRACKET, "']' to close slice range 'a[lo:hi]'");
        ast_node_t *sl = ast_new(p->a, AST_SLICE, &span);
        sl->a = lhs; sl->b = lo; sl->c = hi;
        p_fin(p, sl);
        return sl;
    }

    ast_node_t *idx = ast_new(p->a, AST_INDEX, &span);
    idx->a = lhs;
    idx->b = lo;
    p_expect(p, TK_RBRACKET, "']' after index");
    p_fin(p, idx);
    return idx;
}

static ast_node_t *led_member(parser_t *p, ast_node_t *lhs)
{
    ast_node_t *m = ast_new(p->a, AST_MEMBER, &lhs->span);
    m->a = lhs;
    p_advance(p);                                  
    
    m->name = p_at(p, TK_IDENT) ? p_munch_name(p)
                                : p_member_name(p, "expected member name after '.'");
    p_fin(p, m);
    return m;
}

static ast_node_t *led_cast(parser_t *p, ast_node_t *lhs)
{
    p_advance(p);                                  
    ast_node_t *cast = ast_new(p->a, AST_CAST, &lhs->span);
    cast->a = lhs;
    cast->type = parse_type(p);                    
    p_fin(p, cast);
    return cast;
}

static ast_node_t *led_assign(parser_t *p, ast_node_t *lhs, token_kind_t op, unsigned bp)
{
    p_advance(p);
    ast_node_t *n = ast_new(p->a, AST_ASSIGN, lhs ? &lhs->span : &p_peek(p)->span);
    n->op = op; n->a = lhs;
    n->b = parse_expr_bp(p, BP_RIGHT(bp));         
    p_fin(p, n);
    return n;
}

static ast_node_t *parse_led(parser_t *p, ast_node_t *lhs, token_kind_t op, unsigned bp)
{
    switch (op) {
        case TK_LPAREN:   return led_call(p, lhs);
        case TK_LBRACKET: return led_index(p, lhs);
        case TK_DOT:      return led_member(p, lhs);
        case TK_KW_AS:    return led_cast(p, lhs);
        case TK_ASSIGN:   case TK_PLUS_EQ:  case TK_MINUS_EQ:
        case TK_STAR_EQ:  case TK_SLASH_EQ: case TK_PERCENT_EQ:
            return led_assign(p, lhs, op, bp);
        default: {                                  
            p_advance(p);
            ast_node_t *rhs = parse_expr_bp(p, BP_RIGHT(bp));
            return make_binary(p, op, lhs, rhs);
        }
    }
}

static ast_node_t *parse_expr_bp(parser_t *p, int min_bp)
{
    if (!p_recurse_enter(p, "expression nested too deeply")) return p_error_node(p);
    ast_node_t *lhs = parse_nud(p);
    for (;;) {
        
        if (p->panic) break;
        token_kind_t op = p_peek(p)->kind;
        unsigned     bp = bp_of(op);
        if (BP_LEFT(bp) == 0 || BP_LEFT(bp) < min_bp) break;
        lhs = parse_led(p, lhs, op, bp);
    }
    p_recurse_leave(p);
    return lhs;
}

ast_node_t *parse_expr(parser_t *p) { return parse_expr_bp(p, 0); }
