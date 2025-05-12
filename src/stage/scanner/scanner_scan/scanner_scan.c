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
            default: {
                
            }
        }

        // token_t* token = token_create(TOKEN_TYPE_UNKNOWN);
        // token->value = NULL;
        // array_append(scanner->tokens, token);
        // scanner->index++;
    }
}
