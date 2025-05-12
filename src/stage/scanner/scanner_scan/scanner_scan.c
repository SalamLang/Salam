#include <stage/scanner/scanner_scan/scanner_scan.h>

void scanner_scan(scanner_t* scanner)
{
    while (scanner->source[scanner->index] != '\0') {
        char ch = scanner->source[scanner->index];
        // Ignoring invisible characters
        if (
            scanner->source[scanner->index] == '\r' ||
            scanner->source[scanner->index] == '\a' ||
            scanner->source[scanner->index] == '\b' ||
            scanner->source[scanner->index] == '\f' ||
            scanner->source[scanner->index] == ' ' ||
            scanner->source[scanner->index] == '\t'
        ) {
            scanner->column++;
            scanner->index++;
            continue;
        }
        // Ignoring newline character
        else if (scanner->source[scanner->index] == '\n') {
            scanner->column=0;
            scanner->index++;
            scanner->line++;
            continue;
        }

        token_t* token = token_create(TOKEN_TYPE_UNKNOWN);
        token->value = NULL;

        array_append(scanner->tokens, token);

        scanner->index++;
    }
}
