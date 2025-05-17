#include "ast_statement_expression_json.h"

char* ast_statement_expression_json(ast_statement_expression_t* statement_expression)
{
    buffer_t* temp = buffer_create(24);

    if (statement_expression == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        buffer_append_char(temp, '{');

        // value
        buffer_append_str(temp, "\"value\": ");
        if (statement_expression->value == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            char* name = ast_json(statement_expression->value);
            buffer_append_str(temp, name);
            memory_destroy(name);
        }

        buffer_append_str(temp, "}");
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
