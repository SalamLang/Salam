#include "ast_expressions_create.h"

ast_t* ast_expressions_create()
{
    DEBUG_ME;
    ast_t* ast = ast_alloc(AST_TYPE_EXPRESSIONS);
    ast->raw.expressions_value = memory_allocation(sizeof(ast_expressions_t));
    ast->raw.expressions_value->values = array_create((array_destroy_t)ast_destroy);
    ast->raw.expressions_value->value_count = 0;

    return ast;
}
