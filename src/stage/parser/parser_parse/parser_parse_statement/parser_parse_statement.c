#include <stage/parser/parser_parse/parser_parse_statement/parser_parse_statement.h>

ast_t* parser_parse_statement(parser_t* parser)
{
    DEBUG_ME;
    if (!parser) return NULL;

    log_error("Unknown token type: %s\n", token_name(PARSER_CURRENT->type));
    PARSER_NEXT;
    return NULL;
}
