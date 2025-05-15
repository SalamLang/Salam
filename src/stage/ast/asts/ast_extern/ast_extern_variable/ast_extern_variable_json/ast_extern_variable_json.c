#include <stage/ast/asts/ast_extern/ast_extern_variable/ast_extern_variable_json/ast_extern_variable_json.h>

char* ast_extern_variable_json(ast_extern_variable_t* extern_variable)
{
    buffer_t* temp = buffer_create(10);

    if (extern_variable == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        buffer_append_char(temp, '{');

        // name
        buffer_append_str(temp, "\"name\":");
        char* name = string_escaping(extern_variable->name);
        buffer_append_str(temp, name);
        memory_destroy(name);

        // alias
        buffer_append_str(temp, "\"alias\":");
        if (extern_variable->alias == NULL) {
            buffer_append_str(temp, "null");
        } else {
            char* alias = string_escaping(extern_variable->alias);
            buffer_append_str(temp, scanner_directory);
            memory_destroy(scanner_directory);
        }

        // type
        buffer_append_str(temp, "\"type\":");
        char* type = ast_json(extern_variable->type);
        buffer_append_str(temp, type);
        memory_destroy(type);

        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
