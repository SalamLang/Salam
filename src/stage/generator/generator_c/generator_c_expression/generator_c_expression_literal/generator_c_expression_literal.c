#include "generator_c_expression_literal.h"

buffer_t* generator_c_expression_literal(generator_t* generator, ast_t* ast)
{
    (void)generator; // Suppress unused parameter warning
    DEBUG_ME;
    ast_expression_item_t expression_item = ast->raw.expression_item_value;
    ast_expression_literal_t expression_literal = expression_item.raw.literal;

    buffer_t* temp = buffer_create(12);

    char* value_str = value_json(expression_literal.value);

    switch (expression_literal.value->type) {
        case VALUE_TYPE_STRING:
            buffer_append_char(temp, '"');
            buffer_append_str(temp, expression_literal.value->raw.string_value);
            buffer_append_char(temp, '"');
            break;

        case VALUE_TYPE_NUMBER_INT:
            buffer_append_str(temp, convert_int2string(expression_literal.value->raw.int_value));
            break;

        case VALUE_TYPE_NUMBER_FLOAT:
            buffer_append_str(temp, convert_float2string(expression_literal.value->raw.float_value));
            break;

        case VALUE_TYPE_BOOL:
            buffer_append_str(temp, expression_literal.value->raw.bool_value ? "true" : "false");
            break;

        case VALUE_TYPE_SIZE:
            buffer_append_str(temp, convert_size2string(expression_literal.value->raw.size_value));
            break;

        case VALUE_TYPE_NULL:
            buffer_append_str(temp, "NULL");
            break;

        default:
            log_fatal("Error: unknown value type!");
            break;
    }
    memory_destroy(value_str);

    return temp;
}
