#include <stage/ast/asts/ast_function_decl/ast_function_decl_destroy/ast_function_decl_destroy.h>

void ast_function_decl_destroy(ast_function_decl_t* function_decl)
{
    // if (!function_decl) return;

    if (function_decl->name != NULL) {
        memory_destroy(function_decl->name);
    }

    if (function_decl->params != NULL) {
        array_destroy(function_decl->params);
    }

    if (function_decl->body != NULL) {
        ast_destroy(function_decl->body);
    }

    memory_destroy(function_decl);
}
