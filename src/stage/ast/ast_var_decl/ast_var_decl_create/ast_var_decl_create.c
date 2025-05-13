#include <stage/ast/ast_create_var_decl/ast_create_var_decl.h>

ast_t* ast_create_var_decl(const char* name, ast_t* value)
{
    ast_t* node = ast_alloc(AST_TYPE_VAR_DECL, ast_free);
    node->var_decl.name = string_duplicate(name);
    node->var_decl.value = value;
    return node;
}
