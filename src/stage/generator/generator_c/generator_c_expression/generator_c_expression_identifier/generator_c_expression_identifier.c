#include "generator_c_expression_identifier.h"

buffer_t* generator_c_expression_identifier(generator_t* generator, ast_t* ast)
{
    DEBUG_ME;
    ast_expression_item_t expression_item = ast->raw.expression_item_value;
    ast_expression_identifier_t expression_identifier = expression_item.raw.identifier;

    buffer_t* temp = buffer_create(12);

    // TODO: validate if the variable is already defined or not.
    buffer_append_str(temp, expression_identifier.name);

    return temp;
}
