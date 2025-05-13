#include <stage/ast/ast_destroy/ast_destroy.h>

void ast_destroy(ast_t* ast)
{
    // if (!ast) return;

    ast->base.destroyer(ast);
    memory_destroy(ast);
}
