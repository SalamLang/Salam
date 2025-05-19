#include <utility/string/string_rtrim/string_rtrim.h>

char* string_rtrim(const char* input)
{
    DEBUG_ME;
    if (!input) return NULL;

    size_t len = string_length(input);
    if (len == 0) return string_duplicate("");

    const char* end = input + len - 1;
    while (end > input && isspace((unsigned char)*end)) {
        end--;
    }

    size_t trimmed_len = end - input + 1;
    char* result = memory_allocation(trimmed_len + 1);
    if (result) {
        memory_copy(result, input, trimmed_len);
        result[trimmed_len] = '\0';
    }

    return result;
}
