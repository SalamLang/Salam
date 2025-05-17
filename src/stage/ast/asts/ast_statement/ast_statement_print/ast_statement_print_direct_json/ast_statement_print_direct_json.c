#include "ast_statement_print_direct_json.h"

char* ast_statement_print_direct_json(ast_statement_print_t statement_print)
{
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // value
    buffer_append_str(temp, "\"value\": ");
    if (statement_print.value == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        char* name = ast_json(statement_print.value);
        buffer_append_str(temp, name);
        memory_destroy(name);
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
