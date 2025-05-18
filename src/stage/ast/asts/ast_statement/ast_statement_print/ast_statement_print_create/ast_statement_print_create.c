#include "ast_statement_print_create.h"

ast_t* ast_statement_print_create(ast_t* expressions, size_t expression_count)
{
    DEBUG_ME;
    if (!expressions) return NULL;

    ast_t* ast = ast_aloc(AST_TYPE_STATEMENT_PRINT);
    ast->expression_count = expression_count;
    ast->expressions = expressions;

    return ast;
}
