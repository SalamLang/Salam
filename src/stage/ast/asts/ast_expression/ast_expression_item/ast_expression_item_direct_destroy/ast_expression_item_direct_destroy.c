#include "ast_expression_item_direct_destroy.h"

void ast_expression_item_direct_destroy(ast_expression_item_t expression_item)
{
    DEBUG_ME;
    switch (expression_item.type) {
        case AST_EXPRESSION_TYPE_LITERAL:
            ast_expression_literal_direct_destroy(expression_item.raw.literal);
            break;

        case AST_EXPRESSION_TYPE_IDENTIFIER:
            ast_expression_identifier_direct_destroy(expression_item.raw.identifier);
            break;

        case AST_EXPRESSION_TYPE_BINARY:
            ast_expression_binary_direct_destroy(expression_item.raw.binary);
            break;

        case AST_EXPRESSION_TYPE_UNARY:
            ast_expression_unary_direct_destroy(expression_item.raw.unary);
            break;

        case AST_EXPRESSION_TYPE_INDEX:
            ast_expression_index_direct_destroy(expression_item.raw.index);
            break;

        case AST_EXPRESSION_TYPE_CALL:
            ast_expression_call_direct_destroy(expression_item.raw.call);
            break;
    }

    // runtime_type
    if (expression_item.runtime_type != NULL) {
        ast_type_destroy(expression_item.runtime_type);
    }
}
