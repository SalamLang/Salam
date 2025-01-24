#include "location.h"

/**
 *
 * @function location_string
 * @brief Get the string representation of a location
 * @params {location_t} location - Location
 * @returns {char*}
 *
 */
char *location_stringify(location_t location) {
    DEBUG_ME;
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), "%zu:%zu - %zu:%zu", location.start_line,
             location.start_column, location.end_line, location.end_column);

    return buffer;
}

/**
 *
 * @function location_print
 * @brief Print a location
 * @params {location_t} location - Location
 * @returns {void}
 *
 */
void location_print(location_t location) {
    DEBUG_ME;
    printf("Location: %s\n", location_stringify(location));
}