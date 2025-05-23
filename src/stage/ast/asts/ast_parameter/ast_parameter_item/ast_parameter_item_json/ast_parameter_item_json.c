#include "ast_parameter_item_json.h"

char* ast_parameter_item_json(ast_parameter_item_t* parameter_item)
{
    buffer_t* temp = buffer_create(24);

    if (parameter_item == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        buffer_append_char(temp, '{');

        // name
        buffer_append_str(temp, "\"name\":");
        if (parameter_item->name == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            buffer_append_str(temp, "\"");
            char* name = string_escaping(parameter_item->name);
            buffer_append_str(temp, name);
            memory_destroy(name);
            buffer_append_str(temp, "\"");
        }

        // type
        buffer_append_str(temp, "\"type\":");
        if (parameter_item->type == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            char* type = ast_json(parameter_item->type);
            buffer_append_str(temp, type);
            memory_destroy(type);
        }

        // default_value
        buffer_append_str(temp, "\"default_value\":");
        if (parameter_item->default_value == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            char* default_value = ast_json(parameter_item->default_value);
            buffer_append_str(temp, default_value);
            memory_destroy(default_value);
        }

        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
