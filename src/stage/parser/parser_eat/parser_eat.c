#include <stage/parser/parser_eat/parser_eat.h>

token_t* parser_eat(parser_t* parser)
{
    DEBUG_ME;
    if (!parser) return NULL;
    if (!PARSER_HAS_CURRENT) {
        log_fatal("Error: no current token\n");
        return NULL;
    }

    token_t* token = PARSER_CURRENT;

    parser_next(parser);

    return token;
}
