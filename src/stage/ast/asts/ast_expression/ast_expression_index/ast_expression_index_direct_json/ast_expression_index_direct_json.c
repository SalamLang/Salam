#include "ast_expression_index_direct_json.h"

char* ast_expression_index_direct_json(ast_expression_index_t expression_index)
{
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // object
    buffer_append_str(temp, "\"object\": ");
    if (expression_index.object == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        char* buffer = ast_json(expression_index.object);
        buffer_append_str(temp, buffer);
        memory_destroy(buffer);
    }

    // index
    buffer_append_str(temp, ",\"index\": ");
    if (expression_index.index == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        char* buffer = ast_json(expression_index.index);
        buffer_append_str(temp, buffer);
        memory_destroy(buffer);
    }

    // runtime_type
    buffer_append_str(temp, ",\"runtime_type\": ");
    if (expression_index.runtime_type == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        char* buffer = ast_type_json(expression_index.runtime_type);
        buffer_append_str(temp, buffer);
        memory_destroy(buffer);
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
