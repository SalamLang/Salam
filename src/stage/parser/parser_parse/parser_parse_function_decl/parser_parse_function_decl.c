#include <stage/parser/parser_parse_function_decl/parser_parse_function_decl.h>

ast_t* parser_parse_function_decl(parser_t* parser)
{
    DEBUG_ME;
    parser_expect(parser, TOKEN_TYPE_KEYWORD_FN);
    token_t* name = parser_expect_get(parser, TOKEN_TYPE_IDENTIFIER);
    ast_t* parameters = parser_parse_parameters(parser);
    ast_t* block = parser_parse_block(parser);

    ast_t* ast = ast_function_decl_create(name->source, parameters, block);
    return ast;
}
