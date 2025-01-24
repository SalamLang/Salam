#ifndef _LEXER_LOCATION_H_
#define _LEXER_LOCATION_H_

#include <stdio.h> // size_t

#include "../common/base.h" // DEBUG_ME

typedef struct {
    size_t index;
    size_t length;

    size_t start_line;
    size_t start_column;

    size_t end_line;
    size_t end_column;
} location_t;

/**
 *
 * @function location_string
 * @brief Get the string representation of a location
 * @params {location_t} location - Location
 * @returns {char*}
 *
 */
char *location_stringify(location_t location);

/**
 *
 * @function location_print
 * @brief Print a location
 * @params {location_t} location - Location
 * @returns {void}
 *
 */
void location_print(location_t location);

#endif
