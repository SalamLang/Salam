#ifndef _UTILITY_LOG_LOG_ERROR_LOG_ERROR_H_
#define _UTILITY_LOG_LOG_ERROR_LOG_ERROR_H_

#include <stdarg.h>  // for va_list, va_start, va_end
#include <stdio.h>   // for vfprintf
#include <stdlib.h>  // for exit

void log_error(const char* format, ...);

#endif  // _UTILITY_LOG_LOG_ERROR_LOG_ERROR_H_
