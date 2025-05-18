#include "ast_expression_identifier_direct_json.h"

char* ast_expression_identifier_direct_json(ast_expression_identifier_t expression_identifier)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // name
    buffer_append_str(temp, "\"name\": ");
    if (expression_identifier.name == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        char* buffer = string_escaping(expression_identifier.name);
        buffer_append_str(temp, buffer);
        memory_destroy(buffer);
    }

    // runtime_type
    buffer_append_str(temp, ",\"runtime_type\": ");
    if (expression_identifier.runtime_type == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        char* buffer = ast_type_json(expression_identifier.runtime_type);
        buffer_append_str(temp, buffer);
        memory_destroy(buffer);
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
