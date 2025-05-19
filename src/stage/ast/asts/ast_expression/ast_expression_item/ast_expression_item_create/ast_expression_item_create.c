#include "ast_expression_item_create.h"

ast_t* ast_expression_item_create(ast_expression_type_t type, ast_type_t* runtime_type)
{
    DEBUG_ME;
    ast_t* ast = ast_alloc(AST_TYPE_EXPRESSION_ITEM);
    ast->raw.expression_item_value.type = type;
    ast->raw.expression_item_value.runtime_type = runtime_type;

    return ast;
}
