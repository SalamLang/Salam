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

    AST_TYPE_PROGRAM,

    AST_TYPE_VAR_DECL,

    AST_TYPE_FUNCTION_DECL,

    AST_TYPE_BLOCK,

    // parameters - a list of pairs of name and type
    AST_TYPE_PARAMETER,
    AST_TYPE_PARAMETERS,

    // attributes - a list of pairs of name and value
    AST_TYPE_ATTRIBUTE,
    AST_TYPE_ATTRIBUTES,

    // arguments - a list of values
    AST_TYPE_ARGUMENT,
    AST_TYPE_ARGUMENTS,

    AST_TYPE_IF_STMT,
    AST_TYPE_WHILE_LOOP,
    AST_TYPE_RETURN_STMT,
    AST_TYPE_ASSIGNMENT,

    AST_TYPE_EXPRESSION,
    AST_TYPE_EXPRESSION_LITERAL,
    AST_TYPE_EXPRESSION_IDENTIFIER,
    AST_TYPE_EXPRESSION_BINARY,
    AST_TYPE_EXPRESSION_UNARY,
    AST_TYPE_EXPRESSION_INDEX,
    AST_TYPE_EXPRESSION_FUNCTION_CALL,

    AST_TYPE_EXPRESSION_STATEMENT,
} ast_node_type_t;

typedef struct ast_base_t {
    ast_node_type_t type;
} ast_base_t;

typedef struct {
    value_t* value;
    ast_type_t* runtime_type; // (nullable)
} ast_expression_literal_t;

typedef struct {
    char* name;
    ast_type_t* runtime_type; // (nullable)
} ast_expression_identifier_t;

typedef struct {
    char* op;
    ast_t* left; // ast_expression_t
    ast_t* right; // ast_expression_t
    ast_type_t* runtime_type; // (nullable)
} ast_expression_binary_t;

typedef struct {
    ast_t* object; // ast_expression_t
    ast_t* index; // ast_expression_t
    ast_type_t* runtime_type; // (nullable)
} ast_expression_index_t;

typedef struct {
    token_operator_type_t op_type;
    ast_t* operand; // ast_expression_t
    ast_type_t* runtime_type; // (nullable)
} ast_expression_unary_t;

typedef struct {
    ast_t* callee; // ast_expression_t
    ast_t* args; // ast_expressions_t
    ast_type_t* runtime_type; // (nullable)
} ast_expression_call_t;

typedef struct {
    char* name;
    ast_t* type; // ast_type_t
    ast_t* value; // ast_expression_t (nullable)
} ast_var_decl_t;

typedef struct {
    char* name;
    ast_t* parameters; // ast_parameters_t
    ast_t* block; // ast_block_t
} ast_function_decl_t;

typedef struct {
    array_t* statements;
    size_t statement_count;
} ast_block_t;

typedef struct {
    bool is_last; // true if this is the last statement in the if
    ast_t* condition; // ast_expression_t (nullable)
    ast_t* then_branch; // ast_block_t
    ast_t* else_branch; // ast_block_t (nullable)
} ast_if_t;

typedef struct {
    ast_t* condition; // ast_expression_t
    ast_t* block; // ast_block_t
} ast_while_t;

typedef struct {
    ast_t* value; // ast_expression_t
} ast_expression_statement_t;

typedef struct {

} ast_expression_t;

typedef struct {
    ast_t* value; // ast_expression_t
} ast_return_t;

typedef struct {
    ast_t* value; // ast_expression_t
} ast_print_t;

typedef struct {
    array_t* var_declarations; // ast_var_decl_t
    array_t* function_declarations; // ast_function_decl_t
    size_t declaration_count;
} ast_program_t;

typedef enum {
    AST_TYPE_KIND_INT,
    AST_TYPE_KIND_FLOAT,
    AST_TYPE_KIND_STRING,
    AST_TYPE_KIND_BOOL,
    AST_TYPE_KIND_ARRAY,
    AST_TYPE_KIND_MAP,
    AST_TYPE_KIND_SET,
    AST_TYPE_KIND_TUPLE,
    AST_TYPE_KIND_VOID,
    AST_TYPE_KIND_ANY,
    AST_TYPE_KIND_USER_DEFINED,
} ast_type_kind_t;

typedef struct ast_type_t ast_type_t;

struct ast_type_t {
    ast_type_kind_t kind;

    // Base type modifiers
    bool is_pointer;
    bool is_reference;
    bool is_const;

    // For user-defined types
    char* name; // e.g., "MyStruct"

    // For array types (e.g., int[][] = array of array of int)
    ast_type_t* element_type;

    // For map types: key_type and value_type
    ast_type_t* key_type;
    ast_type_t* value_type;

    // For tuple types: list of element types
    array_t* tuple_elements; // ast_type_t*

    // token_location_t location;
};

// parameters - a list of pairs of name and type
typedef struct {
    char* name;
    ast_t* type;
} ast_parameter_t;

typedef struct {
    array_t* values; // ast_parameter_t
    size_t value_count;
} ast_parameters_t;

// attributes - a list of pairs of name and value
typedef struct {
    char* name;
    ast_t* value;
} ast_attribute_t;

typedef struct {
    array_t* values; // ast_attribute_t
    size_t value_count;
} ast_attributes_t;

// arguments - a list of values
typedef struct {
    ast_t* value;
} ast_argument_t;

typedef struct {
    array_t* values; // ast_argument_t
    size_t value_count;
} ast_arguments_t;

typedef struct ast_t {
    ast_base_t base;
    union {
        ast_program_t program_value;

        ast_var_decl_t var_decl_value;

        ast_function_decl_t function_decl_value;

        ast_block_t block_value;

        // parameters - a list of pairs of name and type
        ast_parameter_t parameter_value;
        ast_parameters_t parameters_value;

        // attributes - a list of pairs of name and value
        ast_attribute_t attribute_value;
        ast_attributes_t attributes_value;

        // arguments - a list of values
        ast_argument_t argument_value;
        ast_arguments_t arguments_value;

        // ast_literal_t literal_value;
        // ast_identifier_t identifier_value;
        // ast_binary_t binary_value;
        // ast_unary_t unary_value;
        // ast_assign_t assign_value;
        // ast_call_t call_value;
        // ast_if_t if_stmt_value;
        // ast_while_t while_stmt_value;
        // ast_return_t return_stmt_value;
        // ast_expression_statement_t expression_stmt_value;
    } raw;
} ast_t;

#endif
