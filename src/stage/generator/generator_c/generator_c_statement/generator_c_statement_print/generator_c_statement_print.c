#include "generator_c_statement_print.h"

buffer_t* generator_c_statement_print(generator_t* generator, ast_t* ast)
{
    buffer_t* temp = buffer_create(1024);

    buffer_append_str(temp, "printf(");

    buffer_append_str(temp, "\"Hi!\\n\"");

    buffer_append_str(temp, ");\n");

    return temp;
}
