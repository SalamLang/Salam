#include "ast_attributes_json.h"

char* ast_attributes_json(ast_attributes_t* attributes)
{
    buffer_t* temp = buffer_create(24);

    if (attributes == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        buffer_append_char(temp, '{');

        // value_count
        buffer_append_str(temp, "\"value_count\": ");
        buffer_append_str(temp, convert_size2string(attributes->value_count));

        // values
        buffer_append_str(temp, ",\"values\": ");
        if (attributes->values == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            buffer_append_char(temp, '[');
            for (size_t i = 0; i < attributes->values->size; i++)
            {
                if (i > 0)
                {
                    buffer_append_str(temp, ", ");
                }
                ast_t* item = attributes->values->items[i];
                char* buffer = ast_json(item);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
            }
            buffer_append_char(temp, ']');
        }

        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
