#include <stage/value/value_json/value_json.h>

char* value_json(const value_t* value)
{
    buffer_t* temp = buffer_create(256);
    if (value == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        buffer_append_str(temp, "{");
        buffer_append_str(temp, "\"type\": ");
        buffer_append_str(temp, value_name(value->type));
        buffer_append_str(temp, ", ");
        buffer_append_str(temp, "\"value\": ");

        switch (value->type) {
            case VALUE_TYPE_NUMBER_INT: {
                buffer_append_str(temp, convert_size2string(value->raw.int_value));
            } break;
            case VALUE_TYPE_NUMBER_FLOAT: {
                buffer_append_str(temp, convert_size2string(value->raw.float_value));
            } break;
            case VALUE_TYPE_STRING: {
                char *value_str = value->raw.string_value;
                if (value_str) {
                    buffer_append_char(temp, '"');
                    buffer_append_str(temp, value_str); // TODO: consider escaping if needed
                    buffer_append_char(temp, '"');
                } else {
                    buffer_append_str(temp, "null");
                }
            } break;
        }
        buffer_append_str(temp, "}");
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
