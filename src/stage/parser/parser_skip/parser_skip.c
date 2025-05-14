#include <stage/parser/parser_skip/parser_skip.h>

bool parser_skip(parser_t* parser, token_type_t type)
{
    if (!parser) return false;
    if (!PARSER_HAS_CURRENT) return false;

    if (PARSER_CURRENT->type == type) {
        parser_next(parser);
        return true;
    }

    return false;
}
