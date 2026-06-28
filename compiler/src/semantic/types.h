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

#ifndef SALAM_SEMANTIC_TYPES_H
#define SALAM_SEMANTIC_TYPES_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/vec.h"

typedef enum {
    TY_ERROR = 0,
    TY_VOID, TY_BOOL, TY_CHAR, TY_STR, TY_NULL,
    TY_I8, TY_I16, TY_I32, TY_I64,
    TY_U8, TY_U16, TY_U32, TY_U64,
    TY_F32, TY_F64,
    TY_ARRAY, TY_PTR, TY_STRUCT, TY_ENUM, TY_MAP, TY_MAP_ITER, TY_VEC, TY_FILE,
    TY_FUNC,
    TY_DYN
} type_kind_t;

typedef struct type_t type_t;

struct type_t {
    type_kind_t kind;
    type_t     *elem;
    size_t      length;
    type_t     *pointee;
    type_t     *key;
    void       *decl;
    const char *name;
    vec_t       params;
};

typedef struct {
    arena_t *a;
    type_t  *prims[TY_F64 + 1];
    vec_t    arrays;
    vec_t    ptrs;
} type_ctx_t;

type_ctx_t *type_ctx_new(arena_t *a);

type_t *type_prim(type_ctx_t *tc, type_kind_t kind);

type_t *type_array(type_ctx_t *tc, type_t *elem, size_t length);

type_t *type_ptr(type_ctx_t *tc, type_t *pointee);

type_t *type_struct(type_ctx_t *tc, void *decl, const char *name);

type_t *type_enum(type_ctx_t *tc, void *decl, const char *name);

type_t *type_dyn(type_ctx_t *tc, void *iface_decl, const char *name);

type_t *type_map(type_ctx_t *tc, type_t *key, type_t *val);

type_t *type_map_iter(type_ctx_t *tc, type_t *key, type_t *val);

type_t *type_vec(type_ctx_t *tc, type_t *elem);

type_t *type_file(type_ctx_t *tc);

type_t *type_func(type_ctx_t *tc, type_t *ret, const vec_t *params);

int type_prim_kind_from_name(const char *name);

bool type_is_error(const type_t *t);

bool type_is_integer(const type_t *t);

bool type_is_float(const type_t *t);

bool type_is_numeric(const type_t *t);

bool type_is_signed(const type_t *t);

bool    type_equiv(const type_t *a, const type_t *b);

bool    type_assignable(const type_t *dst, const type_t *src);

bool    type_castable(const type_t *dst, const type_t *src);

type_t *type_common_arith(type_ctx_t *tc, type_t *a, type_t *b);

const char *type_to_string(type_ctx_t *tc, const type_t *t);

#endif /* SALAM_SEMANTIC_TYPES_H */
