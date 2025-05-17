#include "ast_statement_if_direct_destroy.h"

void ast_statement_if_direct_destroy(ast_statement_if_t statement_if)
{
    DEBUG_ME;
    if (statement_if.condition != NULL) {
        ast_destroy(statement_if.condition);
    }

    if (statement_if.then_branch != NULL) {
        ast_destroy(statement_if.then_branch);
    }

    if (statement_if.else_branch != NULL) {
        ast_destroy(statement_if.else_branch);
    }
}
