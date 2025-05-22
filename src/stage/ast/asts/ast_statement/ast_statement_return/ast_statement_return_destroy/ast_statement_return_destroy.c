#include "ast_statement_return_destroy.h"

void ast_statement_return_destroy(ast_statement_ret_t* statement_return)
{
    DEBUG_ME;
    if (!statement_return) return;

    if (statement_return->values != NULL) {
        ast_destroy(statement_return->values);
    }

    memory_destroy(statement_return);
}
