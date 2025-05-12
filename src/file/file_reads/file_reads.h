#include <memory/memory_allocation/memory_allocation.h>
#include <memory/memory_free/memory_free.h>
#include <log/log_fatal/log_fatal.h>

#include <stdio.h> // for FILE, fopen, fclose, fseek, ftell, rewind, fread

char* file_reads(const char* file_path);
