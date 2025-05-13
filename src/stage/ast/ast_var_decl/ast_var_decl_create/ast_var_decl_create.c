#include <stage/ast/ast_var_decl/ast_var_decl_create/ast_var_decl_create.h>

ast_t* ast_var_decl_create(const char* name, ast_t* value)
{
    ast_t* node = ast_alloc(AST_TYPE_VAR_DECL, ast_destroy);
    node->var_decl.name = string_duplicate(name);
    node->var_decl.value = value;
    return node;
}
