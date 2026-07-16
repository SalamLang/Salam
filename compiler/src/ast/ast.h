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
    AST_IMPORT,
    AST_LINK,
    AST_TYPE_ALIAS,
    AST_CONST_DECL,
    AST_FUNC_DEF,
    AST_PARAM,
    AST_STRUCT_DEF,
    AST_FIELD,
    AST_INTERFACE_DEF,
    AST_IMPL_DEF,
    AST_ENUM_DEF,
    AST_ENUM_MEMBER,
    AST_BLOCK,
    AST_VAR_DECL,
    AST_ASSIGN,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_REPEAT,
    AST_RETURN,
    AST_BREAK,
    AST_CONTINUE,
    AST_DEFER,
    AST_EXPR_STMT,
    AST_BINARY,
    AST_UNARY,
    AST_INCDEC,
    AST_CAST,
    AST_LITERAL,
    AST_IDENTIFIER,
    AST_THIS,
    AST_CALL,
    AST_MEMBER,
    AST_INDEX,
    AST_SLICE,
    AST_LAMBDA,
    AST_ARRAY_LIT,
    AST_STRUCT_LIT,
    AST_FIELD_INIT,
    AST_TYPE,
    AST_LAYOUT_BLOCK,
    AST_LAYOUT_ELEMENT,
    AST_LAYOUT_ATTR,
    AST_LAYOUT_COMPONENT,
    AST__COUNT
} ast_kind_t;

typedef struct ast_node ast_node_t;

struct ast_node {
    ast_kind_t kind;
    src_span_t span;
    const char *name;
    token_value_t value;
    token_kind_t op;
    bool is_mut;
    bool is_pointer;
    bool synthetic;
    bool is_extern;
    bool is_variadic;
    bool is_dyn;
    bool is_ref;
    bool is_pub;
    bool is_inline;
    bool is_noinline;
    bool is_pure;
    bool is_noret;
    bool is_deprecated;
    bool is_slice;
    bool is_prefix;
    ast_node_t *type;
    ast_node_t *a, *b, *c, *d;
    vec_t list;
    vec_t dims;
    vec_t typarams;
    vec_t typaram_bounds;
    vec_t aliases;
    vec_t captures;
    const char *type_str;
};

ast_node_t *ast_new(arena_t *a, ast_kind_t kind, const src_span_t *span);

void ast_add(arena_t *a, ast_node_t *parent, ast_node_t *child);

const char *ast_kind_name(ast_kind_t kind);

ast_node_t *ast_clone(arena_t *a, const ast_node_t *n);

void ast_to_xml(xml_writer_t *w, const ast_node_t *node);

#endif /* SALAM_AST_AST_H */
