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

#ifndef SALAM_LAYOUT_INTERNAL_H
#define SALAM_LAYOUT_INTERNAL_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/sb.h"
#include "core/vec.h"
#include "logger/logger.h"
#include "diag/diag.h"
#include "ast/ast.h"

typedef struct {
    arena_t *a;
    logger_t *log;
    diag_engine_t *diag;
    const char *file;
    sb_t *html;
    sb_t *css;
    sb_t *js;
    sb_t *head;
    const char *title;
    const char *lang;
    const char *dir;
    vec_t css_seen;
    vec_t cls_keys;
    vec_t cls_names;
    unsigned cls_next;
    int indent;
} layout_ctx_t;

#define LWARN(cx, code, span, ...)                                                       \
    diag_report((cx)->diag, SEV_WARNING, code, (cx)->file, (span), __VA_ARGS__)

const char *lfmt(layout_ctx_t *cx, const char *fmt, ...);

void html_line(layout_ctx_t *cx, const char *fmt, ...);

unsigned djb2(const char *s);

const char *class_for_key(layout_ctx_t *cx, const char *key);

bool starts_with(const char *s, const char *p);

const char *hyphenate(layout_ctx_t *cx, const char *s);

const char *html_escape(layout_ctx_t *cx, const char *s);

void sb_put_html_escaped(sb_t *b, const char *s);

const char *val_str(layout_ctx_t *cx, ast_node_t *v);

void emit_rule(layout_ctx_t *cx, const char *rule);

bool is_bool_attr(const char *n);

void gen_element_n(layout_ctx_t *cx, ast_node_t *el, const char *parent,
                   const char *parent_class);

#endif /* SALAM_LAYOUT_INTERNAL_H */
