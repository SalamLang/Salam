#include <utility/utf8/utf8_is_continuation_byte/utf8_is_continuation_byte.h>

/**
 *
 * @function utf8_is_continuation_byte
 * @brief Check if a byte is a UTF-8 continuation byte
 * @params {char} c The byte to check
 * @returns {bool} True if the byte is a continuation byte, false otherwise
 *
 */
bool utf8_is_continuation_byte(char c)
{
    DEBUG_ME;
    return ((unsigned char)c & 0xC0) == 0x80;
}