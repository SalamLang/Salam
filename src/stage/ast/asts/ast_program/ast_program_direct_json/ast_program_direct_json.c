#include <stage/ast/asts/ast_program/ast_program_direct_json/ast_program_direct_json.h>

char* ast_program_direct_json(ast_program_t program)
{
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // variable_declaration_count
    buffer_append_str(temp, "\"variable_declaration_count\": ");
    buffer_append_str(temp, convert_size2string(program.variable_declaration_count));
    buffer_append_str(temp, ", ");

    // function_declaration_count
    buffer_append_str(temp, "\"function_declaration_count\": ");
    buffer_append_str(temp, convert_size2string(program.function_declaration_count));
    buffer_append_str(temp, ", ");

    // variable_declarations
    buffer_append_str(temp, "\"variable_declarations\": ");
    if (program.variable_declarations == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        buffer_append_char(temp, '[');
        for (size_t i = 0; i < program.variable_declarations->size; i++) {
            if (i > 0) {
                buffer_append_str(temp, ", ");
            }
            ast_t* variable_decl = (ast_t*)array_get(program.variable_declarations, i);
            char* buffer = ast_json(variable_decl);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }
        buffer_append_char(temp, ']');
    }

    // function_declarations
    buffer_append_str(temp, ", \"function_declarations\": ");
    if (program.function_declarations == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        buffer_append_char(temp, '[');
        for (size_t i = 0; i < program.function_declarations->size; i++) {
            if (i > 0) {
                buffer_append_str(temp, ", ");
            }
            ast_t* function_decl = (ast_t*)array_get(program.function_declarations, i);
            char* buffer = ast_json(function_decl);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }
        buffer_append_char(temp, ']');
    }

    buffer_append_str(temp, "}");

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
