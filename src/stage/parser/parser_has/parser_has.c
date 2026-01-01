#include <stage/parser/parser_has/parser_has.h>

bool parser_has(parser_t* parser, token_type_t type) {
    DEBUG_ME;
    if (!parser) return false;
    if (!PARSER_HAS_CURRENT) return false;

    if (PARSER_CURRENT->type == type) {
        return true;
    }

    return false;
}
