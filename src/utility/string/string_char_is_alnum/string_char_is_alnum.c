#include <utility/string/string_char_is_alnum/string_char_is_alnum.h>

/**
 *
 * @function string_char_is_alnum
 * @brief Check if a character is an alphabet or a digit
 * @params {char} c - Character
 * @returns {bool}
 *
 */
bool string_char_is_alnum(char c) {
    DEBUG_ME;
    return string_char_is_alpha(c) || string_char_is_digit(c);
}
