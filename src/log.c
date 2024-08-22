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
void panic(const char* message, ...)
{
    DEBUG_ME;
    va_list args;

    va_start(args, message);
    fprintf(stderr, "Panic error: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");

    exit(EXIT_FAILURE);
}

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
void error(int code, const char* message, ...)
{
    DEBUG_ME;
    va_list args;

    va_start(args, message);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");

    exit(code);
}

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
void error_generator(int code, const char* message, ...)
{
    DEBUG_ME;
    va_list args;

    va_start(args, message);
    fprintf(stderr, "Generator Error: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");

    exit(code);
}

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
void error_parser(int code, const char* message, ...)
{
	DEBUG_ME;
    va_list args;

    va_start(args, message);
    fprintf(stderr, "Parser Error: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");

    exit(code);
}

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
void error_lexer(int code, const char* message, ...)
{
	DEBUG_ME;
    va_list args;

    va_start(args, message);
    fprintf(stderr, "Lexer Error: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");

    exit(code);
}

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
void error_ast(int code, const char* message, ...)
{
	DEBUG_ME;
    va_list args;

    va_start(args, message);
    fprintf(stderr, "AST Error: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");

    exit(code);
}

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
void error_validator(int code, const char* message, ...)
{
	DEBUG_ME;
    va_list args;

    va_start(args, message);
    fprintf(stderr, "Validator Error: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");

    exit(code);
}

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
void error_interpreter(int code, const char* message, ...)
{
	DEBUG_ME;
    va_list args;

    va_start(args, message);
    fprintf(stderr, "Interpreter Error: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");

    exit(code);
}

/**
 * 
 * @function warning
 * @brief Print a warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 * 
 */
void warning(const char* message, ...)
{
    DEBUG_ME;
    va_list args;

    va_start(args, message);
    fprintf(stderr, "Warning: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
}

/**
 * 
 * @function warning_generator
 * @brief Print a generator warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 * 
 */
void warning_generator(const char* message, ...)
{
    DEBUG_ME;
    va_list args;

    va_start(args, message);
    fprintf(stderr, "Generator Warning: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
}

/**
 * 
 * @function warning_parser
 * @brief Print a parser warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 * 
 */
void warning_parser(const char* message, ...)
{
    DEBUG_ME;
    va_list args;

    va_start(args, message);
    fprintf(stderr, "Parser Warning: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
}

/**
 * 
 * @function warning_lexer
 * @brief Print a lexer warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 * 
 */
void warning_lexer(const char* message, ...)
{
    DEBUG_ME;
    va_list args;

    va_start(args, message);
    fprintf(stderr, "Lexer Warning: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
}

/**
 * 
 * @function warning_ast
 * @brief Print an AST warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 * 
 */
void warning_ast(const char* message, ...)
{
    DEBUG_ME;
    va_list args;

    va_start(args, message);
    fprintf(stderr, "AST Warning: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
}

/**
 * 
 * @function warning_validator
 * @brief Print a validator warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 * 
 */
void warning_validator(const char* message, ...)
{
    DEBUG_ME;
    va_list args;

    va_start(args, message);
    fprintf(stderr, "Validator Warning: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
}

/**
 * 
 * @function warning_interpreter
 * @brief Print an interpreter warning message
 * @params {const char*} message
 * @params {...} Variable number of arguments to format the message
 * @returns {void}
 * 
 */
void warning_interpreter(const char* message, ...)
{
    DEBUG_ME;
    va_list args;

    va_start(args, message);
    fprintf(stderr, "Interpreter Warning: ");
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
}
