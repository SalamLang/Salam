#include "ast_expression_binary_direct_json.h"

char* ast_expression_binary_direct_json(ast_expression_binary_t expression_binary)
{
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // op
    buffer_append_str(temp, "\"op\": ");
    if (expression_binary.op == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        char* buffer = string_escaping(expression_binary.op);
        buffer_append_str(temp, buffer);
        memory_destroy(buffer);
    }

    // left
    buffer_append_str(temp, ",\"left\": ");
    if (expression_binary.left == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        char* buffer = ast_json(expression_binary.left);
        buffer_append_str(temp, buffer);
        memory_destroy(buffer);
    }

    // right
    buffer_append_str(temp, ",\"right\": ");
    if (expression_binary.right == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        char* buffer = ast_json(expression_binary.right);
        buffer_append_str(temp, buffer);
        memory_destroy(buffer);
    }

    // runtime_type
    buffer_append_str(temp, ",\"runtime_type\": ");
    if (expression_binary.runtime_type == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        char* buffer = ast_json(expression_binary.runtime_type);
        buffer_append_str(temp, buffer);
        memory_destroy(buffer);
    }

    buffer_append_str(temp, "}");

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
