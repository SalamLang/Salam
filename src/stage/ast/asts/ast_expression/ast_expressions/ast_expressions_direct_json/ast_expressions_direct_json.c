#include "ast_expressions_direct_json.h"

char* ast_expressions_direct_json(ast_expressions_t expressions)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // value_count
    buffer_append_str(temp, "\"value_count\": ");
    buffer_append_str(temp, convert_size2string(expressions.value_count));

    // values
    buffer_append_str(temp, "\"values\": ");
    if (expressions.values == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        buffer_append_char(temp, '[');
        for (size_t i = 0; i < expressions.value_count; i++)
        {
            if (i > 0)
            {
                buffer_append_str(temp, ", ");
            }
            ast_t* item = expressions.values->items[i];
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
