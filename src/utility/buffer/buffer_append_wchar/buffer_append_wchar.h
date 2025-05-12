#include <wchar.h> // for wctomb

// base
#include <base.h>

// buffer
#include <utility/buffer/type.h>
#include <utility/buffer/buffer_append_str/buffer_append_str.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

void buffer_append_wchar(buffer_t *str, wchar_t c);
