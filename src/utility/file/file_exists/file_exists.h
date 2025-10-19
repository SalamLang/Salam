#ifndef _SRC_UTILITY_FILE_FILE_EXISTS_FILE_EXISTS_H_
#define _SRC_UTILITY_FILE_FILE_EXISTS_FILE_EXISTS_H_

#include <stdio.h> // for FILE, fputs
#include <stdlib.h> // for fopen, fclose
#include <stdbool.h> // for bool, true, false

bool file_exists(const char* path);

#endif // _SRC_UTILITY_FILE_FILE_EXISTS_FILE_EXISTS_H_
