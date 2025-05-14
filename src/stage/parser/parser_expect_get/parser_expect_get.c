#include <stage/parser/parser_expect_get/parser_expect_get.h>

token_t* parser_expect_get(parser_t* parser, token_type_t type)
{
    DEBUG_ME;
    if (!parser) return NULL;
    if (!PARSER_HAS_CURRENT) {
        log_fatal("Error: no current token\n");
        return NULL;
    }

    token_t* token = PARSER_CURRENT;
    parser_expect(parser, type);

    return token;
}
