#include "ast_statement_return_direct_destroy.h"

void ast_statement_return_direct_destroy(ast_statement_return_t statement_return)
{
    DEBUG_ME;
    if (statement_return.values != NULL) {
        ast_destroy(statement_return.values);
    }
}
