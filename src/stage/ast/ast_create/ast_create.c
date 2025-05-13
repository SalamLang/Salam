#include <stage/ast/ast_create/ast_create.h>

ast_t* ast_create(ast_node_type_t type, ast_destroy_t destroy)
{
    ast_t* ast = memory_allocation(sizeof(ast_t));
    ast->base = (ast_base_t) {
        .type = type,
        .destroy = destroy,
    };
    return ast;
}
