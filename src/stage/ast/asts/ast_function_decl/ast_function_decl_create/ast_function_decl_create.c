#include <stage/ast/asts/ast_function_decl/ast_function_decl_create/ast_function_decl_create.h>

ast_t* ast_function_decl_create(const char* name, array_t* params, size_t param_count, ast_t* body)
{
    ast_t* ast = ast_alloc(AST_TYPE_FUNCTION_DECL);
    ast->raw.function_decl_value.name = string_duplicate(name);
    ast->raw.function_decl_value.params = params;
    ast->raw.function_decl_value.param_count = param_count;
    ast->raw.function_decl_value.body = body;
    return ast;
}
