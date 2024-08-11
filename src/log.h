#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/**
 * @function panic
 * @brief Print a panic error message and exit the program
 * @param {const char*} message
 * @param {...} Variable number of arguments to format the message
 * @returns {void}
 */
void panic(const char* message, ...);

/**
 * @function error
 * @brief Print an error message and exit the program
 * @param {const char*} message
 * @param {...} Variable number of arguments to format the message
 * @returns {void}
 */
void error(int code, const char* message, ...);

/**
 * @function warning
 * @brief Print a warning message
 * @param {const char*} message
 * @param {...} Variable number of arguments to format the message
 * @returns {void}
 */
void warning(const char* message, ...);
