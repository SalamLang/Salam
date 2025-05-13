#include <stage/ast/ast_var_decl_destroy/ast_var_decl_destroy.h>

void ast_var_decl_destroy(ast_var_decl_t* var_decl)
{
    // if (!var_decl) return;

    memory_destroy(var_decl->name);
    ast_destroy(var_decl->value);
    memory_destroy(var_decl);
}
