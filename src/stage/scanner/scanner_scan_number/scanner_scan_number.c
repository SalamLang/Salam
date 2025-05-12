#include <stage/scanner/scanner_scan_number/scanner_scan_number.h>

/**
 *
 * @function scanner_scan_number
 * @brief Lexing a number
 * @params {scanner_t*} scanner - Lexer state
 * @params {char*} uc - UTF8 character
 * @returns {void}
 *
 */
void scanner_scan_number(scanner_t *scanner, char *uc)
{
    DEBUG_ME;
    buffer_t *value = buffer_create(25);
    buffer_append_char(value, convert_utf8_to_english_digit(uc));

    bool is_float = false;

    while (SCANNER_CURRENT != '\0') {
        // size_t num_bytes;
        uc = char_utf8_decode(scanner->source, &scanner->index, NULL);
        if (string_compare(uc, "\0") == 0) {
            memory_destroy(uc);

            break;
        }

        size_t is_dot = string_compare(uc, ".");

        if (!utf8_is_digit(uc) && is_dot != 0) {
            memory_destroy(uc);

            break;
        }

        if (is_dot == 0) {
            if (is_float) {
                memory_destroy(uc);

                break;
            }

            is_float = true;
        }

        buffer_append_char(value, string_convert_utf8_to_english_digit(uc));

        memory_destroy(uc);
    }

    token_type_t type = is_float ? TOKEN_TYPE_NUMBER_FLOAT : TOKEN_TYPE_NUMBER_INT;
    token_t *token = token_create(
        type, (token_location_t){scanner->index, 1, scanner->line, scanner->column,
                           scanner->line, scanner->column});
    // token->data_type = type;
    // if (is_float) {
    //     token->data.number_float = atof(value->data);
    // } else {
    //     token->data.number_int = atoi(value->data);
    // }

    buffer_destroy(value);

    SCANNER_PUSH_TOKEN(token);
}
