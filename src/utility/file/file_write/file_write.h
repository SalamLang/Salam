#ifndef _UTILITY_FILE_FILE_WRITE_FILE_WRITE_H_
#define _UTILITY_FILE_FILE_WRITE_FILE_WRITE_H_

#include <stdbool.h>  // for bool, true, false
#include <stdio.h>    // for FILE, fputs
#include <stdlib.h>   // for fopen, fclose

// base
#include <base.h>

bool file_write(const char* path, const char* content);

#endif  // _UTILITY_FILE_FILE_WRITE_FILE_WRITE_H_
