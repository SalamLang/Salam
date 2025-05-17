#include "ast_statement_return_json.h"

char* ast_statement_return_json(ast_statement_return_t* statement_return)
{
    buffer_t* temp = buffer_create(24);

    if (statement_return == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        buffer_append_char(temp, '{');

        // value
        buffer_append_str(temp, "\"value\": ");
        if (statement_return->value == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            char* name = ast_json(statement_return->value);
            buffer_append_str(temp, name);
            memory_destroy(name);
        }

        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
