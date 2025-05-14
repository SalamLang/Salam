#include <stage/ast/asts/ast_var_decl/ast_var_decl_json/ast_var_decl_json.h>

char* ast_var_decl_json(ast_var_decl_t* var_decl)
{
    buffer_t* temp = buffer_create(24);

    // TODO

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
