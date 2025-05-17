#include "ast_statement_expression_destroy.h"

void ast_statement_expression_destroy(ast_statement_expression_t* statement_expression)
{
    DEBUG_ME;
    if (!statement_expression) return;

    if (statement_expression.value != NULL) {
        ast_destroy(statement_expression.value);
    }

    memory_destroy(statement_expression);
}
