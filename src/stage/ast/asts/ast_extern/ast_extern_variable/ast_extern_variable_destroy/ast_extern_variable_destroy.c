#include <stage/ast/asts/ast_extern/ast_extern_variable/ast_extern_variable_destroy/ast_extern_variable_destroy.h>

void ast_extern_variable_destroy(ast_extern_variable_t* extern_variable)
{
    if (!extern_variable) return NULL;

    if (extern_variable->name != NULL) {
        memory_destroy(extern_variable->name);
    }

    if (extern_variable->alias != NULL) {
        memory_destroy(extern_variable->alias);
    }

    if (extern_variable->type != NULL) {
        ast_destroy(extern_variable->type);
    }

    memory_destroy(extern_variable);
}
