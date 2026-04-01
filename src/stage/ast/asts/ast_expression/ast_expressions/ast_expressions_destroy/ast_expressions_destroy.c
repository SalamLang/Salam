#include "ast_expressions_destroy.h"

void ast_expressions_destroy(ast_expressions_t* expressions) {
    DEBUG_ME;
    if (!expressions) return;

    if (expressions->values != NULL) {
        array_destroy(expressions->values);
    }

    memory_destroy(expressions);
}
