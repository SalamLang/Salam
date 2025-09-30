#include <utility/buffer/buffer_lower_str/buffer_lower_str.h>

/**
 *
 * @function buffer_lower_str
 * @brief Convert a buffer to lowercase
 * @params {const char*} str - buffer
 * @returns {char*} - Lowercase buffer
 *
 */
char* buffer_lower_str(const char* str) {
    DEBUG_ME;
    char* buffer = string_duplicate(str);

    for (size_t i = 0; i < string_length(buffer); i++) {
        buffer[i] = tolower(buffer[i]);
    }

    return buffer;
}
