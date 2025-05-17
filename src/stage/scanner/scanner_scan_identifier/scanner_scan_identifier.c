#include <stage/scanner/scanner_scan_identifier/scanner_scan_identifier.h>

/**
 *
 * @function scanner_scan_identifier
 * @brief Lexing an identifier
 * @params {scanner_t*} scanner - Lexer state
 * @params {char*} uc - UTF8 character
 * @returns {void}
 *
 */
void scanner_scan_identifier(scanner_t *scanner, char *uc)
{
    DEBUG_ME;
    buffer_t *value = buffer_create(25);

    buffer_append_str(value, uc);

    while (SCANNER_CURRENT != '\0') {
        size_t num_bytes;
        uc = utf8_char_decode(scanner->source, &scanner->index, &num_bytes);

        if (!utf8_is_alpha(uc)) {
            scanner->index -= num_bytes;

            memory_destroy(uc);

            break;
        }

        buffer_append_str(value, uc);

        memory_destroy(uc);
    }

    token_type_t type = token_keyword_type(value->data);
    token_operator_type_t operator_type = TOKEN_OPERATOR_TYPE_UNKNOWN;
    if (type == TOKEN_TYPE_IDENTIFIER) {
        operator_type = token_operator_keyword_type(value->data);
        if (operator_type != TOKEN_OPERATOR_TYPE_UNKNOWN) {
            type = TOKEN_TYPE_OPERATOR;
        }
    }

    token_t *token = token_create(type, SCANNER_CURRENT_LOCATION);
    if (type == TOKEN_TYPE_OPERATOR) {
        token->operator_type = operator_type;
    }
    token->source = string_duplicate(value->data);

    buffer_destroy(value);
    SCANNER_PUSH_TOKEN(token);
}
