#include <stage/ast/asts/ast_extern/ast_extern_decl/ast_extern_decl_create/ast_extern_decl_create.h>

ast_t* ast_extern_decl_create(ast_extern_decl_type_t type, ast_t* value)
{
    ast_t* ast = ast_alloc(AST_TYPE_EXTERN_DECL);
    ast->raw.extern_decl_value.type = type;
    ast->raw.extern_decl_value.value = value; // ast_extern_function_t or ast_extern_variable_t
    return ast;
}
