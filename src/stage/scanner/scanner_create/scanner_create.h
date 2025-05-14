// base
#include <base.h>

// memory
#include <utility/memory/memory_allocation/memory_allocation.h>

// string
#include <utility/string/string_length/string_length.h>

// array
#include <utility/array/array_create/array_create.h>

// scanner
#include <stage/scanner/type.h>

// token
#include <stage/token/type.h>
#include <stage/token/token_free/token_free.h>

scanner_t* scanner_create(
    char* source,
    char* directory,
    char* file_name
);
