#include <stage/ast/asts/ast_variable_decl/ast_variable_decl_json/ast_variable_decl_json.h>

char* ast_variable_decl_json(ast_variable_decl_t* variable_decl)
{
    buffer_t* temp = buffer_create(24);

    // TODO

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
