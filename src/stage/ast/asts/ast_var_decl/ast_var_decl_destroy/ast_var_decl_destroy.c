#include <stage/ast/asts/ast_var_decl/ast_var_decl_destroy/ast_var_decl_destroy.h>

void ast_var_decl_destroy(ast_var_decl_t* var_decl)
{
    DEBUG_ME;
    // if (!var_decl) return;

    if (var_decl->name != NULL) {
        memory_destroy(var_decl->name);
    }

    if (var_decl->value != NULL) {
        ast_destroy(var_decl->value);
    }

    memory_destroy(var_decl);
}
