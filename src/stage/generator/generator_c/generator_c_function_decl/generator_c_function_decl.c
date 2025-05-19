#include "generator_c_function_decl.h"

buffer_t* generator_c_function_decl(generator_t* generator, ast_t* ast)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(1024);

    ast_function_decl_t function_decl = ast->raw.function_decl_value;

    buffer_t* return_type = generator_c_type(generator, function_decl.return_type);
    buffer_append(temp, return_type);
    buffer_destroy(return_type);

    buffer_append_char(temp, ' ');
    buffer_append_str(temp, function_decl.name);
    buffer_append_str(temp, "()");

    buffer_t* block = generator_c_node(generator, function_decl.block);
    buffer_append(temp, block);
    buffer_destroy(block);

    return temp;
}
