#ifndef _STAGE_SCANNER_SCANNER_ERROR_SCANNER_ERROR_H_
#define _STAGE_SCANNER_SCANNER_ERROR_SCANNER_ERROR_H_

#include <stdio.h> // for vfprintf
#include <stdarg.h> // for va_list, va_start, va_end
#include <stdlib.h> // for exit

// base
#include <base.h>

void scanner_error(const char* format, ...);

#endif // _STAGE_SCANNER_SCANNER_ERROR_SCANNER_ERROR_H_
