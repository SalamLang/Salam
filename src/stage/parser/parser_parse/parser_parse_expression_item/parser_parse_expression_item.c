#include "parser_parse_expression_item.h"

ast_t* parser_parse_expression_item(parser_t* parser)
{
    if (PARSER_CURRENT->type == TOKEN_TYPE_VALUE_NUMBER_INT) {
        value_t* value = value_create(VALUE_TYPE_NUMBER_INT);
        token->value->raw.int_value = PARSER_CURRENT->value->raw.int_value;
        return ast_expression_literal_create(value, NULL);
    }
    else if (PARSER_CURRENT->type == TOKEN_TYPE_VALUE_NUMBER_FLOAT) {
        value_t* value = value_create(VALUE_TYPE_NUMBER_FLOAT);
        token->value->raw.float_value = PARSER_CURRENT->value->raw.float_value;
        return ast_expression_literal_create(value, NULL);
    }
    else if (PARSER_CURRENT->type == TOKEN_TYPE_VALUE_STRING) {
        value_t* value = value_create(VALUE_TYPE_STRING);
        token->value->raw.int_value = PARSER_CURRENT->value->raw.int_value;
        return ast_expression_literal_create(value, NULL);
    }
    else if (PARSER_CURRENT->type == TOKEN_TYPE_VALUE_TRUE) {
        value_t* value = value_create(VALUE_TYPE_BOOL);
        token->value->raw.bool_value = PARSER_CURRENT->value->raw.bool_value;
        return ast_expression_literal_create(value, NULL);
    }
    else if (PARSER_CURRENT->type == TOKEN_TYPE_VALUE_FALSE) {
        value_t* value = value_create(VALUE_TYPE_NUMBER_INT);
        token->value->raw.bool_value = PARSER_CURRENT->value->raw.bool_value;
        return ast_expression_literal_create(value, NULL);
    }
    else if (PARSER_CURRENT->type == TOKEN_TYPE_VALUE_NULL) {
        value_t* value = value_create(VALUE_TYPE_NULL);
        return ast_expression_literal_create(value, NULL);
    }
    else {
        fatal_error("Expected a value, got: %s\n", token_name(PARSER_CURRENT->type));
    }
    return NULL;
}
