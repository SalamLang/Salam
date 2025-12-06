#ifndef _UTILITY_UTF8_UTF8_STRLEN_UTF8_STRLEN_H_
#define _UTILITY_UTF8_UTF8_STRLEN_UTF8_STRLEN_H_

#include <stddef.h>  // for size_t
#include <string.h>  // for memset_s
#include <wchar.h>   // for mbrtowc, MB_CUR_MAX

// base
#include <base.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

// memory
#include <utility/memory/memory_set/memory_set.h>

size_t utf8_strlen(const char* str);

#endif  // _UTILITY_UTF8_UTF8_STRLEN_UTF8_STRLEN_H_
