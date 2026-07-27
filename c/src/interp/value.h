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

#ifndef SALAM_INTERP_VALUE_H
#define SALAM_INTERP_VALUE_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/vec.h"
#include "ast/ast.h"

typedef enum {
    VAL_NULL = 0,
    VAL_INT,
    VAL_CHAR,
    VAL_FLOAT,
    VAL_BOOL,
    VAL_STR,
    VAL_ARRAY,
    VAL_MAP,
    VAL_MAPITER,
    VAL_STRUCT,
    VAL_FUNC,
    VAL_MODULE,
    VAL_PTR,
    VAL_VARIANT
} val_kind_t;

typedef enum {
    PTR_I8,
    PTR_U8,
    PTR_I16,
    PTR_U16,
    PTR_I32,
    PTR_U32,
    PTR_I64,
    PTR_U64,
    PTR_F32,
    PTR_F64,
    PTR_STR,
    PTR_OPAQUE
} ptr_elem_t;

typedef struct value value_t;

typedef struct sarray sarray_t;

typedef struct smap smap_t;

typedef struct smapiter smapiter_t;

typedef struct sstruct sstruct_t;

typedef struct sclosure sclosure_t;

typedef struct svariant svariant_t;

struct env;

struct module;

typedef struct {
    void *addr;
    ptr_elem_t elem;
} sptr_t;

struct value {
    val_kind_t kind;
    union {
        int64_t i;
        double f;
        bool b;
        const char *s;
        sarray_t *arr;
        smap_t *map;
        smapiter_t *iter;
        sstruct_t *st;
        sclosure_t *fn;
        struct module *mod;
        sptr_t ptr;
        svariant_t *variant;
    } as;
};

struct svariant {
    int32_t tag;
    value_t *boxed;
};

struct sarray {
    value_t *data;
    size_t len;
    size_t cap;
};

typedef struct {
    value_t key;
    value_t val;
    bool used;
} smap_entry_t;

struct smap {
    smap_entry_t *entries;
    size_t count;
    size_t cap;
    uint32_t *index;
    size_t index_cap;
    size_t index_used;
    size_t free_hint;
};

struct smapiter {
    smap_t *map;
    size_t idx;
};

typedef struct {
    const char *name;
    value_t val;
} sfield_t;

struct sstruct {
    const char *type_name;
    ast_node_t *def;
    sfield_t *fields;
    size_t nfields;
};

struct sclosure {
    ast_node_t *fn;
    struct env *env;
};

SAL_INLINE value_t val_null(void)
{
    value_t v;
    v.kind = VAL_NULL;
    return v;
}

SAL_INLINE value_t val_int(int64_t x)
{
    value_t v;
    v.kind = VAL_INT;
    v.as.i = x;
    return v;
}

SAL_INLINE value_t val_char(int64_t x)
{
    value_t v;
    v.kind = VAL_CHAR;
    v.as.i = x;
    return v;
}

SAL_INLINE value_t val_float(double x)
{
    value_t v;
    v.kind = VAL_FLOAT;
    v.as.f = x;
    return v;
}

SAL_INLINE value_t val_bool(bool x)
{
    value_t v;
    v.kind = VAL_BOOL;
    v.as.b = x;
    return v;
}

SAL_INLINE value_t val_str(const char *s)
{
    value_t v;
    v.kind = VAL_STR;
    v.as.s = s ? s : "";
    return v;
}

SAL_INLINE value_t val_module(struct module *m)
{
    value_t v;
    v.kind = VAL_MODULE;
    v.as.mod = m;
    return v;
}

SAL_INLINE value_t val_ptr(void *addr, ptr_elem_t elem)
{
    value_t v;
    v.kind = VAL_PTR;
    v.as.ptr.addr = addr;
    v.as.ptr.elem = elem;
    return v;
}

SAL_INLINE value_t val_variant(svariant_t *variant)
{
    value_t v;
    v.kind = VAL_VARIANT;
    v.as.variant = variant;
    return v;
}

SAL_INLINE size_t ptr_elem_size(ptr_elem_t elem)
{
    switch (elem) {
    case PTR_I8:
    case PTR_U8:
        return 1;
    case PTR_I16:
    case PTR_U16:
        return 2;
    case PTR_I32:
    case PTR_U32:
    case PTR_F32:
        return 4;
    default:
        return 8;
    }
}

#endif /* SALAM_INTERP_VALUE_H */
