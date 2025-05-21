#include "ast_expression_item_create_from.h"

ast_t* ast_expression_item_create_from(ast_t* from)
{
    DEBUG_ME;
    if (!from) return NULL;

    if (from->base.type == AST_TYPE_EXPRESSION_ITEM) return from;

    ast_t* ast = ast_create(AST_TYPE_EXPRESSION_ITEM);

    switch (from->base.type)
    {
        case AST_TYPE_EXPRESSION_LITERAL:
            ast->raw.expression_item_value.raw.literal = from->raw.expression_literal_value;
            break;

        case AST_TYPE_EXPRESSION_IDENTIFIER:
            ast->raw.expression_item_value.raw.identifier = from->raw.expression_identifier_value;
            break;

        case AST_TYPE_EXPRESSION_BINARY:
            ast->raw.expression_item_value.raw.binary = from->raw.expression_binary_value;
            break;

        case AST_TYPE_EXPRESSION_UNARY:
            ast->raw.expression_item_value.raw.unary = from->raw.expression_unary_value;
            break;

        case AST_TYPE_EXPRESSION_INDEX:
            ast->raw.expression_item_value.raw.index = from->raw.expression_index_value;
            break;

        case AST_TYPE_EXPRESSION_CALL:
            ast->raw.expression_item_value.raw.call = from->raw.expression_call_value;
            break;

        default:
            log_fatal("Unsupported expression type: %d", from->base.type);
            break;
    }

    return ast;
}
