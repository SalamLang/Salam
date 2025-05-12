#include <stage/scanner/scanner_scan/scanner_scan.h>

void scanner_scan(scanner_t* scanner)
{
    while (scanner->source[scanner->index] != '\0') {
        char ch = scanner->source[scanner->index];
        switch (ch) {
            case '\r':
            case '\a':
            case '\b':
            case '\f':
            case ' ':
            case '\t': {
                scanner->column++;
                scanner->index++;
                continue;
            } break;
            case '\n': {
                scanner->column=0;
                scanner->index++;
                scanner->line++;
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
            case '9': {
                scanner_scan_number(scanner);
            }
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'h':
            case 'i':
            case 'j':
            case 'k':
            case 'l':
            case 'm':
            case 'n':
            case 'o':
            case 'p':
            case 'q':
            case 'r':
            case 's': {
                scanner_scan_number(scanner);
            }
        }

        token_t* token = token_create(TOKEN_TYPE_UNKNOWN);
        token->value = NULL;

        array_append(scanner->tokens, token);

        scanner->index++;
    }
}
