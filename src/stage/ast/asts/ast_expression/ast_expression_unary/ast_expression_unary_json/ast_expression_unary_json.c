#include "ast_expression_unary_json.h"

char* ast_expression_unary_json(ast_expression_unary_t* expression_unary)
{
    buffer_t* temp = buffer_create(24);

    if (expression_unary == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        buffer_append_char(temp, '{');

        // op_type
        buffer_append_str(temp, "\"op_type\": ");
        buffer_append_str(temp, token_operator_name(expression_unary.op_type));

        // operand
        buffer_append_str(temp, ",\"operand\": ");
        if (expression_unary.operand == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            char* buffer = ast_json(expression_unary.operand);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }

        // runtime_type
        buffer_append_str(temp, ",\"runtime_type\": ");
        if (expression_unary.runtime_type == NULL)
        {
            buffer_append_str(temp, "null");
        }
        else
        {
            char* buffer = ast_type_json(expression_unary.runtime_type);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }

        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
