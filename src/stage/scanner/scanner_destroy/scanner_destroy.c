#include <stage/scanner/scanner_destroy/scanner_destroy.h>

void scanner_destroy(scanner_t* scanner) {
    DEBUG_ME;
    // if (! scanner) return;

    if (scanner->tokens != NULL) {
        array_destroy(scanner->tokens);
    }

    if (scanner->source != NULL) {
        memory_destroy(scanner->source);
    }

    memory_destroy(scanner);
}
