#include "ast.h"

/**
 * 
 * @function ast_node_create
 * @brief Create a new AST node
 * @param {ast_node_type_t} type - Type of the AST node
 * @param {location_t} location - Location of the AST node
 * @returns {ast_node_t*} - Pointer to the created AST node
 * 
 */
ast_node_t* ast_node_create(ast_node_type_t type, location_t location)
{
    ast_node_t* node = memory_allocate(sizeof(ast_node_t));
    node->type = type;
    node->location = location;
    node->print = cast(void (*)(void*), ast_node_print);
    node->free = cast(void (*)(void*), ast_node_free);

    return node;
}

/**
 * 
 * @function ast_node_layout_attribute_create
 * @brief Create a new AST node layout attribute
 * @returns {ast_node_layout_block_t*} - Pointer to the created AST node layout block
 * 
 */
ast_node_layout_block_t* ast_node_layout_block_create()
{
    ast_node_layout_block_t* block = memory_allocate(sizeof(ast_node_layout_block_t));
    block->attributes = hashmap_create(16);
    block->styles = hashmap_create(16);
    block->children = array_create(sizeof(ast_node_layout_node_t*), 16);

    return block;
}

/**
 * 
 * @function ast_node_layout_attribute_create
 * @brief Create a new AST node layout attribute
 * @params {ast_node_layout_node_type_t} type - Type of the layout node
 * @returns {ast_node_layout_node_t*} - Pointer to the created AST node layout attribute
 * 
 */
ast_node_layout_node_t* ast_node_layout_node_create(ast_node_layout_node_type_t type)
{
    ast_node_layout_node_t* node = memory_allocate(sizeof(ast_node_layout_node_t));
    node->tag = NULL;
    node->type = type;
    node->block = ast_node_layout_block_create();

    return node;
}

/**
 * 
 * @function 
 */
ast_node_layout_t* ast_node_layout_create()
{
    ast_node_layout_t* node = memory_allocate(sizeof(ast_node_layout_t));
    node->block = ast_node_layout_block_create();
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
        case AST_NODE_TYPE_FUNCTION:
            printf("Function\n");
            break;
        case AST_NODE_TYPE_FUNCTION_NODE:
            printf("Function Node\n");
            break;
        case AST_NODE_TYPE_FUNCTION_ERROR:
            printf("Function Error\n");
            break;
        case AST_NODE_TYPE_BLOCK:
            printf("Block\n");
            break;
        case AST_NODE_TYPE_NODE:
            printf("Node\n");
            break;
        case AST_NODE_TYPE_IMPORT:
            printf("Import\n");
            break;
        case AST_NODE_TYPE_LAYOUT:
            printf("Layout\n");
            break;
        case AST_NODE_TYPE_LAYOUT_BLOCK:
            printf("Layout Block\n");
            break;
        case AST_NODE_TYPE_LAYOUT_NODE:
            printf("Layout Node\n");
            break;
        case AST_NODE_TYPE_LAYOUT_ATTRIBUTE:
            printf("Layout Attribute\n");
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

    printf("AST Layout\n");
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
