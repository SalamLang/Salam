#include <utility/string/string_arabic2persian/string_arabic2persian.h>

/**
 *
 * @function string_arabic2persian
 */
char* string_arabic2persian(const char* arabic) {
    DEBUG_ME;
    return string_replace_all_substrings(arabic, "ي", "ی");
}
