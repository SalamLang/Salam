#include <stage/scanner/scanner_destroy/scanner_destroy.h>

void scanner_destroy(scanner_t* scanner)
{
    DEBUG_ME;
    // if (! scanner) return;

    memory_destroy(scanner->source);
    memory_destroy(scanner);
}
