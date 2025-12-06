#include "ast_expression_index_destroy.h"

void ast_expression_index_destroy(ast_expression_index_t* expression_index) {
    DEBUG_ME;
    if (!expression_index) return;

    if (expression_index->object != NULL) {
        ast_destroy(expression_index->object);
    }

    if (expression_index->index != NULL) {
        ast_destroy(expression_index->index);
    }

    if (expression_index->runtime_type != NULL) {
        ast_type_destroy(expression_index->runtime_type);
    }

    memory_destroy(expression_index);
}
