#include "ast_expression_identifier_direct_destroy.h"

void ast_expression_identifier_direct_destroy(
    ast_expression_identifier_t expression_identifier) {
    DEBUG_ME;
    if (expression_identifier.name != NULL) {
        memory_destroy(expression_identifier.name);
    }

    if (expression_identifier.runtime_type != NULL) {
        ast_type_destroy(expression_identifier.runtime_type);
    }
}
