#include <stage/ast/ast_block/ast_block_destroy/ast_block_destroy.h>

void ast_block_direct_destroy(ast_block_t block)
{
    for (size_t i = 0; i < block.statement_count; ++i)
        ast_destroy(block.statements[i]);
    memory_destroy(block.statements);
}
