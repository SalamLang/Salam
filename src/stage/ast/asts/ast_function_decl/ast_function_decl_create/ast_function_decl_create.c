#include <stage/ast/asts/ast_function_decl/ast_function_decl_create/ast_function_decl_create.h>

ast_t* ast_function_decl_create(const char* name, ast_t* parameters, ast_t* block)
{
    ast_t* ast = ast_alloc(AST_TYPE_FUNCTION_DECL);
    ast->raw.function_decl_value.return_type = NULL; // TODO: set the return type
    ast->raw.function_decl_value.name = string_duplicate(name);
    ast->raw.function_decl_value.parameters = parameters;
    ast->raw.function_decl_value.block = block;
    return ast;
}
