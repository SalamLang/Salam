#include "ast_kind_decl_direct_json.h"

char* ast_kind_decl_direct_json(ast_kind_decl_t kind_decl)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(256);
    buffer_append_char(temp, '{');

    // kind_type
    buffer_append_str(temp, "\"kind_type\":");
    buffer_append_str(temp, ast_kind_decl_type_name(kind_decl.kind_type));

    // value
    buffer_append_str(temp, ", \"value\":");
    if (kind_decl.value == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        char* buffer = ast_json(kind_decl.value);
        buffer_append_str(temp, buffer);
        memory_destroy(buffer);
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}

