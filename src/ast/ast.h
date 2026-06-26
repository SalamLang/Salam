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

#ifndef SALAM_AST_AST_H
#define SALAM_AST_AST_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/vec.h"
#include "core/span.h"
#include "token/token.h"
#include "xml/xml.h"

typedef enum {
    AST_PROGRAM = 0,
    AST_IMPORT, AST_LINK, AST_TYPE_ALIAS, AST_CONST_DECL,
    AST_FUNC_DEF, AST_PARAM,
    AST_STRUCT_DEF, AST_FIELD,
    AST_INTERFACE_DEF,
    AST_IMPL_DEF,            /* `impl Iface for Type:` block (name=iface, type=target, list=methods) */
    AST_ENUM_DEF, AST_ENUM_MEMBER,
    AST_BLOCK,
    AST_VAR_DECL, AST_ASSIGN,
    AST_IF, AST_WHILE, AST_FOR, AST_REPEAT,
    AST_RETURN, AST_BREAK, AST_CONTINUE, AST_DEFER, AST_EXPR_STMT,
    AST_BINARY, AST_UNARY, AST_CAST,
    AST_LITERAL, AST_IDENTIFIER, AST_THIS,
    AST_CALL, AST_MEMBER, AST_INDEX,
    AST_LAMBDA,
    AST_ARRAY_LIT, AST_STRUCT_LIT, AST_FIELD_INIT,
    AST_TYPE,
    AST_LAYOUT_BLOCK, AST_LAYOUT_ELEMENT, AST_LAYOUT_ATTR,
    AST_LAYOUT_COMPONENT, /* named layout fragment; name; list=params; a=body block */
    AST__COUNT
} ast_kind_t;

typedef struct ast_node ast_node_t;

struct ast_node {
    ast_kind_t    kind;
    src_span_t    span;
    const char   *name;       /* identifier / op-less name / type base / etc. */
    token_value_t value;      /* literal value (TV_NONE otherwise)            */
    token_kind_t  op;         /* operator or literal token kind               */
    bool          is_mut;
    bool          is_pointer; /* AST_TYPE pointer suffix                      */
    bool          synthetic;  /* generated (e.g. a monomorphized generic instance) */
    bool          is_extern;  /* FUNC_DEF/VAR_DECL declared in an `extern "C"` block:
                               * no body/definition; linked to an external C symbol */
    bool          is_variadic;/* FUNC_DEF: trailing `...` (C variadic call ABI)     */
    bool          is_dyn;     /* AST_TYPE: `dyn Iface` dynamic-interface fat pointer */
    bool          is_ref;     /* AST_PARAM: pass-by-reference; AST_IDENTIFIER: refers to a ref param */
    bool          is_pub;     /* top-level def marked `pub`/`عمومی`: exported from its package */
    ast_node_t   *type;       /* annotation / cast target / return / aliased  */
    ast_node_t   *a, *b, *c, *d;
    vec_t         list;       /* variable-arity children                      */
    vec_t         dims;       /* AST_TYPE array dimensions (expr* or NULL)     */
    vec_t         typarams;   /* generic type-parameter names (const char*):
                               * on AST_STRUCT_DEF / AST_FUNC_DEF templates    */
    vec_t         typaram_bounds; /* parallel to typarams: interface bound name
                               * (const char*) for each `T: Iface`, or NULL     */
    vec_t         aliases;    /* localization metadata: alternating (lang, name)
                               * const char* pairs from `@lang "name"` annotations */
    vec_t         captures;   /* AST_LAMBDA: captured enclosing locals (AST_IDENTIFIER
                               * nodes carrying name + resolved type_str) */
    const char   *type_str;   /* resolved type name, filled by semantic phase */
};

ast_node_t *ast_new(arena_t *a, ast_kind_t kind, const src_span_t *span);

void        ast_add(arena_t *a, ast_node_t *parent, ast_node_t *child);

const char *ast_kind_name(ast_kind_t kind);

ast_node_t *ast_clone(arena_t *a, const ast_node_t *n);

void ast_to_xml(xml_writer_t *w, const ast_node_t *node);

#endif /* SALAM_AST_AST_H */
