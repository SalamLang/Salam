#include <stage/parser/parser_expect/parser_expect.h>

void parser_expect(parser_t* parser, token_type_t type)
{
    DEBUG_ME;
    if (!parser) return;
    if (!PARSER_HAS_CURRENT) {
        log_fatal("Error: no current token\n");
    }
    else if (PARSER_CURRENT->type != type) {
        log_fatal("Error: expected token type '%s', but got '%s'\n",
                  token_name(type), token_name(PARSER_CURRENT->type));
    }
    else {
        parser_next(parser);
    }
}
