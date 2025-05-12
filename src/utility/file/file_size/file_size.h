#include <stdio.h> // for FILE, fopen, fclose, fseek, ftell

// memory
#include <utility/memory/memory_allocation/memory_allocation.h>
#include <utility/memory/memory_destroy/memory_destroy.h>
// log
#include <utility/log/log_fatal/log_fatal.h>

long file_size(const char* file_path);
