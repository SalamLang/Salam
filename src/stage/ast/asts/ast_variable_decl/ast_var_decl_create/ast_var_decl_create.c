#include <stage/ast/asts/ast_variable_decl/ast_variable_decl_create/ast_variable_decl_create.h>

ast_t* ast_variable_decl_create(const char* name, ast_t* value)
{
    ast_t* ast = ast_alloc(AST_TYPE_VAR_DECL);
    ast->raw.variable_decl_value.name = string_duplicate(name);
    ast->raw.variable_decl_value.value = value;
    return ast;
}
