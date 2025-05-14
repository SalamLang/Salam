#include <stage/ast/asts/ast_block/ast_block_destroy/ast_block_destroy.h>

void ast_block_destroy(ast_block_t* block)
{
    // if (!block) return;

    array_destroy(block->statements);
    memory_destroy(block);
}
