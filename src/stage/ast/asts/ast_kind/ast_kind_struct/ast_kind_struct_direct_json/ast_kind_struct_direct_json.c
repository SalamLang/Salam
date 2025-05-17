#include "ast_kind_struct_direct_json.h"

char* ast_kind_struct_direct_json(ast_kind_struct_t kind_struct)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(256);
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

    // parameters
    buffer_append_str(temp, ", \"parameters\": ");
    if (kind_decl->parameters == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        char* buffer = ast_json(kind_decl->parameters);
        buffer_append_str(temp, buffer);
        memory_destroy(buffer);
    }

    // functions
    buffer_append_str(temp, ", \"functions\": ");
    if (kind_decl->functions == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        char* buffer = ast_json(kind_decl->functions);
        buffer_append_str(temp, buffer);
        memory_destroy(buffer);
    }

    buffer_append_str(temp, "}");

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}

