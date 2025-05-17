#include <stage/ast/asts/ast_function_decl/ast_function_decl_json/ast_function_decl_json.h>

char* ast_function_decl_json(ast_function_decl_t* function_decl)
{
    buffer_t* temp = buffer_create(24);

    if (function_decl == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else {
        buffer_append_char(temp, '{');

        // name
        buffer_append_str(temp, "\"name\":");
        if (function_decl->name == NULL) {
            buffer_append_str(temp, "null");
        } else {
            buffer_append_str(temp, "\"");
            char* name = string_escaping(function_decl->name);
            buffer_append_str(temp, name);
            memory_destroy(name);
            buffer_append_str(temp, "\"");
        }

        // parameters
        buffer_append_str(temp, ", \"parameters\":");
        if (function_decl->parameters == NULL) {
            buffer_append_str(temp, "null");
        } else {
            char* name = ast_json(function_decl->parameters);
            buffer_append_str(temp, name);
            memory_destroy(name);
        }

        // block
        buffer_append_str(temp, ", \"block\":");
        if (function_decl->block == NULL) {
            buffer_append_str(temp, "null");
        } else {
            char* name = ast_json(function_decl->block);
            buffer_append_str(temp, name);
            memory_destroy(name);
        }

        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
