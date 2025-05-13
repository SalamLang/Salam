#include <stage/scanner/scanner_free/scanner_free.h>

void scanner_free(scanner_t* scanner)
{
    array_destroy(scanner->tokens);
    memory_destroy(scanner->source);
    memory_destroy(scanner);
}
