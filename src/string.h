#ifndef _STRING_H_
#define _STRING_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "base.h"
#include "memory.h"

typedef struct {
	size_t capacity;
	size_t length;
	char* data;
    
    void (*print)(void*);
    void (*destroy)(void*);
} string_t;

/**
 * 
 * @function string_create
 * @brief Create a string
 * @params {size_t} initial_capacity - Initial size of the string
 * @returns {string_t*}
 * 
 */
string_t* string_create(size_t initial_capacity);

/**
 * 
 * @function string_append_char_begin
 * @brief Append a character to the beginning of the string
 * @params {string_t*} str - String
 * @params {char} c - Character
 * @returns {void}
 * 
 */
void string_append_char_begin(string_t* str, char c);

/**
 * 
 * @function string_append_str_begin
 * @brief Append a string to the beginning of the string
 * @params {string_t*} str - String
 * @params {const char*} prefix - Prefix
 * @returns {void}
 * 
 */
void string_append_str_begin(string_t* str, const char* prefix);

/**
 * 
 * @function string_append_char
 * @brief Append a character to the end of the string
 * @params {string_t*} str - String
 * @params {char} c - Character
 * @returns {void}
 * 
 */
void string_append_char(string_t* str, char c);

/**
 * 
 * @function string_append_str
 * @brief Append a string to the end of the string
 * @params {string_t*} str - String
 * @params {const char*} suffix - Suffix
 * @returns {void}
 * 
 */
void string_append_str(string_t* str, const char* suffix);

/**
 * 
 * @function string_destroy
 * @brief Free a string
 * @params {string_t*} str - String
 * @returns {void}
 * 
 */
void string_destroy(string_t* str);

/**
 * 
 * @function string_print
 * @brief Print a string
 * @params {string_t*} str - String
 * @returns {void}
 * 
 */
void string_print(string_t* str);

/**
 * 
 * @function string_set
 * @brief Set the value of a string
 * @params {string_t*} str - String
 * @params {string_t*} value - Value
 * @returns {void}
 * 
 */
void string_set(string_t* str, string_t* value);

/**
 * 
 * @function string_set_str
 * @brief Set the value of a string
 * @params {string_t*} str - String
 * @params {const char*} value - Value
 * @returns {void}
 * 
 */
void string_set_str(string_t* str, const char* value);

/**
 * 
 * @function string_append
 * @brief Append a string to the end of the string
 * @params {string_t*} str - String
 * @params {string_t*} value - Value
 * @returns {void}
 * 
 */
void string_append(string_t* str, string_t* value);

/**
 * 
 * @function string_lower_str
 * @brief Convert a string to lowercase
 * @params {const char*} str - String
 * @returns {char*} - Lowercase string
 * 
 */
char* string_lower_str(const char* str);

/**
 * 
 * @function string_upper_str
 * @brief Convert a string to uppercase
 * @params {const char*} str - String
 * @returns {char*} - Uppercase string
 * 
 */
char* string_upper_str(const char* str);

/**
 * 
 * @function replace_all_substrings
 * @brief Replace all substrings in a string
 * @params {const char*} str - String
 * @params {const char*} old_substr - Old substring
 * @params {const char*} new_substr - New substring
 * @returns {char*} - New string
 * 
 */
char* replace_all_substrings(const char* str, const char* old_substr, const char* new_substr);

#endif
