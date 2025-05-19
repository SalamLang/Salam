#include <stage/parser/parser_operator_skip/parser_operator_skip.h>

bool parser_operator_skip(parser_t* parser, token_operator_type_t operator_type)
{
    DEBUG_ME;
    if (!parser) return false;

    if (!PARSER_HAS_CURRENT) return false;
    else if (PARSER_CURRENT->type != TOKEN_TYPE_OPERATOR) return false;
    else if (PARSER_CURRENT->type == TOKEN_TYPE_OPERATOR && PARSER_CURRENT->operator_type == operator_type) {
        parser_next(parser);
        return true;
    }

    return false;
}
