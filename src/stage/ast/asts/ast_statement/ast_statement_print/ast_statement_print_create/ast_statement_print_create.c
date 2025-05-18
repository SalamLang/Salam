#include "ast_statement_print_create.h"

ast_t* ast_statement_print_create(ast_t* values, size_t value_count)
{
    DEBUG_ME;
    if (!values) return NULL;

    ast_t* ast = ast_alloc(AST_TYPE_STATEMENT_PRINT);
    ast->raw.statement_print_value.value_count = value_count;
    ast->raw.statement_print_value.values = values;

    return ast;
}
