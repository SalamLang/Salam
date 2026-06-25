#include "core/prelude.h"
#include "parser/parser_internal.h"

static ast_node_t *parse_extern_func(parser_t *p);
static ast_node_t *parse_extern_var(parser_t *p);

static const char *canon_link_kind(const char *s)
{
    if (!s) return NULL;
    
    if (!strcmp(s, "static"))    return "static";
    if (!strcmp(s, "dynamic"))   return "dynamic";
    if (!strcmp(s, "framework")) return "framework";
    
    if (!strcmp(s, "ایستا"))     return "static";
    if (!strcmp(s, "پویا"))      return "dynamic";
    if (!strcmp(s, "چارچوب"))    return "framework";
    return NULL;
}

static const char *parse_link_kind(parser_t *p)
{
    if (p_at(p, TK_IDENT)) {
        const char *k = canon_link_kind(p_peek(p)->lexeme);
        if (k) { p_advance(p); return k; }
    }
    return NULL;
}

bool try_link_directive(parser_t *p, ast_node_t *prog)
{
    if (!(p_at(p, TK_IDENT) && strcmp(p_peek(p)->lexeme, "link") == 0 &&
          (p_peek2(p)->kind == TK_STRING ||
           (p_peek2(p)->kind == TK_IDENT && canon_link_kind(p_peek2(p)->lexeme)))))
        return false;
    ast_node_t *ln = p_mk(p, AST_LINK);
    p_advance(p);                         
    ln->name = parse_link_kind(p);        
    if (p_at(p, TK_STRING)) { ln->value = p_peek(p)->value; p_advance(p); }
    else p_error(p, "expected a library-name string after 'link'");
    p_term(p);
    p_fin(p, ln);
    ast_add(p->a, prog, ln);
    return true;
}

bool try_link_attr(parser_t *p, ast_node_t *prog)
{
    if (!(p_at(p, TK_META) && strcmp(p_peek(p)->lexeme, "link") == 0)) return false;
    ast_node_t *ln = p_mk(p, AST_LINK);
    p_advance(p);                                   
    p_expect(p, TK_LPAREN, "'(' after @link");
    if (p_at(p, TK_STRING)) { ln->value = p_peek(p)->value; p_advance(p); }
    else p_error(p, "expected a library-name string in @link(\"...\")");
    
    if (p_match(p, TK_COMMA)) {
        if (p_at(p, TK_IDENT) && strcmp(p_peek(p)->lexeme, "kind") == 0) {
            p_advance(p);
            p_expect(p, TK_ASSIGN, "'=' after 'kind'");
            if (p_at(p, TK_STRING)) {
                const char *k = canon_link_kind(p_peek(p)->value.as.s);
                if (k) ln->name = k;
                else p_error(p, "link kind must be \"static\", \"dynamic\", or \"framework\"");
                p_advance(p);
            } else p_error(p, "expected a kind string after 'kind='");
        } else p_error(p, "expected 'kind=\"...\"' after ',' in @link(...)");
    }
    p_expect(p, TK_RPAREN, "')' to close @link");
    p_skip_terminators(p);
    p_fin(p, ln);
    ast_add(p->a, prog, ln);
    return true;
}

static ast_node_t *parse_extern_func(parser_t *p)
{
    ast_node_t *n = p_mk(p, AST_FUNC_DEF);
    n->is_extern = true;
    p_advance(p);                                   
    n->name = p_name(p, "expected external function name");
    p_expect(p, TK_LPAREN, "'(' after external function name");
    if (!p_at(p, TK_RPAREN)) {
        do {
            if (p_at(p, TK_ELLIPSIS)) { n->is_variadic = true; p_advance(p); break; }
            ast_node_t *param = p_mk(p, AST_PARAM);
            param->name = p_name(p, "expected parameter name");
            if (!param->name) break;
            if (p_match(p, TK_AMP)) param->is_ref = true;  
            param->type = parse_type_anno(p);
            p_fin(p, param);
            ast_add(p->a, n, param);
        } while (p_match(p, TK_COMMA));
    }
    p_expect(p, TK_RPAREN, "')' after parameters");
    if (p_at(p, TK_IDENT) || p_at(p, TK_KW_FUNC)) n->type = parse_type(p);  
    
    if (p_at(p, TK_COLON))
        n->a = parse_block(p);
    else
        p_term(p);
    p_fin(p, n);
    return n;
}

static ast_node_t *parse_extern_var(parser_t *p)
{
    ast_node_t *n = p_mk(p, AST_VAR_DECL);
    n->is_extern = true;
    if      (p_at(p, TK_KW_MUT)) { n->is_mut = true; p_advance(p); }
    else if (p_at(p, TK_IDENT) && strcmp(p_peek(p)->lexeme, "var") == 0)
                                 { n->is_mut = true; p_advance(p); }
    n->name = p_name(p, "expected external variable name");
    n->type = parse_type_anno(p);
    p_term(p);
    p_fin(p, n);
    return n;
}

void parse_extern_into(parser_t *p, ast_node_t *prog)
{
    p_advance(p);                                   
    if (p_at(p, TK_STRING)) {
        const char *abi = p_peek(p)->value.as.s;
        if (abi && strcmp(abi, "C") != 0)
            p_error(p, "unsupported extern ABI (only \"C\" is supported)");
        p_advance(p);
    } else {
        p_error(p, "expected an ABI string after 'extern' (e.g. extern \"C\")");
    }
    p_expect(p, TK_COLON, "':' to open extern block");
    p_skip_terminators(p);
    while (!p_at(p, TK_KW_END) && !p_at_eof(p)) {
        p_skip_terminators(p);
        if (p_at(p, TK_KW_END) || p_at_eof(p)) break;
        ast_node_t *d = NULL;
        if (p_at(p, TK_KW_FUNC)) d = parse_extern_func(p);
        else if (p_at(p, TK_KW_MUT) || p_at(p, TK_IDENT))
            d = parse_extern_var(p);
        else { p_error(p, "expected 'func' or a variable in extern block"); if (!p_at_eof(p)) p_advance(p); }
        if (d) ast_add(p->a, prog, d);
        if (p->panic) p_sync(p);
    }
    p_expect(p, TK_KW_END, "'end' to close extern block");
    p_term(p);
}
