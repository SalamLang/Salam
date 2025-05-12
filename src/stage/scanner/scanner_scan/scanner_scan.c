#include <stage/scanner/scanner_scan/scanner_scan.h>

void scanner_scan(scanner_t* scanner)
{
    while (scanner->source[scanner->index] != '\0') {
        token_t* token = token_create(TOKEN_TYPE_UNKNOWN);
        token->value = NULL;

        array_push(scanner->tokens, token);

        scanner->index++;
    }
}
