#include <stage/ast/ast_json/ast_json.h>

char* ast_json(ast_t* ast)
{
    buffer_t* temp = buffer_create(256);
    if (ast == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        char* buffer;

        switch (ast->base.type) {
            case AST_TYPE_PARAMETER_ITEM:
                buffer = ast_parameter_item_direct_json(ast->raw.parameter_item_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_PARAMETERS:
                buffer = ast_parameters_direct_json(ast->raw.parameters_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_ATTRIBUTE_ITEM:
                buffer = ast_attribute_item_direct_json(ast->raw.attribute_item_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_ATTRIBUTES:
                buffer = ast_attributes_direct_json(ast->raw.attributes_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_ARGUMENT_ITEM:
                buffer = ast_argument_item_direct_json(ast->raw.argument_item_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_ARGUMENTS:
                buffer = ast_arguments_direct_json(ast->raw.arguments_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_PROGRAM:
                buffer = ast_program_direct_json(ast->raw.program_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_VARIABLE_DECL:
                buffer = ast_variable_decl_direct_json(ast->raw.variable_decl_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_FUNCTION_DECL:
                buffer = ast_function_decl_direct_json(ast->raw.function_decl_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_BLOCK:
                buffer = ast_block_direct_json(ast->raw.block_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_PACKAGE:
                buffer = ast_package_direct_json(ast->raw.package_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_IMPORT:
                buffer = ast_import_direct_json(ast->raw.import_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_KIND_DECL:
                buffer = ast_kind_decl_direct_json(ast->raw.kind_decl_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_KIND_STRUCT:
                buffer = ast_kind_struct_direct_json(ast->raw.kind_struct_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_KIND_ENUM:
                buffer = ast_kind_enum_direct_json(ast->raw.kind_enum_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_KIND_UNION:
                buffer = ast_kind_union_direct_json(ast->raw.kind_union_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_EXTERN_DECL:
                buffer = ast_extern_decl_direct_json(ast->raw.extern_decl_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_EXTERN_VARIABLE:
                buffer = ast_extern_variable_direct_json(ast->raw.extern_variable_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_EXTERN_FUNCTION:
                buffer = ast_extern_function_direct_json(ast->raw.extern_function_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_EXPRESSION_ITEM:
                buffer = ast_expression_item_direct_json(ast->raw.expression_item_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_EXPRESSIONS:
                buffer = ast_expressions_direct_json(ast->raw.expressions_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_EXPRESSION_LITERAL:
                buffer = ast_expression_literal_direct_json(ast->raw.expression_literal_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_EXPRESSION_IDENTIFIER:
                buffer = ast_expression_identifier_direct_json(ast->raw.expression_identifier_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_EXPRESSION_BINARY:
                buffer = ast_expression_binary_direct_json(ast->raw.expression_binary_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_EXPRESSION_UNARY:
                buffer = ast_expression_unary_direct_json(ast->raw.expression_unary_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_EXPRESSION_INDEX:
                buffer = ast_expression_index_direct_json(ast->raw.expression_index_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_EXPRESSION_CALL:
                buffer = ast_expression_call_direct_json(ast->raw.expression_call_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_STATEMENT_IF:
                buffer = ast_statement_if_direct_json(ast->raw.statement_if_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_STATEMENT_FOR:
                buffer = ast_statement_for_direct_json(ast->raw.statement_for_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_STATEMENT_FOREACH:
                buffer = ast_statement_foreach_direct_json(ast->raw.statement_foreach_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_STATEMENT_RETURN:
                buffer = ast_statement_return_direct_json(ast->raw.statement_return_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_STATEMENT_PRINT:
                buffer = ast_statement_print_direct_json(ast->raw.statement_print_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_STATEMENT_VARIABLE_DECL:
                buffer = ast_statement_variable_decl_direct_json(ast->raw.statement_variable_decl_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_STATEMENT_EXPRESSION:
                buffer = ast_statement_expression_direct_json(ast->raw.statement_expression_value);
                buffer_append_str(temp, buffer);
                memory_destroy(buffer);
                break;

            case AST_TYPE_UNKNOWN:
                log_fatal("Unhandled AST type in ast_destroy: %d\n", ast->base.type);
                break;

            default:
                log_fatal("Unhandled AST type in ast_destroy: %d\n", ast->base.type);
                break;
        }
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
