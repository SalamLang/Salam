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
    token_t *token = token_create(type);
    token->location = (token_location_t){scanner->line, scanner->column, scanner->index,
                                         scanner->line, scanner->column, scanner->index,
                                         1};
    // token->data_type = TOKEN_IDENTIFIER;
    // token->data.string = buffer_arabic2persian(value->data);

    buffer_destroy(value);

    SCANNER_PUSH_TOKEN(token);
}

