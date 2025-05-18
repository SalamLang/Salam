#include "ast_statement_return_create.h"

ast_t* ast_statement_return_create(ast_t* values, size_t value_count)
{
    DEBUG_ME;
    if (!values) return NULL;

    ast_t* ast = ast_alloc(AST_TYPE_STATEMENT_RET);
    ast->raw.statement_return_value.values = values;
    ast->raw.statement_return_value.value_count = value_count;

    return ast;
}
