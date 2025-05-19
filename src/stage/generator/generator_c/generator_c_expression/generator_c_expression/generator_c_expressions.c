#include "generator_c_expressions.h"

buffer_t* generator_c_expressions(generator_t* generator, ast_t* ast)
{
    DEBUG_ME;
    ast_expressions_t expressions = ast->raw.expressions_value;
    buffer_t* temp = buffer_create(12);

    for (size_t i = 0; i < expressions.value_count; i++) {
        if (i > 0) {
            buffer_append_str(temp, ", ");
        }
        ast_t* expression_item = expressions.values->items[i];
        buffer_t* temp_expression_item = generator_c_expression_item(generator, expression_item);
        buffer_append(temp, temp_expression_item);
        buffer_destroy(temp_expression_item);
    }

    return temp;
}
