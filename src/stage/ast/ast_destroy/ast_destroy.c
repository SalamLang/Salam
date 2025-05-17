#include <stage/ast/ast_destroy/ast_destroy.h>

void ast_destroy(ast_t* ast)
{
    DEBUG_ME;
    // if (!ast) return;

    switch (ast->base.type) {
        case AST_TYPE_PARAMETER_ITEM:
            ast_parameter_item_direct_destroy(ast->raw.parameter_item_value);
            break;
        case AST_TYPE_PARAMETERS:
            ast_parameters_direct_destroy(ast->raw.parameters_value);
            break;

        case AST_TYPE_ATTRIBUTE_ITEM:
            ast_attribute_direct_destroy(ast->raw.attribute_item_value);
            break;
        case AST_TYPE_ATTRIBUTES:
            ast_attributes_direct_destroy(ast->raw.attributes_value);
            break;

        case AST_TYPE_ARGUMENT_ITEM:
            ast_argument_direct_destroy(ast->raw.argument_item_value);
            break;
        case AST_TYPE_ARGUMENTS:
            ast_arguments_direct_destroy(ast->raw.arguments_value);
            break;

        case AST_TYPE_PROGRAM:
            ast_program_direct_destroy(ast->raw.program_value);
            break;

        case AST_TYPE_VARIABLE_DECL:
            ast_variable_decl_direct_destroy(ast->raw.variable_decl_value);
            break;

        case AST_TYPE_FUNCTION_DECL:
            ast_function_decl_direct_destroy(ast->raw.function_decl_value);
            break;

        case AST_TYPE_BLOCK:
            ast_block_direct_destroy(ast->raw.block_value);
            break;

        case AST_TYPE_UNKNOWN:
            break;
    }
    memory_destroy(ast);
}
