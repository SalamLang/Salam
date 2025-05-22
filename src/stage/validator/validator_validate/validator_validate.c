#include "validator_validate.h"

void validator_validate(ast_t* ast)
{
    DEBUG_ME;
    if (!ast) return;

    switch (ast->base.type) {
        case AST_TYPE_PARAMETER_ITEM:
            DEBUG_ME;
            validator_validate(ast->raw.parameter_item_value.type);
            validator_validate(ast->raw.parameter_item_value.default_value);
            break;

        case AST_TYPE_PARAMETERS:
            DEBUG_ME;
            for (size_t i = 0; i < ast->raw.parameters_value.values->size; i++) {
                ast_t* value = array_get(ast->raw.parameters_value.values, i);
                validator_validate(value);
            }
            break;

        case AST_TYPE_ATTRIBUTE_ITEM:
            DEBUG_ME;
            validator_validate(ast->raw.attribute_item_value.value);
            break;

        case AST_TYPE_ATTRIBUTES:
            DEBUG_ME;
            for (size_t i = 0; i < ast->raw.attributes_value.values->size; i++) {
                ast_t* value = array_get(ast->raw.attributes_value.values, i);
                validator_validate(value);
            }
            break;

        case AST_TYPE_ARGUMENT_ITEM:
            DEBUG_ME;
            validator_validate(ast->raw.attribute_item_value.value);
            break;

        case AST_TYPE_ARGUMENTS:
            DEBUG_ME;
            for (size_t i = 0; i < ast->raw.arguments_value.values->size; i++) {
                ast_t* value = array_get(ast->raw.arguments_value.values, i);
                validator_validate(value);
            }
            break;

        case AST_TYPE_PROGRAM:
            DEBUG_ME;
            for (size_t i = 0; ast->raw.program_value.variable_declarations->size > i; i++) {
                ast_t* variable_decl = array_get(ast->raw.program_value.variable_declarations, i);
                validator_validate(variable_decl);
            }

            for (size_t i = 0; ast->raw.program_value.function_declarations->size > i; i++) {
                ast_t* function_decl = array_get(ast->raw.program_value.function_declarations, i);
                validator_validate(function_decl);
            }
            break;

        case AST_TYPE_VARIABLE_DECL:
            DEBUG_ME;
            validator_validate_type(ast->raw.variable_decl_value.type);
            validator_validate(ast->raw.variable_decl_value.value);
            break;

        case AST_TYPE_FUNCTION_DECL:
            DEBUG_ME;
            validator_validate(ast->raw.function_decl_value.parameters);
            validator_validate(ast->raw.function_decl_value.block);
            validator_validate_type(ast->raw.function_decl_value.return_type);
            break;

        default:
            DEBUG_ME;
            log_fatal("Unknown AST type in validator_validate: %d\n", ast->base.type);
            break;
    }
}
