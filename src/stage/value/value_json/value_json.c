#include <stage/value/value_json/value_json.h>

char* value_json(const value_t* value) {
    DEBUG_ME;
    buffer_t* temp = buffer_create(256);
    if (value == NULL) {
        buffer_append_str(temp, "null");
    } else {
        log_info("Value type: %s\n", value_name(value->type));
        buffer_append_char(temp, '{');

        // base
        buffer_append_str(temp, "\"base\": \"value_t\",");

        // type
        buffer_append_str(temp, "\"type\":");
        buffer_append_char(temp, '"');
        buffer_append_str(temp, value_name(value->type));
        buffer_append_char(temp, '"');
        buffer_append_str(temp, ", ");

        log_info("Value type: %s\n", value_name(value->type));

        // value
        buffer_append_str(temp, "\"value\":");

        switch (value->type) {
            case VALUE_TYPE_NUMBER_INT: {
                buffer_append_str(temp,
                                  convert_size2string(value->raw.int_value));
            } break;
            case VALUE_TYPE_NUMBER_FLOAT: {
                buffer_append_str(temp,
                                  convert_size2string(value->raw.float_value));
            } break;
            case VALUE_TYPE_BOOL: {
                buffer_append_str(temp,
                                  value->raw.bool_value ? "true" : "false");
            } break;
            case VALUE_TYPE_NULL: {
                buffer_append_str(temp, "null");
            } break;
            case VALUE_TYPE_STRING: {
                if (value->raw.string_value == NULL) {
                    buffer_append_str(temp, "null");
                } else {
                    buffer_append_char(temp, '"');
                    char* value_str = string_escaping(value->raw.string_value);
                    buffer_append_str(temp, value_str);
                    memory_destroy(value_str);
                    buffer_append_char(temp, '"');
                }
            } break;
            case VALUE_TYPE_SIZE: {
                buffer_append_str(temp,
                                  convert_size2string(value->raw.size_value));
            } break;
            default: {
                buffer_append_str(temp, "\"value error\"");
            } break;
        }

        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
