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

#ifndef SALAM_INTERP_INTERNAL_H
#define SALAM_INTERP_INTERNAL_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/sb.h"
#include "core/vec.h"
#include "logger/logger.h"
#include "ast/ast.h"
#include "semantic/sema.h"
#include "token/token.h"
#include "interp/interp.h"
#include "interp/value.h"
#include <setjmp.h>
#include <time.h>

typedef struct {
    const char *name;
    value_t val;
} binding_t;

typedef struct {
    const char **keys;
    void **vals;
    size_t cap;
    size_t count;
} itab_t;

typedef struct {
    const void **keys;
    void **vals;
    size_t cap;
    size_t count;
} ptab_t;

typedef struct env {
    struct env *parent;
    vec_t bindings;
    itab_t *index;
} env_t;

typedef struct module {
    const char *name;
    env_t *env;
} module_t;

typedef enum { FLOW_NORMAL = 0, FLOW_RETURN, FLOW_BREAK, FLOW_CONTINUE } flow_t;

typedef struct {
    ast_node_t *stmt;
    env_t *env;
} defer_t;

typedef struct {
    vec_t defers;
} frame_t;

typedef struct {
    arena_t *a;
    logger_t *log;
    sema_result_t *sem;
    const char *entry;
    FILE *out;
    FILE *err;
    const char *lang;
    env_t *globals;
    vec_t funcs;
    vec_t structs;
    vec_t enums;
    vec_t impls;
    vec_t modules;
    vec_t extern_fns;
    vec_t extern_decls;
    vec_t def_envs;
    itab_t tab_funcs;
    itab_t tab_structs;
    itab_t tab_enums;
    itab_t tab_modules;
    itab_t tab_extern_fns;
    itab_t tab_extern_decls;
    ptab_t tab_def_envs;
    const char *in_data;
    size_t in_pos;
    jmp_buf on_error;
    char errmsg[512];
    src_span_t errspan;
    bool have_errspan;
    unsigned long long steps;
    clock_t deadline;
    unsigned depth;
} interp_t;

typedef enum {
    ILOC_VAR,
    ILOC_FIELD,
    ILOC_ARR,
    ILOC_PTR,
    ILOC_OPIDX,
} iloc_kind_t;

typedef struct {
    iloc_kind_t kind;
    ast_node_t *target;
    binding_t *b;      /* ILOC_VAR */
    value_t obj;       /* ILOC_FIELD / ILOC_OPIDX: the struct value */
    size_t field_idx;  /* ILOC_FIELD */
    sarray_t *arr;     /* ILOC_ARR */
    int64_t idx;       /* ILOC_ARR / ILOC_PTR */
    sptr_t ptr;        /* ILOC_PTR */
    value_t key;       /* ILOC_OPIDX */
    ast_node_t *get_m; /* ILOC_OPIDX */
    ast_node_t *set_m; /* ILOC_OPIDX */
} iloc_t;

#define INTERP_MAX_DEPTH 1000

#if defined(__GNUC__) || defined(__clang__)
#  define SALAM_NORETURN __attribute__((noreturn))
#else
#  define SALAM_NORETURN
#endif

SALAM_NORETURN void rt_error(interp_t *I, ast_node_t *n, const char *fmt, ...);

void tick(interp_t *I);

env_t *env_new(interp_t *I, env_t *parent);

binding_t *env_find_local(env_t *e, const char *name);

binding_t *env_find(env_t *e, const char *name);

void env_define(interp_t *I, env_t *e, const char *name, value_t v);

ast_node_t *find_struct(interp_t *I, const char *name);

ast_node_t *find_enum(interp_t *I, const char *name);

ast_node_t *find_func(interp_t *I, const char *name, size_t nargs);

module_t *find_module(interp_t *I, const char *name);

value_t *find_extern_fn(interp_t *I, const char *name);

ast_node_t *find_extern_decl(interp_t *I, const char *name);

void register_method_envs(interp_t *I, ast_node_t *def, env_t *env);

env_t *find_def_env(interp_t *I, ast_node_t *def);

value_t call_native_extern(interp_t *I, ast_node_t *call, ast_node_t *decl, value_t *args,
                           size_t nargs);

ast_node_t *struct_method(ast_node_t *sdef, const char *name);

ast_node_t *struct_method_arity(ast_node_t *sdef, const char *name, size_t nparams);

ast_node_t *find_impl_method(interp_t *I, const char *typestr, const char *method,
                             size_t nargs);

const char *afmt(interp_t *I, const char *fmt, ...);

bool to_bool(value_t v);

double to_float(value_t v);

int64_t to_int(value_t v);

const char *to_str(interp_t *I, value_t v);

sarray_t *array_new(interp_t *I, size_t cap);

sarray_t *array_view(interp_t *I, sarray_t *base, size_t off, size_t len);

void array_push(interp_t *I, sarray_t *a, value_t v);

value_t mk_array(interp_t *I, sarray_t *a);

smap_t *map_new(interp_t *I);

void map_put(interp_t *I, smap_t *m, value_t k, value_t val);

smap_entry_t *map_find(smap_t *m, value_t k);

bool map_remove(smap_t *m, value_t k);

value_t mk_map(smap_t *m);

value_t mk_struct(interp_t *I, const char *name, ast_node_t *def, size_t nfields);

value_t mk_closure(interp_t *I, ast_node_t *fn, env_t *env);

bool is_int_typename(const char *b);

bool is_float_typename(const char *b);

void base_typename(const char *ts, char *out, size_t cap);

ptr_elem_t ptr_elem_from_typestr(const char *ts);

value_t ptr_load(sptr_t p, int64_t idx);

void ptr_store(sptr_t p, int64_t idx, value_t v);

value_t default_for_type(interp_t *I, const char *ts);

value_t arith(interp_t *I, ast_node_t *n, token_kind_t op, value_t a, value_t b);

int value_cmp(interp_t *I, value_t a, value_t b);

bool value_eq(interp_t *I, value_t a, value_t b);

token_kind_t compound_base(token_kind_t op);

value_t try_struct_op(interp_t *I, token_kind_t op, value_t a, value_t b, bool has_b,
                      bool *found);

value_t eval(interp_t *I, env_t *env, ast_node_t *n);

void interp_assign_to(interp_t *I, env_t *env, ast_node_t *target, value_t v);

iloc_t interp_resolve_loc(interp_t *I, env_t *env, ast_node_t *target);

value_t interp_loc_get(interp_t *I, iloc_t *loc);

void interp_loc_set(interp_t *I, iloc_t *loc, value_t v);

value_t call_func(interp_t *I, ast_node_t *fn, env_t *defenv, value_t *thisv,
                  value_t *args, size_t nargs);

flow_t exec_stmt(interp_t *I, env_t *env, frame_t *fr, ast_node_t *n, value_t *ret);

flow_t exec_list(interp_t *I, env_t *env, frame_t *fr, vec_t *list, value_t *ret);

void do_print(interp_t *I, env_t *env, ast_node_t *call, bool newline, bool to_err);

value_t do_input(interp_t *I);

value_t call_module_func(interp_t *I, ast_node_t *call, module_t *mod, const char *fn,
                         value_t *args, size_t nargs);

value_t call_intrinsic(interp_t *I, ast_node_t *call, const char *name, value_t *args,
                       size_t nargs, bool *handled);

value_t call_builtin_method(interp_t *I, ast_node_t *call, value_t recv,
                            const char *method, value_t *args, size_t nargs);

void build_modules(interp_t *I, ast_node_t *program);

#endif /* SALAM_INTERP_INTERNAL_H */
