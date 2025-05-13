#include <stage/token/token_json/token_json.h>

char* token_json(const token_t* token)
{
    buffer_t *temp = buffer_create(256);
    if (token == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        buffer_append_str(temp, "{");

        // type
        buffer_append_str(temp, "\"type\": ");
        buffer_append_str(temp, token_name(token->type));
        buffer_append_str(temp, ", ");

        // source
        buffer_append_str(temp, "\"source\": ");
        if (token->source) {
            buffer_append_char(temp, '"');
            buffer_append_str(temp, token->source);  // Consider escaping if needed
            buffer_append_char(temp, '"');
        } else {
            buffer_append_str(temp, "null");
        }
        buffer_append_str(temp, ", ");

        // location
        buffer_append_str(temp, "\"location\": ");
        char* json_location = token_location_json(token->location);
        buffer_append_str(temp, json_location);
        memory_destroy(json_location);

        buffer_append_str(temp, ", ");

        // value
        buffer_append_str(temp, "\"value\": ");
        char* json_value = value_json(token->value);
        buffer_append_str(temp, json_value);
        memory_destroy(json_value);

        buffer_append_str(temp, "}");
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
