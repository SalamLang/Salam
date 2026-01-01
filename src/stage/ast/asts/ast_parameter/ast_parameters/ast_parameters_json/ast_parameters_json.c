#include "ast_parameters_json.h"

char* ast_parameters_json(ast_parameters_t* parameters) {
    buffer_t* temp = buffer_create(24);

    if (parameters == NULL) {
        buffer_append_str(temp, "null");
    } else {
        buffer_append_char(temp, '{');

        // base
        buffer_append_str(temp, "\"base\": \"ast_parameters_t\",");

        // value_count
        buffer_append_str(temp, "\"value_count\":");
        buffer_append_str(temp, convert_size2string(parameters->value_count));

        // values
        buffer_append_str(temp, ",\"values\":");
        if (parameters->values == NULL) {
            buffer_append_str(temp, "null");
        } else {
            buffer_append_char(temp, '[');
            for (size_t i = 0; i < parameters->value_count; i++) {
                if (i > 0) {
                    buffer_append_str(temp, ", ");
                }
                ast_t* item = parameters->values->items[i];
                char* buffer = ast_json(item);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
            }
            buffer_append_char(temp, ']');
        }

        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
