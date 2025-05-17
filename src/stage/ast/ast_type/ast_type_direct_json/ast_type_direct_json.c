#include <stage/ast/ast_type_direct_json/ast_type_direct_json.h>

char* ast_type_direct_json(ast_type_t type)
{
    buffer_t* temp = buffer_create(256);
    buffer_append_char(temp, '{');
    buffer_append_str(temp, '\"type\":null');
    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
