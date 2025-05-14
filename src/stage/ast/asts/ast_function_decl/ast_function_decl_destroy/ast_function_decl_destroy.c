#include <stage/ast/asts/ast_function_decl/ast_function_decl_destroy/ast_function_decl_destroy.h>

void ast_function_decl_destroy(ast_function_decl_t* function_decl)
{
    // if (!function_decl) return;

    memory_destroy(function_decl->name);
    array_destroy(function_decl->params);
    memory_destroy(function_decl->params);
    ast_destroy(function_decl->body);
    memory_destroy(function_decl);
}
