#include "ast_expression_index_direct_destroy.h"

void ast_expression_index_direct_destroy(ast_expression_index_t expression_index)
{
    DEBUG_ME;
    if (expression_index.operand != NULL) {
        ast_destroy(expression_index.operand);
    }

    if (expression_index.runtime_type != NULL) {
        ast_type_destroy(expression_index.runtime_type);
    }
}
