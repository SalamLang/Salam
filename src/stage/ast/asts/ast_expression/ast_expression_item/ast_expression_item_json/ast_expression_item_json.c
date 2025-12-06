#include "ast_expression_item_json.h"

char* ast_expression_item_json(ast_expression_item_t* expression_item) {
    buffer_t* temp = buffer_create(24);

    if (expression_item == NULL) {
        buffer_append_str(temp, "null");
    } else {
        buffer_append_char(temp, '{');

        // type
        buffer_append_str(temp, "\"type\":");
        buffer_append_char(temp, '"');
        buffer_append_str(temp,
                          ast_expression_type_name(expression_item->type));
        buffer_append_char(temp, '"');
        buffer_append_char(temp, ',');

        // raw
        char* value;
        switch (expression_item->type) {
            case AST_EXPRESSION_TYPE_LITERAL:
                buffer_append_str(temp, "\"literal\":");
                value = ast_expression_literal_direct_json(
                    expression_item->raw.literal);
                buffer_append_str(temp, value);
                memory_destroy(value);
                break;

            case AST_EXPRESSION_TYPE_IDENTIFIER:
                buffer_append_str(temp, "\"identifier\":");
                value = ast_expression_identifier_direct_json(
                    expression_item->raw.identifier);
                buffer_append_str(temp, value);
                memory_destroy(value);
                break;

            case AST_EXPRESSION_TYPE_BINARY:
                buffer_append_str(temp, "\"binary\":");
                value = ast_expression_binary_direct_json(
                    expression_item->raw.binary);
                buffer_append_str(temp, value);
                memory_destroy(value);
                break;

            case AST_EXPRESSION_TYPE_UNARY:
                buffer_append_str(temp, "\"unary\":");
                value = ast_expression_unary_direct_json(
                    expression_item->raw.unary);
                buffer_append_str(temp, value);
                memory_destroy(value);
                break;

            case AST_EXPRESSION_TYPE_INDEX:
                buffer_append_str(temp, "\"index\":");
                value = ast_expression_index_direct_json(
                    expression_item->raw.index);
                buffer_append_str(temp, value);
                memory_destroy(value);
                break;

            case AST_EXPRESSION_TYPE_CALL:
                buffer_append_str(temp, "\"call\":");
                value =
                    ast_expression_call_direct_json(expression_item->raw.call);
                buffer_append_str(temp, value);
                memory_destroy(value);
                break;

            default:
                buffer_append_str(temp, "\"unknown\": null");
                break;
        }

        // runtime_type
        buffer_append_str(temp, ",\"runtime_type\":");
        if (expression_item->runtime_type == NULL) {
            buffer_append_str(temp, "null");
        } else {
            char* buffer = ast_type_json(expression_item->runtime_type);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }

        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
