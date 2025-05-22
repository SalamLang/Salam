#include "ast_statement_foreach_direct_destroy.h"

void ast_statement_foreach_direct_destroy(ast_statement_foreach_t statement_foreach)
{
    DEBUG_ME;
    if (statement_foreach.loop_variable != NULL) {
        memory_destroy(statement_foreach.loop_variable);
    }

    if (statement_foreach.iterable != NULL) {
        ast_destroy(statement_foreach.iterable);
    }

    if (statement_foreach.type != NULL) {
        ast_destroy(statement_foreach.type);
    }

    if (statement_foreach.block != NULL) {
        ast_destroy(statement_foreach.block);
    }
}
