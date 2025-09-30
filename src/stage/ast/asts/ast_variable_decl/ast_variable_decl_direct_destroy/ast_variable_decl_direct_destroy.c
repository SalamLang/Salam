#include <stage/ast/asts/ast_variable_decl/ast_variable_decl_direct_destroy/ast_variable_decl_direct_destroy.h>

void ast_variable_decl_direct_destroy(ast_variable_decl_t variable_decl) {
    DEBUG_ME;
    if (variable_decl.name != NULL) {
        memory_destroy(variable_decl.name);
    }

    if (variable_decl.type != NULL) {
        ast_destroy(variable_decl.type);
    }

    if (variable_decl.value != NULL) {
        ast_destroy(variable_decl.value);
    }
}
