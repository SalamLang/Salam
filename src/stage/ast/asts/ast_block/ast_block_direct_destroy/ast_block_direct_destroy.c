#include <stage/ast/asts/ast_block/ast_block_destroy/ast_block_destroy.h>

void ast_block_direct_destroy(ast_block_t block) {
    DEBUG_ME;
    // if (! block) return;

    array_destroy(block.statements);
}
