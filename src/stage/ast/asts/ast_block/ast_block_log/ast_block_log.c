#include <stage/ast/asts/ast_block/ast_block_log/ast_block_log.h>

void ast_block_log(ast_block_t* block)
{
    if (!block) {
        log_info("Block is NULL");
        return;
    }

    log_info("Block:");
    log_info("Statement count: %zu", block->statement_count);
}
