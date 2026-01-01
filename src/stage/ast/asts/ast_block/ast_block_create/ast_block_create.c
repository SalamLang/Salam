#include <stage/ast/asts/ast_block/ast_block_create/ast_block_create.h>

ast_t* ast_block_create(array_t* statements, size_t statement_count) {
    ast_t* ast = ast_alloc(AST_TYPE_BLOCK);
    ast->raw.block_value.statements = statements;
    ast->raw.block_value.statement_count = statement_count;
    return ast;
}
