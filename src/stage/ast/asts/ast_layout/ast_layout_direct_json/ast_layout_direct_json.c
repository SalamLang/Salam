#include <src/strage/ast/asts/ast_layout/ast_layout_direct_json/ast_layout_direct_json.h>

char* ast_layout_direct_json(ast_layout_t ast_layout)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
