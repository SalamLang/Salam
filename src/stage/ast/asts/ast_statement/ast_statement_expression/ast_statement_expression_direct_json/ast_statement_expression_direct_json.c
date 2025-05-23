#include "ast_statement_expression_direct_json.h"

char* ast_statement_expression_direct_json(ast_statement_expression_t statement_expression)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // value
    buffer_append_str(temp, "\"value\":");
    if (statement_expression.value == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        char* name = ast_json(statement_expression.value);
        buffer_append_str(temp, name);
        memory_destroy(name);
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
