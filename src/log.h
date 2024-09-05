#ifndef _LOG_H_
#define _LOG_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "base.h"
#include "log.h"

/**
 *
 * @function panic
 * @brief Print a panic error message and exit the program
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 *
 */
void panic(const char *message, ...);

/**
 *
 * @function error
 * @brief Print an error message and exit the program
 * @params {int} code
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 *
 */
void error(int code, const char *message, ...);

/**
 *
 * @function error_generator
 * @brief Print a generator error message and exit the program
 * @params {int} code
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 *
 */
void error_generator(int code, const char *message, ...);

/**
 *
 * @function error_parser
 * @brief Print a parser error message and exit the program
 * @params {int} code
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 *
 */
void error_parser(int code, const char *message, ...);

/**
 *
 * @function error_lexer
 * @brief Print a lexer error message and exit the program
 * @params {int} code
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 *
 */
void error_lexer(int code, const char *message, ...);

/**
 *
 * @function error_ast
 * @brief Print an AST error message
 * @params {int} code
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 *
 */
void error_ast(int code, const char *message, ...);

/**
 *
 * @function error_validator
 * @brief Print a validator error message
 * @params {int} code
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 *
 */
void error_validator(int code, const char *message, ...);

/**
 *
 * @function error_interpreter
 * @brief Print an interpreter error message
 * @params {int} code
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 *
 */
void error_interpreter(int code, const char *message, ...);

/**
 *
 * @function warning
 * @brief Print a warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 *
 */
void warning(const char *message, ...);

/**
 *
 * @function warning_generator
 * @brief Print a generator warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 *
 */
void warning_generator(const char *message, ...);

/**
 *
 * @function warning_parser
 * @brief Print a parser warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 *
 */
void warning_parser(const char *message, ...);

/**
 *
 * @function warning_lexer
 * @brief Print a lexer warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 *
 */
void warning_lexer(const char *message, ...);

/**
 *
 * @function warning_ast
 * @brief Print an AST warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 *
 */
void warning_ast(const char *message, ...);

/**
 *
 * @function warning_validator
 * @brief Print a validator warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 *
 */
void warning_validator(const char *message, ...);

/**
 *
 * @function warning_interpreter
 * @brief Print an interpreter warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 *
 */
void warning_interpreter(const char *message, ...);

#endif
