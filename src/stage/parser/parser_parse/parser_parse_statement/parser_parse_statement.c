#include <stage/parser/parser_parse/parser_parse_statement/parser_parse_statement.h>

ast_t* parser_parse_statement(parser_t* parser)
{
    DEBUG_ME;
    if (!parser) return NULL;

    switch (PARSER_CURRENT->type) {
        case TOKEN_TYPE_KEYWORD_ENUM:
        case TOKEN_TYPE_KEYWORD_UNION:
        case TOKEN_TYPE_KEYWORD_STRUCT:
        case TOKEN_TYPE_KEYWORD_PACKAGE:
        case TOKEN_TYPE_KEYWORD_IMPORT:
        case TOKEN_TYPE_KEYWORD_FN:
            log_fatal("Cannot write this statement inside a sub block, you need to define it in the main block of the file");
            return NULL;

        case TOKEN_TYPE_KEYWORD_TYPE:
            return parser_parse_variable_decl(parser);

        case TOKEN_TYPE_KEYWORD_PRINT:
            return parser_parse_statement_print(parser);

        case TOKEN_TYPE_KEYWORD_RAW:
            return parser_parse_statement_raw(parser);

        case TOKEN_TYPE_KEYWORD_RET:
            return parser_parse_statement_return(parser);

        default:
            break;
    }

    log_error("Unknown token type: %s\n", token_name(PARSER_CURRENT->type));
    PARSER_NEXT;
    return NULL;
}
