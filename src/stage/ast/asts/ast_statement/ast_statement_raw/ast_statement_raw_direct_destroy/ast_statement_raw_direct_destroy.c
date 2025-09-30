#include "ast_statement_raw_direct_destroy.h"

void ast_statement_raw_direct_destroy(ast_statement_raw_t statement_raw) {
    DEBUG_ME;
    if (statement_raw.value != NULL) {
        memory_destroy(statement_raw.value);
    }
}
