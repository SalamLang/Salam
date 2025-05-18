#include <stage/ast/ast_destroy/ast_destroy.h>

void ast_destroy(ast_t* ast)
{
    DEBUG_ME;
    if (!ast) return;

    switch (ast->base.type) {
        case AST_TYPE_PARAMETER_ITEM:
            ast_parameter_item_direct_destroy(ast->raw.parameter_item_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_PARAMETERS:
            ast_parameters_direct_destroy(ast->raw.parameters_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_ATTRIBUTE_ITEM:
            ast_attribute_item_direct_destroy(ast->raw.attribute_item_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_ATTRIBUTES:
            ast_attributes_direct_destroy(ast->raw.attributes_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_ARGUMENT_ITEM:
            ast_argument_item_direct_destroy(ast->raw.argument_item_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_ARGUMENTS:
            ast_arguments_direct_destroy(ast->raw.arguments_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_PROGRAM:
            ast_program_direct_destroy(ast->raw.program_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_VARIABLE_DECL:
            ast_variable_decl_direct_destroy(ast->raw.variable_decl_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_FUNCTION_DECL:
            ast_function_decl_direct_destroy(ast->raw.function_decl_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_BLOCK:
            ast_block_direct_destroy(ast->raw.block_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_PACKAGE:
            ast_package_direct_destroy(ast->raw.package_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_IMPORT:
            ast_import_direct_destroy(ast->raw.import_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_KIND_DECL:
            ast_kind_decl_direct_destroy(ast->raw.kind_decl_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_KIND_STRUCT:
            ast_kind_struct_direct_destroy(ast->raw.kind_struct_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_KIND_ENUM:
            ast_kind_enum_direct_destroy(ast->raw.kind_enum_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_KIND_UNION:
            ast_kind_union_direct_destroy(ast->raw.kind_union_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_EXTERN_DECL:
            ast_extern_decl_direct_destroy(ast->raw.extern_decl_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_EXTERN_VARIABLE:
            ast_extern_variable_direct_destroy(ast->raw.extern_variable_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_EXTERN_FUNCTION:
            ast_extern_function_direct_destroy(ast->raw.extern_function_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_EXPRESSION_ITEM:
            ast_expression_item_direct_destroy(ast->raw.expression_item_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_EXPRESSIONS:
            ast_expressions_direct_destroy(ast->raw.expressions_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_EXPRESSION_LITERAL:
            ast_expression_literal_direct_destroy(ast->raw.expression_literal_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_EXPRESSION_IDENTIFIER:
            ast_expression_identifier_direct_destroy(ast->raw.expression_identifier_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_EXPRESSION_BINARY:
            ast_expression_binary_direct_destroy(ast->raw.expression_binary_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_EXPRESSION_UNARY:
            ast_expression_unary_direct_destroy(ast->raw.expression_unary_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_EXPRESSION_INDEX:
            ast_expression_index_direct_destroy(ast->raw.expression_index_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_EXPRESSION_CALL:
            ast_expression_call_direct_destroy(ast->raw.expression_call_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_STATEMENT_IF:
            ast_statement_if_direct_destroy(ast->raw.statement_if_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_STATEMENT_FOR:
            ast_statement_for_direct_destroy(ast->raw.statement_for_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_STATEMENT_FOREACH:
            ast_statement_foreach_direct_destroy(ast->raw.statement_foreach_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_STATEMENT_RET:
            ast_statement_return_direct_destroy(ast->raw.statement_return_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_STATEMENT_PRINT:
            ast_statement_print_direct_destroy(ast->raw.statement_print_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_STATEMENT_VARIABLE_DECL:
            ast_statement_variable_decl_direct_destroy(ast->raw.statement_variable_decl_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_STATEMENT_EXPRESSION:
            ast_statement_expression_direct_destroy(ast->raw.statement_expression_value);
            memory_destroy(ast);
            return;
            break;

        case AST_TYPE_UNKNOWN:
            log_fatal("Unknown AST type in ast_destroy: %d\n", ast->base.type);
            memory_destroy(ast);
            return;
            break;
    }

    memory_destroy(ast);
    log_fatal("Unhandled AST type in ast_destroy: %d\n", ast->base.type);
}
