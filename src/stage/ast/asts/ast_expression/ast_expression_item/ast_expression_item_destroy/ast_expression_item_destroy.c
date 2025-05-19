#include "ast_expression_item_destroy.h"

void ast_expression_item_destroy(ast_expression_item_t* expression_item)
{
    DEBUG_ME;
    if (!expression_item) return;

    // raw.literal
    if (expression_item->type == AST_EXPRESSION_TYPE_LITERAL && expression_item->raw.literal != NULL) {
        ast_expression_item_destroy(expression_item->raw.literal);
    }

    // raw.identifier
    if (expression_item->type == AST_EXPRESSION_TYPE_IDENTIFIER && expression_item->raw.identifier != NULL) {
        ast_expression_identifier_destroy(expression_item->raw.identifier);
    }

    // raw.binary
    if (expression_item->type == AST_EXPRESSION_TYPE_BINARY && expression_item->raw.binary != NULL) {
        ast_expression_binary_destroy(expression_item->raw.binary);
    }

    // raw.unary
    if (expression_item->type == AST_EXPRESSION_TYPE_UNARY && expression_item->raw.unary != NULL) {
        ast_expression_unary_destroy(expression_item->raw.unary);
    }

    // raw.index
    if (expression_item->type == AST_EXPRESSION_TYPE_INDEX && expression_item->raw.index != NULL) {
        ast_expression_index_destroy(expression_item->raw.index);
    }

    // raw.call
    if (expression_item->type == AST_EXPRESSION_TYPE_CALL && expression_item->raw.call != NULL) {
        ast_expression_call_destroy(expression_item->raw.call);
    }

    // runtime_type
    if (expression_item->runtime_type != NULL) {
        ast_type_destroy(expression_item->runtime_type);
    }

    memory_destroy(expression_item);
}
