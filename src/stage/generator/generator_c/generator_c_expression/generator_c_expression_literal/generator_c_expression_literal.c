#include "generator_c_expression_literal.h"

buffer_t* generator_c_expression_literal(generator_t* generator, ast_t* ast)
{
    DEBUG_ME;
    ast_expression_item_t expression_item = ast->raw.expression_item_value;
    ast_expression_literal_t expression_literal = expression_item->raw.literal;

    buffer_t* temp = buffer_create(12);

    buffer_append_str(temp, "\"value\"");

    return temp;
}
