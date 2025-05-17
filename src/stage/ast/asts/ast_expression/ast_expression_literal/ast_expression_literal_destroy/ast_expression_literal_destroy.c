#include "ast_expression_literal_destroy.h"

void ast_expression_literal_destroy(ast_expression_literal_t* expression_literal)
{
    DEBUG_ME;
    if (!expression_literal) return;

    if (expression_literal->values != NULL) {
        value_destroy(expression_literal->value);
    }

    if (expression_literal->runtime_type != NULL) {
        ast_type_destroy(expression_literal->runtime_type);
    }

    memory_destroy(expression_literal);
}
