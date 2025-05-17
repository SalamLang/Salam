#include "ast_kind_struct_json.h"

char* ast_kind_struct_json(ast_kind_struct_t* kind_struct)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(256);
    if (kind_struct == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        buffer_append_char(temp, '{');

        // name
        buffer_append_str(temp, ", \"name\": ");
        if (kind_struct->name == NULL) {
            buffer_append_str(temp, "null");
        }
        else {
            char* buffer = string_escaping(kind_struct->name);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }

        // parameters
        buffer_append_str(temp, ", \"parameters\": ");
        if (kind_struct->parameters == NULL) {
            buffer_append_str(temp, "null");
        }
        else {
            char* buffer = ast_json(kind_struct->parameters);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }

        // functions
        buffer_append_str(temp, ", \"functions\": ");
        if (kind_struct->functions == NULL) {
            buffer_append_str(temp, "null");
        }
        else {
            char* buffer = ast_json(kind_struct->functions);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }

        buffer_append_char(temp, '}');
    }
    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}

