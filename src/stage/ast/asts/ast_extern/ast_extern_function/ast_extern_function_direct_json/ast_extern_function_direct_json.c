#include <stage/ast/asts/ast_extern/ast_extern_function/ast_extern_function_direct_json/ast_extern_function_direct_json.h>

char* ast_extern_function_direct_json(ast_extern_function_t extern_function)
{
    buffer_t* temp = buffer_create(10);

    buffer_append_char(temp, '{');

    // name
    buffer_append_str(temp, "\"name\":");
    char* name = string_escaping(extern_function.name);
    buffer_append_str(temp, name);
    memory_destroy(name);

    // alias
    buffer_append_str(temp, "\"alias\":");
    if (extern_function.alias == NULL) {
        buffer_append_str(temp, "null");
    } else {
        char* alias = string_escaping(extern_function.alias);
        buffer_append_str(temp, alias);
        memory_destroy(alias);
    }

    // type
    buffer_append_str(temp, "\"type\":");
    char* type = ast_json(extern_function.type);
    buffer_append_str(temp, type);
    memory_destroy(type);

    // attributes
    buffer_append_str(temp, "\"attributes\":");
    char* attributes = ast_json(extern_function.attributes);
    buffer_append_str(temp, attributes);
    memory_destroy(attributes);

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
