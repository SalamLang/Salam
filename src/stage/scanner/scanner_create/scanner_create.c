#include <stage/scanner/scanner_create/scanner_create.h>

scanner_t* scanner_create()
{
    scanner_t* scanner = memory_allocation(sizeof(scanner_t));
    scanner->tokens = array_create((array_free_func_t)token_free);
    
    return scanner;
}
