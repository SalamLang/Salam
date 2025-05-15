#ifndef _UTILITY_FILE_FILE_CREATE_FILE_CREATE_H_
#define _UTILITY_FILE_FILE_CREATE_FILE_CREATE_H_

#include <stdio.h> // for FILE, fputs
#include <stdlib.h> // for fopen, fclose
#include <stdbool.h> // for bool, true, false

bool file_create(const char* path);

#endif // _UTILITY_FILE_FILE_CREATE_FILE_CREATE_H_
