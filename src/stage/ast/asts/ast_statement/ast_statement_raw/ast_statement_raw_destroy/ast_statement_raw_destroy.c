#include "ast_statement_print_destroy.h"

void ast_statement_raw_destroy(ast_statement_raw_t* statement_raw)
{
    DEBUG_ME;
    if (!statement_raw) return;

    if (statement_raw->value != NULL) {
        memory_destroy(statement_raw->value);
    }

    memory_destroy(statement_raw);
}
