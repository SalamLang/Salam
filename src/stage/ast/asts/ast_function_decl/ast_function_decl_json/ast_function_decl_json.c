#include <stage/ast/asts/ast_function_decl/ast_function_decl_json/ast_function_decl_json.h>

char* ast_function_decl_json(ast_function_decl_t* function_decl)
{
    buffer_t* temp = buffer_create(24);

    // TODO

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
