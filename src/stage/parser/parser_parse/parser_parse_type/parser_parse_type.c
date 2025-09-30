#include "parser_parse_type.h"

ast_t* parser_parse_type(parser_t* parser) {
    DEBUG_ME;
    if (!parser) return NULL;

    ast_t* type = ast_type_create_from_token(PARSER_CURRENT);
    if (type == NULL) {
        log_error("Failed to create type from token: %s\n",
                  token_name(PARSER_CURRENT->type));
        return NULL;
    }

    PARSER_NEXT;

    return type;
}
