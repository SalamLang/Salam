#ifndef _SRC_UTILITY_FILE_FILE_WRITE_FILE_WRITE_H_
#define _SRC_UTILITY_FILE_FILE_WRITE_FILE_WRITE_H_

#include <stdio.h> // for FILE, fputs
#include <stdlib.h> // for fopen, fclose
#include <stdbool.h> // for bool, true, false

// base
#include <base.h>

bool file_write(const char* path, const char* content);

#endif // _SRC_UTILITY_FILE_FILE_WRITE_FILE_WRITE_H_
