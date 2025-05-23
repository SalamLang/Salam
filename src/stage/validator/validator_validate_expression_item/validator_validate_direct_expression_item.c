#include "validator_validate_direct_expression_item.h"

ast_expression_item_t validator_validate_direct_expression_item(ast_expression_item_t expression_item)
{
    DEBUG_ME;
    if (expression_item.runtime_type != NULL) {
        validator_validate_type(expression_item.runtime_type);
    }

    switch (expression_item.type) {
        case AST_EXPRESSION_TYPE_LITERAL:
            expression_item.raw.literal.runtime_type = ast_type_exact_create_primitive(expression_item.raw.literal.value->type, false, false, false);
            expression_item.runtime_type = ast_type_exact_create_primitive(expression_item.raw.literal.value->type, false, false, false);
            break;

        case AST_EXPRESSION_TYPE_IDENTIFIER:
            expression_item.raw.identifier.runtime_type = ast_type_exact_create_primitive(AST_KIND_TYPE_INT, false, false, false);
            expression_item.runtime_type = ast_type_exact_create_primitive(AST_KIND_TYPE_INT, false, false, false);
            break;

        case AST_EXPRESSION_TYPE_BINARY:
            expression_item.runtime_type = expression_item.raw.binary.runtime_type;
            break;

        case AST_EXPRESSION_TYPE_UNARY:
            expression_item.runtime_type = expression_item.raw.unary.runtime_type;
            break;

        case AST_EXPRESSION_TYPE_INDEX:
            expression_item.runtime_type = expression_item.raw.index.runtime_type;
            break;

        case AST_EXPRESSION_TYPE_CALL:
            expression_item.runtime_type = expression_item.raw.call.runtime_type;
            break;
    }
    return expression_item;
}
