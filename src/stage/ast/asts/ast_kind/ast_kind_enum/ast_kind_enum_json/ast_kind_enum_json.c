#include "ast_kind_enum_json.h"

char* ast_kind_enum_json(ast_kind_enum_t* kind_enum)
{
    buffer_t* temp = buffer_create(256);
    if (kind_enum == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        buffer_append_char(temp, '{');

        // name
        buffer_append_str(temp, ", \"name\": ");
        if (kind_enum->name == NULL) {
            buffer_append_str(temp, "null");
        }
        else {
            char* buffer = string_escaping(kind_enum->name);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }

        // alias
        buffer_append_str(temp, ", \"alias\": ");
        if (kind_enum->alias == NULL) {
            buffer_append_str(temp, "null");
        }
        else {
            char* buffer = string_escaping(kind_enum->alias);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }

        // values
        buffer_append_str(temp, ", \"values\": ");
        if (kind_enum->values == NULL) {
            buffer_append_str(temp, "null");
        }
        else {
            // TODO
            buffer_append_str(temp, "\"TODO\"");
        }

        buffer_append_char(temp, '}');
    }
    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}

