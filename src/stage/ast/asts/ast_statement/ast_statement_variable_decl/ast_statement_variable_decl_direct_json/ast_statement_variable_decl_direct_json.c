#include "ast_statement_variable_decl_direct_json.h"

char* ast_statement_variable_decl_direct_json(
    ast_statement_variable_decl_t statement_variable_decl) {
    DEBUG_ME;
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // name
    buffer_append_str(temp, "\"name\":");
    if (statement_variable_decl.name == NULL) {
        buffer_append_str(temp, "null");
    } else {
        buffer_append_str(temp, "\"");
        char* name = string_escaping(statement_variable_decl.name);
        buffer_append_str(temp, name);
        memory_destroy(name);
        buffer_append_str(temp, "\"");
    }

    // type
    buffer_append_str(temp, "\"type\":");
    if (statement_variable_decl.type == NULL) {
        buffer_append_str(temp, "null");
    } else {
        char* type = ast_json(statement_variable_decl.type);
        buffer_append_str(temp, type);
        memory_destroy(type);
    }

    // value
    buffer_append_str(temp, "\"value\":");
    if (statement_variable_decl.value == NULL) {
        buffer_append_str(temp, "null");
    } else {
        char* value = ast_json(statement_variable_decl.value);
        buffer_append_str(temp, value);
        memory_destroy(value);
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
