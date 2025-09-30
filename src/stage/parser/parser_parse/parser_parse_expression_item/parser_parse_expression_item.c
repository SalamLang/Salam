#include "parser_parse_expression_item.h"

ast_t* parser_parse_expression_item(parser_t* parser) {
    DEBUG_ME;
    if (!parser) return NULL;

    // TODO: only supporting literal value for now...
    ast_t* ast = parser_parse_expression_literal(parser);

    return ast_expression_item_create_from(ast);
}
