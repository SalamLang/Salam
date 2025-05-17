#include <stage/parser/parser_expect/parser_expect.h>

void parser_operator_expect(parser_t* parser, token_operator_type_t operator_type)
{
    DEBUG_ME;
    if (!parser) return;

    if (!PARSER_HAS_CURRENT) {
        log_fatal("Error: no current token\n");
    }
    else if (PARSER_CURRENT->type != TOKEN_TYPE_OPERATOR) {
        log_fatal("Error: expected operator type '%s', but got '%s' token which is not an operator\n",
                  token_operator_name(operator_type), token_name(PARSER_CURRENT->type));
    }
    else if (PARSER_CURRENT->type == TOKEN_TYPE_OPERATOR && PARSER_CURRENT->operator_type != operator_type) {
        log_fatal("Error: expected operator type '%s', but got '%s'\n",
                  token_operator_name(operator_type), token_operator_name(PARSER_CURRENT->operator_type));
    }
    else {
        parser_next(parser);
    }
}
