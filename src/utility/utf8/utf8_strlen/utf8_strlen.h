#ifndef _SRC_UTILITY_UTF8_UTF8_STRLEN_UTF8_STRLEN_H_
#define _SRC_UTILITY_UTF8_UTF8_STRLEN_UTF8_STRLEN_H_

#include <wchar.h> // for mbrtowc, MB_CUR_MAX
#include <stddef.h> // for size_t
#include <string.h> // for memset_s

// base
#include <base.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

// memory
#include <utility/memory/memory_set/memory_set.h>

size_t utf8_strlen(const char *str);

#endif // _SRC_UTILITY_UTF8_UTF8_STRLEN_UTF8_STRLEN_H_
