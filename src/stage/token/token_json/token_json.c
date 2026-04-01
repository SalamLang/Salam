#include <stage/token/token_json/token_json.h>

char* token_json(const token_t* token) {
    buffer_t* temp = buffer_create(256);
    if (token == NULL) {
        buffer_append_str(temp, "null");
    } else {
        buffer_append_char(temp, '{');

        // base
        buffer_append_str(temp, "\"base\": \"token_t\",");

        // type
        buffer_append_str(temp, "\"type\":");
        buffer_append_char(temp, '"');
        buffer_append_str(temp, token_name(token->type));
        buffer_append_char(temp, '"');

        // operator_type
        buffer_append_str(temp, ",\"operator_type\":");
        if (token->type != TOKEN_TYPE_OPERATOR) {
            buffer_append_str(temp, "null");
        } else {
            buffer_append_char(temp, '\"');
            buffer_append_str(temp, token_operator_name(token->operator_type));
            buffer_append_char(temp, '\"');
        }

        // source
        buffer_append_str(temp, ",\"source\":");
        if (token->source == NULL) {
            buffer_append_str(temp, "null");
        } else {
            buffer_append_char(temp, '"');
            char* token_source = string_escaping(token->source);
            buffer_append_str(temp, token_source);
            memory_destroy(token_source);
            buffer_append_char(temp, '"');
        }
        buffer_append_str(temp, ",");

        // location
        buffer_append_str(temp, "\"location\":");
        char* json_location = token_location_json(token->location);
        buffer_append_str(temp, json_location);
        memory_destroy(json_location);

        buffer_append_str(temp, ",");

        // value
        buffer_append_str(temp, "\"value\":");
        char* json_value = value_json(token->value);
        buffer_append_str(temp, json_value);
        memory_destroy(json_value);

        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
