#include <stage/ast/asts/ast_function_decl/ast_function_decl_direct_json/ast_function_decl_direct_json.h>

char* ast_function_decl_direct_json(ast_function_decl_t function_decl) {
    DEBUG_ME;
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // base
    buffer_append_str(temp, "\"base\": \"ast_function_decl_t\",");

    // name
    buffer_append_str(temp, "\"name\":");
    if (function_decl.name == NULL) {
        buffer_append_str(temp, "null");
    } else {
        buffer_append_str(temp, "\"");
        char* name = string_escaping(function_decl.name);
        buffer_append_str(temp, name);
        memory_destroy(name);
        buffer_append_str(temp, "\"");
    }

    // return_type
    buffer_append_str(temp, ", \"return_type\":");
    if (function_decl.return_type == NULL) {
        buffer_append_str(temp, "null");
    } else {
        char* return_type = ast_json(function_decl.return_type);
        buffer_append_str(temp, return_type);
        memory_destroy(return_type);
    }

    // parameters
    buffer_append_str(temp, ", \"parameters\":");
    if (function_decl.parameters == NULL) {
        buffer_append_str(temp, "null");
    } else {
        char* parameters = ast_json(function_decl.parameters);
        buffer_append_str(temp, parameters);
        memory_destroy(parameters);
    }

    // block
    buffer_append_str(temp, ", \"block\":");
    if (function_decl.block == NULL) {
        buffer_append_str(temp, "null");
    } else {
        char* block = ast_json(function_decl.block);
        buffer_append_str(temp, block);
        memory_destroy(block);
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
