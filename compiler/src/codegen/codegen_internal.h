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

#ifndef SALAM_CODEGEN_INTERNAL_H
#define SALAM_CODEGEN_INTERNAL_H

#include "core/prelude.h"
#include "codegen/codegen.h"
#include "core/sb.h"
#include "semantic/symbol.h"
#include "semantic/types.h"

typedef struct {
    arena_t *a;
    logger_t *log;
    sema_result_t *sem;
    const char *module;
    sb_t *h;
    sb_t *c;
    int indent;
    vec_t locals;
    symbol_t *cur_struct;
    scope_t *cur_fn_home;
    bool safe;
    bool debug_info;
    bool single_threaded;
    const char *src_path;
    int tmpn;
    bool cur_sret;
    vec_t vec_types;
    vec_t dyn_ifaces;
    vec_t dyn_impls;
    const char *pkg;
    const char *entry;
    vec_t deferred;
    vec_t fn_defers;
    sb_t *lam_decls;
    sb_t *lam_defs;
    int lam_n;
    int clos_n;
    ast_node_t *cur_lambda;
    bool is_gui_mode;
    const char *target_triple;
} cg_t;

const char *cg_fmt(cg_t *cg, const char *fmt, ...);

void cg_indent(cg_t *cg);

void cg_source_line(cg_t *cg, const src_span_t *span);

void cg_line(cg_t *cg, const char *fmt, ...);

bool local_known(cg_t *cg, const char *name);

void local_add(cg_t *cg, const char *name);

void cg_kv(const char *ts, char *kbuf, char *vbuf, size_t cap);

const char *cg_cident(cg_t *cg, const char *name);

void cg_put_ident_byte(sb_t *b, unsigned char c);

void parse_typestr(const char *ts, char *base, size_t cap, bool *ptr, vec_t *dims,
                   arena_t *a);

bool cg_is_int_typestr(const char *ts);

void cg_vec_elem(const char *ts, char *ebuf, size_t cap);

const char *cg_vec_cname(cg_t *cg, const char *ts);

bool cg_is_slice_ts(const char *ts);

void cg_slice_elem(const char *ts, char *ebuf, size_t cap);

const char *cg_ctype(cg_t *cg, const char *ts);

const char *cg_decl(cg_t *cg, const char *ts, const char *name);

const char *cg_mangle_in(cg_t *cg, const char *pkg, const char *struct_name,
                         const char *fn, vec_t *params);

const char *cg_mangle(cg_t *cg, const char *struct_name, const char *fn, vec_t *params);

const char *cg_mangle_method(cg_t *cg, const char *sname, symbol_t *owner, const char *fn,
                             vec_t *params);

const char *cg_mangle_ti(cg_t *cg, const char *typestr, const char *fn, vec_t *params);

const char *func_ret_of(const char *ts);

const char *func_cast_params_env(cg_t *cg, const char *ts);

bool type_is_byval_agg(const type_t *t);

long array_size_of(const char *ts);

int print_tag(const char *ts);

const char *prim_suffix(int tag);

func_sig_t *sig_of_decl(symbol_t *fsym, ast_node_t *decl);

func_sig_t *pick_overload(cg_t *cg, symbol_t *fsym, ast_node_t *call);

symbol_t *struct_by_name(cg_t *cg, const char *name);

const char *cg_op_method(token_kind_t k);

token_kind_t cg_compound_base(token_kind_t k);

func_sig_t *pick_op_overload(cg_t *cg, symbol_t *msym, size_t want_nparams);

bool cg_addressable(const ast_node_t *n);

symbol_t *cg_struct_of(cg_t *cg, const char *ts, char *sname, size_t cap);

const char *cg_op(token_kind_t k);

const char *cg_cescape(cg_t *cg, const char *s);

const char *cg_str_operand(cg_t *cg, ast_node_t *n);

const char *cg_emit_op_call(cg_t *cg, ast_node_t *lhs_node, symbol_t *ssym,
                            const char *sname, const char *mname, ast_node_t *rhs_node);

const char *cg_call(cg_t *cg, ast_node_t *n);

const char *cg_expr(cg_t *cg, ast_node_t *n);

void cg_emit_defers(cg_t *cg);

void cg_stmt(cg_t *cg, ast_node_t *n);

void cg_block(cg_t *cg, ast_node_t *block);

void cg_emit_lambda(cg_t *cg, ast_node_t *n);

const char *cg_lambda_value(cg_t *cg, ast_node_t *n);

void cg_lift_walk(cg_t *cg, ast_node_t *node);

const char *func_signature(cg_t *cg, ast_node_t *fn, symbol_t *owner, func_sig_t *sig,
                           bool is_main);

const char *cg_extern_proto(cg_t *cg, ast_node_t *fn, func_sig_t *sig);

void cg_function(cg_t *cg, ast_node_t *fn, symbol_t *owner);

void cg_emit_dyn_vtables(cg_t *cg, sb_t *c);

void cg_header(cg_t *cg, ast_node_t *program);

#endif /* SALAM_CODEGEN_INTERNAL_H */
