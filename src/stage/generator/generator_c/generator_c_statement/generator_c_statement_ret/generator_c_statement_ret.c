#include "generator_c_statement_ret.h"

buffer_t* generator_c_statement_ret(generator_t* generator, ast_t* ast)
{
    buffer_t* temp = buffer_create(1024);

    buffer_append_str(temp, "return ");

    buffer_append_str(temp, ";\n");

    return temp;
}
