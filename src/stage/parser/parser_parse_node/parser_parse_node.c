#include <stage/parser/parser_parse_node/parser_parse_node.h>

ast_t* parser_parse_node(parser_t* parser)
{
    if (!parser) return NULL;

    if (PARSER_CURRENT->type == TOKEN_TYPE_KEYWORD_FN) {
        return parser_parse_function_decl(parser);
    }

    log_error("Unknown token type: %s\n", token_name(PARSER_CURRENT->type));
    PARSER_NEXT;
    return NULL;
}
