#include <stage/ast/ast_alloc/ast_alloc.h>

ast_t* ast_alloc(ast_node_type_t type)
{
    ast_t* ast = memory_calloc(1, sizeof(ast_t));
    ast->base.type = type;
    return ast;
}
