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
#include "core/sb.h"
#include "parser/parser.h"
#include "parser/parser_internal.h"
#include "i18n/i18n.h"

static ast_node_t *parse_program(parser_t *p);
static ast_node_t *parse_string_import_entry(parser_t *p);
static ast_node_t *parse_ident_import_entry(parser_t *p);
static void parse_imports_into(parser_t *p, ast_node_t *prog);
static void parse_top_level_item_into(parser_t *p, ast_node_t *dest, vec_t *pending);
static ast_node_t *parse_toplevel_if(parser_t *p);
static ast_node_t *parse_toplevel_if_tail(parser_t *p);

static void attach_pending(parser_t *p, ast_node_t *target, vec_t *pending)
{
    {
        size_t i = 0;
        for (; i < pending->len; i++)
            vec_push(p->a, &target->aliases, pending->data[i]);
    }
    pending->len = 0;
}

static bool metas_have_lang(const vec_t *out, const char *lang)
{
    size_t i = 0;
    for (; i + 1 < out->len; i += 2)
        if (strcmp((const char *)out->data[i], lang) == 0) return true;
    return false;
}

void parse_metas(parser_t *p, vec_t *out)
{
    p_skip_terminators(p);
    while (p_at(p, TK_META)) {
        const char *lang = p_peek(p)->lexeme;
        if (metas_have_lang(out, lang)) {
            char buf[192];
            sal_snprintf(buf, sizeof buf,
                         i18n_tr("duplicate '@%s' marker on one definition; give one "
                                 "marker several values instead: @%s \"a\" \"b\""),
                         lang, lang);
            p_error(p, buf);
        }
        p_advance(p);
        if (p_at(p, TK_LBRACKET)) {
            p_advance(p);
            while (!p_at(p, TK_RBRACKET) && !p_at_eof(p)) {
                if (p_at(p, TK_STRING)) {
                    vec_push(p->a, out, CONST_CAST(lang));
                    vec_push(p->a, out, CONST_CAST(p_peek(p)->value.as.s));
                    p_advance(p);
                } else if (p_at(p, TK_COMMA) || p_at(p, TK_STMT_END)) {
                    p_advance(p);
                } else
                    break;
            }
            p_expect(p, TK_RBRACKET, "']' to close annotation list");
        } else if (p_at(p, TK_STRING)) {
            while (p_at(p, TK_STRING)) {
                vec_push(p->a, out, CONST_CAST(lang));
                vec_push(p->a, out, CONST_CAST(p_peek(p)->value.as.s));
                p_advance(p);
            }
        } else {
            p_error(p, "expected a string after '@' annotation");
        }
        p_skip_terminators(p);
    }
}

static bool parse_package(parser_t *p, ast_node_t *prog, vec_t *pending)
{
    if (!p_at(p, TK_KW_PACKAGE)) return false;
    p_advance(p);
    if (p_at(p, TK_IDENT) || p_at(p, TK_KW_LAYOUT)) {
        prog->name = p_peek(p)->lexeme;
        p_advance(p);
    } else
        p_error(p, "expected package name after 'package'");
    attach_pending(p, prog, pending);
    p_term(p);
    return true;
}

static void parse_top_level_item_into(parser_t *p, ast_node_t *dest, vec_t *pending)
{
    p_skip_terminators(p);
    while (try_link_attr(p, dest)) {
    }
    parse_metas(p, pending);
    if (p_at_eof(p)) return;
    if (p_at(p, TK_KW_IMPORT)) {
        pending->len = 0;
        parse_imports_into(p, dest);
        return;
    }
    if (p_at(p, TK_KW_EXTERN)) {
        pending->len = 0;
        parse_extern_into(p, dest);
        return;
    }
    if (try_link_directive(p, dest)) {
        pending->len = 0;
        return;
    }
    if (p_at(p, TK_KW_IF)) {
        pending->len = 0;
        ast_add(p->a, dest, parse_toplevel_if(p));
        return;
    }
    ast_node_t *def = parse_top_level(p);
    if (def) {
        attach_pending(p, def, pending);
        ast_add(p->a, dest, def);
    }
    pending->len = 0;
    if (p->panic) p_sync(p);
}

static void fill_toplevel_block_body(parser_t *p, ast_node_t *n)
{
    vec_t pending;
    vec_init(&pending);
    p_skip_terminators(p);
    while (!p_at(p, TK_KW_END) && !p_at(p, TK_KW_ELSE) && !p_at_eof(p)) {
        p_skip_terminators(p);
        if (p_at(p, TK_KW_END) || p_at(p, TK_KW_ELSE) || p_at_eof(p)) break;
        parse_top_level_item_into(p, n, &pending);
        if (p->panic) p_sync(p);
    }
}

static ast_node_t *parse_toplevel_if_tail(parser_t *p)
{
    if (!p_recurse_enter(p, "if/else chain nested too deeply")) return p_mk(p, AST_IF);
    ast_node_t *n = p_mk(p, AST_IF);
    n->a = parse_cond_expr(p);
    n->b = p_mk(p, AST_BLOCK);
    p_expect(p, TK_COLON, "':' after if condition");
    fill_toplevel_block_body(p, n->b);
    p_fin(p, n->b);
    p_skip_terminators(p);
    if (p_match(p, TK_KW_ELSE)) {
        p_match(p, TK_KW_IF);
        if (!p_at(p, TK_COLON)) {
            n->c = parse_toplevel_if_tail(p);
        } else {
            n->c = p_mk(p, AST_BLOCK);
            p_expect(p, TK_COLON, "':' after else");
            fill_toplevel_block_body(p, n->c);
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

static ast_node_t *parse_toplevel_if(parser_t *p)
{
    P_RULE(p, "toplevel_if_stmt");
    p_advance(p);
    return parse_toplevel_if_tail(p);
}

static ast_node_t *parse_program(parser_t *p)
{
    P_RULE(p, "program");
    ast_node_t *prog = p_mk(p, AST_PROGRAM);
    prog->name = "main";

    vec_t pending;
    vec_init(&pending);
    p_skip_terminators(p);
    while (try_link_attr(p, prog)) {
    }
    parse_metas(p, &pending);
    parse_package(p, prog, &pending);
    while (!p_at_eof(p))
        parse_top_level_item_into(p, prog, &pending);
    p_fin(p, prog);
    return prog;
}

static bool p_at_string_import(parser_t *p)
{
    return p_at(p, TK_STRING) || (p_at(p, TK_IDENT) && p_peek2(p)->kind == TK_STRING);
}

static ast_node_t *parse_string_import_entry(parser_t *p)
{
    ast_node_t *n = p_mk(p, AST_IMPORT);
    const char *alias = NULL;
    if (p_at(p, TK_IDENT) && p_peek2(p)->kind == TK_STRING) {
        alias = p_peek(p)->lexeme;
        p_advance(p);
    }
    if (p_at(p, TK_STRING)) {
        n->name = alias;
        n->value = p_peek(p)->value;
        p_advance(p);
    } else {
        p_error(p, "expected import path string");
    }
    p_fin(p, n);
    return n;
}

static ast_node_t *parse_ident_import_entry(parser_t *p)
{
    ast_node_t *n = p_mk(p, AST_IMPORT);
    sb_t b;
    sb_init(&b);
    sb_puts(&b, p_munch_name(p));
    while (p_at(p, TK_DOT)) {
        p_advance(p);
        sb_putc(&b, '.');
        sb_puts(&b, p_munch_name(p));
    }
    n->name = arena_strdup(p->a, sb_cstr(&b));
    sb_free(&b);
    n->value.kind = TV_NONE;
    p_fin(p, n);
    return n;
}

static void parse_imports_into(parser_t *p, ast_node_t *prog)
{
    p_advance(p);
    if (p_at(p, TK_LPAREN)) {
        p_error(p, "grouped imports are not supported; write import \"a\" \"b\"");
        while (!p_at(p, TK_RPAREN) && !p_at_eof(p))
            p_advance(p);
        p_match(p, TK_RPAREN);
        p_term(p);
        return;
    }
    if (p_at_string_import(p)) {
        ast_add(p->a, prog, parse_string_import_entry(p));
        while (p_at_string_import(p))
            ast_add(p->a, prog, parse_string_import_entry(p));
    } else if (p_at(p, TK_IDENT)) {
        ast_add(p->a, prog, parse_ident_import_entry(p));
    } else {
        p_error(p, "expected import path string");
    }
    p_term(p);
}

bool parser_run(arena_t *a, logger_t *log, const token_stream_t *toks,
                ast_node_t **out_program)
{
    parser_t p;
    memset(&p, 0, sizeof(p));
    p.toks = toks;
    p.count = token_stream_count(toks);
    p.pos = 0;
    p.file = token_stream_file(toks);
    p.a = a;
    p.log = log;
    LOG_I(log, PH_PARSER, "parsing %zu tokens", p.count);
    ast_node_t *prog = parse_program(&p);
    *out_program = prog;
    LOG_I(log, PH_PARSER, "parse complete: %zu top-level defs%s", prog->list.len,
          p.had_error ? i18n_tr(" (with errors)") : "");
    return !p.had_error;
}
