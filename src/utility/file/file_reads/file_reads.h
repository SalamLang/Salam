#ifndef _UTILITY_FILE_FILE_READS_FILE_READS_H_
#define _UTILITY_FILE_FILE_READS_FILE_READS_H_

#include <utility/memory/memory_allocation/memory_allocation.h>
#include <utility/memory/memory_destroy/memory_destroy.h>
#include <utility/log/log_fatal/log_fatal.h>

#include <stdio.h> // for FILE, fopen, fclose, fseek, ftell, rewind, fread

char* file_reads(const char* file_path);

#endif // _UTILITY_FILE_FILE_READS_FILE_READS_H_
