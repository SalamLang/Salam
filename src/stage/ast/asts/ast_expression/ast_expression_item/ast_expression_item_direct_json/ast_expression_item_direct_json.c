#include "ast_expression_item_direct_json.h"

char* ast_expression_item_direct_json(ast_expression_item_t expression_item)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // base
    buffer_append_str(temp, "\"base\": \"ast_expression_item_t\",");

    // type
    buffer_append_str(temp, "\"type\":");
    buffer_append_char(temp, '"');
    buffer_append_str(temp, ast_expression_type_name(expression_item.type));
    buffer_append_char(temp, '"');
    buffer_append_char(temp, ',');

    char* value;

    #define HANDLE_EXPR_JSON(TYPE, FIELD, FUNC, KEY) \
        case TYPE: \
            log_info("ast_expression_item_direct_json - Entering case " #TYPE "\n"); \
            buffer_append_str(temp, "\"" KEY "\":"); \
            value = FUNC(expression_item.raw.FIELD); \
            buffer_append_str(temp, value); \
            memory_destroy(value); \
            log_info("ast_expression_item_direct_json - Exiting case " #TYPE "\n"); \
            break;

    switch (expression_item.type)
    {
        HANDLE_EXPR_JSON(AST_EXPRESSION_TYPE_LITERAL,    literal,    ast_expression_literal_direct_json,    "literal")
        HANDLE_EXPR_JSON(AST_EXPRESSION_TYPE_IDENTIFIER, identifier, ast_expression_identifier_direct_json, "identifier")
        HANDLE_EXPR_JSON(AST_EXPRESSION_TYPE_BINARY,     binary,     ast_expression_binary_direct_json,     "binary")
        HANDLE_EXPR_JSON(AST_EXPRESSION_TYPE_UNARY,      unary,      ast_expression_unary_direct_json,      "unary")
        HANDLE_EXPR_JSON(AST_EXPRESSION_TYPE_INDEX,      index,      ast_expression_index_direct_json,      "index")
        HANDLE_EXPR_JSON(AST_EXPRESSION_TYPE_CALL,       call,       ast_expression_call_direct_json,       "call")

        default:
            log_info("ast_expression_item_direct_json - Entering default case (unknown type)");
            buffer_append_str(temp, "\"unknown\": null");
            log_info("ast_expression_item_direct_json - Exiting default case");
            break;
    }

    #undef HANDLE_EXPR_JSON

    // runtime_type
    buffer_append_str(temp, ",\"runtime_type\":");
    if (expression_item.runtime_type == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        char* buffer = ast_type_json(expression_item.runtime_type);
        buffer_append_str(temp, buffer);
        memory_destroy(buffer);
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
