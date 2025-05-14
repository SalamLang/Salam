#include <stage/ast/asts/ast_extern/ast_extern_function/ast_extern_function_direct_destroy/ast_extern_function_direct_destroy.h>

void ast_extern_function_direct_destroy(ast_extern_function_t extern_function)
{
    if (extern_function.value != NULL) {
        ast_destroy(extern_function.value);
    }
}
