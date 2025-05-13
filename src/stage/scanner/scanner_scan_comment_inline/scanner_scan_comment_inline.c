#include <stage/scanner/scanner_scan_comment_inline/scanner_scan_comment_inline.h>

void scanner_scan_comment_inline(scanner_t* scanner)
{
    SCANNER_NEXT;  // move past '/'
    SCANNER_NEXT_COLUMN;

    while (scanner->index < scanner->length) {
        if (SCANNER_CURRENT == '\0') {
            break;
        }
        else if (SCANNER_CURRENT == '\n') {
            SCANNER_NEXT;
            SCANNER_NEXT_COLUMN;
            break;
        }
        SCANNER_NEXT;
        SCANNER_NEXT_COLUMN;
    }
}
