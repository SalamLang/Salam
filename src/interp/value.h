#ifndef SALAM_INTERP_VALUE_H
#define SALAM_INTERP_VALUE_H
#include "core/prelude.h"
#include "core/arena.h"
#include "core/vec.h"
#include "ast/ast.h"

typedef enum {
    VAL_NULL = 0,
    VAL_INT,        /* i64 (covers all Salam integer widths)            */
    VAL_CHAR,       /* char: an int code, but prints as its glyph       */
    VAL_FLOAT,      /* f64                                              */
    VAL_BOOL,
    VAL_STR,        /* arena, NUL-terminated UTF-8                      */
    VAL_ARRAY,      /* fixed arrays and Vector<T> alike                 */
    VAL_MAP,        /* HashMap<K,V>                                     */
    VAL_MAPITER,    /* MapIter<K,V>                                     */
    VAL_STRUCT,     /* user struct instance                            */
    VAL_FUNC,       /* function / lambda value (closure)               */
    VAL_MODULE      /* an imported package namespace (std .salam)     */
} val_kind_t;
typedef struct value      value_t;
typedef struct sarray     sarray_t;
typedef struct smap       smap_t;
typedef struct smapiter   smapiter_t;
typedef struct sstruct    sstruct_t;
typedef struct sclosure   sclosure_t;
struct env;      /* defined in interp_internal.h */
struct module;   /* defined in interp_internal.h: an imported package */
struct value {
    val_kind_t kind;
    union {
        int64_t      i;
        double       f;
        bool         b;
        const char  *s;
        sarray_t    *arr;
        smap_t      *map;
        smapiter_t  *iter;
        sstruct_t   *st;
        sclosure_t  *fn;
        struct module *mod;
    } as;
};
struct sarray {
    value_t *data;
    size_t   len;
    size_t   cap;
};
typedef struct {
    value_t key;
    value_t val;
    bool    used;
} smap_entry_t;
struct smap {
    smap_entry_t *entries;
    size_t        count;   /* live entries */
    size_t        cap;     /* allocated slots */
};
struct smapiter {
    smap_t *map;
    size_t  idx;           /* next slot to visit */
};
typedef struct {
    const char *name;
    value_t     val;
} sfield_t;
struct sstruct {
    const char *type_name;   /* struct def name, as written in source        */
    ast_node_t *def;         /* AST_STRUCT_DEF (used for method lookup)       */
    sfield_t   *fields;
    size_t      nfields;
};
struct sclosure {
    ast_node_t  *fn;         /* AST_FUNC_DEF or AST_LAMBDA */
    struct env  *env;        /* captured environment       */
};
static inline value_t val_null(void)        { value_t v; v.kind = VAL_NULL;  return v; }

static inline value_t val_int(int64_t x)    { value_t v; v.kind = VAL_INT;   v.as.i = x; return v; }

static inline value_t val_char(int64_t x)   { value_t v; v.kind = VAL_CHAR;  v.as.i = x; return v; }

static inline value_t val_float(double x)   { value_t v; v.kind = VAL_FLOAT; v.as.f = x; return v; }

static inline value_t val_bool(bool x)      { value_t v; v.kind = VAL_BOOL;  v.as.b = x; return v; }

static inline value_t val_str(const char *s){ value_t v; v.kind = VAL_STR;   v.as.s = s ? s : ""; return v; }

static inline value_t val_module(struct module *m){ value_t v; v.kind = VAL_MODULE; v.as.mod = m; return v; }
#endif /* SALAM_INTERP_VALUE_H */
