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

#ifndef SALAM_PARSER_INTERNAL_H
#define SALAM_PARSER_INTERNAL_H

#include "core/prelude.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "token/token.h"
#include "ast/ast.h"

typedef struct {
    const token_stream_t *toks;
    size_t count;
    size_t pos;
    const char *file;
    arena_t *a;
    logger_t *log;
    bool had_error;
    bool panic;
    bool no_struct_lit;
    int depth;
} parser_t;

const token_t *p_peek(const parser_t *p);

const token_t *p_peek2(const parser_t *p);

const token_t *p_prev(const parser_t *p);

bool p_at(const parser_t *p, token_kind_t k);

bool p_at_eof(const parser_t *p);

const token_t *p_advance(parser_t *p);

bool p_match(parser_t *p, token_kind_t k);

bool p_expect(parser_t *p, token_kind_t k, const char *what);

void p_skip_terminators(parser_t *p);

void p_error(parser_t *p, const char *msg);

void p_sync(parser_t *p);

ast_node_t *p_mk(parser_t *p, ast_kind_t k);

void p_fin(parser_t *p, ast_node_t *n);

void p_term(parser_t *p);

const char *p_name(parser_t *p, const char *what);

const char *p_member_name(parser_t *p, const char *what);

const token_t *p_peekn(const parser_t *p, size_t k);

size_t p_ident_run_len(const parser_t *p);

const char *p_munch_name(parser_t *p);

const char *parse_decl_name(parser_t *p);

ast_node_t *p_error_node(parser_t *p);

bool p_recurse_enter(parser_t *p, const char *what);

void p_recurse_leave(parser_t *p);

typedef ast_node_t *(*p_elem_fn)(parser_t *p);

void p_comma_list(parser_t *p, ast_node_t *parent, token_kind_t close, p_elem_fn elem);

#define P_RULE(p, nm)                                                                    \
    LOG_T_AT((p)->log, PH_PARSER, (p)->file, p_peek(p)->span, "rule %s @ %s", (nm),      \
             token_kind_name(p_peek(p)->kind))

ast_node_t *parse_expr(parser_t *p);

ast_node_t *parse_cond_expr(parser_t *p);

ast_node_t *parse_primary(parser_t *p);

ast_node_t *parse_type(parser_t *p);

ast_node_t *parse_type_anno(parser_t *p);

ast_node_t *parse_top_level(parser_t *p);

ast_node_t *parse_const(parser_t *p);

ast_node_t *parse_block(parser_t *p);

ast_node_t *parse_var_decl(parser_t *p);

ast_node_t *parse_bare_var_decl(parser_t *p);

ast_node_t *parse_layout_block(parser_t *p);

ast_node_t *parse_component(parser_t *p);

void parse_metas(parser_t *p, vec_t *out);

void parse_extern_into(parser_t *p, ast_node_t *prog);

bool try_link_attr(parser_t *p, ast_node_t *prog);

bool try_link_directive(parser_t *p, ast_node_t *prog);

#endif /* SALAM_PARSER_INTERNAL_H */
