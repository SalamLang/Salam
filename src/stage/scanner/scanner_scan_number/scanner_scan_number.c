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
void scanner_scan_number(scanner_t* scanner, char* uc) {
    DEBUG_ME;
    buffer_t* temp = buffer_create(25);
    buffer_append_char(temp, string_convert_utf8_to_english_digit(uc));

    bool is_float = false;

    while (SCANNER_CURRENT != '\0') {
        uc = utf8_char_decode(scanner->source, &scanner->index, NULL);
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

        buffer_append_char(temp, string_convert_utf8_to_english_digit(uc));

        memory_destroy(uc);
    }

    token_type_t type =
        is_float ? TOKEN_TYPE_VALUE_NUMBER_FLOAT : TOKEN_TYPE_VALUE_NUMBER_INT;
    token_t* token = token_create(type, SCANNER_CURRENT_LOCATION);
    token->source = string_duplicate(temp->data);
    token->value = value_create(is_float ? VALUE_TYPE_NUMBER_FLOAT
                                         : VALUE_TYPE_NUMBER_INT);
    log_info("New number token - type: %s\n", token_name(type));
    if (is_float) {
        token->value->raw.float_value = atof(temp->data);
    } else {
        token->value->raw.int_value = atoi(temp->data);
    }

    buffer_destroy(temp);
    SCANNER_PUSH_TOKEN(token);
}
