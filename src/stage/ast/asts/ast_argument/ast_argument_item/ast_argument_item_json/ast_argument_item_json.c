#include "ast_argument_item_json.h"

char* ast_argument_item_json(ast_argument_item_t* argument_item)
{
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // value
    buffer_append_str(temp, "\"value\":");
    if (argument_item->value == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        char* value = ast_json(argument_item->value);
        buffer_append_str(temp, value);
        memory_destroy(value);
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
