#include <stage/scanner/scanner_log/scanner_log.h>

void scanner_log(scanner_t* scanner)
{
    for (char* p = scanner->source; *p != '\0'; ++p) {
        log_info("Char: '%c' (0x%02X)\n", *p, (unsigned char)*p);
    }
}
