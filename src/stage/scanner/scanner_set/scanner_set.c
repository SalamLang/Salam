#include <stage/scanner/scanner_set/scanner_set.h>

void scanner_set(
    scanner_t* scanner,
    char* source,
    char* directory,
    char* file_name
)
{
    scanner->source = source;
    scanner->length = string_length(source);

    scanner->directory = directory;
    scanner->file_name = file_name;
}
