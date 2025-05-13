#include <stage/scanner/scanner_scan/scanner_scan.h>

void scanner_scan(scanner_t* scanner)
{
    char c;
    while ((c = SCANNER_CURRENT) && c != '\0' &&
           scanner->index < scanner->length) {
        size_t num_bytes;
        char *uc = utf8_char_decode(scanner->source, &scanner->index, &num_bytes);

        switch (num_bytes) {
            case 0: {
                if (uc != NULL) {
                    memory_destroy(uc);
                }

                scanner_error("Invalid UTF-8 encoding detected at index %d",
                            scanner->index);
            } break;

            case 1: {
                char ucf = uc[0];

                switch (ucf) {
                    // End of file
                    case '\0':
                    {
                        if (uc != NULL) {
                            memory_destroy(uc);
                        }
                    } break;

                    // New line
                    case '\n':
                    {
                        if (uc != NULL) {
                            memory_destroy(uc);
                        }

                        SCANNER_NEXT_LINE;
                        SCANNER_ZERO_COLUMN;

                        continue;
                    } break;

                    case '\a':  // Alert
                    case '\b':  // Backspace
                    case '\f':  // Form feed
                    case '\r':  // Carriage return
                    case '\t':  // Horizontal tab
                    case '\v':  // Vertical tab
                    case ' ':   // Space
                    {
                        if (uc != NULL) {
                            memory_destroy(uc);
                        }

                        continue;
                    } break;

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
                    {
                        if (uc != NULL) {
                            memory_destroy(uc);
                        }

                        printf("Cur: %c, Next: %c\n", ucf, SCANNER_CURRENT);
                        
                        if (ucf == '/' && SCANNER_CURRENT == '/') {
                            printf("scanner_scan_comment_inline\n");
                            scanner_scan_comment_inline(scanner);

                            continue;
                        }
                        else if (ucf == '/' && SCANNER_CURRENT == '*') {
                            printf("scanner_scan_comment_multiline\n");
                            scanner_scan_comment_multiline(scanner);

                            continue;
                        }
                        else {
                            printf("not inline and multiline comments\n");
                            token_type_t type = token_char_type(c);
                            token_t *token = token_create(type);
                            token->location = (token_location_t){scanner->line, scanner->column, scanner->index,
                                                                    scanner->line, scanner->column, scanner->index,
                                                                1};
                            SCANNER_PUSH_TOKEN(token);
                            continue;
                        }
                    } break;

                    case '"':
                    {
                        if (uc != NULL) {
                            memory_destroy(uc);
                        }

                        scanner_scan_string(scanner, 0);
                        continue;
                    } break;

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
                    {
                        scanner_scan_number(scanner, uc);

                        if (uc != NULL) {
                            memory_destroy(uc);
                        }
                        continue;
                    } break;

                    default:
                    {
                        if (c == '_' || string_char_is_alpha(c)) {
                            scanner_scan_identifier(scanner, uc);
                        } else {
                            scanner_error("Unknown character '%s' at line %zu, "
                                        "column %zu",
                                        uc, scanner->line, scanner->column);
                        }

                        if (uc != NULL) {
                            memory_destroy(uc);
                        }
                        continue;
                    } break;
                }
            } break;

            case 2:
            case 3:
            case 4:
            default: {
                if (string_compare(uc, "«") == 0) {
                    scanner_scan_string(scanner, 1);
                } else if (string_compare(uc, "“") == 0) {
                    scanner_scan_string(scanner, 2);
                } else if (utf8_is_digit(uc)) {
                    scanner_scan_number(scanner, uc);
                } else if (c == '_' || utf8_is_alpha(uc) || string_char_is_alpha(c)) {
                    scanner_scan_identifier(scanner, uc);
                } else {
                    scanner_error("Unknown character '%s' at line %zu, column %zu", uc,
                        scanner->line, scanner->column);
                }

                if (uc != NULL) {
                    memory_destroy(uc);
                }
                continue;
            } break;
        }
    }

    token_t *token = token_create(TOKEN_TYPE_EOF);
    token->location = (token_location_t){scanner->line, scanner->column, scanner->index,
                                         scanner->line, scanner->column, scanner->index,
                                         1};
    SCANNER_PUSH_TOKEN(token);
}
