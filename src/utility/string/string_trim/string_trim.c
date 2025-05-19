#include <utility/string/string_trim/string_trim.h>

char* string_trim(const char* input)
{
    DEBUG_ME;
    if (!input) return NULL;

    char* left_trimmed = string_ltrim(input);
    if (!left_trimmed) return NULL;

    char* result = string_rtrim(left_trimmed);
    memory_destroy(left_trimmed);

    return result;
}
