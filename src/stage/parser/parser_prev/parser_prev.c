#include <stage/parser/parser_prev/parser_prev.h>

void parser_prev(parser_t* parser) {
    DEBUG_ME;
    if (!parser) return;
    if (!PARSER_HAS_PREV) {
        log_fatal("Error: no previous token\n");
    }

    parser->index--;
}
