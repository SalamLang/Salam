#ifndef _STAGE_AST_TYPE_TYPE_H_
#define _STAGE_AST_TYPE_TYPE_H_

#include <stddef.h>

// base
#include <base.h>

// array
#include <utility/array/type.h>

struct ast_t;

typedef struct ast_t ast_t;

typedef enum ast_node_type_t {
    AST_TYPE_UNKNOWN = 0,
    // AST_TYPE_LITERAL,
    // AST_TYPE_IDENTIFIER,
    // AST_TYPE_BINARY_EXPR,
    // AST_TYPE_UNARY_EXPR,
    AST_TYPE_VAR_DECL,
    AST_TYPE_FUNCTION_DECL,
    // AST_TYPE_FUNCTION_CALL,
    AST_TYPE_BLOCK,
    // AST_TYPE_IF_STMT,
    // AST_TYPE_WHILE_LOOP,
    // AST_TYPE_RETURN_STMT,
    // AST_TYPE_ASSIGNMENT,
    AST_TYPE_PROGRAM,
    // AST_TYPE_EXPRESSION_STATEMENT,
} ast_node_type_t;

typedef struct ast_base_t {
    ast_node_type_t type;
} ast_base_t;

// typedef struct {
//     char* value;
// } ast_literal_t;

// typedef struct {
//     char* name;
// } ast_identifier_t;

// typedef struct {
//     char* op;
//     ast_t* left;
//     ast_t* right;
// } ast_binary_t;

// typedef struct {
//     char* op;
//     ast_t* operand;
// } ast_unary_t;

typedef struct {
    char* name;
    ast_t* value;
} ast_var_decl_t;

// typedef struct {
//     char* name;
//     ast_t* value;
// } ast_assign_t;

typedef struct {
    char* name;
    array_t* params;
    size_t param_count;
    ast_t* body;
} ast_function_decl_t;

// typedef struct {
//     ast_t* callee;
//     array_t* args;
//     size_t arg_count;
// } ast_call_t;

typedef struct {
    array_t* statements;
    size_t statement_count;
} ast_block_t;

// typedef struct {
//     ast_t* condition;
//     ast_t* then_branch;
//     ast_t* else_branch;
// } ast_if_t;

// typedef struct {
//     ast_t* condition;
//     ast_t* body;
// } ast_while_t;

// typedef struct {
//     ast_t* value;
// } ast_return_t;

// typedef struct {
//     ast_t* expression;
// } ast_expression_statement_t;

typedef struct {
    array_t* declarations;
    size_t declaration_count;
} ast_program_t;

typedef struct ast_t {
    ast_base_t base;
    union {
        // ast_literal_t literal_value;
        // ast_identifier_t identifier_value;
        // ast_binary_t binary_value;
        // ast_unary_t unary_value;
        ast_var_decl_t var_decl_value;
        // ast_assign_t assign_value;
        ast_function_decl_t function_decl_value;
        // ast_call_t call_value;
        ast_block_t block_value;
        // ast_if_t if_stmt_value;
        // ast_while_t while_stmt_value;
        // ast_return_t return_stmt_value;
        // ast_expression_statement_t expression_stmt_value;
        ast_program_t program_value;
    } raw;
} ast_t;

// now write create and destroy function for all.

#endif
