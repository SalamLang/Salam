#include "log.h"

/**
 * @function panic
 * @brief Print a panic error message and exit the program
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 */
void panic(const char* message, ...)
{
    va_list args;
    va_start(args, message);
    fprintf(stderr, "Panic error: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");

    exit(EXIT_FAILURE);
}

/**
 * @function error
 * @brief Print an error message and exit the program
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 */
void error(int code, const char* message, ...)
{
    va_list args;
    va_start(args, message);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(code);
}

/**
 * @function warning
 * @brief Print a warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 */
void warning(const char* message, ...)
{
    va_list args;
    va_start(args, message);
    fprintf(stderr, "Warning: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
}
