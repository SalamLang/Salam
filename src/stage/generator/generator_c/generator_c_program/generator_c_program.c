#include "generator_c_program.h"

buffer_t* generator_c_program(generator_t* generator, ast_t* ast) {
    DEBUG_ME;
    buffer_t* temp = buffer_create(1024);

    array_t* variable_declarations =
        ast->raw.program_value.variable_declarations;
    size_t variable_declaration_count = variable_declarations->size;

    array_t* function_declarations =
        ast->raw.program_value.function_declarations;
    size_t function_declaration_count = function_declarations->size;

    log_info("variable_declaration_count: %zu\n", variable_declaration_count);
    log_info("function_declaration_count: %zu\n", function_declaration_count);

    if (function_declaration_count > 0) {
        buffer_append_str(temp, "#include <stdio.h>\n");
        buffer_append_str(temp, "#include <stdlib.h>\n");
        buffer_append_str(temp, "\n");
    }

    if (variable_declaration_count > 0) {
        for (size_t i = 0; i < variable_declaration_count; i++) {
            log_info("variable_declarations: %zu\n", i);
            ast_t* node = array_get(variable_declarations, i);
            buffer_t* temp_node = generator_c_node(generator, node);
            buffer_append(temp, temp_node);
            buffer_destroy(temp_node);
        }
    }

    if (function_declaration_count > 0) {
        for (size_t i = 0; i < function_declaration_count; i++) {
            log_info("function_declarations: %zu\n", i);
            ast_t* node = array_get(function_declarations, i);
            buffer_t* temp_node = generator_c_node(generator, node);
            buffer_append(temp, temp_node);
            buffer_destroy(temp_node);
        }
    }

    return temp;
}
