#include "ast_statement_expression_direct_destroy.h"

void ast_statement_expression_direct_destroy(ast_statement_expression_t statement_expression)
{
    DEBUG_ME;
    if (statement_expression.value != NULL) {
        ast_destroy(statement_expression.value);
    }
}
