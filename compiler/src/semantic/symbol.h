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

#ifndef SALAM_SEMANTIC_SYMBOL_H
#define SALAM_SEMANTIC_SYMBOL_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/vec.h"
#include "ast/ast.h"
#include "semantic/types.h"

typedef enum {
    SYM_VAR,
    SYM_CONST,
    SYM_PARAM,
    SYM_FIELD,
    SYM_FUNC,
    SYM_METHOD,
    SYM_STRUCT,
    SYM_ENUM,
    SYM_ALIAS,
    SYM_ENUM_MEMBER,
    SYM_PACKAGE,
    SYM_INTERFACE,
    SYM_TYPEIMPL
} sym_kind_t;

typedef struct scope_t scope_t;

typedef struct symbol_t symbol_t;

typedef struct {
    vec_t params;
    size_t required;
    type_t *ret;
    ast_node_t *decl;
    symbol_t *owner;
    const char *mangled;
    bool variadic;
    bool infer_ret;
} func_sig_t;

struct symbol_t {
    sym_kind_t kind;
    const char *name;
    type_t *type;
    bool is_mut;
    bool is_ref;
    bool is_pub;
    bool used;
    ast_node_t *decl;
    vec_t overloads;
    scope_t *members;
    long long enum_value;
    bool has_ival;
    long long ival;
    const char *pkgname;
    const char *generic_base;
    vec_t generic_args;
    scope_t *home;
};

typedef enum { SCOPE_GLOBAL, SCOPE_STRUCT, SCOPE_FUNC, SCOPE_BLOCK } scope_kind_t;

struct scope_t {
    scope_kind_t kind;
    scope_t *parent;
    vec_t symbols;
    type_t *self_type;
    func_sig_t *func;
    const char *label;
    scope_t *aux;
    const char *lang;
};

scope_t *scope_new(arena_t *a, scope_kind_t kind, scope_t *parent);

symbol_t *symbol_new(arena_t *a, sym_kind_t kind, const char *name);

symbol_t *scope_define(arena_t *a, scope_t *s, symbol_t *sym);

symbol_t *scope_lookup_local(scope_t *s, const char *name);

symbol_t *scope_lookup(scope_t *s, const char *name);

const char *mangle_func(arena_t *a, const char *struct_name, const char *fn,
                        const vec_t *param_types);

const char *impl_owner_key(arena_t *a, const char *typestr);

#endif /* SALAM_SEMANTIC_SYMBOL_H */
