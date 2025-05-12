#include <utility/utf8/utf8_char_decode/utf8_char_decode.h>

/**
 *
 * @function utf8_char_decode
 * @brief Get the UTF-8 character and return the length
 * @params {char*} source - Source code
 * @params {size_t*} index - Index of the current character in source string
 * @params {size_t*} num_bytes - Number of bytes
 * @returns {int}
 *
 */
char *utf8_char_decode(char *source, size_t *index, size_t *num_bytes)
{
    DEBUG_ME;
    char *utf8_char = memory_allocate(5);
    utf8_char[0] = source[*index];

    size_t bytes = 0;

    if ((source[*index] & 0x80) == 0) {
        bytes = 1;
    } else if ((source[*index] & 0xE0) == 0xC0) {
        bytes = 2;
    } else if ((source[*index] & 0xF0) == 0xE0) {
        bytes = 3;
    } else if ((source[*index] & 0xF8) == 0xF0) {
        bytes = 4;
    } else {
        bytes = 0;
        if (num_bytes != NULL) {
            *num_bytes = bytes;
        }

        log_fatal(3, "Invalid UTF-8 encoding detected at index %d", *index);
    }

    for (size_t i = 1; i < bytes; ++i) {
        utf8_char[i] = source[*index + i];
    }

    utf8_char[bytes] = '\0';

    *index += bytes;

    if (num_bytes != NULL) {
        *num_bytes = bytes;
    }

    return utf8_char;
}
