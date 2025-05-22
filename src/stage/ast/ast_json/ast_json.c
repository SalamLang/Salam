#include <stage/ast/ast_json/ast_json.h>

char* ast_json(ast_t* ast)
{
    DEBUG_ME;
    log_info("Ast Json for type %s\n", ast_node_type_name(ast->base.type));
    buffer_t* temp = buffer_create(256);

    if (ast == NULL) {
        buffer_append_str(temp, "null");
    } else {
        char* buffer;

        #define HANDLE_AST_JSON(TYPE, FIELD, FUNC) \
            case TYPE: \
                log_info("ast_json - Entering case " #TYPE "\n"); \
                buffer = FUNC(ast->raw.FIELD); \
                buffer_append_str(temp, buffer); \
                memory_destroy(buffer); \
                log_info("ast_json - Exiting case " #TYPE "\n"); \
                break;

        switch (ast->base.type) {
            HANDLE_AST_JSON(AST_TYPE_TYPE, type_value, ast_type_direct_json)
            HANDLE_AST_JSON(AST_TYPE_PARAMETER_ITEM, parameter_item_value, ast_parameter_item_direct_json)
            HANDLE_AST_JSON(AST_TYPE_PARAMETERS, parameters_value, ast_parameters_direct_json)
            HANDLE_AST_JSON(AST_TYPE_ATTRIBUTE_ITEM, attribute_item_value, ast_attribute_item_direct_json)
            HANDLE_AST_JSON(AST_TYPE_ATTRIBUTES, attributes_value, ast_attributes_direct_json)
            HANDLE_AST_JSON(AST_TYPE_ARGUMENT_ITEM, argument_item_value, ast_argument_item_direct_json)
            HANDLE_AST_JSON(AST_TYPE_ARGUMENTS, arguments_value, ast_arguments_direct_json)
            HANDLE_AST_JSON(AST_TYPE_PROGRAM, program_value, ast_program_direct_json)
            HANDLE_AST_JSON(AST_TYPE_VARIABLE_DECL, variable_decl_value, ast_variable_decl_direct_json)
            HANDLE_AST_JSON(AST_TYPE_FUNCTION_DECL, function_decl_value, ast_function_decl_direct_json)
            HANDLE_AST_JSON(AST_TYPE_BLOCK, block_value, ast_block_direct_json)
            HANDLE_AST_JSON(AST_TYPE_PACKAGE, package_value, ast_package_direct_json)
            HANDLE_AST_JSON(AST_TYPE_IMPORT, import_value, ast_import_direct_json)
            HANDLE_AST_JSON(AST_TYPE_KIND_DECL, kind_decl_value, ast_kind_decl_direct_json)
            HANDLE_AST_JSON(AST_TYPE_KIND_STRUCT, kind_struct_value, ast_kind_struct_direct_json)
            HANDLE_AST_JSON(AST_TYPE_KIND_ENUM, kind_enum_value, ast_kind_enum_direct_json)
            HANDLE_AST_JSON(AST_TYPE_KIND_UNION, kind_union_value, ast_kind_union_direct_json)
            HANDLE_AST_JSON(AST_TYPE_EXTERN_DECL, extern_decl_value, ast_extern_decl_direct_json)
            HANDLE_AST_JSON(AST_TYPE_EXTERN_VARIABLE, extern_variable_value, ast_extern_variable_direct_json)
            HANDLE_AST_JSON(AST_TYPE_EXTERN_FUNCTION, extern_function_value, ast_extern_function_direct_json)
            HANDLE_AST_JSON(AST_TYPE_EXPRESSION_ITEM, expression_item_value, ast_expression_item_direct_json)
            HANDLE_AST_JSON(AST_TYPE_EXPRESSIONS, expressions_value, ast_expressions_direct_json)
            HANDLE_AST_JSON(AST_TYPE_EXPRESSION_LITERAL, expression_literal_value, ast_expression_literal_direct_json)
            HANDLE_AST_JSON(AST_TYPE_EXPRESSION_IDENTIFIER, expression_identifier_value, ast_expression_identifier_direct_json)
            HANDLE_AST_JSON(AST_TYPE_EXPRESSION_BINARY, expression_binary_value, ast_expression_binary_direct_json)
            HANDLE_AST_JSON(AST_TYPE_EXPRESSION_UNARY, expression_unary_value, ast_expression_unary_direct_json)
            HANDLE_AST_JSON(AST_TYPE_EXPRESSION_INDEX, expression_index_value, ast_expression_index_direct_json)
            HANDLE_AST_JSON(AST_TYPE_EXPRESSION_CALL, expression_call_value, ast_expression_call_direct_json)
            HANDLE_AST_JSON(AST_TYPE_STATEMENT_IF, statement_if_value, ast_statement_if_direct_json)
            HANDLE_AST_JSON(AST_TYPE_STATEMENT_FOR, statement_for_value, ast_statement_for_direct_json)
            HANDLE_AST_JSON(AST_TYPE_STATEMENT_FOREACH, statement_foreach_value, ast_statement_foreach_direct_json)
            HANDLE_AST_JSON(AST_TYPE_STATEMENT_RET, statement_ret_value, ast_statement_return_direct_json)
            HANDLE_AST_JSON(AST_TYPE_STATEMENT_PRINT, statement_print_value, ast_statement_print_direct_json)
            HANDLE_AST_JSON(AST_TYPE_STATEMENT_RAW, statement_raw_value, ast_statement_raw_direct_json)
            HANDLE_AST_JSON(AST_TYPE_STATEMENT_VARIABLE_DECL, statement_variable_decl_value, ast_statement_variable_decl_direct_json)
            HANDLE_AST_JSON(AST_TYPE_STATEMENT_EXPRESSION, statement_expression_value, ast_statement_expression_direct_json)

            case AST_TYPE_UNKNOWN:
                log_fatal("Unknown AST type in ast_json: %d\n", ast->base.type);
                break;

            default:
                log_fatal("Unhandled AST type in ast_json: %d\n", ast->base.type);
                break;
        }

        #undef HANDLE_AST_JSON
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
