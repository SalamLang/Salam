#include <utility/convert/convert_size2string/convert_size2string.h>

/**
 *
 * @function convert_size2string
 * @brief Convert an integer to a string
 * @params {size_t} value - Integer value
 * @returns {char*}
 *
 */
char *convert_size2string(size_t value)
{
    DEBUG_ME;
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), "%zu", value);

    return buffer;
}
