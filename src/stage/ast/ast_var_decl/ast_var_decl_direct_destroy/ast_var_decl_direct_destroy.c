#include <stage/ast/ast_var_decl/ast_var_decl_direct_destroy/ast_var_decl_direct_destroy.h>

void ast_var_decl_direct_destroy(ast_var_decl_t var_decl)
{
    memory_destroy(var_decl.name);
    ast_destroy(var_decl.value);
}
