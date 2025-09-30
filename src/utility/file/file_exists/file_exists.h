#ifndef _UTILITY_FILE_FILE_EXISTS_FILE_EXISTS_H_
#define _UTILITY_FILE_FILE_EXISTS_FILE_EXISTS_H_

#include <stdbool.h>  // for bool, true, false
#include <stdio.h>    // for FILE, fputs
#include <stdlib.h>   // for fopen, fclose

bool file_exists(const char* path);

#endif  // _UTILITY_FILE_FILE_EXISTS_FILE_EXISTS_H_
