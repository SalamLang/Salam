#ifndef _UTILITY_LOG_LOG_FATAL_LOG_FATAL_H_
#define _UTILITY_LOG_LOG_FATAL_LOG_FATAL_H_

#include <stdio.h> // for vfprintf
#include <stdarg.h> // for va_list, va_start, va_end
#include <stdlib.h> // for exit

void log_fatal(const char* format, ...);

#endif // _UTILITY_LOG_LOG_FATAL_LOG_FATAL_H_
