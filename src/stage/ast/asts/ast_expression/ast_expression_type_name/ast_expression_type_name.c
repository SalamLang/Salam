#include "ast_expression_type_name.h"

char* ast_expression_type_name(ast_expression_type_t type) {
    switch (type) {
        case AST_EXPRESSION_TYPE_LITERAL:
            return "literal";
            break;
        case AST_EXPRESSION_TYPE_IDENTIFIER:
            return "identifier";
            break;
        case AST_EXPRESSION_TYPE_BINARY:
            return "binary";
            break;
        case AST_EXPRESSION_TYPE_UNARY:
            return "unary";
            break;
        case AST_EXPRESSION_TYPE_INDEX:
            return "index";
            break;
        case AST_EXPRESSION_TYPE_CALL:
            return "call";
            break;
    }
    return "EXPRESSION_TYPE_UNKNOWN";
}
