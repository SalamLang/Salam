#include "ast_expressions_direct_destroy.h"

void ast_expressions_direct_destroy(ast_expressions_t expressions) {
    DEBUG_ME;
    if (expressions.values != NULL) {
        array_destroy(expressions.values);
    }
}
