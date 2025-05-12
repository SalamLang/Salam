#include <utility/utf8/utf8_char_length/utf8_char_length.h>

size_t utf8_char_length(char c)
{
    DEBUG_ME;
    if ((c & 0x80) == 0) {
        return 1;  // 0xxxxxxx
    } else if ((c & 0xE0) == 0xC0) {
        return 2;  // 110xxxxx
    } else if ((c & 0xF0) == 0xE0) {
        return 3;  // 1110xxxx
    } else if ((c & 0xF8) == 0xF0) {
        return 4;  // 11110xxx
    }

    return 0;  // Invalid UTF-8
}
