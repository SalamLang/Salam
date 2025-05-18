#include "ast_statement_print_direct_json.h"

char* ast_statement_print_direct_json(ast_statement_print_t statement_print)
{
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // values
    buffer_append_str(temp, "\"values\": ");
    if (statement_print.values == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        char* values = ast_json(statement_print.values);
        buffer_append_str(temp, values);
        memory_destroy(values);
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
