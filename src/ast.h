#ifndef _AST_H_
#define _AST_H_

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
} ast_node_type_t;

struct ast_node_t;


typedef enum {
    AST_NODE_BLOCK_TYPE_LAYOUT,
    AST_NODE_BLOCK_TYPE_FUNCTION,
    AST_NODE_BLOCK_TYPE_ERROR,
} ast_node_block_type_t;

typedef struct {
    ast_node_block_type_t type;
    array_node_t* children;
} ast_node_block_t;

typedef struct ast_node_import_t {
    array_t* path;
} ast_node_import_t;

typedef struct ast_node_function_t {
    char* name;
    array_t* parameters;
    ast_node_block_t* block;
} ast_node_function_t;

typedef struct ast_node_layout_block_t {
    hashmap_array_t* attributes;
    hashmap_array_t* styles;
    array_node_layout_t* children;
} ast_node_layout_block_t;

typedef struct ast_node_layout_t {
    ast_node_layout_block_t* block;
} ast_node_layout_t;

typedef enum {
    AST_NODE_LAYOUT_NODE_TYPE_PARAGRAPH,
    AST_NODE_LAYOUT_NODE_TYPE_BUTTON,
    AST_NODE_LAYOUT_NODE_TYPE_FORM,
    AST_NODE_LAYOUT_NODE_TYPE_DIV,
} ast_node_layout_node_type_t;

typedef struct ast_node_layout_node_t {
    char* tag;
    ast_node_layout_node_type_t type;
    ast_node_layout_block_t* block;
} ast_node_layout_node_t;

typedef union {
    ast_node_block_t* block;
    ast_node_import_t* import;
    ast_node_function_t* function;
    ast_node_layout_t* layout;
    ast_node_layout_block_t* layout_block;
    ast_node_layout_node_t* layout_node;
} ast_node_union_t;

typedef struct ast_node_t {
    ast_node_type_t type;
    location_t location;

    void (*free)(void* node);
    void (*print)(void* node);

    ast_node_union_t data;
} ast_node_t;

typedef struct {
    array_t* layout;
} ast_t;

/**
 * 
 * @function ast_node_create
 * @brief Create a new AST node
 * @param {ast_node_type_t} type - Type of the AST node
 * @param {location_t} location - Location of the AST node
 * @returns {ast_node_t*} - Pointer to the created AST node
 * 
 */
ast_node_t* ast_node_create(ast_node_type_t type, location_t location);

/**
 * 
 * @function ast_node_print
 * @brief Print the AST node
 * @param {ast_node_t*} node - AST node
 * @returns {void}
 * 
 */
void ast_node_print(ast_node_t* node);

/**
 * 
 * @function ast_node_free
 * @brief Free the AST node
 * @param {ast_node_t*} node - AST node
 * @returns {void}
 * 
 */
void ast_node_free(ast_node_t* node);

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
 * @param {ast_t*} ast - AST
 * @returns {void}
 * 
 */
void ast_debug(ast_t* ast);

/**
 * 
 * @function ast_free
 * @brief Free the AST
 * @param {ast_t*} ast - AST
 * @returns {void}
 * 
 */
void ast_free(ast_t* ast);

#endif
