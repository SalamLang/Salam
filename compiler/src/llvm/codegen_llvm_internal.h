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

typedef struct {
    const char *ref;
    const char *ts;
} llv_t;

typedef struct {
    const char *ptr;
    const char *ts;
} ll_addr_t;

typedef struct {
    const char *name;
    const char *ptr;
    const char *ts;
} lvar_t;

typedef struct {
    const char *bytes;
    size_t len;
    size_t hash;
    const char *gref;
} lstr_t;

typedef enum {
    LL_H_SUBSTR,
    LL_H_STRCAT,
    LL_H_ISWS,
    LL_H_TRIM,
    LL_H_STRHASH,
    LL_H_INTHASH,
    LL_H_I64STR,
    LL_H_U64STR,
    LL_H_F64STR,
    LL_H_CHARSTR,
    LL_H_OUTBUF,
    LL_H_COUNT
} ll_helper_t;

typedef struct {
    arena_t *a;
    logger_t *log;
    sema_result_t *sem;
    const char *module;
    const char *entry;
    sb_t *g;
    sb_t *b;
    sb_t *allocas;
    int tmp;
    int lbl;
    bool term;
    vec_t locals;
    vec_t strings;
    const char *ret_ts;
    bool is_main;
    const char *brk[64];
    const char *cont[64];
    int nloop;
    vec_t defers;
    const char *self_ts;
    const char *this_ref;
    bool self_byval;
    vec_t globals;
    vec_t gdefer;
    vec_t extern_names;
    scope_t *pkg_scope;
    vec_t emitted;
    vec_t pkg_touched;
    ast_node_t *cur_lambda;
    const char *env_ref;
    const char *env_ty;
    int lam_n;
    bool ok;
    const char *first_error;
    bool debug;
    sb_t *meta;
    int meta_n;
    const char *di_file;
    const char *di_cu;
    const char *di_subty;
    const char *cur_sp;
    const char *cur_dbg;
    const char *di_flag_dwarf;
    const char *di_flag_debug;
    const char *src_file;
    const char *src_dir;
    const char *triple;
    const char *usize;
    int ptr_bits;
    bool optsize;
    bool minsize;
    bool single_threaded;
    sb_t *hg;
    bool hneed[LL_H_COUNT];
    const char *match_result_ptr;
    const char *match_result_ts;
    const char *match_merge_block;
} ll_t;

SAL_INLINE bool ll_is_str(const char *ts)
{
    return ts && (!strcmp(ts, "str") || !strcmp(ts, "uchar"));
}

SAL_INLINE bool ll_is_bool(const char *ts)
{
    return ts && !strcmp(ts, "bool");
}

SAL_INLINE bool ll_is_float(const char *ts)
{
    return ts && (!strcmp(ts, "f32") || !strcmp(ts, "f64"));
}

SAL_INLINE bool ll_is_ptr_ts(const char *ts)
{
    return ts && *ts && ts[strlen(ts) - 1] == '*';
}

SAL_INLINE llv_t ll_poison(const char *ts)
{
    return (llv_t){"0", ts ? ts : "i32"};
}

const char *ll_fmt(ll_t *ll, const char *fmt, ...);

const char *ll_safe_name(ll_t *ll, const char *raw);

void ll_emit(ll_t *ll, const char *fmt, ...);

void ll_emit_alloca(ll_t *ll, const char *fmt, ...);

void ll_emit_label(ll_t *ll, const char *label);

void ll_emit_term(ll_t *ll, const char *fmt, ...);

const char *ll_new_tmp(ll_t *ll);

const char *ll_new_lbl(ll_t *ll, const char *tag);

void ll_error(ll_t *ll, const ast_node_t *n, const char *fmt, ...);

const char *ll_strconst(ll_t *ll, const char *s);

void ll_local_add(ll_t *ll, const char *name, const char *ptr, const char *ts);

lvar_t *ll_local_find(ll_t *ll, const char *name);

lvar_t *ll_global_find(ll_t *ll, const char *name);

int ll_int_bits(const char *ts);

int ll_target_ptr_bits(const char *triple);

bool ll_target_is_windows(const char *triple);

const char *ll_usize_to_i32(ll_t *ll, const char *ref);

bool ll_is_int(const char *ts);

bool ll_is_signed(const char *ts);

const char *ll_ty(ll_t *ll, const char *ts);

const char *ll_conv(ll_t *ll, llv_t v, const char *to_ts);

const char *ll_common(const char *a, const char *b);

const char *ll_as_i1(ll_t *ll, llv_t v);

long ll_array_dim(const char *ts);

const char *ll_array_elem(ll_t *ll, const char *ts);

bool ll_is_slice_ts(const char *ts);

const char *ll_slice_elem(ll_t *ll, const char *ts);

const char *ll_struct_ltype(ll_t *ll, const char *name);

size_t ll_variant_members(const char *ts, char members[][160], size_t max_members);

const char *ll_variant_cname(ll_t *ll, const char *ts);

void ll_ensure_variant_type(ll_t *ll, const char *ts);

void ll_type_layout(ll_t *ll, const char *ts, size_t *out_size, size_t *out_align);

int ll_variant_tag_of(const char *variant_ts, const char *member_ts);

symbol_t *ll_sym(ll_t *ll, const char *name);

symbol_t *ll_struct_sym(ll_t *ll, const char *name);

symbol_t *ll_enum_sym(ll_t *ll, const char *name);

int ll_field_index(symbol_t *ssym, const char *field, symbol_t **out_field);

const char *ll_zero(const char *ts);

const char *ll_func_ret(ll_t *ll, const char *ts);

void ll_func_params(ll_t *ll, const char *ts, vec_t *out);

llv_t ll_expr(ll_t *ll, ast_node_t *n);

const char *ll_match_arm_cond(ll_t *ll, ast_node_t *arm, llv_t subj, bool is_variant);

llv_t ll_binary(ll_t *ll, ast_node_t *n);

ll_addr_t ll_addr_of(ll_t *ll, ast_node_t *n);

bool ll_index_set(ll_t *ll, ast_node_t *index_target, ast_node_t *value);

void ll_stmt(ll_t *ll, ast_node_t *n);

void ll_block(ll_t *ll, ast_node_t *block);

void ll_block_top(ll_t *ll, ast_node_t *block);

void ll_emit_return(ll_t *ll, ast_node_t *value);

const char *ll_mangle(ll_t *ll, const char *owner, const char *fn, func_sig_t *sig);

func_sig_t *ll_pick_overload(ll_t *ll, symbol_t *sym, ast_node_t *call);

void ll_function(ll_t *ll, ast_node_t *fn, symbol_t *owner);

void ll_emit_lambda(ll_t *ll, ast_node_t *n);

void ll_emit_struct_types(ll_t *ll, ast_node_t *program);

void ll_emit_globals(ll_t *ll, ast_node_t *program);

void ll_emit_externs(ll_t *ll);

void ll_emit_impls(ll_t *ll);

void ll_touch_pkg(ll_t *ll, symbol_t *pk);

void ll_touch_pkg_named(ll_t *ll, const char *pkgname);

void ll_ensure_fn(ll_t *ll, ast_node_t *fn, symbol_t *owner, scope_t *pscope);

const char *ll_box_dyn(ll_t *ll, llv_t v, const char *iface);

const char *ll_box_variant(ll_t *ll, llv_t v, int tag, const char *variant_ts);

llv_t ll_unwrap_variant(ll_t *ll, llv_t v, const char *member_ts);

const char *ll_mangle_ti(ll_t *ll, const char *typestr, const char *fn, func_sig_t *sig);

void ll_emit_global_inits(ll_t *ll);

const char *ll_meta_add(ll_t *ll, const char *text);

void ll_debug_init(ll_t *ll, const char *src_path);

const char *ll_debug_subprogram(ll_t *ll, const char *name, unsigned line);

const char *ll_debug_location(ll_t *ll, unsigned line, unsigned col);

void ll_debug_finalize(ll_t *ll);

void ll_need(ll_t *ll, ll_helper_t which);

#endif /* SALAM_LLVM_CODEGEN_LLVM_INTERNAL_H */
