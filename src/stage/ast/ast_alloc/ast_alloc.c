#include <stage/ast/ast_alloc/ast_alloc.h>

ast_t* ast_alloc(ast_node_type_t type, ast_destroy_t free_func)
{
    ast_t* node = memory_calloc(1, sizeof(ast_t));
    node->base.type = type;
    node->base.destroy = free_func;
    return node;
}
