#include "ast_kind_union_direct_json.h"

char* ast_kind_union_direct_json(ast_kind_union_t kind_union)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(256);
    buffer_append_char(temp, '{');

    // TODO
    buffer_append_str(temp, "TODO");

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
