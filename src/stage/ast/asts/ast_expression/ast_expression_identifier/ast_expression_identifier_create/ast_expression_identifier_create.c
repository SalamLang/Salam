#include "ast_expression_identifier_create.h"

ast_t* ast_expression_identifier_create(char* name, ast_type_t* runtime_type) {
    DEBUG_ME;
    ast_t* ast = ast_alloc(AST_TYPE_EXPRESSION_IDENTIFIER);
    // ast->raw.expression_literal_value =
    // memory_allocation(sizeof(ast_expression_literal_t));
    ast->raw.expression_identifier_value.name = string_duplicate(name);
    ast->raw.expression_identifier_value.runtime_type = runtime_type;

    return ast;
}
