#include "generator_c_expression_item.h"

buffer_t* generator_c_expression_item(generator_t* generator, ast_t* ast)
{
    DEBUG_ME;
    ast_expression_item_t expression_item = ast->raw.expression_item_value;

    buffer_t* temp = buffer_create(12);
    buffer_t* buffer;

    switch (expression_item.type) {
        case AST_EXPRESSION_TYPE_LITERAL:
            buffer = generator_c_expression_literal(generator, ast);
            buffer_append(temp, buffer);
            buffer_destroy(buffer);
            break;

        case AST_EXPRESSION_TYPE_IDENTIFIER:
            buffer = generator_c_expression_identifier(generator, ast);
            buffer_append(temp, buffer);
            buffer_destroy(buffer);
            break;

        case AST_EXPRESSION_TYPE_BINARY:
            // buffer = generator_c_expression_binary(generator, ast);
            // buffer_append(temp, buffer);
            // buffer_destroy(buffer);
            break;

        case AST_EXPRESSION_TYPE_UNARY:
            // buffer = generator_c_expression_unary(generator, ast);
            // buffer_append(temp, buffer);
            // buffer_destroy(buffer);
            break;

        case AST_EXPRESSION_TYPE_INDEX:
            // buffer = generator_c_expression_index(generator, ast);
            // buffer_append(temp, buffer);
            // buffer_destroy(buffer);
            break;

        case AST_EXPRESSION_TYPE_CALL:
            // buffer = generator_c_expression_call(generator, ast);
            // buffer_append(temp, buffer);
            // buffer_destroy(buffer);
            break;

        default:
            log_fatal("Unknown expression item type: %d\n", expression_item.type);
            break;
    }

    return temp;
}
