#include "ast_kind_enum_direct_json.h"

char* ast_kind_enum_direct_json(ast_kind_enum_t kind_enum)
{
    buffer_t* temp = buffer_create(256);
    if (kind_enum == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        buffer_append_char(temp, '{');

        // name
        buffer_append_str(temp, ", \"name\": ");
        if (kind_decl->name == NULL) {
            buffer_append_str(temp, "null");
        }
        else {
            char* buffer = string_escaping(kind_decl->name);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }

        // alias
        buffer_append_str(temp, ", \"alias\": ");
        if (kind_decl->alias == NULL) {
            buffer_append_str(temp, "null");
        }
        else {
            char* buffer = string_escaping(kind_decl->alias);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }

        // values
        buffer_append_str(temp, ", \"values\": ");
        if (kind_decl->values == NULL) {
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

