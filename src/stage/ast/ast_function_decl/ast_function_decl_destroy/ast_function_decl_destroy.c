#include <stage/ast/ast_function_decl/ast_function_decl_destroy/ast_function_decl_destroy.h>

void ast_function_decl_destroy(ast_function_decl_t* function_decl)
{
    // if (!function_decl) return;

    memory_destroy(function_decl->name);
    for (size_t i = 0; i < function_decl->param_count; ++i)
        memory_destroy(function_decl->params[i]);
    memory_destroy(function_decl->params);
    ast_destroy(function_decl->body);
    memory_destroy(function_decl);
}
