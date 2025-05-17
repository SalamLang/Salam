#include "ast_statement_for_json.h"

char* ast_statement_for_json(ast_statement_for_t* statement_for)
{
    buffer_t* temp = buffer_create(24);

    if (statement_for == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        buffer_append_char(temp, '{');

        // initializer
        buffer_append_str(temp, "\"initializer\": ");
        if (statement_for->initializer == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            char* initializer = ast_json(statement_for->initializer);
            buffer_append_str(temp, initializer);
            memory_destroy(initializer);
        }

        // condition
        buffer_append_str(temp, "\"condition\": ");
        if (statement_for->condition == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            char* condition = ast_json(statement_for->condition);
            buffer_append_str(temp, condition);
            memory_destroy(condition);
        }

        // increment
        buffer_append_str(temp, "\"increment\": ");
        if (statement_for->increment == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            char* increment = ast_json(statement_for->increment);
            buffer_append_str(temp, increment);
            memory_destroy(increment);
        }

        // block
        buffer_append_str(temp, "\"block\": ");
        if (statement_for->block == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            char* block = ast_json(statement_for->block);
            buffer_append_str(temp, block);
            memory_destroy(block);
        }


        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
