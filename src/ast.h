/**
 * ---------------------------------------------------------------------------
 * Salam Programming Language
 * ---------------------------------------------------------------------------
 * 
 * Welcome to the Salam Programming Language! Salam is a modern, efficient, 
 * and developer-friendly language created to inspire and empower programmers 
 * of all ages. With its unique features and Persian roots, Salam aims to make 
 * programming more accessible and enjoyable for everyone.
 * 
 * This file is part of the core implementation of Salam, including its runtime 
 * and compiler components. By contributing to or using this codebase, you are 
 * part of a growing community dedicated to innovation and inclusivity in 
 * programming.
 * 
 * Explore Salam further:
 * - Website: https://www.salamlang.ir/
 * - GitHub: https://github.com/SalamLang/Salam
 * 
 * Thank you for being part of this journey!
 * ---------------------------------------------------------------------------
 */

#ifndef _AST_H_
#define _AST_H_

#include <stdio.h>
#include <string.h>

#include "array.h"
#include "array_custom.h"
#include "base.h"
#include "hashmap.h"
#include "lexer.h"
#include "memory.h"
// #include "hashmap_custom.h"
#include "string_buffer.h"

typedef enum {
#undef ADD_TYPE
#define ADD_TYPE(TYPE, NAME, NAME_LOWER) TYPE,

#include "ast_type.h"
} ast_type_t;

typedef enum {
#undef ADD_BLOCK_TYPE
#define ADD_BLOCK_TYPE(TYPE, NAME, NAME_LOWER) TYPE,

#include "ast_block_type.h"
} ast_block_type_t;

struct ast_t;

typedef struct {
    ast_block_type_t type;
    ast_type_t parent_type;

    array_node_t *children;

    void (*destroy)(void *node);
    void (*print)(void *node);
} ast_block_t;

typedef struct ast_import_t {
    array_t *path;

    void (*destroy)(void *node);
    void (*print)(void *node);
} ast_import_t;

typedef enum ast_value_kind_t {
#undef ADD_VALUE_KIND
#define ADD_VALUE_KIND(TYPE, NAME, NAME_LOWER) TYPE,

#include "ast_value_kind.h"
} ast_value_kind_t;

typedef struct ast_value_type_t {
    ast_value_kind_t kind;
    location_t location;

    void (*destroy)(void *node);
    void (*print)(void *node);
} ast_value_type_t;

#include "ast_layout.h"
#include "ast_layout_style.h"

typedef struct ast_function_t {
    char *name;
    array_function_parameter_t *parameters;
    ast_block_t *block;
    ast_value_type_t *return_type;

    void (*destroy)(void *node);
    void (*print)(void *node);
} ast_function_t;

typedef struct ast_value_t {
    ast_value_type_t *type;
    union data {
        int int_value;
        float float_value;
        char char_value;
        bool bool_value;
        char *string_value;
    } data;

    void (*destroy)(void *node);
    void (*print)(void *node);
    char *(*get_data)(void *node);
} ast_value_t;

typedef struct ast_if_t {
    ast_value_t *condition;
    ast_block_t *block;
    array_block_t *else_blocks;  // NULLABLE for else if blocks

    void (*destroy)(void *node);
    void (*print)(void *node);
} ast_if_t;

typedef struct ast_return_t {
    array_value_t *values;

    void (*destroy)(void *node);
    void (*print)(void *node);
} ast_return_t;

typedef struct ast_print_t {
    array_value_t *values;

    void (*destroy)(void *node);
    void (*print)(void *node);
} ast_print_t;

typedef struct ast_function_parameter_t {
    char *name;
    ast_value_type_t *type;

    void (*destroy)(void *node);
    void (*print)(void *node);
} ast_function_parameter_t;

typedef union ast_union_t {
    ast_block_t *block;
    ast_import_t *import;
    ast_function_t *function;
    ast_if_t *ifclause;
    ast_return_t *returns;
    ast_print_t *print;

    struct ast_layout_t *layout;
} ast_union_t;

typedef struct ast_node_t {
    ast_type_t type;
    location_t location;
    ast_union_t data;

    void (*destroy)(void *node);
    void (*print)(void *node);
} ast_node_t;

typedef struct ast_t {
    struct ast_layout_t *layout;
    array_function_t *functions;

    void (*destroy)(void *node);
    void (*print)(void *node);
} ast_t;

/**
 *
 * @function ast_node_destroy_notall
 * @brief Free the AST node (not all)
 * @params {ast_node_t*} value - AST node
 * @returns {void}
 *
 */
void ast_node_destroy_notall(ast_node_t *value);

/**
 *
 * @function ast_node_create
 * @brief Create a new AST node
 * @params {ast_type_t} type - Type of the AST node
 * @params {location_t} location - Location of the AST node
 * @returns {ast_node_t*} - Pointer to the created AST node
 *
 */
ast_node_t *ast_node_create(ast_type_t type, location_t location);

/**
 *
 * @function ast_node_print
 * @brief Print the AST node
 * @params {ast_node_t*} node - AST node
 * @returns {void}
 *
 */
void ast_node_print(ast_node_t *node);

/**
 *
 * @function ast_node_destroy
 * @brief Free the AST node
 * @params {ast_node_t*} value - AST node
 * @returns {void}
 *
 */
void ast_node_destroy(ast_node_t *value);

/**
 *
 * @function ast_create
 * @brief Create a new AST
 * @returns {ast_t*} - Pointer to the created AST
 *
 */
ast_t *ast_create();

/**
 *
 * @function ast_debug
 * @brief Debug the AST
 * @params {ast_t*} ast - AST
 * @returns {void}
 *
 */
void ast_debug(ast_t *ast);

/**
 *
 * @function ast_destroy
 * @brief Free the AST
 * @params {ast_t*} ast - AST
 * @returns {void}
 *
 */
void ast_destroy(ast_t *ast);

/**
 *
 * @function ast_block_create
 * @brief Create a new AST block node
 * @params {ast_block_type_t} type - Block type
 * @params {ast_type_t} parent_type - Parent type
 * @returns {ast_block_t*} - AST block node
 *
 */
ast_block_t *ast_block_create(ast_block_type_t type, ast_type_t parent_type);

/**
 *
 * @function ast_function_create
 * @brief Create a new AST node function
 * @params {char*} name - Name of the function
 * @returns {ast_function_t*} - Pointer to the created AST node function
 *
 */
ast_function_t *ast_function_create(char *name);

/**
 *
 * @function ast_function_parameter_destroy
 * @brief Free the AST function parameter
 * @params {ast_function_parameter_t*} value - AST function parameter
 * @returns {void}
 *
 */
void ast_function_parameter_destroy(ast_function_parameter_t *value);

/**
 *
 * @function ast_function_parameter_print
 * @brief Print the AST function parameter
 * @params {ast_function_parameter_t*} value - AST function parameter
 * @returns {void}
 *
 */
void ast_function_parameter_print(ast_function_parameter_t *value);

/**
 *
 * @function ast_function_print
 * @brief Print the AST function
 * @params {ast_function_t*} value - AST function
 * @returns {void}
 *
 */
void ast_function_print(ast_function_t *value);

/**
 *
 * @function ast_function_destroy
 * @brief Free the AST function
 * @params {ast_function_t*} value - AST function
 * @returns {void}
 *
 */
void ast_function_destroy(ast_function_t *value);

/**
 *
 * @function ast_value_type_create
 * @brief Create a new AST value type
 * @params {ast_value_kind_t} kind - Kind of the value type
 * @params {location_t} location - Location of the value type
 * @returns {ast_value_type_t*} - Pointer to the created AST value type
 *
 */
ast_value_type_t *ast_value_type_create(ast_value_kind_t kind,
                                        location_t location);

/**
 *
 * @function ast_value_type_print
 * @brief Print the AST value type
 * @params {ast_value_type_t*} type - AST value type
 * @returns {void}
 *
 */
void ast_value_type_print(ast_value_type_t *type);

/**
 *
 * @function ast_value_type_name
 * @brief Get the name of the AST value type
 * @params {ast_value_type_t*} type - AST value type
 * @returns {char*} - Name of the AST value type
 *
 */
char *ast_value_type_name(ast_value_type_t *type);

/**
 *
 * @function ast_value_type_destroy
 * @brief Free the AST value type
 * @params {ast_value_type_t*} type - AST value type
 * @returns {void}
 *
 */
void ast_value_type_destroy(ast_value_type_t *type);

/**
 *
 * @function ast_print
 * @brief Print the AST
 * @params {ast_t*} ast - AST
 * @returns {void}
 *
 */
void ast_print(ast_t *ast);

/**
 *
 * @function ast_block_print
 * @brief Print the AST block node
 * @params {ast_block_t*} block - AST block node
 * @returns {void}
 *
 */
void ast_block_print(ast_block_t *block);

/**
 *
 * @function ast_block_destroy
 * @brief Free the AST block node
 * @params {ast_block_t*} block - AST block node
 * @returns {void}
 *
 */
void ast_block_destroy(ast_block_t *block);

/**
 *
 * @function ast_if_create
 * @brief Create a new AST node if
 * @returns {ast_if_t*} - Pointer to the created AST node if
 *
 */
void ast_if_print(ast_if_t *node);

/**
 *
 * @function ast_elseif_create
 * @brief Create a new AST node else if
 * @params {ast_value_t*} condition - Condition of the else if
 * @returns {ast_if_t*} - Pointer to the created AST node else if
 *
 */
ast_if_t *ast_elseif_create(ast_value_t *condition);

/**
 *
 * @function ast_if_destroy
 * @brief Free the AST if node
 * @params {ast_if_t*} node - AST if node
 * @returns {void}
 *
 */
void ast_if_destroy(ast_if_t *node);

/**
 *
 * @function ast_if_create
 * @brief Create a new AST node if
 * @params {ast_value_t*} condition - Condition of the if
 * @returns {ast_if_t*} - Pointer to the created AST node if
 *
 */
ast_if_t *ast_if_create(ast_value_t *condition);

/**
 *
 * @function ast_value_create
 * @brief Create a new AST value
 * @params {ast_value_type_t*} type - Value type
 * @params {void*} data - Value data
 * @returns {ast_value_t*} - Pointer to the created AST value
 *
 */
ast_value_t *ast_value_create(ast_value_type_t *type, void *data);

/**
 *
 * @function ast_value_print
 * @brief Print the AST value
 * @params {ast_value_t*} value - AST Value
 * @returns {void}
 *
 */
void ast_value_print(ast_value_t *value);

/**
 *
 * @function ast_value_destroy
 * @brief Free the AST value
 * @params {ast_value_t*} value - AST Value
 * @returns {void}
 *
 */
void ast_value_destroy(ast_value_t *value);

/**
 *
 * @function ast_block_type_name
 * @brief Print the AST block type
 * @params {ast_block_type_t} type - AST block type
 * @returns {char*} - Name of the AST block type
 */
char *ast_block_type_name(ast_block_type_t type);

/**
 *
 * @function ast_else_create
 * @brief Create a new AST node else
 * @returns {ast_if_t*} - Pointer to the created AST node else
 *
 */
ast_if_t *ast_else_create();

/**
 *
 * @function ast_return_print
 * @brief Print the AST return node
 * @params {ast_return_t*} node - AST return node
 * @returns {void}
 */
void ast_return_print(ast_return_t *node);

/**
 *
 * @function ast_return_destroy
 * @brief Free the AST return node
 * @params {ast_return_t*} node - AST return node
 * @returns {void}
 *
 */
void ast_return_destroy(ast_return_t *node);

/**
 *
 * @function ast_return_create
 * @brief Create a new AST node return
 * @params {array_value_t*} values - Values of the return
 * @returns {ast_return_t*} - Pointer to the created AST node if
 *
 */
ast_return_t *ast_return_create(array_value_t *values);

/**
 *
 * @function ast_print_create
 * @brief Create a new AST node print
 * @params {array_value_t*} values - Values of the print
 * @returns {ast_print_t*} - Pointer to the created AST node if
 *
 */
ast_print_t *ast_print_create(array_value_t *values);

/**
 *
 * @function ast_print_destroy
 * @brief Free the AST print node
 * @params {ast_print_t*} node - AST print node
 * @returns {void}
 *
 */
void ast_print_destroy(ast_print_t *node);

/**
 *
 * @function ast_print_print
 * @brief Print the AST print node
 * @params {ast_print_t*} node - AST print node
 * @returns {void}
 *
 */
void ast_print_print(ast_print_t *node);

/**
 *
 * @function ast_value_print
 * @brief Print the AST value
 * @params {ast_value_t*} value - AST layout attribute value
 * @returns {void}
 *
 */
void ast_value_print(ast_value_t *value);

/**
 *
 * @function ast_value_destroy
 * @brief Free the AST layout attribute value
 * @params {ast_value_t*} value - AST layout attribute value
 * @returns {void}
 *
 */
void ast_value_destroy(ast_value_t *value);

/**
 *
 * @function ast_value_print
 * @brief Print the AST layout value
 * @params {ast_value_t*} value - AST Layout Value
 * @returns {void}
 *
 */
void ast_value_print(ast_value_t *value);

/**
 *
 * @function ast_value_data
 * @brief Get the string of the AST layout attribute value
 * @params {ast_value_t*} value - AST layout attribute value
 * @returns {char*} - String of the AST layout attribute value
 *
 */
char *ast_value_data(ast_value_t *value);

/**
 *
 * @function ast_value_copy
 * @brief Copy the AST layout attribute value
 * @params {ast_value_t*} value - AST Layout Attribute Value
 * @returns {ast_value_t*} - Copied AST Layout Attribute Value
 *
 */
ast_value_t *ast_value_copy(ast_value_t *value);

/**
 *
 * @function ast_value_type_copy
 * @brief Copy the AST layout attribute value type
 * @params {ast_value_type_t*} type - AST Layout Attribute Value Type
 * @returns {ast_value_type_t*} - Copied AST Layout Attribute Value Type
 *
 */
ast_value_type_t *ast_value_type_copy(ast_value_type_t *type);

#endif
