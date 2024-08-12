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
    AST_NODE_TYPE_NODE,
    AST_NODE_TYPE_FUNCTION,
    AST_NODE_TYPE_FUNCTION_NODE,
    AST_NODE_TYPE_FUNCTION_ERROR,
    AST_NODE_TYPE_LAYOUT,
    AST_NODE_TYPE_LAYOUT_ATTRIBUTE,
    AST_NODE_TYPE_LAYOUT_BLOCK,
    AST_NODE_TYPE_LAYOUT_NODE,
    AST_NODE_TYPE_ERROR,
} ast_node_type_t;

struct ast_node_t;

typedef struct {
    ast_node_type_t type;
    location_t location;

    union {
        struct ast_node_t* block;
        struct ast_node_t* function;
        struct ast_node_t* layout;
    };

    struct ast_node_t* block;
    array_node_t* children;
    array_node_t* layout_styles;
    array_node_t* layout_attributes;
    void (*free)(void* node);
    void (*print)(void* node);
} ast_node_t;

typedef struct {
    hashmap_array_t* attributes;
    hashmap_array_t* styles;
    array_node_layout_t* children;
} ast_node_layout_t;

typedef enum {
    AST_NODE_BLOCK_TYPE_LAYOUT,
    AST_NODE_BLOCK_TYPE_FUNCTION,
    AST_NODE_BLOCK_TYPE_ERROR,
} ast_node_block_type_t;

typedef struct {
    ast_node_block_type_t type;
    array_node_t* children;
} ast_node_block_t;

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
