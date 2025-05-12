#include <utility/utf8/utf8_is_alpha/utf8_is_alpha.h>

/**
 *
 * @function utf8_is_alpha
 * @brief Check if a UTF-8 string is an alphabet
 * @params {char*} utf8 - UTF-8 string
 * @returns {bool} True if the string is an alphabet, false otherwise
 *
 */
bool utf8_is_alpha(char *utf8)
{
    DEBUG_ME;
    // wchar_t wc;
    // int wcl = mbtowc(&wc, utf8, MB_CUR_MAX);
    // if (wcl <= 0) {
    //     return false;
    // }
    //
    // return iswalpha(wc);
    if (utf8 == NULL) {
        return false;
    }

    uint32_t codepoint;
    const unsigned char *s = (const unsigned char *)utf8;

    // Decode UTF-8 byte sequence to codepoint
    if (*s <= 0x7F) {
        codepoint = *s;  // 1-byte sequence (ASCII)
    } else if ((*s >> 5) == 0x6) {
        if ((s[1] & 0xC0) != 0x80) return false;  // Invalid continuation byte
        codepoint = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
    } else if ((*s >> 4) == 0xE) {
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80)
            return false;  // Invalid continuation bytes
        codepoint =
            ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
    } else if ((*s >> 3) == 0x1E) {
        if ((s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 ||
            (s[3] & 0xC0) != 0x80)
            return false;  // Invalid continuation bytes
        codepoint = ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12) |
                    ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
    } else {
        return false;  // Invalid UTF-8 start byte
    }

    // Convert codepoint to wide character
    // wchar_t wc = (wchar_t)codepoint;
    // return iswalpha(wc);

    // Check if the codepoint is alphabetic (manual check for Unicode ranges)
    if ((codepoint >= 0x41 && codepoint <= 0x5A) ||      // A-Z
        (codepoint >= 0x61 && codepoint <= 0x7A) ||      // a-z
        (codepoint >= 0x0600 && codepoint <= 0x06FF) ||  // Arabic alphabet
        iswalpha(codepoint)) {  // Fallback to iswalpha for other languages
        return true;
    }

    return false;
}
