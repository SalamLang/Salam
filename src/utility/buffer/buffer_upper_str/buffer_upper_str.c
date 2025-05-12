#include <utility/buffer/buffer_upper_str/buffer_upper_str.h>

/**
 *
 * @function buffer_upper_str
 * @brief Convert a buffer to uppercase
 * @params {const char*} str - buffer
 * @returns {char*} - Uppercase buffer
 *
 */
char *buffer_upper_str(const char *str)
{
    DEBUG_ME;
    char *buffer = string_duplicate(str);

    for (size_t i = 0; i < string_length(buffer); i++) {
        buffer[i] = tolower(buffer[i]);
    }

    return buffer;
}
