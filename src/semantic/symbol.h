#ifndef SALAM_SEMANTIC_SYMBOL_H
#define SALAM_SEMANTIC_SYMBOL_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/vec.h"
#include "ast/ast.h"
#include "semantic/types.h"

typedef enum {
    SYM_VAR, SYM_CONST, SYM_PARAM, SYM_FIELD,
    SYM_FUNC, SYM_METHOD,
    SYM_STRUCT, SYM_ENUM, SYM_ALIAS, SYM_ENUM_MEMBER,
    SYM_PACKAGE, SYM_INTERFACE,
    SYM_TYPEIMPL    /* synthetic owner of `impl Iface for Type` methods (Type may be a primitive); members = its methods; type = target type */
} sym_kind_t;

typedef struct scope_t scope_t;

typedef struct symbol_t symbol_t;

typedef struct {
    vec_t        params;        /* type_t* */
    size_t       required;      /* params without defaults */
    type_t      *ret;
    ast_node_t  *decl;          /* AST_FUNC_DEF */
    symbol_t    *owner;         /* owning struct symbol for methods, else NULL */
    const char  *mangled;
    bool         variadic;      /* extern "C" fn with trailing `...` (C varargs) */
    bool         infer_ret;     /* lambda with an inferred return type: the first
                                 * `ret` sets `ret` instead of being checked against it */
} func_sig_t;

struct symbol_t {
    sym_kind_t   kind;
    const char  *name;
    type_t      *type;          /* var/const/param/field type; struct/enum -> its type */
    bool         is_mut;
    bool         is_ref;       /* SYM_PARAM: pass-by-reference (C pointer, Salam transparent) */
    bool         is_pub;       /* exported from its package (declared `pub`/`عمومی`) */
    bool         used;         /* SYM_VAR: referenced after declaration (unused-var check) */
    ast_node_t  *decl;
    vec_t        overloads;     /* func_sig_t*  (SYM_FUNC / SYM_METHOD)        */
    scope_t     *members;       /* struct member scope / enum member list      */
    long long    enum_value;    /* SYM_ENUM_MEMBER                             */
    bool         has_ival;      /* SYM_CONST with a folded integer value       */
    long long    ival;          /* the folded value (for array sizes, etc.)    */
    const char  *pkgname;       /* SYM_PACKAGE: canonical package name (mangling) */
    const char  *generic_base;  /* monomorphized instance: base template name ("Option") */
    vec_t        generic_args;  /* monomorphized instance: concrete type_t* arguments    */
    scope_t     *home;          /* defining package's global scope (for instantiating an
                                 * imported/prelude generic in its own package context)   */
};

typedef enum { SCOPE_GLOBAL, SCOPE_STRUCT, SCOPE_FUNC, SCOPE_BLOCK } scope_kind_t;

struct scope_t {
    scope_kind_t kind;
    scope_t     *parent;
    vec_t        symbols;       /* symbol_t* */
    type_t      *self_type;     /* method bodies: *Struct, else NULL */
    func_sig_t  *func;          /* enclosing function/method (return checks) */
    const char  *label;         /* function/struct name for XML */
    scope_t     *aux;           /* secondary fallback scope, searched by scope_lookup
                                 * after the parent chain misses. Set on a cross-package
                                 * generic instance's member scope to the DEFINING
                                 * package scope, so its deferred method bodies resolve
                                 * that package's imports/private funcs (e.g. `mem`,
                                 * `salam_alloc`) even though the instance is checked in
                                 * the use-site's global. The parent chain still reaches
                                 * the use-site global for use-site-local type args. */
};

scope_t  *scope_new(arena_t *a, scope_kind_t kind, scope_t *parent);

symbol_t *symbol_new(arena_t *a, sym_kind_t kind, const char *name);

symbol_t *scope_define(arena_t *a, scope_t *s, symbol_t *sym);

symbol_t *scope_lookup_local(scope_t *s, const char *name);

symbol_t *scope_lookup(scope_t *s, const char *name);

const char *mangle_func(arena_t *a, const char *struct_name /*nullable*/,
                        const char *fn, const vec_t *param_types);

                        const char *impl_owner_key(arena_t *a, const char *typestr);

#endif /* SALAM_SEMANTIC_SYMBOL_H */
