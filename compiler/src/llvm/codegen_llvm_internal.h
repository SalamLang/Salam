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

#ifndef SALAM_LLVM_CODEGEN_LLVM_INTERNAL_H
#define SALAM_LLVM_CODEGEN_LLVM_INTERNAL_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/sb.h"
#include "logger/logger.h"
#include "ast/ast.h"
#include "semantic/sema.h"
#include "semantic/symbol.h"
#include "semantic/types.h"
#include "llvm/codegen_llvm.h"

typedef struct { const char *ref; const char *ts; } llv_t;


typedef struct { const char *ptr; const char *ts; } ll_addr_t;

typedef struct { const char *name; const char *ptr; const char *ts; } lvar_t;

typedef struct { const char *bytes; size_t len; const char *gref; } lstr_t;

typedef struct {
    arena_t       *a;
    logger_t      *log;
    sema_result_t *sem;
    const char    *module;
    const char    *entry;     /* source entry-point name -> emitted as C `main` */
    sb_t          *g;         /* module-level: string constants, function defs   */
    sb_t          *b;         /* current function body                           */
    int            tmp;       /* SSA temporary counter (%tN)                     */
    int            lbl;       /* label counter (%LN)                             */
    bool           term;      /* current basic block already has a terminator   */
    vec_t          locals;    /* lvar_t* in scope                               */
    vec_t          strings;   /* lstr_t* interned string constants             */
    const char    *ret_ts;    /* current function's return type string          */
    bool           is_main;   /* current function is the entry point            */
    const char    *brk[64];   /* loop break-target labels (stack)              */
    const char    *cont[64];  /* loop continue-target labels (stack)           */
    int            nloop;
    vec_t          defers;    /* ast_node_t* deferred statements (LIFO)         */
    const char    *self_ts;   /* receiver type when emitting a method body, else NULL */
    const char    *this_ref;  /* pointer to the receiver (param %this, or its spill) */
    bool           self_byval;/* receiver is by value (impl-on-type method), so
                               * `this` loads from this_ref instead of being the ptr */
    vec_t          globals;   /* lvar_t* module-level globals (ptr = "@g.<id>")     */
    vec_t          gdefer;    /* ast_node_t* globals whose init runs at main entry  */
    vec_t          extern_names; /* const char* extern symbols already `declare`d   */
    scope_t       *pkg_scope;  /* members of the package whose body is being emitted,
                                * else NULL: a fallback for resolving sibling symbols */
    vec_t          emitted;    /* const char* mangled names already defined (dedup +
                                * lazy on-demand emission of imported package fns)   */
    ast_node_t    *cur_lambda;/* AST_LAMBDA being emitted (captures resolve via env) */
    const char    *env_ref;   /* the lambda's environment pointer SSA ("%env")      */
    const char    *env_ty;    /* the lambda's environment struct type ("%lam.N.env")*/
    int            lam_n;      /* lifted-lambda counter (unique names)               */
    bool           ok;        /* cleared by ll_error on any unsupported construct */
    const char    *first_error; /* first message ll_error recorded, or NULL    */
    bool           debug;     /* emit DWARF metadata                            */
    sb_t          *meta;      /* accumulates "!N = ..." metadata node defs      */
    int            meta_n;    /* next metadata node id                          */
    const char    *di_file;   /* "!N" of the DIFile                             */
    const char    *di_cu;     /* "!N" of the DICompileUnit                      */
    const char    *di_subty;  /* "!N" of a generic DISubroutineType (reused)    */
    const char    *cur_sp;    /* "!N" of the function's DISubprogram            */
    const char    *cur_dbg;   /* "!N" DILocation to attach to emitted instrs    */
    const char    *di_flag_dwarf;  /* "!N" Dwarf Version module-flag node       */
    const char    *di_flag_debug;  /* "!N" Debug Info Version module-flag node  */
    const char    *src_file;  /* DIFile filename (source basename)             */
    const char    *src_dir;   /* DIFile directory                              */
    const char    *triple;    /* target triple (--target=...), or NULL = host  */
    const char    *usize;     /* target size_t/uintptr IR type: "i64" or "i32" */
    int            ptr_bits;  /* target pointer width in bits: 64 or 32         */
} ll_t;

SAL_INLINE bool ll_is_str(const char *ts)    { return ts && !strcmp(ts, "str"); }

SAL_INLINE bool ll_is_bool(const char *ts)   { return ts && !strcmp(ts, "bool"); }

SAL_INLINE bool ll_is_float(const char *ts)  { return ts && (!strcmp(ts,"f32") || !strcmp(ts,"f64")); }

SAL_INLINE bool ll_is_ptr_ts(const char *ts) { return ts && *ts && ts[strlen(ts)-1] == '*'; }

SAL_INLINE llv_t ll_poison(const char *ts) { return (llv_t){ "0", ts ? ts : "i32" }; }

const char *ll_fmt(ll_t *ll, const char *fmt, ...);        /* arena printf            */

void        ll_emit(ll_t *ll, const char *fmt, ...);       /* one indented instr line */

void        ll_emit_label(ll_t *ll, const char *label);    /* place a BB label        */

void        ll_emit_term(ll_t *ll, const char *fmt, ...);  /* a terminator (br/ret)   */

const char *ll_new_tmp(ll_t *ll);                          /* fresh %tN               */

const char *ll_new_lbl(ll_t *ll, const char *tag);         /* fresh label LN_tag      */

void        ll_error(ll_t *ll, const ast_node_t *n, const char *fmt, ...); /* report + ok=false */

const char *ll_strconst(ll_t *ll, const char *s);          /* intern -> ptr global    */

void        ll_local_add(ll_t *ll, const char *name, const char *ptr, const char *ts);

lvar_t     *ll_local_find(ll_t *ll, const char *name);

lvar_t     *ll_global_find(ll_t *ll, const char *name);

int         ll_int_bits(const char *ts);

int         ll_target_ptr_bits(const char *triple);

const char *ll_usize_to_i32(ll_t *ll, const char *ref);

bool        ll_is_int(const char *ts);

bool        ll_is_signed(const char *ts);

const char *ll_ty(ll_t *ll, const char *ts);

const char *ll_conv(ll_t *ll, llv_t v, const char *to_ts);

const char *ll_common(const char *a, const char *b);

const char *ll_as_i1(ll_t *ll, llv_t v);

long        ll_array_dim(const char *ts);


const char *ll_array_elem(ll_t *ll, const char *ts);

const char *ll_struct_ltype(ll_t *ll, const char *name);

symbol_t   *ll_sym(ll_t *ll, const char *name);

symbol_t   *ll_struct_sym(ll_t *ll, const char *name);

symbol_t   *ll_enum_sym(ll_t *ll, const char *name);

int         ll_field_index(symbol_t *ssym, const char *field, symbol_t **out_field);

const char *ll_zero(const char *ts);

const char *ll_func_ret(ll_t *ll, const char *ts);

void        ll_func_params(ll_t *ll, const char *ts, vec_t *out);

llv_t ll_expr(ll_t *ll, ast_node_t *n);

llv_t ll_binary(ll_t *ll, ast_node_t *n);

ll_addr_t ll_addr_of(ll_t *ll, ast_node_t *n);

bool ll_index_set(ll_t *ll, ast_node_t *index_target, ast_node_t *value);

void ll_stmt(ll_t *ll, ast_node_t *n);

void ll_block(ll_t *ll, ast_node_t *block);

void ll_emit_return(ll_t *ll, ast_node_t *value);

const char *ll_mangle(ll_t *ll, const char *owner, const char *fn, func_sig_t *sig);

func_sig_t *ll_pick_overload(ll_t *ll, symbol_t *sym, ast_node_t *call);


void        ll_function(ll_t *ll, ast_node_t *fn, symbol_t *owner); /* owner=struct for methods */

void        ll_emit_lambda(ll_t *ll, ast_node_t *n);  /* lift a lambda -> a function + env type */

void        ll_emit_struct_types(ll_t *ll, ast_node_t *program); /* %struct.X = type {..} */

void        ll_emit_globals(ll_t *ll, ast_node_t *program);      /* @g.<id> = global ...   */

void        ll_emit_externs(ll_t *ll);                           /* declare @extern_fn(..)  */

void        ll_emit_impls(ll_t *ll);                             /* `impl Iface for T` methods */

void        ll_emit_packages(ll_t *ll);                          /* imported modules' defs   */

void        ll_ensure_fn(ll_t *ll, ast_node_t *fn, symbol_t *owner, scope_t *pscope);

const char *ll_box_dyn(ll_t *ll, llv_t v, const char *iface);

const char *ll_mangle_ti(ll_t *ll, const char *typestr, const char *fn, func_sig_t *sig);

void        ll_emit_global_inits(ll_t *ll);                      /* deferred inits, at main */

const char *ll_meta_add(ll_t *ll, const char *text);  /* "!N = text"; returns "!N" */

void        ll_debug_init(ll_t *ll, const char *src_path);   /* DIFile/CU/flags    */

const char *ll_debug_subprogram(ll_t *ll, const char *name, unsigned line);

const char *ll_debug_location(ll_t *ll, unsigned line, unsigned col); /* in cur_sp */

void        ll_debug_finalize(ll_t *ll);              /* emit named + node metadata */

#endif /* SALAM_LLVM_CODEGEN_LLVM_INTERNAL_H */
