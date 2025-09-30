#include "ast_statement_raw_direct_json.h"

char* ast_statement_raw_direct_json(ast_statement_raw_t statement_raw) {
    DEBUG_ME;
    buffer_t* temp = buffer_create(124);

    buffer_append_char(temp, '{');

    // base
    buffer_append_str(temp, "\"base\":\"ast_statement_raw_t\",");

    // value
    buffer_append_str(temp, "\"value\":");
    if (statement_raw.value == NULL) {
        buffer_append_str(temp, "null");
    } else {
        buffer_append_char(temp, '"');
        char* value = string_escaping(statement_raw.value);
        buffer_append_str(temp, value);
        memory_destroy(value);
        buffer_append_char(temp, '"');
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
