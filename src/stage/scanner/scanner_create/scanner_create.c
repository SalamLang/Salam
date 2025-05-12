#include <stage/scanner/scanner_create/scanner_create.h>

scanner_t* scanner_create()
{
    scanner_t* scanner = memory_allocation(sizeof(scanner_t));
    
    return scanner;
}
