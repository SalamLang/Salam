#include "validator_validate.h"

void validator_validate(ast_t* ast) {
    DEBUG_ME;
    if (!ast) return;

    switch (ast->base.type) {
        case AST_TYPE_UNKNOWN:
            DEBUG_ME;
            break;

        case AST_TYPE_TYPE:
            DEBUG_ME;
            validator_validate_direct_type(ast->raw.type_value);
            break;

        case AST_TYPE_PROGRAM:
            DEBUG_ME;
            for (size_t i = 0;
                 i < ast->raw.program_value.variable_declarations->size; i++) {
                validator_validate(
                    array_get(ast->raw.program_value.variable_declarations, i));
            }
            for (size_t i = 0;
                 i < ast->raw.program_value.function_declarations->size; i++) {
                validator_validate(
                    array_get(ast->raw.program_value.function_declarations, i));
            }
            break;

        case AST_TYPE_PACKAGE:
            DEBUG_ME;
            break;

        case AST_TYPE_IMPORT:
            DEBUG_ME;
            break;

        case AST_TYPE_KIND_DECL:
            DEBUG_ME;
            validator_validate(ast->raw.kind_decl_value.value);
            break;

        case AST_TYPE_KIND_STRUCT:
            DEBUG_ME;
            validator_validate(ast->raw.kind_struct_value.functions);
            validator_validate(ast->raw.kind_struct_value.parameters);
            break;

        case AST_TYPE_KIND_ENUM:
            DEBUG_ME;
            break;

        case AST_TYPE_KIND_UNION:
            DEBUG_ME;
            break;

        case AST_TYPE_EXTERN_DECL:
            DEBUG_ME;
            validator_validate(ast->raw.extern_decl_value.value);
            break;

        case AST_TYPE_EXTERN_VARIABLE:
            DEBUG_ME;
            validator_validate(ast->raw.extern_variable_value.type);
            break;

        case AST_TYPE_EXTERN_FUNCTION:
            DEBUG_ME;
            validator_validate(ast->raw.extern_function_value.type);
            validator_validate(ast->raw.extern_function_value.attributes);
            break;

        case AST_TYPE_VARIABLE_DECL:
            DEBUG_ME;
            validator_validate(ast->raw.variable_decl_value.type);
            validator_validate(ast->raw.variable_decl_value.value);
            validator_validate(ast->raw.variable_decl_value.value);
            break;

        case AST_TYPE_FUNCTION_DECL:
            DEBUG_ME;
            validator_validate(ast->raw.function_decl_value.parameters);
            validator_validate(ast->raw.function_decl_value.block);
            validator_validate(ast->raw.function_decl_value.return_type);
            break;

        case AST_TYPE_BLOCK:
            DEBUG_ME;
            for (size_t i = 0; i < ast->raw.block_value.statements->size; i++) {
                validator_validate(
                    array_get(ast->raw.block_value.statements, i));
            }
            break;

        case AST_TYPE_PARAMETER_ITEM:
            DEBUG_ME;
            validator_validate(ast->raw.parameter_item_value.type);
            validator_validate(ast->raw.parameter_item_value.default_value);
            break;

        case AST_TYPE_PARAMETERS:
            DEBUG_ME;
            for (size_t i = 0; i < ast->raw.parameters_value.values->size;
                 i++) {
                validator_validate(
                    array_get(ast->raw.parameters_value.values, i));
            }
            break;

        case AST_TYPE_ATTRIBUTE_ITEM:
            DEBUG_ME;
            validator_validate(ast->raw.attribute_item_value.value);
            break;

        case AST_TYPE_ATTRIBUTES:
            DEBUG_ME;
            for (size_t i = 0; i < ast->raw.attributes_value.values->size;
                 i++) {
                validator_validate(
                    array_get(ast->raw.attributes_value.values, i));
            }
            break;

        case AST_TYPE_ARGUMENT_ITEM:
            DEBUG_ME;
            validator_validate(ast->raw.argument_item_value.value);
            break;

        case AST_TYPE_ARGUMENTS:
            DEBUG_ME;
            for (size_t i = 0; i < ast->raw.arguments_value.values->size; i++) {
                validator_validate(
                    array_get(ast->raw.arguments_value.values, i));
            }
            break;

        case AST_TYPE_EXPRESSION_ITEM:
            DEBUG_ME;
            ast->raw.expression_item_value =
                validator_validate_direct_expression_item(
                    ast->raw.expression_item_value);
            break;

        case AST_TYPE_EXPRESSIONS:
            DEBUG_ME;
            for (size_t i = 0; i < ast->raw.expressions_value.values->size;
                 i++) {
                validator_validate(
                    array_get(ast->raw.expressions_value.values, i));
            }
            break;

        case AST_TYPE_EXPRESSION_LITERAL:
            DEBUG_ME;
            break;

        case AST_TYPE_EXPRESSION_IDENTIFIER:
            DEBUG_ME;
            break;

        case AST_TYPE_EXPRESSION_BINARY:
            DEBUG_ME;
            break;

        case AST_TYPE_EXPRESSION_UNARY:
            DEBUG_ME;
            break;

        case AST_TYPE_EXPRESSION_INDEX:
            DEBUG_ME;
            break;

        case AST_TYPE_EXPRESSION_CALL:
            DEBUG_ME;
            break;

        case AST_TYPE_STATEMENT_IF:
            DEBUG_ME;
            validator_validate(ast->raw.statement_if_value.condition);
            validator_validate(ast->raw.statement_if_value.then_branch);
            validator_validate(ast->raw.statement_if_value.else_branch);
            break;

        case AST_TYPE_STATEMENT_FOR:
            DEBUG_ME;
            validator_validate(ast->raw.statement_for_value.initializer);
            validator_validate(ast->raw.statement_for_value.condition);
            validator_validate(ast->raw.statement_for_value.increment);
            validator_validate(ast->raw.statement_for_value.block);
            break;

        case AST_TYPE_STATEMENT_FOREACH:
            DEBUG_ME;
            validator_validate(ast->raw.statement_foreach_value.iterable);
            validator_validate(ast->raw.statement_foreach_value.iterable);
            validator_validate(ast->raw.statement_foreach_value.block);
            break;

        case AST_TYPE_STATEMENT_RET:
            DEBUG_ME;
            validator_validate(ast->raw.statement_ret_value.values);
            break;

        case AST_TYPE_STATEMENT_PRINT:
            DEBUG_ME;
            validator_validate(ast->raw.statement_print_value.values);
            break;

        case AST_TYPE_STATEMENT_RAW:
            DEBUG_ME;
            break;

        case AST_TYPE_STATEMENT_VARIABLE_DECL:
            DEBUG_ME;
            validator_validate(ast->raw.statement_variable_decl_value.type);
            validator_validate(ast->raw.statement_variable_decl_value.value);
            break;

        case AST_TYPE_STATEMENT_EXPRESSION:
            DEBUG_ME;
            validator_validate(ast->raw.statement_expression_value.value);
            break;

        default:
            DEBUG_ME;
            log_fatal("Unknown AST type in validator_validate: %d\n",
                      ast->base.type);
            break;
    }
}
