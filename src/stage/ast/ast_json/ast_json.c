#include <stage/ast/ast_json/ast_json.h>

char* ast_json(ast_t* ast)
{
    buffer_t* temp = buffer_create(256);

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
