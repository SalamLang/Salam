#include <utility/string/string_length/string_length.h>

size_t string_length(const char* input)
{
    DEBUG_ME;
    if (!input) return 0;
    log_info("String length of: %s\n", input);

    return strlen(input);
}
