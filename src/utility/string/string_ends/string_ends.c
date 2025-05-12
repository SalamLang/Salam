#include <utility/string/string_ends/string_ends.h>

/**
 *
 * @function string_ends
 * @brief Detect if a string ends with another string
 * @params {const char*} source - Input string
 * @params {const char*} search - Search string
 * @returns {bool} ends with it or not
 *
 */
bool string_ends(const char *source, const char *search)
{
    DEBUG_ME;
    size_t source_len = string_length(source);
    size_t search_len = string_length(search);

    if (source_len < search_len) {
        return false;
    }

    return strncmp(source + source_len - search_len, search, search_len) == 0;
}
