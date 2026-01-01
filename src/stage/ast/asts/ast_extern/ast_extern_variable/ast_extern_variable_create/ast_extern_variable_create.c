#include <stage/ast/asts/ast_extern/ast_extern_variable/ast_extern_variable_create/ast_extern_variable_create.h>

ast_t* ast_extern_variable_create(char* name, char* alias, ast_t* type) {
    ast_t* ast = ast_alloc(AST_TYPE_EXTERN_VARIABLE);
    ast->raw.extern_variable_value.name = name;
    ast->raw.extern_variable_value.alias = alias;
    ast->raw.extern_variable_value.type = type;
    return ast;
}
