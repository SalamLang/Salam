#include "parser_parse_expression_literal.h"

ast_t* parser_parse_expression_literal(parser_t* parser) {
    DEBUG_ME;
    if (!parser) return NULL;

    if (PARSER_CURRENT->type == TOKEN_TYPE_VALUE_NUMBER_INT) {
        value_t* value = value_create(VALUE_TYPE_NUMBER_INT);
        value->raw.int_value = PARSER_CURRENT->value->raw.int_value;
        PARSER_NEXT;
        return ast_expression_literal_create(value, NULL);
    } else if (PARSER_CURRENT->type == TOKEN_TYPE_VALUE_NUMBER_FLOAT) {
        value_t* value = value_create(VALUE_TYPE_NUMBER_FLOAT);
        value->raw.float_value = PARSER_CURRENT->value->raw.float_value;
        PARSER_NEXT;
        return ast_expression_literal_create(value, NULL);
    } else if (PARSER_CURRENT->type == TOKEN_TYPE_VALUE_STRING) {
        value_t* value = value_create(VALUE_TYPE_STRING);
        value->raw.string_value =
            string_duplicate(PARSER_CURRENT->value->raw.string_value);
        PARSER_NEXT;
        return ast_expression_literal_create(value, NULL);
    } else if (PARSER_CURRENT->type == TOKEN_TYPE_VALUE_TRUE ||
               PARSER_CURRENT->type == TOKEN_TYPE_VALUE_FALSE) {
        value_t* value = value_create(VALUE_TYPE_BOOL);
        value->raw.bool_value = PARSER_CURRENT->value->raw.bool_value;
        PARSER_NEXT;
        return ast_expression_literal_create(value, NULL);
    } else if (PARSER_CURRENT->type == TOKEN_TYPE_VALUE_NULL) {
        value_t* value = value_create(VALUE_TYPE_NULL);
        PARSER_NEXT;
        return ast_expression_literal_create(value, NULL);
    } else if (PARSER_CURRENT->type == TOKEN_TYPE_IDENTIFIER) {
        char* name = PARSER_CURRENT->source;
        PARSER_NEXT;
        return ast_expression_identifier_create(name, NULL);
    }

    log_fatal("parser_parse_expression_literal - Expected a value, got: %s\n",
              token_name(PARSER_CURRENT->type));
    return NULL;
}
