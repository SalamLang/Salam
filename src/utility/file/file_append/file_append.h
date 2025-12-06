#ifndef _UTILITY_FILE_FILE_APPEND_FILE_APPEND_H_
#define _UTILITY_FILE_FILE_APPEND_FILE_APPEND_H_

#include <stdbool.h>  // for bool, true, false
#include <stdio.h>    // for FILE, fputs
#include <stdlib.h>   // for fopen, fclose

bool file_append(const char* path, const char* content);

#endif  // _UTILITY_FILE_FILE_APPEND_FILE_APPEND_H_
