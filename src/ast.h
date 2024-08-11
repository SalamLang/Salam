#ifndef _AST_H_
#define _AST_H_

#include "base.h"
#include "memory.h"
#include "array.h"
#include "lexer.h"

typedef enum {
    AST_NODE_TYPE_PROGRAM,
    AST_NODE_TYPE_FUNCTION,
    AST_NODE_TYPE_LAYOUT,
    AST_NODE_TYPE_ERROR,
} ast_node_type_t;

typedef struct {
    ast_node_type_t type;
    location_t location;

    void (*free)(void* node);
    void (*print)(void* node);
} ast_node_t;

typedef struct {
    array_t* layout;
} ast_t;

/**
 * 
 * @function ast_node_create
 * @brief Create a new AST node
 * @param {ast_node_type_t} type - Type of the AST node
 * @returns {ast_node_t*} - Pointer to the created AST node
 * 
 */
ast_node_t* ast_node_create(ast_node_type_t type);

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