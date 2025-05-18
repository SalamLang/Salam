#include <stage/ast/asts/ast_block/ast_block_log/ast_block_log.h>

void ast_block_log(ast_block_t* block)
{
    if (!block) {
        log_info("Block is NULL\n");
        return;
    }

    log_info("Block:\n");
    log_info("Statement count: %zu\n", block->statement_count);
}
