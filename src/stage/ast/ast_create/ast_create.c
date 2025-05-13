#include <stage/ast/ast_create/ast_create.h>

ast_t* ast_create()
{
    ast_t* ast = memory_allocation(sizeof(ast_t));
    return ast;
}
