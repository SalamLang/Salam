#ifndef _SRC_UTILITY_FILE_FILE_SIZE_FILE_SIZE_H_
#define _SRC_UTILITY_FILE_FILE_SIZE_FILE_SIZE_H_

#include <stdio.h> // for FILE, fopen, fclose, fseek, ftell

// memory
#include <utility/memory/memory_allocation/memory_allocation.h>
#include <utility/memory/memory_destroy/memory_destroy.h>
// log
#include <utility/log/log_fatal/log_fatal.h>

long file_size(const char* file_path);

#endif // _SRC_UTILITY_FILE_FILE_SIZE_FILE_SIZE_H_
