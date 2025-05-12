#include <stage/scanner/scanner_scan/scanner_scan.h>

#define SCANNER_CURRENT (scanner->source[scanner->index])
#define SCANNER_CURRENT_PREV (scanner->source[scanner->index - 1])
#define SCANNER_CURRENT_NEXT (scanner->source[scanner->index + 1])

#define SCANNER_NEXT scanner->index++
#define SCANNER_PREV scanner->index--

#define SCANNER_NEXT_LINE scanner->line++
#define SCANNER_NEXT_COLUMN scanner->column++

#define SCANNER_ZERO_COLUMN scanner->column = 0

#define SCANNER_PUSH_TOKEN(TOKEN) array_append(scanner->tokens, TOKEN)

void scanner_scan(scanner_t* scanner)
{
    char c;
    /*
    while ((c = SCANNER_CURRENT) && c != '\0' &&
           scanner->index < scanner->length) {
        size_t num_bytes;
        char *uc = utf8_decode(scanner->source, &scanner->index, &num_bytes);

        switch (num_bytes) {
            case 0: {
                if (uc != NULL) {
                    memory_destroy(uc);
                }

                error_scanner(3, "Invalid UTF-8 encoding detected at index %d",
                            scanner->index);
            } break;

            case 1: {
                char ucf = uc[0];

                switch (ucf) {
                    // End of file
                    case '\0':
                        if (uc != NULL) {
                            memory_destroy(uc);
                        }

                        break;

                    // New line
                    case '\n':
                        if (uc != NULL) {
                            memory_destroy(uc);
                        }

                        SCANNER_NEXT_LINE;
                        SCANNER_ZERO_COLUMN;
                        continue;

                    case '\a':  // Alert
                    case '\b':  // Backspace
                    case '\f':  // Form feed
                    case '\r':  // Carriage return
                    case '\t':  // Horizontal tab
                    case '\v':  // Vertical tab
                    case ' ':   // Space
                        if (uc != NULL) {
                            memory_destroy(uc);
                        }

                        continue;

                    case '{':
                    case '}':
                    case '[':
                    case ']':
                    case ':':
                    case ',':
                    case '(':
                    case ')':
                    case '+':
                    case '-':
                    case '*':
                    case '/':
                    case '%':
                    case '^':
                    case '=':
                    case '<':
                    case '>':
                    case '!':
                        if (uc != NULL) {
                            memory_destroy(uc);
                        }

                        if (SCANNER_CURRENT == '/') {
                            SCANNER_NEXT;
                            SCANNER_NEXT_COLUMN;

                            while (SCANNER_CURRENT != '\n' &&
                                   SCANNER_CURRENT != '\0') {
                                SCANNER_NEXT;
                                SCANNER_NEXT_COLUMN;
                            }
                        } else if (SCANNER_CURRENT == '*') {
                            SCANNER_NEXT;
                            SCANNER_NEXT_COLUMN;

                            while (1) {
                                if (SCANNER_CURRENT == '\0') {
                                    error_scanner(
                                        2,
                                        "Unterminated multiline comment at "
                                        "line %zu, column %zu",
                                        scanner->line, scanner->column);
                                    break;
                                }

                                if (SCANNER_CURRENT == '*') {
                                    SCANNER_NEXT;
                                    SCANNER_NEXT_COLUMN;

                                    if (SCANNER_CURRENT == '/') {
                                        SCANNER_NEXT;
                                        SCANNER_NEXT_COLUMN;
                                        break;
                                    }
                                } else {
                                    SCANNER_NEXT;
                                    SCANNER_NEXT_COLUMN;
                                }
                            }
                        } else {
                            token_type_t type = token_char_type(c);
                            token_t *token = token_create(
                                type, (token_location_t){scanner->index, 1, scanner->line,
                                                   scanner->column, scanner->line,
                                                   scanner->column});

                            SCANNER_PUSH_TOKEN(token);
                        }
                        continue;

                    case '"':
                        if (uc != NULL) {
                            memory_destroy(uc);
                        }

                        scanner_lex_string(scanner, 0);
                        continue;

                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        scanner_lex_number(scanner, uc);

                        if (uc != NULL) {
                            memory_destroy(uc);
                        }
                        continue;

                    default:
                        if (c == '_' || is_char_alpha(c)) {
                            scanner_lex_identifier(scanner, uc);
                        } else {
                            error_scanner(1,
                                        "Unknown character '%s' at line %zu, "
                                        "column %zu",
                                        uc, scanner->line, scanner->column);
                        }

                        if (uc != NULL) {
                            memory_destroy(uc);
                        }
                        continue;
                }
            } break;

            case 2:
            case 3:
            case 4:
            default: {
                if (strcmp(uc, "«") == 0) {
                    scanner_lex_string(scanner, 1);
                } else if (strcmp(uc, "“") == 0) {
                    scanner_lex_string(scanner, 2);
                } else if (is_utf8_digit(uc)) {
                    scanner_lex_number(scanner, uc);
                } else if (c == '_' || is_utf8_alpha(uc) || is_char_alpha(c)) {
                    scanner_lex_identifier(scanner, uc);
                } else {
                    error_scanner(
                        1, "Unknown character '%s' at line %zu, column %zu", uc,
                        scanner->line, scanner->column);
                }

                if (uc != NULL) {
                    memory_destroy(uc);
                }
                continue;
            } break;
        }
    }
    */

    token_t *token = token_create(TOKEN_TYPE_EOF);
    token->location = (token_location_t){scanner->line, scanner->column, scanner->index,
                                   scanner->line, scanner->column, scanner->index,
                                   1};
    SCANNER_PUSH_TOKEN(token);
}
