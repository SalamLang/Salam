#include "ast_expression_unary_direct_destroy.h"

void ast_expression_unary_direct_destroy(ast_expression_unary_t expression_unary)
{
    DEBUG_ME;
    if (expression_unary.operand != NULL) {
        ast_destroy(expression_unary.operand);
    }

    if (expression_unary.runtime_type != NULL) {
        ast_type_destroy(expression_unary.runtime_type);
    }
}
