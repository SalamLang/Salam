#include "ast_statement_variable_decl_direct_destroy.h"

void ast_statement_variable_decl_direct_destroy(ast_statement_variable_decl_t statement_variable_decl)
{
    DEBUG_ME;
    if (statement_variable_decl.name != NULL) {
        memory_destroy(statement_variable_decl.name);
    }

    if (statement_variable_decl.type != NULL) {
        ast_destroy(statement_variable_decl.type);
    }

    if (statement_variable_decl.value != NULL) {
        ast_destroy(statement_variable_decl.value);
    }
}
