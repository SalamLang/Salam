#include <stage/parser/parser_parse_function_decl/parser_parse_function_decl.h>

ast_t* parser_parse_function_decl(parser_t* parser)
{
    DEBUG_ME;
    ast_t* ast = NULL;
    parser_expect(parser, TOKEN_TYPE_KEYWORD_FN);
    token_t* name = parser_expect_get(parser, TOKEN_TYPE_IDENTIFIER);

    ast = ast_function_decl_create(name->source, NULL, 0, NULL);
    return ast;
}
