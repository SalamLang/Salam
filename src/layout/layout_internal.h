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
    arena_t       *a;
    logger_t      *log;
    diag_engine_t *diag;
    const char    *file;
    sb_t          *html;
    sb_t          *css;
    sb_t          *js;
    const char    *title;
    const char    *lang;
    const char    *dir;
    vec_t          css_seen;   /* const char* rule strings (dedup) */
    int            indent;
    unsigned       cls_seed;
} layout_ctx_t;

#define LWARN(cx, code, span, ...) \
    diag_report((cx)->diag, SEV_WARNING, code, (cx)->file, (span), __VA_ARGS__)

const char *lfmt(layout_ctx_t *cx, const char *fmt, ...);   /* arena-printf      */

void        html_line(layout_ctx_t *cx, const char *fmt, ...); /* indented + '\n' */

unsigned    djb2(const char *s);                            /* hash -> auto class */

bool        starts_with(const char *s, const char *p);

const char *hyphenate(layout_ctx_t *cx, const char *s);     /* '_'/' ' -> '-'    */

const char *html_escape(layout_ctx_t *cx, const char *s);   /* & < > -> entities */

const char *val_str(layout_ctx_t *cx, ast_node_t *v);       /* attr expr -> text */

void        emit_rule(layout_ctx_t *cx, const char *rule);  /* append CSS, dedup */

bool        is_bool_attr(const char *n);                    /* required/checked/… */

void gen_element_n(layout_ctx_t *cx, ast_node_t *el, const char *parent,
                   const char *parent_class);

#endif /* SALAM_LAYOUT_INTERNAL_H */
