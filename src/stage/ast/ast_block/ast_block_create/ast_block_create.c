#include <stage/ast/ast_block/ast_block_create/ast_block_create.h>

ast_t* ast_block_create(array_t* statements, size_t statement_count)
{
    ast_t* node = ast_alloc(AST_TYPE_BLOCK, ast_destroy);
    node->block.statements = statements;
    node->block.statement_count = statement_count;
    return node;
}
