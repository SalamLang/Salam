#include "ast_expression_literal_create.h"

ast_t* ast_expression_literal_create(value_t* value, ast_type_t* runtime_type)
{
    DEBUG_ME;
    ast_t* ast = ast_alloc(AST_TYPE_EXPRESSION_LITERAL);
    // ast->raw.expression_literal_value = memory_allocation(sizeof(ast_expression_literal_t));
    ast->raw.expression_literal_value.value = value;
    ast->raw.expression_literal_value.runtime_type = runtime_type;

    return ast;
}
