#include "ast_package_direct_json.h"

char* ast_package_direct_json(ast_package_t package)
{
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // name
    buffer_append_str(temp, "\"name\": ");
    if (package.name == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        buffer_append_str(temp, "\"");
        char* name = string_escaping(package.name);
        buffer_append_str(temp, name);
        memory_destroy(name);
        buffer_append_str(temp, "\"");
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
