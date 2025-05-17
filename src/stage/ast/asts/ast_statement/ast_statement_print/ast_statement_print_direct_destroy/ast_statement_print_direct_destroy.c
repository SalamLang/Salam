#include "ast_statement_print_direct_destroy.h"

void ast_statement_print_direct_destroy(ast_statement_print_t statement_print)
{
    DEBUG_ME;
    if (statement_print.value != NULL) {
        ast_destroy(statement_print.value);
    }
}
