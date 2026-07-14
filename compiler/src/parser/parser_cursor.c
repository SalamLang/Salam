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
#include "parser/parser_internal.h"
#include "i18n/i18n.h"

static bool is_comment_kind(token_kind_t k)
{
    return k == TK_COMMENT_LINE || k == TK_COMMENT_BLOCK;
}

static size_t skip_comments_from(const parser_t *p, size_t from)
{
    while (from < p->count - 1 && is_comment_kind(token_stream_at(p->toks, from)->kind))
        from++;
    return from;
}

const token_t *p_peek(const parser_t *p)
{
    size_t i = skip_comments_from(p, p->pos);
    if (i >= p->count) i = p->count - 1;
    return token_stream_at(p->toks, i);
}

const token_t *p_peek2(const parser_t *p)
{
    size_t i = skip_comments_from(p, p->pos);
    if (i < p->count - 1) i = skip_comments_from(p, i + 1);
    if (i >= p->count) i = p->count - 1;
    return token_stream_at(p->toks, i);
}

const token_t *p_prev(const parser_t *p)
{
    size_t i = p->pos > 0 ? p->pos - 1 : 0;
    return token_stream_at(p->toks, i);
}

bool p_at(const parser_t *p, token_kind_t k)
{
    return p_peek(p)->kind == k;
}

bool p_at_eof(const parser_t *p)
{
    return p_peek(p)->kind == TK_EOF;
}

const token_t *p_advance(parser_t *p)
{
    p->pos = skip_comments_from(p, p->pos);
    const token_t *t = token_stream_at(p->toks, p->pos);
    if (p->pos < p->count - 1) {
        p->pos++;
        p->pos = skip_comments_from(p, p->pos);
    }
    return t;
}

bool p_match(parser_t *p, token_kind_t k)
{
    if (p_at(p, k)) {
        p_advance(p);
        return true;
    }
    return false;
}

void p_skip_terminators(parser_t *p)
{
    while (p_at(p, TK_STMT_END))
        p_advance(p);
}

void p_error(parser_t *p, const char *msg)
{
    if (p->panic) return;
    const token_t *t = p_peek(p);

    LOG_E_AT(p->log, PH_PARSER, p->file, t->span, i18n_tr("%s (near '%s')"), i18n_tr(msg),
             t->lexeme ? t->lexeme : "<eof>");
    p->had_error = true;
    p->panic = true;
}

bool p_expect(parser_t *p, token_kind_t k, const char *what)
{
    if (p_at(p, k)) {
        p_advance(p);
        return true;
    }
    char buf[128];
    sal_snprintf(buf, sizeof(buf), i18n_tr("expected %s"), i18n_tr(what));
    p_error(p, buf);
    return false;
}

void p_sync(parser_t *p)
{
    p->panic = false;
    while (!p_at_eof(p)) {
        token_kind_t k = p_peek(p)->kind;
        if (k == TK_STMT_END) {
            p_advance(p);
            return;
        }
        switch (k) {
        case TK_KW_FUNC:
        case TK_KW_STRUCT:
        case TK_KW_ENUM:
        case TK_KW_TYPE:
        case TK_KW_CONST:
        case TK_KW_IMPORT:
        case TK_KW_LAYOUT:
        case TK_KW_IF:
        case TK_KW_WHILE:
        case TK_KW_FOR:
        case TK_KW_RET:
        case TK_RBRACE:
        case TK_KW_END:
            return;
        default:
            break;
        }
        p_advance(p);
    }
}

ast_node_t *p_mk(parser_t *p, ast_kind_t k)
{
    src_span_t s = p_peek(p)->span;
    return ast_new(p->a, k, &s);
}

void p_fin(parser_t *p, ast_node_t *n)
{
    n->span.end = p_prev(p)->span.end;
}

void p_term(parser_t *p)
{
    if (p_at(p, TK_STMT_END)) p_advance(p);
}

static bool p_at_reserved_word(parser_t *p)
{
    const token_t *t = p_peek(p);
    if (tk_is_keyword(t->kind)) return true;
    if (t->kind == TK_AND || t->kind == TK_OR)
        return t->lexeme && (unsigned char)t->lexeme[0] >= 0x80;
    return false;
}

const char *p_name(parser_t *p, const char *what)
{
    if (p_at(p, TK_IDENT)) {
        const char *s = p_peek(p)->lexeme;
        p_advance(p);
        return s;
    }
    if (p_at_reserved_word(p)) {
        char buf[160];
        sal_snprintf(buf, sizeof(buf),
                     i18n_tr("%s (reserved word cannot be used as a name)"),
                     i18n_tr(what));
        p_error(p, buf);
        p_advance(p);
        return "<error>";
    }
    p_error(p, what);
    return "<error>";
}

const char *p_member_name(parser_t *p, const char *what)
{
    if (p_at(p, TK_IDENT) || tk_is_keyword(p_peek(p)->kind)) {
        const char *s = p_peek(p)->lexeme;
        p_advance(p);
        return s;
    }
    p_error(p, what);
    return NULL;
}

ast_node_t *p_error_node(parser_t *p)
{
    ast_node_t *n = p_mk(p, AST_IDENTIFIER);
    n->name = "<error>";
    p_fin(p, n);
    return n;
}

const token_t *p_peekn(const parser_t *p, size_t k)
{
    size_t i = skip_comments_from(p, p->pos);
    {
        size_t c = 0;
        for (; c < k; c++) {
            if (i < p->count - 1) {
                i++;
                i = skip_comments_from(p, i);
            }
        }
    }
    if (i >= p->count) i = p->count - 1;
    return token_stream_at(p->toks, i);
}

size_t p_ident_run_len(const parser_t *p)
{
    size_t m = 0;
    while (p_peekn(p, m)->kind == TK_IDENT)
        m++;
    return m;
}

const char *p_munch_name(parser_t *p)
{
    if (!p_at(p, TK_IDENT)) return p_name(p, "expected name");
    sb_t b;
    sb_init(&b);
    bool first = true;
    while (p_at(p, TK_IDENT)) {
        if (!first) sb_putc(&b, ' ');
        sb_puts(&b, p_peek(p)->lexeme);
        first = false;
        p_advance(p);
    }
    const char *r = arena_strdup(p->a, sb_cstr(&b));
    sb_free(&b);
    return r;
}

const char *parse_decl_name(parser_t *p)
{
    /* A type, when present, is always introduced by a mandatory ':', so the
     * whole leading run of identifiers belongs to the (possibly multi-word)
     * name; no heuristic split is needed. */
    return p_munch_name(p);
}

#define P_MAX_DEPTH 128
bool p_recurse_enter(parser_t *p, const char *what)
{
    if (p->depth >= P_MAX_DEPTH) {
        p_error(p, what);
        return false;
    }
    p->depth++;
    return true;
}

void p_recurse_leave(parser_t *p)
{
    p->depth--;
}

void p_comma_list(parser_t *p, ast_node_t *parent, token_kind_t close, p_elem_fn elem)
{
    if (p_at(p, close)) return;
    do {
        ast_add(p->a, parent, elem(p));
    } while (p_match(p, TK_COMMA));
}

bool p_try_return_type(parser_t *p, ast_node_t **out_type)
{
    if (!p_at(p, TK_COLON)) return false;
    size_t save_pos = p->pos;
    uint32_t colon_line = p_peek(p)->span.begin.line;
    p_advance(p);
    if (p_peek(p)->span.begin.line != colon_line) {
        p->pos = save_pos;
        return false;
    }
    bool save_panic = p->panic;
    bool save_had_error = p->had_error;
    p->panic = true;
    ast_node_t *t = parse_type(p);
    bool valid_name = t->name && strcmp(t->name, "<error>") != 0;
    if (valid_name && (p_at(p, TK_COLON) || p_at(p, TK_STMT_END) || p_at_eof(p))) {
        p->panic = save_panic;
        *out_type = t;
        return true;
    }
    p->pos = save_pos;
    p->panic = save_panic;
    p->had_error = save_had_error;
    return false;
}
