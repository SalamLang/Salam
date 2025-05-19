#include "generator_c_statement_print.h"

buffer_t* generator_c_statement_print(generator_t* generator, ast_t* ast)
{
    buffer_t* temp = buffer_create(1024);

    buffer_t* ident = generator_c_ident(generator);
    buffer_append(temp, ident);

    buffer_append_str(temp, "printf(");

    buffer_append_str(temp, "\"Hi!\\n\"");

    buffer_append_str(temp, ");\n");

    buffer_destroy(ident);

    return temp;
}
