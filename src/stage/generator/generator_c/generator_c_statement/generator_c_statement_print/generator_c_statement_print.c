#include "generator_c_statement_print.h"

buffer_t* generator_c_statement_print(generator_t* generator, ast_t* ast)
{
    DEBUG_ME;
    ast_statement_print_t print = ast->raw.statement_print_value;
    buffer_t* temp = buffer_create(1024);

    buffer_t* ident = generator_c_ident(generator);
    buffer_append(temp, ident);

    buffer_append_str(temp, "printf(");

    buffer_append_char(temp, '\"');
    for (size_t i = 0; i < print.value_count; i++) {
        buffer_append_str(temp, "%s");
    }
    buffer_append_char(temp, '\"');

    buffer_append_str(temp, ", ");

    buffer_t* expressions = generator_c_expressions(generator, print.values);
    buffer_append(temp, expressions);
    buffer_destroy(expressions);

    buffer_append_str(temp, ");\n");

    buffer_destroy(ident);

    return temp;
}
