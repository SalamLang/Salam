#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/**
 * @function panic
 * @brief Print a panic error message and exit the program
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 */
void panic(const char* message, ...);

/**
 * @function error
 * @brief Print an error message and exit the program
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 */
void error(int code, const char* message, ...);

/**
 * @function warning
 * @brief Print a warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 */
void warning(const char* message, ...);

#endif
