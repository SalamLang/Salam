#ifndef _STAGE_AST_TYPE_H_
#define _STAGE_AST_TYPE_H_

#include <stddef.h>

// base
// #include <base.h>
// TODO

// array
#include <utility/array/type.h>

// hashmap
#include <utility/hashmap/type.h>

// token
#include <stage/token/type.h>

// value
#include <stage/value/type.h>

struct ast_t;

typedef struct ast_t ast_t;

typedef enum ast_node_type_t {
    AST_TYPE_UNKNOWN = 0,

    AST_TYPE_PROGRAM,

    AST_TYPE_PACKAGE,

    AST_TYPE_IMPORT,

    // kind
    AST_TYPE_KIND_DECL,
    AST_TYPE_KIND_STRUCT,
    AST_TYPE_KIND_ENUM,
    AST_TYPE_KIND_UNION,

    // extern
    AST_TYPE_EXTERN_DECL,
    AST_TYPE_EXTERN_VARIABLE,
    AST_TYPE_EXTERN_FUNCTION,

    AST_TYPE_VARIABLE_DECL,
    AST_TYPE_FUNCTION_DECL,

    AST_TYPE_BLOCK,

    // parameters - a list of pairs of name, type, and optional default value
    AST_TYPE_PARAMETER_ITEM,
    AST_TYPE_PARAMETERS,

    // attributes - a list of pairs of name and value
    AST_TYPE_ATTRIBUTE_ITEM,
    AST_TYPE_ATTRIBUTES,

    // arguments - a list of values
    AST_TYPE_ARGUMENT_ITEM,
    AST_TYPE_ARGUMENTS,

    // expressions
    AST_TYPE_EXPRESSION_ITEM,
    AST_TYPE_EXPRESSIONS,
    AST_TYPE_EXPRESSION_LITERAL,
    AST_TYPE_EXPRESSION_IDENTIFIER,
    AST_TYPE_EXPRESSION_BINARY,
    AST_TYPE_EXPRESSION_UNARY,
    AST_TYPE_EXPRESSION_INDEX,
    AST_TYPE_EXPRESSION_FUNCTION_CALL,

    // statements
    AST_TYPE_STATEMENT_IF,
    AST_TYPE_STATEMENT_FOR,
    AST_TYPE_STATEMENT_FOREACH,
    AST_TYPE_STATEMENT_RETURN,
    AST_TYPE_STATEMENT_PRINT,
    AST_TYPE_STATEMENT_VARIABLE_DECL,
    AST_TYPE_STATEMENT_EXPRESSION,
} ast_node_type_t;

typedef struct ast_base_t {
    ast_node_type_t type;
} ast_base_t;

typedef struct ast_type_t ast_type_t;

typedef struct
{
    value_t* value;
    ast_type_t* runtime_type; // (nullable)
} ast_expression_literal_t;

typedef struct
{
    char* name;
    ast_type_t* runtime_type; // (nullable)
} ast_expression_identifier_t;

typedef struct
{
    char* op;
    ast_t* left; // ast_expression_item_t
    ast_t* right; // ast_expression_item_t
    ast_type_t* runtime_type; // (nullable)
} ast_expression_binary_t;

typedef struct
{
    ast_t* object; // ast_expression_item_t
    ast_t* index; // ast_expression_item_t
    ast_type_t* runtime_type; // (nullable)
} ast_expression_index_t;

typedef struct
{
    token_operator_type_t op_type;
    ast_t* operand; // ast_expression_item_t
    ast_type_t* runtime_type; // (nullable)
} ast_expression_unary_t;

typedef struct
{
    ast_t* callee; // ast_expression_item_t
    ast_t* args; // ast_expressions_t
    ast_type_t* runtime_type; // (nullable)
} ast_expression_call_t;

typedef struct
{
    char* name;
    ast_t* type; // ast_type_t
    ast_t* value; // ast_expression_item_t (nullable)
} ast_variable_decl_t;

typedef struct
{
    char* name;
} ast_package_t;

typedef struct
{
    char* name;
    // TODO
} ast_import_t;

typedef enum
{
    AST_EXTERN_TYPE_FUNCTION,
    AST_EXTERN_TYPE_VARIABLE,
} ast_extern_decl_type_t;

typedef struct
{
    char* name;
    char* alias; // (nullable)
    ast_t* type; // ast_type_t
    ast_t* attributes; // ast_attributes_t (nullable)
} ast_extern_function_t;

typedef struct
{
    char* name;
    char* alias; // (nullable)
    ast_t* type; // ast_type_t
} ast_extern_variable_t;

typedef struct
{
    ast_extern_decl_type_t type;
    ast_t* value; // ast_extern_function_t or ast_extern_variable_t
} ast_extern_decl_t;

typedef enum
{
    AST_KIND_TYPE_STRUCT,
    AST_KIND_TYPE_ENUM,
    AST_KIND_TYPE_UNION,
} ast_kind_type_t;

typedef struct
{
    char* name;
    ast_t* parameters; // ast_parameters_t (type, name, and optional default value)
    ast_t* functions; // ast_function_declarations_t
} ast_kind_struct_t;

typedef struct
{
    char* name;
    char* alias; // (nullable)
    hashmap_t* values; // key is string, value is optional ast_expression_item_t
} ast_kind_enum_t;

typedef struct
{
    // TODO
} ast_kind_union_t;

typedef struct
{
    ast_kind_type_t kind_type;
    ast_t* value; // ast_kind_struct_t, or ast_kind_enum_t, or ast_kind_union_t
} ast_kind_decl_t;

typedef struct
{
    char* name;
    ast_t* type; // ast_type_t
    ast_t* value; // ast_expression_item_t (nullable)
} ast_statement_variable_decl_t;

typedef struct
{
    char* name;
    ast_t* parameters; // ast_parameters_t
    ast_t* block; // ast_block_t
} ast_function_decl_t;

typedef struct
{
    array_t* statements;
    size_t statement_count;
} ast_block_t;

typedef struct
{
    bool is_last; // true if this is the last statement in the if
    ast_t* condition; // ast_expression_item_t (nullable)
    ast_t* then_branch; // ast_block_t
    ast_t* else_branch; // ast_block_t (nullable)
} ast_statement_if_t;

typedef struct {
    ast_t* initializer; // ast_variable_decl_t or ast_statement_expression_t (nullable)
    ast_t* condition;   // ast_expression_item_t (nullable)
    ast_t* increment;   // ast_expression_item_t (nullable)
    ast_t* block;       // ast_block_t
} ast_statement_for_t;

typedef struct {
    char* loop_variable;   // name of the item variable
    ast_t* iterable;       // ast_expression_item_t (e.g., an array or map)
    ast_type_t* type;      // type of the loop variable (nullable)
    ast_t* block;          // ast_block_t
} ast_statement_foreach_t;

typedef struct
{
    ast_t* value; // ast_expression_item_t
} ast_statement_expression_t;

typedef struct
{
    ast_node_type_t type;

    union {
        ast_expression_literal_t literal;
        ast_expression_identifier_t identifier;
        ast_expression_binary_t binary;
        ast_expression_unary_t unary;
        ast_expression_index_t index;
        ast_expression_call_t call;
    } raw;

    ast_type_t* runtime_type; // (nullable)
} ast_expression_item_t;

typedef struct
{
    array_t* values; // array of ast_expression_item_t
    size_t value_count;
} ast_expressions_t;

typedef struct
{
    ast_t* value; // ast_expression_item_t
} ast_statement_return_t;

typedef struct
{
    ast_t* value; // ast_expression_item_t
} ast_statement_print_t;

typedef struct
{
    array_t* variable_declarations; // ast_variable_decl_t
    array_t* function_declarations; // ast_function_decl_t
    size_t variable_declaration_count;
    size_t function_declaration_count;
} ast_program_t;

typedef enum {
    AST_KIND_TYPE_INT,
    AST_KIND_TYPE_FLOAT,
    AST_KIND_TYPE_STRING,
    AST_KIND_TYPE_BOOL,
    AST_KIND_TYPE_ARRAY,
    AST_KIND_TYPE_MAP,
    AST_KIND_TYPE_SET,
    AST_KIND_TYPE_TUPLE,
    AST_KIND_TYPE_VOID,
    AST_KIND_TYPE_ANY,
    AST_KIND_TYPE_USER_DEFINED,
} ast_type_type_t;

struct ast_type_t
{
    ast_type_type_t kind;

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
typedef struct
{
    char* name;
    ast_t* type;
    ast_t* default_value; // ast_expression_item_t (nullable)
} ast_parameter_item_t;

typedef struct
{
    array_t* values; // ast_parameter_item_t
    size_t value_count;
} ast_parameters_t;

// attributes - a list of pairs of name and value
typedef struct
{
    char* name;
    ast_t* value;
} ast_attribute_item_t;

typedef struct
{
    array_t* values; // ast_attribute_item_t
    size_t value_count;
} ast_attributes_t;

// arguments - a list of values
typedef struct
{
    ast_t* value;
} ast_argument_item_t;

typedef struct
{
    array_t* values; // ast_argument_item_t
    size_t value_count;
} ast_arguments_t;

typedef struct ast_t {
    ast_base_t base;
    union {
        // program
        ast_program_t program_value;

        // package
        ast_package_t package_value;

        // import
        ast_import_t import_value;

        // extern
        ast_extern_decl_t extern_decl_value;
        ast_extern_variable_t extern_variable_value;
        ast_extern_function_t extern_function_value;

        // global kind
        ast_kind_decl_t kind_decl_value;
        ast_kind_struct_t struct_value;
        ast_kind_enum_t enum_value;
        ast_kind_union_t union_value;

        // global variable
        ast_variable_decl_t variable_decl_value;

        // global function
        ast_function_decl_t function_decl_value;

        // block
        ast_block_t block_value;

        // parameters - a list of pairs of name, type, and optional default value
        ast_parameter_item_t parameter_item_value;
        ast_parameters_t parameters_value;

        // attributes - a list of pairs of name and value
        ast_attribute_item_t attribute_item_value;
        ast_attributes_t attributes_value;

        // arguments - a list of values
        ast_argument_item_t argument_item_value;
        ast_arguments_t arguments_value;

        // expression(s)
        ast_expression_literal_t expression_literal_value;
        ast_expression_identifier_t expression_identifier_value;
        ast_expression_binary_t expression_binary_value;
        ast_expression_unary_t expression_unary_value;
        ast_expression_index_t expression_index_value;
        ast_expression_call_t expression_call_value;
        ast_expression_item_t expression_item_value;
        ast_expressions_t expressions_value;

        // statement(s)
        ast_statement_if_t statement_if_value;
        ast_statement_for_t statement_for_value;
        ast_statement_foreach_t statement_foreach_value;
        ast_statement_return_t statement_return_value;
        ast_statement_print_t statement_print_value;
        ast_statement_expression_t statement_expression_value;
    } raw;
} ast_t;

#endif // _STAGE_AST_TYPE_H_
