#include <stage/scanner/scanner_scan_comment_multiline/scanner_scan_comment_multiline.h>

void scanner_scan_comment_multiline(scanner_t* scanner)
{
    SCANNER_NEXT;  // move past '/'
    SCANNER_NEXT_COLUMN;
    SCANNER_NEXT;  // move past '*'
    SCANNER_NEXT_COLUMN;
    size_t opened_comments = 1;

    while (scanner->index < scanner->length) {
        if (SCANNER_CURRENT == '\0') {
            scanner_error("Unterminated multiline comment at line %zu, column %zu",
                        scanner->line, scanner->column);
            break;
        }

        if (SCANNER_CURRENT == '/' && SCANNER_HAS_NEXT && SCANNER_CURRENT_NEXT == '*') {
            opened_comments++;
            SCANNER_NEXT;  // move past '/'
            SCANNER_NEXT_COLUMN;
            SCANNER_NEXT;  // move past '*'
            SCANNER_NEXT_COLUMN;
        }
        else if (SCANNER_CURRENT == '*' && SCANNER_HAS_NEXT && SCANNER_CURRENT_NEXT == '/') {
            opened_comments--;
            SCANNER_NEXT;  // move past '*'
            SCANNER_NEXT_COLUMN;
            SCANNER_NEXT;  // move past '/'
            SCANNER_NEXT_COLUMN;

            if (opened_comments == 0) {
                break;
            }
        }
        else if (SCANNER_CURRENT == '\n') {
            SCANNER_NEXT;
            SCANNER_NEXT_LINE;
            SCANNER_ZERO_COLUMN;
        } else {
            SCANNER_NEXT;
            SCANNER_NEXT_COLUMN;
        }
    }

    if (opened_comments != 0) {
        scanner_error("Unterminated multiline comment at line %zu, column %zu",
                    scanner->line, scanner->column);
    }
}
