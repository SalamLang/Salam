#include "ast.h"

/**
 * 
 * @function ast_node_create
 * @brief Create a new AST node
 * @param {ast_node_type_t} type - Type of the AST node
 * @returns {ast_node_t*} - Pointer to the created AST node
 * 
 */
ast_node_t* ast_node_create(ast_node_type_t type)
{
    ast_node_t* node = memory_allocate(sizeof(ast_node_t));
    node->type = type;
    node->location = (location_t){0, 0, 0, 0, 0, 0};
    node->print = cast(void (*)(void*), ast_node_print);
    node->free = cast(void (*)(void*), ast_node_free);
    return node;
}

/**
 * 
 * @function ast_node_print
 * @brief Print the AST node
 * @param {ast_node_t*} node - AST node
 * @returns {void}
 * 
 */
void ast_node_print(ast_node_t* node)
{
    switch (node->type) {
        case AST_NODE_TYPE_PROGRAM:
            printf("Program\n");
            break;
        case AST_NODE_TYPE_FUNCTION:
            printf("Function\n");
            break;
        case AST_NODE_TYPE_LAYOUT:
            printf("Layout\n");
            break;
        case AST_NODE_TYPE_ERROR:
            printf("Error\n");
            break;
    }
}

/**
 * 
 * @function ast_node_free
 * @brief Free the AST node
 * @param {ast_node_t*} node - AST node
 * @returns {void}
 * 
 */
void ast_node_free(ast_node_t* node)
{
    memory_free(node);
}

/**
 * 
 * @function ast_create
 * @brief Create a new AST
 * @returns {ast_t*} - Pointer to the created AST
 * 
 */
ast_t* ast_create()
{
    ast_t* ast = memory_allocate(sizeof(ast_t));
    ast->layout = array_create(sizeof(ast_node_t*), 16);
    return ast;
}

/**
 * 
 * @function ast_debug
 * @brief Debug the AST
 * @param {ast_t*} ast - AST
 * @returns {void}
 * 
 */
void ast_debug(ast_t* ast)
{
    printf("============= START AST DEBUG =============\n");

    printf("AST\n");
    array_node_print(ast->layout);

    printf("============= END AST DEBUG =============\n");
}

/**
 * 
 * @function ast_free
 * @brief Free the AST
 * @param {ast_t*} ast - AST
 * @returns {void}
 * 
 */
void ast_free(ast_t* ast)
{
    array_node_free(ast->layout);
    memory_free(ast);
}
