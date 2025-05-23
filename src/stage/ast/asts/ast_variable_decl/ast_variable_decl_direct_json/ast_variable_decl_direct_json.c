#include <stage/ast/asts/ast_variable_decl/ast_variable_decl_direct_json/ast_variable_decl_direct_json.h>

char* ast_variable_decl_direct_json(ast_variable_decl_t variable_decl)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // base
    buffer_append_str(temp, "\"base\":\"ast_variable_decl_t\"");

    // name
    buffer_append_str(temp, ",\"name\": ");
    if (variable_decl.name == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        buffer_append_str(temp, "\"");
        char* name = string_escaping(variable_decl.name);
        buffer_append_str(temp, name);
        memory_destroy(name);
        buffer_append_str(temp, "\"");
    }

    // type
    buffer_append_str(temp, ", \"type\": ");
    if (variable_decl.type == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        char* buffer = ast_json(variable_decl.type);
        buffer_append_str(temp, buffer);
        memory_destroy(buffer);
    }

    // value
    buffer_append_str(temp, ", \"value\": ");
    if (variable_decl.value == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        char* buffer = ast_json(variable_decl.value);
        buffer_append_str(temp, buffer);
        memory_destroy(buffer);
    }

    buffer_append_char(temp, '}');


    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
