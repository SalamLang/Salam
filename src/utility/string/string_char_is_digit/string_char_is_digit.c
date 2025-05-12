#include <utility/string/string_char_is_digit/string_char_is_digit.h>

/**
 *
 * @function is_char_digit
 * @brief Check if a character is a digit
 * @params {char} c - Character
 * @returns {bool}
 *
 */
bool string_char_is_digit(char c)
{
    DEBUG_ME;
    return c >= '0' && c <= '9';
}
