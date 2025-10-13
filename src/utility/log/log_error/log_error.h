#ifndef _SRC_UTILITY_LOG_LOG_ERROR_LOG_ERROR_H_
#define _SRC_UTILITY_LOG_LOG_ERROR_LOG_ERROR_H_

#include <stdio.h> // for vfprintf
#include <stdarg.h> // for va_list, va_start, va_end
#include <stdlib.h> // for exit

void log_error(const char* format, ...);

#endif // _SRC_UTILITY_LOG_LOG_ERROR_LOG_ERROR_H_
