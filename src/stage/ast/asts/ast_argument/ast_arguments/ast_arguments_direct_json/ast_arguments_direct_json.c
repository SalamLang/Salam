#include "ast_arguments_direct_json.h"

char* ast_arguments_direct_json(ast_arguments_t arguments)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // base
    buffer_append_str(temp, "\"base\": \"ast_arguments_t\",");

    // value_count
    buffer_append_str(temp, "\"value_count\":");
    buffer_append_str(temp, convert_size2string(arguments.value_count));

    // values
    buffer_append_str(temp, ",\"values\":");
    if (arguments.values == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        buffer_append_char(temp, '[');
        for (size_t i = 0; i < arguments.values->size; i++)
        {
            if (i > 0)
            {
                buffer_append_str(temp, ", ");
            }
            ast_t* item = arguments.values->items[i];
            char* buffer = ast_json(item);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }
        buffer_append_char(temp, ']');
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
