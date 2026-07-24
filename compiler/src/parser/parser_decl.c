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
#include "core/sb.h"
#include "core/sal_format.h"
#include "parser/parser_internal.h"

static ast_node_t *parse_type_alias(parser_t *p);
static ast_node_t *parse_enum(parser_t *p);
static ast_node_t *parse_struct(parser_t *p);
static ast_node_t *parse_interface(parser_t *p);
static ast_node_t *parse_impl(parser_t *p);
static ast_node_t *parse_field(parser_t *p);
static ast_node_t *parse_function(parser_t *p, bool allow_untyped);
static void parse_params(parser_t *p, ast_node_t *fn);
static void parse_typarams(parser_t *p, ast_node_t *n);
static void finish_untyped_params(parser_t *p, ast_node_t *fn, bool allow_untyped);
static ast_node_t *parse_interface_method(parser_t *p);
static const char *parse_op_method_name(parser_t *p);

ast_node_t *parse_top_level(parser_t *p)
{
    P_RULE(p, "top_level");

    bool is_pub = false;
    if (p_at(p, TK_KW_PUB)) {
        is_pub = true;
        p_advance(p);
    }
    bool is_deprecated = false;
    if (p_at(p, TK_KW_DEPRECATED)) {
        is_deprecated = true;
        p_advance(p);
    }
    bool is_inline = false, is_noinline = false;
    if (p_at(p, TK_KW_INLINE)) {
        is_inline = true;
        p_advance(p);
    } else if (p_at(p, TK_KW_NOINLINE)) {
        is_noinline = true;
        p_advance(p);
    }
    bool is_pure = false, is_noret = false;
    if (p_at(p, TK_KW_PURE)) {
        is_pure = true;
        p_advance(p);
    } else if (p_at(p, TK_KW_NORET)) {
        is_noret = true;
        p_advance(p);
    }
    bool any_mod = is_deprecated || is_inline || is_noinline || is_pure || is_noret;
    if (p_at(p, TK_KW_PUB) || p_at(p, TK_KW_DEPRECATED) || p_at(p, TK_KW_INLINE) ||
        p_at(p, TK_KW_NOINLINE) || p_at(p, TK_KW_PURE) || p_at(p, TK_KW_NORET)) {
        p_error(p, "function modifiers must appear in this order: "
                   "'pub deprecated inline|noinline pure|noret func'");
        while (p_at(p, TK_KW_PUB) || p_at(p, TK_KW_DEPRECATED) || p_at(p, TK_KW_INLINE) ||
               p_at(p, TK_KW_NOINLINE) || p_at(p, TK_KW_PURE) || p_at(p, TK_KW_NORET)) {
            switch (p_peek(p)->kind) {
            case TK_KW_PUB:
                is_pub = true;
                break;
            case TK_KW_DEPRECATED:
                is_deprecated = true;
                break;
            case TK_KW_INLINE:
                is_inline = true;
                break;
            case TK_KW_NOINLINE:
                is_noinline = true;
                break;
            case TK_KW_PURE:
                is_pure = true;
                break;
            default:
                is_noret = true;
                break;
            }
            p_advance(p);
        }
        any_mod = true;
    }
    if (any_mod && !p_at(p, TK_KW_FUNC))
        p_error(p, "function modifiers ('deprecated', 'inline', 'noinline', 'pure', "
                   "'noret') are only allowed on a function definition");
    ast_node_t *n = NULL;
    switch (p_peek(p)->kind) {
    case TK_KW_TYPE:
        n = parse_type_alias(p);
        break;
    case TK_KW_CONST:
        n = parse_const(p);
        break;
    case TK_KW_ENUM:
        n = parse_enum(p);
        break;
    case TK_KW_STRUCT:
        n = parse_struct(p);
        break;
    case TK_KW_INTERFACE:
        n = parse_interface(p);
        break;
    case TK_KW_IMPL:
        n = parse_impl(p);
        break;
    case TK_KW_FUNC:
        n = parse_function(p, true);
        break;
    case TK_KW_LAYOUT:
        if (is_pub) p_error(p, "'pub' cannot be applied to a layout block");
        return parse_layout_block(p);
    case TK_KW_COMPONENT:
        if (is_pub) p_error(p, "'pub' cannot be applied to a component");
        return parse_component(p);
    case TK_KW_MUT:
        n = parse_var_decl(p);
        p_term(p);
        break;
    case TK_IDENT:
        n = parse_bare_var_decl(p);
        p_term(p);
        break;
    case TK_KW_PACKAGE:
        p_error(p, "'package' can be declared only once, and only as the very "
                   "first statement in the file");
        if (!p_at_eof(p)) p_advance(p);
        return NULL;
    default:
        p_error(p, is_pub ? "expected a definition after 'pub'"
                          : "expected a top-level definition");
        if (!p_at_eof(p)) p_advance(p);
        return NULL;
    }
    if (n) {
        n->is_pub = is_pub;
        if (n->kind == AST_FUNC_DEF) {
            n->is_inline = is_inline;
            n->is_noinline = is_noinline;
            n->is_pure = is_pure;
            n->is_noret = is_noret;
            n->is_deprecated = is_deprecated;
        }
    }
    return n;
}

static ast_node_t *parse_type_alias(parser_t *p)
{
    P_RULE(p, "type_alias");
    ast_node_t *n = p_mk(p, AST_TYPE_ALIAS);
    p_advance(p);
    n->name = p_name(p, "expected alias name");
    p_expect(p, TK_ASSIGN, "'=' in type alias");
    n->type = parse_type(p);
    p_term(p);
    p_fin(p, n);
    return n;
}

ast_node_t *parse_const(parser_t *p)
{
    P_RULE(p, "const_decl");
    ast_node_t *n = p_mk(p, AST_CONST_DECL);
    p_advance(p);
    n->name = p_name(p, "expected constant name");
    if (p_at(p, TK_COLON)) {
        p_error(p, "constants with a type annotation were removed; use "
                   "'const NAME := value' (or 'const NAME := value as Type')");
        n->type = parse_type_anno(p);
        p_match(p, TK_ASSIGN);
        n->a = parse_expr(p);
        p_term(p);
        p_fin(p, n);
        return n;
    }
    if (p_at(p, TK_ASSIGN)) {
        p_error(p, "constants are declared with ':=' ('const NAME := value')");
        p_advance(p);
    } else {
        p_expect(p, TK_COLON_ASSIGN, "':=' in const declaration");
    }
    n->a = parse_expr(p);
    p_term(p);
    p_fin(p, n);
    return n;
}

static ast_node_t *parse_enum(parser_t *p)
{
    P_RULE(p, "enum_def");
    ast_node_t *n = p_mk(p, AST_ENUM_DEF);
    p_advance(p);
    n->name = p_name(p, "expected enum name");
    p_expect(p, TK_COLON, "':' after enum name");
    p_skip_terminators(p);
    while (!p_at(p, TK_KW_END) && !p_at_eof(p)) {
        size_t before = p->pos;
        p_skip_terminators(p);
        if (p_at(p, TK_KW_END) || p_at_eof(p)) break;
        ast_node_t *m = p_mk(p, AST_ENUM_MEMBER);
        m->name = p_name(p, "expected enum member name");
        if (p_match(p, TK_ASSIGN)) m->a = parse_expr(p);
        p_fin(p, m);
        ast_add(p->a, n, m);

        if (!p_match(p, TK_COMMA)) p_skip_terminators(p);
        if (p->panic) p_sync(p);

        if (p->pos == before && !p_at(p, TK_KW_END) && !p_at_eof(p)) p_advance(p);
    }
    p_expect(p, TK_KW_END, "'end' to close enum");
    p_fin(p, n);
    return n;
}

static void parse_typarams(parser_t *p, ast_node_t *n)
{
    if (!p_match(p, TK_LT)) return;
    do {
        const char *tp = p_name(p, "expected type parameter name");
        if (!tp) break;
        vec_push(p->a, &n->typarams, CONST_CAST(tp));
        const char *bound = NULL;
        if (p_match(p, TK_COLON))
            bound =
                p_name(p, "expected an interface name after ':' in type parameter bound");
        vec_push(p->a, &n->typaram_bounds, CONST_CAST(bound));
    } while (p_match(p, TK_COMMA));
    p_expect(p, TK_GT, "'>' to close type parameters");
}

static ast_node_t *parse_interface_method(parser_t *p)
{
    ast_node_t *n = p_mk(p, AST_FUNC_DEF);
    p_advance(p);
    n->name = p_name(p, "expected method name in interface");
    if (p_match(p, TK_LPAREN)) {
        parse_params(p, n);
        p_expect(p, TK_RPAREN, "')' after parameters");
    }
    finish_untyped_params(p, n, false);
    if (p_at(p, TK_COLON)) {
        p_advance(p);
        n->type = parse_type(p);
    }
    p_term(p);
    p_fin(p, n);
    return n;
}

static ast_node_t *parse_interface(parser_t *p)
{
    P_RULE(p, "interface_def");
    ast_node_t *n = p_mk(p, AST_INTERFACE_DEF);
    p_advance(p);
    n->name = p_name(p, "expected interface name");
    parse_typarams(p, n);
    p_expect(p, TK_COLON, "':' after interface name");
    p_skip_terminators(p);
    while (!p_at(p, TK_KW_END) && !p_at_eof(p)) {
        p_skip_terminators(p);
        if (p_at(p, TK_KW_END) || p_at_eof(p)) break;
        if (p_at(p, TK_KW_FUNC)) {
            ast_node_t *m = parse_interface_method(p);
            if (m) ast_add(p->a, n, m);
        } else {
            p_error(p, "expected a method signature ('func ...') in interface");
            if (!p_at_eof(p)) p_advance(p);
        }
        if (p->panic) p_sync(p);
    }
    p_expect(p, TK_KW_END, "'end' to close interface");
    p_fin(p, n);
    return n;
}

static ast_node_t *parse_impl(parser_t *p)
{
    P_RULE(p, "impl_def");
    ast_node_t *n = p_mk(p, AST_IMPL_DEF);
    p_advance(p);
    n->name = p_name(p, "expected interface name after 'impl'");
    p_expect(p, TK_KW_ON, "'on' after interface name in impl block");
    n->type = parse_type(p);
    p_expect(p, TK_COLON, "':' after impl target type");
    p_skip_terminators(p);
    while (!p_at(p, TK_KW_END) && !p_at_eof(p)) {
        p_skip_terminators(p);
        if (p_at(p, TK_KW_END) || p_at_eof(p)) break;
        if (p_at(p, TK_KW_FUNC)) {
            ast_node_t *m = parse_function(p, false);
            if (m) ast_add(p->a, n, m);
        } else {
            p_error(p, "expected a method ('func ...') in impl block");
            if (!p_at_eof(p)) p_advance(p);
        }
        if (p->panic) p_sync(p);
    }
    p_expect(p, TK_KW_END, "'end' to close impl block");
    p_fin(p, n);
    return n;
}

static ast_node_t *parse_struct(parser_t *p)
{
    P_RULE(p, "struct_def");
    ast_node_t *n = p_mk(p, AST_STRUCT_DEF);
    p_advance(p);
    n->name = p_name(p, "expected struct name");
    parse_typarams(p, n);
    p_expect(p, TK_COLON, "':' after struct name");
    p_skip_terminators(p);
    vec_t mpend;
    vec_init(&mpend);
    while (!p_at(p, TK_KW_END) && !p_at_eof(p)) {
        size_t before = p->pos;
        p_skip_terminators(p);
        parse_metas(p, &mpend);
        if (p_at(p, TK_KW_END) || p_at_eof(p)) break;
        bool m_pub = false;
        if (p_at(p, TK_KW_PUB)) {
            m_pub = true;
            p_advance(p);
        }
        ast_node_t *member =
            p_at(p, TK_KW_FUNC) ? parse_function(p, false) : parse_field(p);
        if (member) {
            member->is_pub = m_pub;
            {
                size_t i = 0;
                for (; i < mpend.len; i++)
                    vec_push(p->a, &member->aliases, mpend.data[i]);
            }
            ast_add(p->a, n, member);
        }
        mpend.len = 0;
        if (p->panic) p_sync(p);

        if (p->pos == before && !p_at(p, TK_KW_END) && !p_at_eof(p)) p_advance(p);
    }
    p_expect(p, TK_KW_END, "'end' to close struct");
    p_fin(p, n);
    return n;
}

static ast_node_t *parse_field(parser_t *p)
{
    P_RULE(p, "field");
    ast_node_t *n = p_mk(p, AST_FIELD);

    n->name =
        p_at(p, TK_IDENT) ? parse_decl_name(p) : p_member_name(p, "expected field name");
    if (!n->name) return NULL;
    n->type = parse_type_anno(p);
    if (p_match(p, TK_ASSIGN)) n->a = parse_expr(p);
    p_term(p);
    p_fin(p, n);
    return n;
}

static const char *parse_op_method_name(parser_t *p)
{
    token_kind_t k = p_peek(p)->kind;
    switch (k) {
    case TK_PLUS:
        p_advance(p);
        return "operator_add";
    case TK_MINUS:
        p_advance(p);
        return "operator_sub";
    case TK_STAR:
        p_advance(p);
        return "operator_mul";
    case TK_SLASH:
        p_advance(p);
        return "operator_div";
    case TK_PERCENT:
        p_advance(p);
        return "operator_mod";
    case TK_POWER:
        p_advance(p);
        return "operator_pow";
    case TK_EQ:
        p_advance(p);
        return "operator_eq";
    case TK_NE:
        p_advance(p);
        return "operator_ne";
    case TK_LT:
        p_advance(p);
        return "operator_lt";
    case TK_GT:
        p_advance(p);
        return "operator_gt";
    case TK_LE:
        p_advance(p);
        return "operator_le";
    case TK_GE:
        p_advance(p);
        return "operator_ge";
    case TK_NOT:
        p_advance(p);
        return "operator_not";
    case TK_LBRACKET:
        p_advance(p);
        p_expect(p, TK_RBRACKET, "']' in operator[]");
        if (p_at(p, TK_ASSIGN)) {
            p_advance(p);
            return "operator_index_set";
        }
        return "operator_index";
    default:
        p_error(p, "unsupported operator for overloading; "
                   "allowed: + - * / % ** == != < > <= >= ! []");
        return "operator_error";
    }
}

static ast_node_t *parse_function(parser_t *p, bool allow_untyped)
{
    P_RULE(p, "function_def");
    ast_node_t *n = p_mk(p, AST_FUNC_DEF);
    p_advance(p);
    if (p_at(p, TK_KW_OPERATOR)) {
        p_advance(p);
        n->name = parse_op_method_name(p);
    } else if (p_at(p, TK_IDENT)) {
        size_t m = p_ident_run_len(p);
        token_kind_t after = p_peekn(p, m)->kind;
        if (m >= 2 && after != TK_LPAREN && after != TK_LT) {
            sb_t b;
            sb_init(&b);
            {
                size_t i = 0;
                for (; i < m - 1; i++) {
                    if (i) sb_putc(&b, ' ');
                    sb_puts(&b, p_peek(p)->lexeme);
                    p_advance(p);
                }
            }
            n->name = arena_strdup(p->a, sb_cstr(&b));
            sb_free(&b);
        } else {
            n->name = p_munch_name(p);
        }
    } else {
        n->name = p_name(p, "expected function name or 'operator <op>'");
    }
    parse_typarams(p, n);
    if (p_match(p, TK_LPAREN)) {
        parse_params(p, n);
        p_expect(p, TK_RPAREN, "')' after parameters");
    }
    finish_untyped_params(p, n, allow_untyped);
    p_try_return_type(p, &n->type);
    n->a = parse_block(p);
    p_fin(p, n);
    return n;
}

static void parse_params(parser_t *p, ast_node_t *fn)
{
    if (p_at(p, TK_RPAREN)) return;
    do {
        ast_node_t *param = p_mk(p, AST_PARAM);
        param->name = parse_decl_name(p);
        if (!param->name) return;
        if (p_match(p, TK_AMP)) param->is_ref = true;
        if (p_at(p, TK_COLON)) param->type = parse_type_anno(p);
        if (p_match(p, TK_ASSIGN)) param->a = parse_expr(p);
        p_fin(p, param);
        ast_add(p->a, fn, param);
    } while (p_match(p, TK_COMMA));
}

static void finish_untyped_params(parser_t *p, ast_node_t *fn, bool allow_untyped)
{
    size_t next = 0;
    size_t i = 0;
    for (; i < fn->list.len; i++) {
        ast_node_t *param = (ast_node_t *)fn->list.data[i];
        if (param->kind != AST_PARAM || param->type) continue;
        if (!allow_untyped) {
            p_error(p, "method and interface parameters must have explicit types");
            return;
        }
        {
            char buf[24];
            sal_snprintf(buf, sizeof buf, "__T%zu", next++);
            const char *tn = arena_strdup(p->a, buf);
            ast_node_t *t = ast_new(p->a, AST_TYPE, &param->span);
            t->name = tn;
            param->type = t;
            vec_push(p->a, &fn->typarams, CONST_CAST(tn));
            vec_push(p->a, &fn->typaram_bounds, NULL);
        }
    }
}
