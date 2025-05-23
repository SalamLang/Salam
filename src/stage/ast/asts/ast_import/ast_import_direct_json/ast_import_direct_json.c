#include "ast_import_direct_json.h"

char* ast_import_direct_json(ast_import_t import)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // name
    buffer_append_str(temp, "\"name\":");
    if (import.name == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        buffer_append_str(temp, "\"");
        char* name = string_escaping(import.name);
        buffer_append_str(temp, name);
        memory_destroy(name);
        buffer_append_str(temp, "\"");
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
