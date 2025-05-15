#include <stage/ast/asts/ast_extern/ast_extern_function/ast_extern_function_create/ast_extern_function_create.h>

ast_t* ast_extern_function_create(char* name, char* alias, ast_t* type, ast_t* attributes)
{
    ast_t* ast = ast_alloc(AST_TYPE_EXTERN_FUNCTION);
    ast->raw.extern_function_value.name = name;
    ast->raw.extern_function_value.alias = alias;
    ast->raw.extern_function_value.type = type;
    ast->raw.extern_function_value.attributes = attributes;
    return ast;
}
