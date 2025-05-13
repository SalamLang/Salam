#include <stage/scanner/scanner_create/scanner_create.h>

scanner_t* scanner_create()
{
    scanner_t* scanner = memory_allocation(sizeof(scanner_t));
    scanner->tokens = array_create((array_free_func_t)token_free);

    scanner->line = 1;
    scanner->column = 0;
    scanner->index = 0;
    
    return scanner;
}
