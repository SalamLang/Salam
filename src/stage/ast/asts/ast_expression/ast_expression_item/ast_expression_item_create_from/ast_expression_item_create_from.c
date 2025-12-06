#include "ast_expression_item_create_from.h"

ast_t* ast_expression_item_create_from(ast_t* from) {
    DEBUG_ME;
    if (!from) return NULL;

    if (from->base.type == AST_TYPE_EXPRESSION_ITEM) return from;

    ast_t* ast;

    switch (from->base.type) {
        case AST_TYPE_EXPRESSION_LITERAL:
            ast = ast_expression_item_create(AST_EXPRESSION_TYPE_LITERAL, NULL);
            ast->raw.expression_item_value.raw.literal =
                from->raw.expression_literal_value;
            return ast;
            break;

        case AST_TYPE_EXPRESSION_IDENTIFIER:
            ast = ast_expression_item_create(AST_EXPRESSION_TYPE_IDENTIFIER,
                                             NULL);
            ast->raw.expression_item_value.raw.identifier =
                from->raw.expression_identifier_value;
            return ast;
            break;

        case AST_TYPE_EXPRESSION_BINARY:
            ast = ast_expression_item_create(AST_EXPRESSION_TYPE_BINARY, NULL);
            ast->raw.expression_item_value.raw.binary =
                from->raw.expression_binary_value;
            return ast;
            break;

        case AST_TYPE_EXPRESSION_UNARY:
            ast = ast_expression_item_create(AST_EXPRESSION_TYPE_UNARY, NULL);
            ast->raw.expression_item_value.raw.unary =
                from->raw.expression_unary_value;
            return ast;
            break;

        case AST_TYPE_EXPRESSION_INDEX:
            ast = ast_expression_item_create(AST_EXPRESSION_TYPE_INDEX, NULL);
            ast->raw.expression_item_value.raw.index =
                from->raw.expression_index_value;
            return ast;
            break;

        case AST_TYPE_EXPRESSION_CALL:
            ast = ast_expression_item_create(AST_EXPRESSION_TYPE_CALL, NULL);
            ast->raw.expression_item_value.raw.call =
                from->raw.expression_call_value;
            return ast;
            break;

        default:
            log_fatal("Unsupported expression type: %d", from->base.type);
            break;
    }

    return NULL;
}
