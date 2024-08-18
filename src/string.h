#ifndef _STRING_H_
#define _STRING_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wctype.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "base.h"
#include "memory.h"
#include "lexer.h"

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
 * @params {c*} str - String
 * @params {string_t*} value - Value
 * @returns {void}
 * 
 */
void string_append(string_t* str, const string_t* value);

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

/**
 * 
 * @function string_is
 * @brief Check if two strings are equal
 * @params {string_t*} str1 - String 1
 * @params {const char*} str2 - String 2
 * @returns {bool}
 * 
 */
bool string_is(string_t* str1, const char* str2);

/**
 * 
 * @function string_equals
 * @brief Check if two strings are equal
 * @params {string_t*} str1 - String 1
 * @params {string_t*} str2 - String 2
 * @returns {bool}
 * 
 */
bool string_equals(string_t* str1, string_t* str2);

/**
 * 
 * @function string_compare
 * @brief Compare two strings
 * @params {string_t*} str1 - String 1
 * @params {string_t*} str2 - String 2
 * @returns {bool}
 * 
 */
bool string_compare(string_t* str1, string_t* str2);

/**
 * 
 * @function is_utf8_continuation_byte
 * @brief Check if a byte is a UTF-8 continuation byte
 * @params {char} c The byte to check
 * @returns {bool} True if the byte is a continuation byte, false otherwise
 * 
 */
bool is_utf8_continuation_byte(char c);

/**
 * 
 * @function is_valid_utf8
 * @brief Check if a string is a valid UTF-8 sequence
 * @params {const char*} str The string to check
 * @returns {bool} True if the string is a valid UTF-8 sequence, false otherwise
 * 
 */
bool is_valid_utf8(const char* str);

/**
 * 
 * @function is_char_alpha
 * @brief Check if a character is an alphabet
 * @params {char} c - Character
 * @returns {bool} True if the character is alphabetic, false otherwise
 * 
 */
bool is_char_alpha(const char c);

/**
 * 
 * @function is_schar_alpha
 * @brief Check if a character is an alphabet
 * @params {char} c - Character
 * @returns {bool}
 * 
 */
bool is_schar_alpha(const char* c);

/**
 * 
 * @function is_char_alnum
 * @brief Check if a character is an alphabet or a digit
 * @params {char} c - Character
 * @returns {bool}
 * 
 */
bool is_char_alnum(char c);

/**
 * 
 * @function is_char_whitespace
 * @brief Check if a character is a whitespace
 * @params {char} c - Character
 * @returns {bool}
 * 
 */
bool is_char_whitespace(char c);

/**
 * 
 * @function int2string
 * @brief Convert an integer to a string
 * @params {int} value - Integer value
 * @returns {char*}
 * 
 */
char* int2string(int value);

/**
 * 
 * @function float2string
 * @brief Convert a float to a string
 * @params {float} value - Float value
 * @returns {char*}
 * 
 */
char* float2string(float value);

/**
 * 
 * @function double2string
 * @brief Convert a double to a string
 * @params {double} value - Double value
 * @returns {char*}
 * 
 */
char* double2string(double value);

/**
 * 
 * @function string_destroy_and_get
 * @brief Free a string and return the string data
 * @params {string_t*} str - String
 * @returns {char*}
 * 
 */
char* string_destroy_and_get(string_t* str);

/**
 * 
 * @function utf8_char_length
 * @brief Get the length of a UTF-8 character
 * @params {char} c - UTF-8 character
 * @returns {size_t} Length of the character
 * 
 */
size_t utf8_char_length(char c);

/**
 * 
 * @function utf8_decode
 * @brief Decode a UTF-8 character
 * @params {const char*} source - Source string
 * @params {size_t*} index - Index of the character
 * @returns {uint32_t} Decoded character
 * 
 */
uint32_t utf8_decode(const char* source, size_t* index);

/**
 * 
 * @function is_wchar_alpha
 * @brief Check if a wide character is alphabetic
 * @params {uint32_t} codepoint - Wide character
 * @returns {bool} True if the character is alphabetic, false otherwise
 * 
 */
bool is_wchar_alpha(uint32_t codepoint);

/**
 * 
 * @function is_wchar_digit
 * @brief Check if a wide character is a digit
 * @params {uint32_t} codepoint - Wide character
 * @returns {bool} True if the character is a digit, false otherwise
 * 
 */
bool is_wchar_digit(uint32_t codepoint);

/**
 * 
 * @function convert_to_english_digit
 * @brief Convert a wide character to an English digit
 * @params {wchar_t} ch - Wide character
 * @returns {char} English digit
 * 
 */
char convert_to_english_digit(wchar_t ch);

/**
 * 
 * @function string_number2number
 * @brief Convert digits in a string to English digits
 * @params {char*} str - Input string
 * @returns {void}
 * 
 */
void string_number2number(char* str);

#endif
