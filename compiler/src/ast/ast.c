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

#include "core/prelude.h"
#include "ast/ast.h"

static const char *const k_ast_names[AST__COUNT] = {
    [AST_PROGRAM] = "Program",
    [AST_IMPORT] = "Import",
    [AST_LINK] = "Link",
    [AST_TYPE_ALIAS] = "TypeAlias",
    [AST_CONST_DECL] = "ConstDecl",
    [AST_FUNC_DEF] = "FunctionDef",
    [AST_PARAM] = "Param",
    [AST_STRUCT_DEF] = "StructDef",
    [AST_FIELD] = "Field",
    [AST_INTERFACE_DEF] = "InterfaceDef",
    [AST_IMPL_DEF] = "ImplDef",
    [AST_ENUM_DEF] = "EnumDef",
    [AST_ENUM_MEMBER] = "EnumMember",
    [AST_BLOCK] = "Block",
    [AST_VAR_DECL] = "VariableDecl",
    [AST_ASSIGN] = "Assignment",
    [AST_IF] = "IfStmt",
    [AST_WHILE] = "WhileStmt",
    [AST_FOR] = "ForStmt",
    [AST_REPEAT] = "RepeatStmt",
    [AST_RETURN] = "ReturnStmt",
    [AST_BREAK] = "BreakStmt",
    [AST_CONTINUE] = "ContinueStmt",
    [AST_DEFER] = "DeferStmt",
    [AST_EXPR_STMT] = "ExprStmt",
    [AST_BINARY] = "BinaryExpr",
    [AST_UNARY] = "UnaryExpr",
    [AST_CAST] = "Cast",
    [AST_LITERAL] = "Literal",
    [AST_IDENTIFIER] = "Identifier",
    [AST_THIS] = "This",
    [AST_CALL] = "Call",
    [AST_MEMBER] = "MemberAccess",
    [AST_INDEX] = "ArrayAccess",
    [AST_SLICE] = "SliceExpr",
    [AST_ARRAY_LIT] = "ArrayLiteral",
    [AST_LAMBDA] = "Lambda",
    [AST_STRUCT_LIT] = "StructLiteral",
    [AST_FIELD_INIT] = "FieldInit",
    [AST_TYPE] = "Type",
    [AST_LAYOUT_BLOCK] = "LayoutBlock",
    [AST_LAYOUT_ELEMENT] = "LayoutElement",
    [AST_LAYOUT_ATTR] = "LayoutAttribute",
    [AST_LAYOUT_COMPONENT] = "LayoutComponent",
};

const char *ast_kind_name(ast_kind_t kind)
{
    if (kind < 0 || kind >= AST__COUNT || !k_ast_names[kind]) return "Unknown";
    return k_ast_names[kind];
}

ast_node_t *ast_new(arena_t *a, ast_kind_t kind, const src_span_t *span)
{
    ast_node_t *n = (ast_node_t *)arena_alloc(a, sizeof(*n));
    memset(n, 0, sizeof(*n));
    n->kind = kind;
    if (span) n->span = *span;
    n->value.kind = TV_NONE;
    vec_init(&n->list);
    vec_init(&n->dims);
    vec_init(&n->typarams);
    vec_init(&n->typaram_bounds);
    vec_init(&n->aliases);
    vec_init(&n->captures);
    return n;
}

void ast_add(arena_t *a, ast_node_t *parent, ast_node_t *child)
{
    if (child) vec_push(a, &parent->list, child);
}

ast_node_t *ast_clone(arena_t *a, const ast_node_t *n)
{
    if (!n) return NULL;
    ast_node_t *c = (ast_node_t *)arena_alloc(a, sizeof(*c));
    *c = *n;
    vec_init(&c->list);
    vec_init(&c->dims);
    vec_init(&c->typarams);
    vec_init(&c->typaram_bounds);
    vec_init(&c->aliases);
    vec_init(&c->captures);
    c->type = ast_clone(a, n->type);
    c->a = ast_clone(a, n->a);
    c->b = ast_clone(a, n->b);
    c->c = ast_clone(a, n->c);
    c->d = ast_clone(a, n->d);
    {
        size_t i = 0;
        for (; i < n->list.len; i++)
            vec_push(a, &c->list, ast_clone(a, (const ast_node_t *)n->list.data[i]));
    }
    {
        size_t i = 0;
        for (; i < n->dims.len; i++)
            vec_push(a, &c->dims, ast_clone(a, (const ast_node_t *)n->dims.data[i]));
    }
    {
        size_t i = 0;
        for (; i < n->typarams.len; i++)
            vec_push(a, &c->typarams, n->typarams.data[i]);
    }
    {
        size_t i = 0;
        for (; i < n->typaram_bounds.len; i++)
            vec_push(a, &c->typaram_bounds, n->typaram_bounds.data[i]);
    }
    {
        size_t i = 0;
        for (; i < n->aliases.len; i++)
            vec_push(a, &c->aliases, n->aliases.data[i]);
    }
    {
        size_t i = 0;
        for (; i < n->captures.len; i++)
            vec_push(a, &c->captures,
                     ast_clone(a, (const ast_node_t *)n->captures.data[i]));
    }
    return c;
}
