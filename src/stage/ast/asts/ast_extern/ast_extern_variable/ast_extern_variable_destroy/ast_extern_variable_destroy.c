#include <stage/ast/asts/ast_extern/ast_extern_function/ast_extern_function_destroy/ast_extern_function_destroy.h>

void ast_extern_function_destroy(ast_extern_function_t* extern_function)
{
    if (!extern_function) return NULL;

    if (extern_function->value != NULL) {
        ast_destroy(extern_function->value);
    }

    memory_destroy(extern_function);
}
