#include "ast_expression_binary_destroy.h"

void ast_expression_binary_destroy(ast_expression_binary_t* expression_binary)
{
    DEBUG_ME;
    if (!expression_binary) return;

    if (expression_binary->op != NULL) {
        memory_destroy(expression_binary->op);
    }

    if (expression_binary->left != NULL) {
        ast_destroy(expression_binary->left);
    }

    if (expression_binary->right != NULL) {
        ast_destroy(expression_binary->right);
    }

    if (expression_binary->runtime_type != NULL) {
        ast_type_destroy(expression_binary->runtime_type);
    }

    memory_destroy(expression_binary);
}
