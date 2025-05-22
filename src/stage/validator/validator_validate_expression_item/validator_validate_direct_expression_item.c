#include "validator_validate_direct_expression_item.h"

void validator_validate_direct_expression_item(ast_expression_item_t expression_item)
{
    DEBUG_ME;
    if (expression_item.runtime_type != NULL) {
        validator_validate_type(expression_item.runtime_type);
    }
}
