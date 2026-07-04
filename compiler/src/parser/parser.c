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
#include "parser/parser.h"
#include "parser/parser_internal.h"
#include "i18n/i18n.h"

static ast_node_t *parse_program(parser_t *p);
static ast_node_t *parse_import_entry(parser_t *p);
static void parse_imports_into(parser_t *p, ast_node_t *prog);

static void attach_pending(parser_t *p, ast_node_t *target, vec_t *pending)
{
    {
        size_t i = 0;
        for (; i < pending->len; i++)
            vec_push(p->a, &target->aliases, pending->data[i]);
    }
    pending->len = 0;
}

void parse_metas(parser_t *p, vec_t *out)
{
    p_skip_terminators(p);
    while (p_at(p, TK_META)) {
        const char *lang = p_peek(p)->lexeme;
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
            vec_push(p->a, out, CONST_CAST(lang));
            vec_push(p->a, out, CONST_CAST(p_peek(p)->value.as.s));
            p_advance(p);
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
    while (!p_at_eof(p)) {
        p_skip_terminators(p);
        while (try_link_attr(p, prog)) {
        }
        parse_metas(p, &pending);
        if (p_at_eof(p)) break;
        if (p_at(p, TK_KW_IMPORT)) {
            pending.len = 0;
            parse_imports_into(p, prog);
            continue;
        }
        if (p_at(p, TK_KW_EXTERN)) {
            pending.len = 0;
            parse_extern_into(p, prog);
            continue;
        }
        if (try_link_directive(p, prog)) {
            pending.len = 0;
            continue;
        }
        ast_node_t *def = parse_top_level(p);
        if (def) {
            attach_pending(p, def, &pending);
            ast_add(p->a, prog, def);
        }
        pending.len = 0;
        if (p->panic) p_sync(p);
    }
    p_fin(p, prog);
    return prog;
}

static ast_node_t *parse_import_entry(parser_t *p)
{
    ast_node_t *n = p_mk(p, AST_IMPORT);
    const char *alias = NULL;
    if (p_at(p, TK_IDENT) && p_peek2(p)->kind == TK_STRING) {
        alias = p_peek(p)->lexeme;
        p_advance(p);
    }
    const token_t *t = p_peek(p);
    if (t->kind == TK_STRING) {
        n->name = alias;
        n->value = t->value;
        p_advance(p);
    } else if (t->kind == TK_IDENT) {
        n->name = t->lexeme;
        p_advance(p);
    } else {
        p_error(p, "expected import path string");
    }
    p_fin(p, n);
    return n;
}

static void parse_imports_into(parser_t *p, ast_node_t *prog)
{
    p_advance(p);
    if (p_match(p, TK_LPAREN)) {
        p_skip_terminators(p);
        while (!p_at(p, TK_RPAREN) && !p_at_eof(p)) {
            size_t before = p->pos;
            ast_add(p->a, prog, parse_import_entry(p));
            p_skip_terminators(p);
            if (p->pos == before) p_advance(p);
        }
        p_expect(p, TK_RPAREN, "')' to close import group");
    } else {
        ast_add(p->a, prog, parse_import_entry(p));
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
