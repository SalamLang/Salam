#include "generator_c_expression_item.h"

buffer_t* generator_c_expression_item(generator_t* generator, ast_t* ast)
{
    DEBUG_ME;
    ast_expression_item_t expression_item = ast->raw.expression_item_value;

    buffer_t* temp = buffer_create(12);

    buffer_append_str(temp, "\"Hey!\\n\"");

    return temp;
}
