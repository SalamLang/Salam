#include <utility/string/string_char_is_alpha/string_char_is_alpha.h>

/**
 *
 * @function string_char_is_alpha
 * @brief Check if a character is an alphabet
 * @params {char} c - Character
 * @returns {bool}
 *
 */
bool string_char_is_alpha(const char c)
{
    DEBUG_ME;
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
