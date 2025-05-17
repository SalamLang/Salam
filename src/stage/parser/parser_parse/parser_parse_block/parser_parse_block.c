#include <stage/parser/parser_parse/parser_parse_block/parser_parse_block.h>

ast_t* parser_parse_block(parser_t* parser)
{
    DEBUG_ME;
    if (!parser) return NULL;

    parser_operator_expect(parser, TOKEN_OPERATOR_TYPE_COLON);

    size_t statement_count = 0;
    array_t* statements = array_create((array_destroy_t)ast_destroy);

    while (PARSER_HAS_CURRENT && PARSER_CURRENT->type != TOKEN_TYPE_KEYWORD_END) {
        ast_t* statement = parser_parse_statement(parser);
        if (statement) {
            array_append(statements, statement);
            statement_count++;
        }
    }

    parser_expect(parser, TOKEN_TYPE_KEYWORD_END);

    ast_t* ast = ast_block_create(statements, statement_count);
    return ast;
}
