#include <stage/ast/asts/ast_extern/ast_extern_function/ast_extern_function_create/ast_extern_function_create.h>

ast_t* ast_extern_function_create(ast_extern_function_type_t type, ast_t* value)
{
    ast_t* ast = ast_alloc(AST_TYPE_EXTERN_DECL);
    ast->raw.extern_function_value.type = type;
    ast->raw.extern_function_value.value = value;
    return ast;
}
