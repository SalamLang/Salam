#include "ast_expression_call_destroy.h"

void ast_expression_call_destroy(ast_expression_call_t* expression_call) {
    DEBUG_ME;
    if (!expression_call) return;

    if (expression_call->callee != NULL) {
        ast_destroy(expression_call->callee);
    }

    if (expression_call->args != NULL) {
        ast_destroy(expression_call->args);
    }

    if (expression_call->runtime_type != NULL) {
        ast_type_destroy(expression_call->runtime_type);
    }

    memory_destroy(expression_call);
}
