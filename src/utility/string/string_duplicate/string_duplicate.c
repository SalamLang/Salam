#include <utility/string/string_duplicate/string_duplicate.h>

char* string_duplicate(const char* s) {
    DEBUG_ME;
    if (s == NULL) return NULL;

    size_t len = string_length(s) + 1;
    char* dup = memory_allocation(len);
    if (dup) {
        memory_copy(dup, s, len);
    }
    return dup;
}
