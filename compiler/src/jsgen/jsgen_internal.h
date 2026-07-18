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

#ifndef SALAM_JSGEN_JSGEN_INTERNAL_H
#define SALAM_JSGEN_JSGEN_INTERNAL_H

#include "core/prelude.h"
#include "jsgen/jsgen.h"
#include "codegen/codegen_internal.h"

typedef struct {
    cg_t cg;
    const char *entry_mangled;
    const char *last_fn_name;
    vec_t local_muts;
    vec_t local_emit;
    vec_t taken;
    vec_t fn_used_names;
    vec_t minify_keys;
    vec_t minify_vals;
    bool enable_minify;
    const char *minify_last;
} jg_t;

#define JSP_PRIMARY 20
#define JSP_CALL 18
#define JSP_MEMBER 17
#define JSP_UNARY 15
#define JSP_TERNARY 3
#define JSP_LAMBDA 2

typedef struct {
    const char *name;
    const char *tmpl;
} js_host_t;

const js_host_t *js_host_lookup(const char *name);

const char *jsg_ident(jg_t *g, const char *name);

const char *jsg_minify_next(jg_t *g);

const char *jsg_minify_cached(jg_t *g, const char *key);

const char *jsg_escape(jg_t *g, const char *s);

bool jsg_taken(jg_t *g, const char *name);

const char *jsg_local(jg_t *g, const char *name, bool is_mut);

const char *jsg_local_ref(jg_t *g, const char *name);

void jsg_scope_reset(jg_t *g, size_t mark);

bool jsg_local_is_mut(jg_t *g, const char *name);

const char *jsg_fresh(jg_t *g, const char *base);

const char *jsg_fn_name(jg_t *g, const char *pkg, const char *sname, const char *fname,
                        symbol_t *fsym, func_sig_t *sig, bool is_extern, bool synthetic);

const char *jsg_global_ref(jg_t *g, const char *pkg, const char *name);

const char *jsg_zero(jg_t *g, const char *ts);

const char *jsg_expr(jg_t *g, ast_node_t *n);

const char *jsg_expr_p(jg_t *g, ast_node_t *n, int minprec);

const char *jsg_call(jg_t *g, ast_node_t *n);

const char *jsg_emit_op_call(jg_t *g, ast_node_t *lhs, symbol_t *ssym, const char *sname,
                             const char *mname, ast_node_t *rhs);

void jsg_stmt(jg_t *g, ast_node_t *n);

void jsg_block(jg_t *g, ast_node_t *block);

void jsg_emit_defers(jg_t *g);

void jsg_function(jg_t *g, ast_node_t *fn, symbol_t *owner);

#endif /* SALAM_JSGEN_JSGEN_INTERNAL_H */
