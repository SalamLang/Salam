#ifndef _SRC_UTILITY_LOG_LOG_INFO_LOG_INFO_H_
#define _SRC_UTILITY_LOG_LOG_INFO_LOG_INFO_H_

#include <stdio.h> // for vfprintf
#include <stdarg.h> // for va_list, va_start, va_end
#include <stdlib.h> // for exit

void log_info(const char* format, ...);

#endif // _SRC_UTILITY_LOG_LOG_INFO_LOG_INFO_H_
