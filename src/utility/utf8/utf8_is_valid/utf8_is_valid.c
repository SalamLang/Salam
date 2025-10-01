#include <utility/utf8/utf8_is_valid/utf8_is_valid.h>

/**
 *
 * @function utf8_is_valid
 * @brief Check if a string is a valid UTF-8 sequence
 * @params {const char*} str The string to check
 * @returns {bool} True if the string is a valid UTF-8 sequence, false otherwise
 *
 */
bool utf8_is_valid(const char* str) {
    DEBUG_ME;
    const unsigned char* bytes = (const unsigned char*)str;

    while (*bytes) {
        if (*bytes <= 0x7F) {
            // ASCII byte
            bytes += 1;
        } else if ((*bytes & 0xE0) == 0xC0) {
            // 2-byte sequence
            if ((bytes[1] & 0xC0) != 0x80) {
                return false;
            }

            bytes += 2;
        } else if ((*bytes & 0xF0) == 0xE0) {
            // 3-byte sequence
            if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80) {
                return false;
            }

            bytes += 3;
        } else if ((*bytes & 0xF8) == 0xF0) {
            // 4-byte sequence
            if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80 ||
                (bytes[3] & 0xC0) != 0x80) {
                return false;
            }

            bytes += 4;
        } else {
            return false;
        }
    }

    return true;
}
