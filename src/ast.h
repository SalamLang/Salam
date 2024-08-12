#ifndef _AST_H_
#define _AST_H_

#include <stdio.h>
#include <string.h>

#include "base.h"
#include "memory.h"
#include "array.h"
#include "lexer.h"
#include "hashmap.h"

typedef enum {
    AST_NODE_TYPE_BLOCK,
    AST_NODE_TYPE_IMPORT,
    AST_NODE_TYPE_FUNCTION,
    AST_NODE_TYPE_FUNCTION_ERROR,
    AST_NODE_TYPE_LAYOUT,
    AST_NODE_TYPE_LAYOUT_BLOCK,
    AST_NODE_TYPE_LAYOUT_NODE,
    AST_NODE_TYPE_ERROR,
} ast_type_t;

typedef enum {
    AST_NODE_BLOCK_TYPE_LAYOUT,
    AST_NODE_BLOCK_TYPE_FUNCTION,
    AST_NODE_BLOCK_TYPE_ERROR,
} ast_block_type_t;

typedef enum {
    AST_NODE_LAYOUT_NODE_TYPE_PARAGRAPH,
    AST_NODE_LAYOUT_NODE_TYPE_BUTTON,
    AST_NODE_LAYOUT_NODE_TYPE_FORM,
    AST_NODE_LAYOUT_NODE_TYPE_DIV,
} ast_layout_node_type_t;

struct ast_t;

typedef struct {
    ast_block_type_t type;
    array_node_t* children;
    ast_type_t parent_type;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_block_t;

typedef struct ast_import_t {
    array_t* path;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_import_t;

typedef struct ast_function_t {
    char* name;
    array_t* parameters;
    ast_block_t* block;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_function_t;

typedef struct ast_layout_block_t {
    ast_type_t parent_type;
    struct hashmap_t* attributes;
    struct hashmap_t* styles;
    array_node_layout_t* children;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_layout_block_t;

typedef struct ast_layout_attribute_t {
    char* key;
    array_t* values;
    bool isStyle;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_layout_attribute_t;

typedef struct ast_layout_t {
    ast_layout_block_t* block;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_layout_t;

typedef struct ast_layout_node_t {
    char* tag;
    ast_layout_node_type_t type;
    ast_layout_block_t* block;

    void (*destroy)(void* node);
    void (*print)(void* node);
} ast_layout_node_t;

typedef union {
    ast_block_t* block;
    ast_import_t* import;
    ast_function_t* function;
    ast_layout_t* layout;
    ast_layout_block_t* layout_block;
    ast_layout_node_t* layout_node;
} ast_union_t;

typedef struct ast_t {
    ast_type_t type;
    location_t location;

    void (*destroy)(void* node);
    void (*print)(void* node);

    ast_union_t data;
} ast_node_t;

typedef struct {
    ast_layout_t* layout;
} ast_t;

/**
 * 
 * @function ast_node_create
 * @brief Create a new AST node
 * @params {ast_type_t} type - Type of the AST node
 * @params {location_t} location - Location of the AST node
 * @returns {ast_node_t*} - Pointer to the created AST node
 * 
 */
ast_node_t* ast_node_create(ast_type_t type, location_t location);

/**
 * 
 * @function ast_layout_node_print
 * @beief Print the AST layout node
 * @paramss {ast_layout_node_t*} value - AST layout node
 * @returns {void}
 * 
 */
void ast_layout_node_print(ast_layout_node_t* value);

/**
 * 
 * @function ast_layout_node_destroy
 * @brief Free the AST node layout attribute
 * @paramss {ast_layout_node_t*} value - AST layout node
 * @returns {void}
 * 
 */
void ast_layout_node_destroy(ast_layout_node_t* value);

/**
 * 
 * @function ast_layout_create
 * @brief Create a new AST node layout attribute
 * @returns {ast_layout_attribute_t*} - Pointer to the created AST node layout attribute
 * 
 */
ast_layout_t* ast_layout_create();

/**
 * 
 * @function ast_layout_destroy
 * @brief Create a new AST node layout attribute
 * @paramss {ast_layout_t*} value - AST layout node
 * @returns {void}
 * 
 */
void ast_layout_destroy(ast_layout_t* value);

/**
 * 
 * @function ast_node_print
 * @brief Print the AST node
 * @params {ast_node_t*} node - AST node
 * @returns {void}
 * 
 */
void ast_node_print(ast_node_t* node);

/**
 * 
 * @function ast_node_destroy
 * @brief Free the AST node
 * @params {ast_node_t*} value - AST node
 * @returns {void}
 * 
 */
void ast_node_destroy(ast_node_t* value);

/**
 * 
 * @function ast_create
 * @brief Create a new AST
 * @returns {ast_t*} - Pointer to the created AST
 * 
 */
ast_t* ast_create();

/**
 * 
 * @function ast_debug
 * @brief Debug the AST
 * @params {ast_t*} ast - AST
 * @returns {void}
 * 
 */
void ast_debug(ast_t* ast);

/**
 * 
 * @function ast_destroy
 * @brief Free the AST
 * @params {ast_t*} ast - AST
 * @returns {void}
 * 
 */
void ast_destroy(ast_t* ast);

/**
 * 
 * @function ast_layout_block_create
 * @brief Create a new AST node layout attribute
 * @returns {ast_layout_block_t*} - Pointer to the created AST node layout block
 * 
 */
ast_layout_block_t* ast_layout_block_create();

/**
 * 
 * @function ast_layout_block_print
 * @brief Free the AST node layout block
 * @paramss {ast_layout_block_t*} value - AST layout block
 * @returns {void}
 * 
 */
void ast_layout_block_print(ast_layout_block_t* value);

/**
 * 
 * @function ast_layout_block_destroy
 * @brief Free the AST node layout block
 * @paramss {ast_layout_block_t*} value - AST layout block
 * @returns {void}
 * 
 */
void ast_layout_block_destroy(ast_layout_block_t* value);

/**
 * 
 * @function ast_layout_node_create
 * @brief Create a new AST node layout attribute
 * @paramss {ast_layout_node_type_t} type - Type of the layout node
 * @returns {ast_layout_node_t*} - Pointer to the created AST node layout attribute
 * 
 */
ast_layout_node_t* ast_layout_node_create(ast_layout_node_type_t type);

/**
 * 
 * @function ast_layout_attribute_destroy
 * @brief Create a new AST node layout attribute
 * @paramss {ast_layout_attribute_t*} value - AST layout attribute
 * @returns {void}
 * 
 */
void ast_layout_attribute_destroy(ast_layout_attribute_t* value);

/**
 * 
 * @function ast_layout_attribute_print
 * @brief Print the AST layout attribute
 * @paramss {ast_layout_attribute_t*} value - AST layout attribute
 * @returns {void}
 * 
 */
void ast_layout_attribute_print(ast_layout_attribute_t* value);

/**
 * 
 * @function ast_layout_print
 * @brief Print the AST layout
 * @params {ast_layout_t*} value - AST layout
 * @returns {void}
 * 
 */
void ast_layout_print(ast_layout_t* value);

/**
 * 
 * @function ast_layout_attribute_create
 * @brief Create a new AST node layout attribute
 * @paramss {const char*} key - Key of the attribute
 * @paramss {array_t*} values - Values of the attribute
 * @returns {ast_layout_attribute_t*} - Pointer to the created AST node layout attribute
 * 
 */
ast_layout_attribute_t* ast_layout_attribute_create(char* key, array_t* values);

#endif
