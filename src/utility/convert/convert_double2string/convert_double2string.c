#include <utility/convert/convert_double2string/convert_double2string.h>

/**
 *
 * @function convert_double2string
 * @brief Convert a double to a string
 * @params {double} value - Double value
 * @returns {char*}
 *
 */
char* convert_double2string(double value) {
    DEBUG_ME;
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), "%f", value);

    return buffer;
}
