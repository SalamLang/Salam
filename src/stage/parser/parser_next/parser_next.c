#include <stage/parser/parser_next/parser_next.h>

void parser_next(parser_t* parser) {
    DEBUG_ME;
    if (!parser) return;
    if (!PARSER_HAS_NEXT) {
        log_fatal("Error: no next token\n");
    }

    parser->index++;
}
