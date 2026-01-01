#include "ast_statement_raw_create.h"

ast_t* ast_statement_raw_create(token_t* token) {
    DEBUG_ME;
    if (!token) return NULL;

    ast_t* ast = ast_alloc(AST_TYPE_STATEMENT_RAW);
    ast->raw.statement_raw_value.value = string_duplicate(token->source);

    return ast;
}
