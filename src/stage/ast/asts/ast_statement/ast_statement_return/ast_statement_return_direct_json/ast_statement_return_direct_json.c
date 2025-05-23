#include "ast_statement_return_direct_json.h"

char* ast_statement_return_direct_json(ast_statement_ret_t statement_return)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // values
    buffer_append_str(temp, "\"values\":");
    if (statement_return.values == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        char* values = ast_json(statement_return.values);
        buffer_append_str(temp, values);
        memory_destroy(values);
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
