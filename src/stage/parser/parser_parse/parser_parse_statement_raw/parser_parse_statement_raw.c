#include "parser_parse_statement_raw.h"

ast_t* parser_parse_statement_raw(parser_t* parser) {
    DEBUG_ME;
    if (!parser) return NULL;

    token_t* token = PARSER_CURRENT;

    parser_expect(parser, TOKEN_TYPE_KEYWORD_RAW);

    return ast_statement_raw_create(token);
}
