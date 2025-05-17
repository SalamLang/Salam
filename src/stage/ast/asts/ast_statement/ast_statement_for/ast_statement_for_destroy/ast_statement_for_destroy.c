#include "ast_statement_for_destroy.h"

void ast_statement_for_destroy(ast_statement_for_t* statement_for)
{
    DEBUG_ME;
    if (!statement_for) return;

    if (statement_for->initializer != NULL) {
        ast_destroy(statement_for->initializer);
    }

    if (statement_for->condition != NULL) {
        ast_destroy(statement_for->condition);
    }

    if (statement_for->increment != NULL) {
        ast_destroy(statement_for->increment);
    }

    if (statement_for->block != NULL) {
        ast_destroy(statement_for->block);
    }

    memory_destroy(statement_for);
}
