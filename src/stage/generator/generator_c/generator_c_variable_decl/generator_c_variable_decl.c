#include "generator_c_variable_decl.h"

buffer_t* generator_c_variable_decl(generator_t* generator, ast_t* ast)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(1024);

    ast_variable_decl_t variable_decl = ast->raw.variable_decl_value;

    buffer_t* ident = generator_c_ident(generator);
    buffer_append(temp, ident);

    buffer_t* type = generator_c_node(generator, variable_decl.type);
    buffer_append(temp, type);
    buffer_destroy(type);

    buffer_append_char(temp, ' ');
    buffer_append_str(temp, variable_decl.name);

    if (variable_decl.value == NULL) {
        buffer_append_str(temp, ";\n");
    } else {
        buffer_append_str(temp, " = ");
        buffer_t* value = generator_c_expression_item(generator, variable_decl.value);
        buffer_append(temp, value);
        buffer_destroy(value);
        buffer_append_str(temp, ";\n");
    }

    buffer_destroy(ident);

    return temp;
}
