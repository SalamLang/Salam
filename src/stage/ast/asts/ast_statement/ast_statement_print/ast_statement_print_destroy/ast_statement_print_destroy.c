#include "ast_statement_print_destroy.h"

void ast_statement_print_destroy(ast_statement_print_t* statement_print) {
    DEBUG_ME;
    if (!statement_print) return;

    if (statement_print->values != NULL) {
        ast_destroy(statement_print->values);
    }

    memory_destroy(statement_print);
}
