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

#ifndef SALAM_SEMANTIC_SEMA_INTERNAL_H
#define SALAM_SEMANTIC_SEMA_INTERNAL_H

#include "core/prelude.h"
#include "logger/logger.h"
#include "diag/diag.h"
#include "ast/ast.h"
#include "semantic/types.h"
#include "semantic/symbol.h"

typedef struct lambda_ctx_t {
    scope_t                *boundary;
    ast_node_t             *node;
    struct lambda_ctx_t    *prev;
} lambda_ctx_t;

typedef struct {
    arena_t       *a;
    logger_t      *log;
    diag_engine_t *diag;
    const char    *file;
    const char    *lang;   /* language of the module being checked ("fa"/"en") */
    type_ctx_t    *tc;
    scope_t       *global;
    scope_t       *cur;
    type_t        *self_type;
    func_sig_t    *cur_func;
    int            loop_depth;
    const char    *dir;
    vec_t          imported;
    const char    *pkg;
    vec_t          pkg_cache;
    vec_t          loading;
    ast_node_t    *program;
    vec_t          pending;
    type_t        *expected;
    lambda_ctx_t  *lam;
    scope_t       *gen_pkg;
    scope_t       *prelude;
    bool           prelude_tried;
} sema_t;

void sema_load_prelude(sema_t *s);

#define SERR(s, code, span, ...)  diag_report((s)->diag, SEV_ERROR,   (code), (s)->file, (span), __VA_ARGS__)

#define SWARN(s, code, span, ...) diag_report((s)->diag, SEV_WARNING, (code), (s)->file, (span), __VA_ARGS__)

type_t     *sema_ty(sema_t *s, type_kind_t k);

type_t     *sema_err_ty(sema_t *s);

type_t     *sema_decorate(sema_t *s, ast_node_t *n, type_t *t);

symbol_t   *struct_sym_of(type_t *t);

type_t     *sema_vec_str(sema_t *s, const src_span_t *span);

const char *alias_for_lang(const vec_t *aliases, const char *lang);

const char *scope_member_canon(scope_t *members, const char *name);

const char *pkg_member_canon(symbol_t *pk, const char *name);

const char *local_canon(sema_t *s, const char *name);

const char *intrinsic_type_canon(const char *name);

const char *intrinsic_method_canon(const char *name);

symbol_t *generic_template(sema_t *s, const char *name, sym_kind_t kind);

symbol_t *g_instantiate_struct(sema_t *s, ast_node_t *tmpl, vec_t *targ_nodes, const src_span_t *span);

symbol_t *g_infer_call(sema_t *s, symbol_t *tsym, vec_t *argtypes,
                       const src_span_t *span, type_t *expected);

ast_node_t *coerce_to_dyn(sema_t *s, type_t *expected, ast_node_t *expr, type_t *etype);

void        coerce_args_to_dyn(sema_t *s, ast_node_t *call, vec_t *argtypes, func_sig_t *sig);

type_t *sema_resolve_type(sema_t *s, ast_node_t *tnode);

type_t *sema_check_expr(sema_t *s, ast_node_t *node);

void    sema_check_layout(sema_t *s, ast_node_t *layout, const char *parent);

const char  *sema_op_method(token_kind_t k);

token_kind_t sema_compound_base(token_kind_t k);

type_t      *sema_try_op_overload(sema_t *s, ast_node_t *n, symbol_t *ssym,
                                  const char *mname, type_t *rhs_type);

                                  bool         sema_type_is_stringable(type_t *t);

type_t     *check_call(sema_t *s, ast_node_t *n);

func_sig_t *resolve_overload(sema_t *s, symbol_t *fsym, vec_t *argtypes,
                             const src_span_t *span, const char *what);

bool        stamp_empty_intrinsic(sema_t *s, ast_node_t *val, type_t *target);

type_t *check_struct_lit(sema_t *s, ast_node_t *n);

type_t *check_array_lit(sema_t *s, ast_node_t *n);

type_t *check_lambda(sema_t *s, ast_node_t *n);

bool    defined_within(scope_t *start, scope_t *boundary, const char *name);

void    record_capture(sema_t *s, ast_node_t *id, type_t *t);

void    sema_check_block(sema_t *s, ast_node_t *block);

type_t *sema_check_var_decl(sema_t *s, ast_node_t *n);

func_sig_t *build_sig(sema_t *s, ast_node_t *fn, symbol_t *owner);

symbol_t   *get_or_make_func(sema_t *s, scope_t *sc, const char *name, sym_kind_t kind);

void        sema_collect(sema_t *s, ast_node_t *program);

void        sema_check_pass(sema_t *s, ast_node_t *program);

#endif /* SALAM_SEMANTIC_SEMA_INTERNAL_H */
