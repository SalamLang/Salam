#include "ast_statement_foreach_json.h"

char* ast_statement_foreach_json(ast_statement_foreach_t* statement_foreach)
{
    buffer_t* temp = buffer_create(24);

    if (statement_foreach == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        buffer_append_char(temp, '{');

        // loop_variable
        buffer_append_str(temp, "\"loop_variable\": ");
        if (statement_foreach->loop_variable == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            char* loop_variable = string_escaping(statement_foreach->loop_variable);
            buffer_append_str(temp, loop_variable);
            memory_destroy(loop_variable);
        }

        // iterable
        buffer_append_str(temp, "\"iterable\": ");
        if (statement_foreach->iterable == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            char* iterable = ast_json(statement_foreach->iterable);
            buffer_append_str(temp, iterable);
            memory_destroy(iterable);
        }

        // type
        buffer_append_str(temp, "\"type\": ");
        if (statement_foreach->type == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            char* type = ast_type_json(statement_foreach->type);
            buffer_append_str(temp, type);
            memory_destroy(type);
        }

        // block
        buffer_append_str(temp, "\"block\": ");
        if (statement_foreach->block == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            char* block = ast_json(statement_foreach->block);
            buffer_append_str(temp, block);
            memory_destroy(block);
        }


        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
