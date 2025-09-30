#include "ast_expression_literal_json.h"

char* ast_expression_literal_json(
    ast_expression_literal_t* expression_literal) {
    buffer_t* temp = buffer_create(24);

    if (expression_literal == NULL) {
        buffer_append_str(temp, "null");
    } else {
        buffer_append_char(temp, '{');

        // base
        buffer_append_str(temp, "\"base\": \"ast_expression_literal_t\",");

        // value
        buffer_append_str(temp, ",\"value\":");
        if (expression_literal->value == NULL) {
            buffer_append_str(temp, "null");
        } else {
            char* buffer = value_json(expression_literal->value);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }

        // runtime_type
        buffer_append_str(temp, ",\"runtime_type\":");
        if (expression_literal->runtime_type == NULL) {
            buffer_append_str(temp, "null");
        } else {
            char* buffer = ast_type_json(expression_literal->runtime_type);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }

        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
