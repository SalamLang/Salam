#include <stage/scanner/scanner_free/scanner_free.h>

void scanner_free(scanner_t* scanner)
{
    memory_free(scanner->source);
    memory_free(scanner);
}
