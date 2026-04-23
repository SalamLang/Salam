#include <stage/scanner/scanner_scan_string/scanner_scan_string.h>

/**
 *
 * @function scanner_scan_string
 * @brief Lexing a string
 * @params {scanner_t*} scanner - Lexer state
 * @params {int} type - String type (0 = english strings, 1= persian strings, 2=
 * second persian strings)
 * @returns {void}
 *
 */
void scanner_scan_string(scanner_t* scanner, int type) {
    DEBUG_ME;
    // Opening quote is already consumed
    buffer_t* temp = buffer_create(25);

    size_t num_bytes;
    char* uc = utf8_char_decode(scanner->source, &scanner->index, &num_bytes);

    while (string_compare(uc, "\0") != 0 &&
           ((type == 0 && string_compare(uc, "\"") != 0) ||
            (type == 1 && string_compare(uc, "»") != 0) ||
            (type == 2 && string_compare(uc, "”") != 0))) {
        buffer_append_str(temp, uc);
        memory_destroy(uc);
        uc = utf8_char_decode(scanner->source, &scanner->index, &num_bytes);
    }

    if ((type == 0 && string_compare(uc, "\"") != 0) ||
        (type == 1 && string_compare(uc, "»") != 0) ||
        (type == 2 && string_compare(uc, "”") != 0)) {
        buffer_destroy(temp);
        if (uc != NULL) {
            memory_destroy(uc);
        }
        scanner_error("Unterminated string value at line %zu, column %zu",
                      scanner->line, scanner->column);
        return;
    }

    if (uc != NULL) {
        memory_destroy(uc);
    }

    token_t* token =
        token_create(TOKEN_TYPE_VALUE_STRING, SCANNER_CURRENT_LOCATION);
    token->source = string_duplicate(temp->data);
    token->value = value_create(VALUE_TYPE_STRING);
    token->value->raw.string_value = string_duplicate(temp->data);

    buffer_destroy(temp);
    SCANNER_PUSH_TOKEN(token);
}
