#include "validator_validate_direct_expression_item.h"

ast_expression_item_t validator_validate_direct_expression_item(ast_expression_item_t expression_item)
{
    DEBUG_ME;
    if (expression_item.runtime_type != NULL) {
        validator_validate_type(expression_item.runtime_type);
    }

    ast_type_type_t type;
    switch (expression_item.type) {
        case AST_EXPRESSION_TYPE_LITERAL:
            switch (expression_item.raw.literal.value->type) {
                case VALUE_TYPE_STRING:
                    type = AST_KIND_TYPE_STRING;
                    break;
                case VALUE_TYPE_NUMBER_INT:
                    type = AST_KIND_TYPE_INT;
                    break;
                case VALUE_TYPE_NUMBER_FLOAT:
                    type = AST_KIND_TYPE_FLOAT;
                    break;
                case VALUE_TYPE_BOOL:
                    type = AST_KIND_TYPE_BOOL;
                    break;
                case VALUE_TYPE_SIZE:
                    type = AST_KIND_TYPE_SIZE;
                    break;
                case VALUE_TYPE_NULL:
                    type = AST_KIND_TYPE_VOID;
                    break;
                default:
                    log_fatal("Unknown value type in validator_validate_direct_expression_item: %d\n", expression_item.raw.literal.value->type);
                    break;
            }
            expression_item.raw.literal.runtime_type = ast_type_exact_create_primitive(type, false, false, false);
            expression_item.runtime_type = ast_type_exact_create_primitive(type, false, false, false);
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
