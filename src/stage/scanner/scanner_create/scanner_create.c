#include <stage/scanner/scanner_create/scanner_create.h>

scanner_t* scanner_create(
    char* source,
    char* directory,
    char* file_name
)
{
    DEBUG_ME;
    scanner_t* scanner = memory_allocation(sizeof(scanner_t));

    scanner->source = source;
    scanner->directory = directory;
    scanner->file_name = file_name;

    scanner->line = 1;
    scanner->column = 0;
    scanner->index = 0;
    scanner->length = string_length(source);

    scanner->tokens = array_create((array_destroy_t)token_free);

    return scanner;
}
