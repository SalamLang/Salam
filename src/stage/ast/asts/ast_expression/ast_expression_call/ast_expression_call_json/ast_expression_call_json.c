#include "ast_expression_call_json.h"

char* ast_expression_call_json(ast_expression_call_t* expression_call) {
    buffer_t* temp = buffer_create(24);

    if (expression_call == NULL) {
        buffer_append_str(temp, "null");
    } else {
        buffer_append_char(temp, '{');

        // callee
        buffer_append_str(temp, "\"callee\":");
        if (expression_call->callee == NULL) {
            buffer_append_str(temp, "null");
        } else {
            char* buffer = ast_json(expression_call->callee);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }

        // args
        buffer_append_str(temp, ",\"args\":");
        if (expression_call->args == NULL) {
            buffer_append_str(temp, "null");
        } else {
            char* buffer = ast_json(expression_call->args);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }

        // runtime_type
        buffer_append_str(temp, ",\"runtime_type\":");
        if (expression_call->runtime_type == NULL) {
            buffer_append_str(temp, "null");
        } else {
            char* buffer = ast_type_json(expression_call->runtime_type);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }

        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
