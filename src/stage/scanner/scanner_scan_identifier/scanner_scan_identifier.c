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

    if (type == TOKEN_TYPE_KEYWORD_RAW) {
        log_info("Oh yes, we have a TOKEN_TYPE_KEYWORD_RAW here...\n");
        while (SCANNER_CURRENT != '\0' && isspace(SCANNER_CURRENT)) {
            SCANNER_NEXT;
        }

        if (SCANNER_CURRENT != '{') {
            log_fatal("Expected '{' after raw keyword, but got '%c'", SCANNER_CURRENT);
        }

        SCANNER_NEXT;

        buffer_t *raw_value = buffer_create(64);
        size_t brace_depth = 1;

        while (SCANNER_CURRENT != '\0' && brace_depth > 0) {
            char c = SCANNER_CURRENT;
            log_info("Inside raw block - checking char %c\n", c);

            if (c == '{') {
                brace_depth++;
            } else if (c == '}') {
                brace_depth--;
                if (brace_depth == 0) {
                    SCANNER_NEXT;
                    break;
                }
            }

            log_info("Inside raw block - appending %c to raw_value\n", c);
            buffer_append_char(raw_value, c);
            SCANNER_NEXT;
        }

        if (brace_depth != 0) {
            log_fatal("Unterminated raw block; expected matching '}'");
        }

        char* trim_raw_value = string_trim(raw_value->data);
        token->source = string_duplicate(trim_raw_value);
        buffer_destroy(raw_value);
        memory_destroy(trim_raw_value);
    } else {
        token->source = string_duplicate(value->data);
    }

    buffer_destroy(value);
    SCANNER_PUSH_TOKEN(token);
}
