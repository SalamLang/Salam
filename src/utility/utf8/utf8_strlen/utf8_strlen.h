#include <wchar.h> // for mbrtowc, MB_CUR_MAX
#include <stddef.h> // for size_t
#include <string.h> // for memset_s

// base
#include <base.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

size_t utf8_strlen(const char *str);
