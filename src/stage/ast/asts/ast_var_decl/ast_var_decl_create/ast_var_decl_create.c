#include <stage/ast/asts/ast_var_decl/ast_var_decl_create/ast_var_decl_create.h>

ast_t* ast_var_decl_create(const char* name, ast_t* value)
{
    ast_t* node = ast_alloc(AST_TYPE_VAR_DECL);
    node->raw.var_decl_value.name = string_duplicate(name);
    node->raw.var_decl_value.value = value;
    return node;
}
