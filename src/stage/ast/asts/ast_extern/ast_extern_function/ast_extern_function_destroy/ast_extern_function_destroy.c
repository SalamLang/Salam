#include <stage/ast/asts/ast_extern/ast_extern_function/ast_extern_function_destroy/ast_extern_function_destroy.h>

void ast_extern_function_destroy(ast_extern_function_t* extern_function)
{
    if (!extern_function) return;

    if (extern_function->name != NULL) {
        memory_destroy(extern_function->name);
    }

    if (extern_function->alias != NULL) {
        memory_destroy(extern_function->alias);
    }

    if (extern_function->type != NULL) {
        ast_destroy(extern_function->type);
    }

    if (extern_function->attributes != NULL) {
        ast_destroy(extern_function->attributes);
    }

    memory_destroy(extern_function);
}
